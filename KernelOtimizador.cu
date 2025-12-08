#include "KernelOtimizador.cuh"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <iostream>

#define MAX_SLOTS 200 // Limite seguro para alocação estática na stack da thread

// --- LÓGICA DE NEGÓCIO (DEVICE) ---
__device__ long long int calcularPontuacaoGPU(int* slots, const VooGPU* catalogo, int numSlots) {
    long long int score = 0;
    int localizacaoAtual = 0; // Base GRU
    
    for (int i = 0; i < numSlots; i++) {
        int idx = slots[i];
        if (idx == -1) continue; // Folga

        VooGPU v = catalogo[idx];
        if (v.origem == localizacaoAtual) {
            score += (v.duracao * 10);
            localizacaoAtual = v.destino;
        } else {
            score -= 50000; // Penalidade Quebra
            score -= 2000;  // Deadhead
            localizacaoAtual = v.destino;
        }
    }
    if (localizacaoAtual != 0) score -= 5000;
    return score;
}

// --- KERNEL ---
__global__ void hillClimbingKernel(
    const VooGPU* catalogo, int tamCatalogo,
    long long int* outScores,   // Saída: Score de cada thread
    int* outEscalas,            // Saída: Vetor de escalas de TODAS as threads (GIGANTE)
    int numSlots, int maxIter, unsigned long seed
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Inicializa RNG
    curandState state;
    curand_init(seed, idx, 0, &state);

    // Memória local rápida
    int solucaoAtual[MAX_SLOTS];
    int solucaoVizinha[MAX_SLOTS];

    // 1. Solução Inicial Aleatória
    for(int i=0; i<numSlots; i++) {
        if(curand_uniform(&state) < 0.3f) solucaoAtual[i] = -1;
        else solucaoAtual[i] = curand(&state) % tamCatalogo;
        solucaoVizinha[i] = solucaoAtual[i];
    }

    long long int scoreAtual = calcularPontuacaoGPU(solucaoAtual, catalogo, numSlots);

    // 2. Loop de Otimização
    for(int k=0; k<maxIter; k++) {
        // Mutação
        int tipo = curand(&state) % 3;
        int slot = curand(&state) % numSlots;
        int backup = solucaoVizinha[slot];

        if(tipo == 0) solucaoVizinha[slot] = curand(&state) % tamCatalogo;
        else if(tipo == 1) solucaoVizinha[slot] = -1;
        else {
            int slot2 = curand(&state) % numSlots;
            int temp = solucaoVizinha[slot];
            solucaoVizinha[slot] = solucaoVizinha[slot2];
            solucaoVizinha[slot2] = temp;
            // Nota: Swap simplificado (não reverte perfeitamente aqui por brevidade, mas funciona estocasticamente)
        }

        long long int scoreVizinho = calcularPontuacaoGPU(solucaoVizinha, catalogo, numSlots);

        if(scoreVizinho > scoreAtual) {
            scoreAtual = scoreVizinho;
            for(int i=0; i<numSlots; i++) solucaoAtual[i] = solucaoVizinha[i];
        } else {
            for(int i=0; i<numSlots; i++) solucaoVizinha[i] = solucaoAtual[i];
        }
    }

    // 3. Salvar resultados na memória global
    outScores[idx] = scoreAtual;
    
    // Cada thread escreve sua escala na posição correta do "vetorzão" global
    // Offset = idx * numSlots
    for(int i=0; i<numSlots; i++) {
        outEscalas[(idx * numSlots) + i] = solucaoAtual[i];
    }
}

// --- WRAPPER (HOST) ---
long long int rodarOtimizacaoCUDA(
    const std::vector<Voo>& catalogoHost,
    int numTentativas, int numSlots, int maxIter,
    std::vector<int>& melhorEscalaIndices
) {
    // 1. Prepara Catálogo
    std::vector<VooGPU> flatCat;
    for(const auto& v : catalogoHost) 
        flatCat.push_back({v.getId(), v.getOrigem(), v.getDestino(), v.getDuracao()});

    // 2. Alocação
    VooGPU* d_cat;
    long long int* d_scores;
    int* d_escalas;

    size_t szCat = flatCat.size() * sizeof(VooGPU);
    size_t szScores = numTentativas * sizeof(long long int);
    size_t szEscalas = numTentativas * numSlots * sizeof(int);

    cudaMalloc(&d_cat, szCat);
    cudaMalloc(&d_scores, szScores);
    cudaMalloc(&d_escalas, szEscalas);

    cudaMemcpy(d_cat, flatCat.data(), szCat, cudaMemcpyHostToDevice);

    // 3. Execução
    int threads = 256;
    int blocks = (numTentativas + threads - 1) / threads;
    
    // Recalcula numTentativas real para bater com grid (evitar acesso fora de memória)
    int totalThreads = blocks * threads;

    std::cout << ">> [GPU] Lancando " << blocks << " blocos (" << totalThreads << " threads pararelas)..." << std::endl;

    hillClimbingKernel<<<blocks, threads>>>(d_cat, flatCat.size(), d_scores, d_escalas, numSlots, maxIter, time(NULL));
    cudaDeviceSynchronize();

    // 4. Recuperar Resultados
    std::vector<long long int> h_scores(totalThreads);
    std::vector<int> h_escalas(totalThreads * numSlots);

    cudaMemcpy(h_scores.data(), d_scores, totalThreads * sizeof(long long int), cudaMemcpyDeviceToHost);
    cudaMemcpy(h_escalas.data(), d_escalas, totalThreads * numSlots * sizeof(int), cudaMemcpyDeviceToHost);

    // 5. Redução na CPU (Achar o vencedor)
    long long int melhorScore = -999999999999;
    int indiceVencedor = -1;

    // Varremos apenas até numTentativas solicitado
    for(int i=0; i<numTentativas; i++) {
        if(h_scores[i] > melhorScore) {
            melhorScore = h_scores[i];
            indiceVencedor = i;
        }
    }

    // 6. Extrair a escala vencedora
    melhorEscalaIndices.clear();
    if(indiceVencedor != -1) {
        int start = indiceVencedor * numSlots;
        for(int i=0; i<numSlots; i++) {
            melhorEscalaIndices.push_back(h_escalas[start + i]);
        }
    }

    cudaFree(d_cat);
    cudaFree(d_scores);
    cudaFree(d_escalas);

    return melhorScore;
}
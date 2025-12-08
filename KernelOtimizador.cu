#include "KernelOtimizador.cuh"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <iostream>
#include <ctime>
#include <cstdio>

// --- DEFINIÇÕES INTERNAS ---

#define MAX_SLOTS_GPU 400 

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true) {
   if (code != cudaSuccess) {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

// --- DEVICE FUNCTIONS ---

__device__ long long int calcularPontuacaoGPU(int* slots, const VooGPU* catalogo, int numSlots) {
    long long int score = 0;
    int localizacaoAtual = 0; // Base GRU
    
    for (int i = 0; i < numSlots; i++) {
        int idx = slots[i];
        
        if (idx == -1) {
            score += 200; 
            continue; 
        }

        VooGPU v = catalogo[idx];
        
        if (v.origem == localizacaoAtual) {
            score += (v.duracao * 20); 
            localizacaoAtual = v.destino;
        } else {
            score -= 50000; 
            score -= 2000;  
            localizacaoAtual = v.destino;
        }
    }
    
    if (localizacaoAtual != 0) score -= 10000;
    return score;
}

// --- KERNEL GLOBAL ---

__global__ void hillClimbingKernel(
    const VooGPU* catalogo, int tamCatalogo,
    long long int* outScores,   
    int* outEscalas,            
    int numSlots, int maxIter, unsigned long seed
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    curandState state;
    curand_init(seed, idx, 0, &state);

    int solucaoAtual[MAX_SLOTS_GPU];
    int solucaoVizinha[MAX_SLOTS_GPU];

    if (numSlots > MAX_SLOTS_GPU) return; 

    // 1. Solução Inicial
    for(int i=0; i<numSlots; i++) {
        if(curand_uniform(&state) < 0.2f) {
            solucaoAtual[i] = -1;
        } else {
            solucaoAtual[i] = curand(&state) % tamCatalogo;
        }
        solucaoVizinha[i] = solucaoAtual[i];
    }

    long long int scoreAtual = calcularPontuacaoGPU(solucaoAtual, catalogo, numSlots);

    // 2. Loop de Otimização
    for(int k=0; k<maxIter; k++) {
        int tipo = curand(&state) % 100;
        int slot = curand(&state) % numSlots;
        
        if(tipo < 50) { 
            solucaoVizinha[slot] = curand(&state) % tamCatalogo;
        } else if(tipo < 70) { 
            solucaoVizinha[slot] = -1;
        } else { 
            int slot2 = curand(&state) % numSlots;
            int temp = solucaoVizinha[slot];
            solucaoVizinha[slot] = solucaoVizinha[slot2];
            solucaoVizinha[slot2] = temp;
        }

        long long int scoreVizinho = calcularPontuacaoGPU(solucaoVizinha, catalogo, numSlots);

        if(scoreVizinho >= scoreAtual) {
            scoreAtual = scoreVizinho;
            for(int i=0; i<numSlots; i++) solucaoAtual[i] = solucaoVizinha[i];
        } else {
            for(int i=0; i<numSlots; i++) solucaoVizinha[i] = solucaoAtual[i];
        }
    }

    outScores[idx] = scoreAtual;
    int offset = idx * numSlots;
    for(int i=0; i<numSlots; i++) {
        outEscalas[offset + i] = solucaoAtual[i];
    }
}

// --- IMPLEMENTAÇÃO DA FUNÇÃO WRAPPER (HOST) ---

long long int rodarOtimizacaoCUDA(
    const std::vector<Voo>& catalogoHost,
    int numTentativas, int numSlots, int maxIter,
    std::vector<int>& melhorEscalaIndices
) {
    if (numSlots > MAX_SLOTS_GPU) {
        std::cerr << "ERRO CUDA: NumSlots (" << numSlots << ") > MAX_SLOTS_GPU (" << MAX_SLOTS_GPU << ")" << std::endl;
        return -1;
    }

    // 1. Preparar Dados do Catálogo
    std::vector<VooGPU> flatCat;
    flatCat.reserve(catalogoHost.size());
    for(const auto& v : catalogoHost) {
        flatCat.push_back({ (int)v.getId(), v.getOrigem(), v.getDestino(), v.getDuracao() });
    }

    // --- CORREÇÃO AQUI: Calcular dimensões ANTES de alocar ---
    int threads = 256;
    int blocks = (numTentativas + threads - 1) / threads;
    int totalThreads = blocks * threads; // Sempre múltiplo de 256
    
    // std::cout << ">> [GPU] Grid: " << blocks << " blocos x " << threads << " threads = " << totalThreads << " total." << std::endl;

    // 2. Alocar GPU (Usando totalThreads para evitar overflow)
    VooGPU* d_cat;
    long long int* d_scores;
    int* d_escalas;

    size_t szCat = flatCat.size() * sizeof(VooGPU);
    // IMPORTANTE: Alocar para totalThreads, não apenas numTentativas
    size_t szScores = totalThreads * sizeof(long long int);
    size_t szEscalas = totalThreads * numSlots * sizeof(int);

    gpuErrchk(cudaMalloc(&d_cat, szCat));
    gpuErrchk(cudaMalloc(&d_scores, szScores));
    gpuErrchk(cudaMalloc(&d_escalas, szEscalas));

    gpuErrchk(cudaMemcpy(d_cat, flatCat.data(), szCat, cudaMemcpyHostToDevice));
    gpuErrchk(cudaMemset(d_scores, 0, szScores)); 

    // 3. Rodar Kernel
    unsigned long seed = (unsigned long)time(NULL) + (unsigned long)clock();

    hillClimbingKernel<<<blocks, threads>>>(
        d_cat, (int)flatCat.size(), 
        d_scores, d_escalas, 
        numSlots, maxIter, seed
    );
    
    gpuErrchk(cudaPeekAtLastError());
    gpuErrchk(cudaDeviceSynchronize());

    // 4. Copiar de volta (Agora os tamanhos batem!)
    std::vector<long long int> h_scores(totalThreads);
    std::vector<int> h_escalas(totalThreads * numSlots);

    gpuErrchk(cudaMemcpy(h_scores.data(), d_scores, szScores, cudaMemcpyDeviceToHost));
    gpuErrchk(cudaMemcpy(h_escalas.data(), d_escalas, szEscalas, cudaMemcpyDeviceToHost));

    // 5. Redução (CPU)
    long long int melhorScore = -9223372036854775800LL;
    int indiceVencedor = -1;

    // Iteramos apenas pelas tentativas úteis, ou totalThreads (tanto faz, as extras são lixo mas válidas em memória)
    for(int i=0; i<numTentativas; i++) {
        if(h_scores[i] > melhorScore) {
            melhorScore = h_scores[i];
            indiceVencedor = i;
        }
    }

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
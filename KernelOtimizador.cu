#include "KernelOtimizador.cuh"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <iostream>
#include <ctime>
#include <cstdio>

// --- DEFINIÇÕES E CONSTANTES ---

#define MAX_SLOTS_GPU  250
#define MAX_CATALOGO_GPU 100

#define BONUS_HORA_VOO          20
#define BONUS_DESCANSO_CURTO    500
#define PENALIDADE_QUEBRA       100000
#define PENALIDADE_FADIGA       15000
#define PENALIDADE_OCIOSIDADE   5000
#define PENALIDADE_MANHA_OCIOSA 5000
#define PENALIDADE_REPETICAO    8000
#define CUSTO_DEADHEAD_FIXO     2000
#define LIMITE_REPETICAO_VOO    3

// --- DEVICE FUNCTION: Lógica Idêntica à CPU ---
__device__ long long int calcularPontuacaoGPU(
    int* slots, 
    const VooGPU* catalogo, 
    int numSlots,
    int voosPorDia
) {
    long long int score = 0;
    int localizacaoAtual = 0; // Base GRU
    
    int voosSeguidos = 0;
    int folgasSeguidas = 0;

    // Array local para contar repetições (Map simplificado)
    unsigned char freqVoos[MAX_CATALOGO_GPU];
    // Zera o contador (unroll loop para velocidade)
    for(int i=0; i<MAX_CATALOGO_GPU; i++) freqVoos[i] = 0;

    for (int i = 0; i < numSlots; i++) {
        int idx = slots[i];
        bool inicioDoDia = (i % voosPorDia == 0);
        
        // --- CASO 1: FOLGA ---
        if (idx == -1) {
            voosSeguidos = 0;
            folgasSeguidas++;
            
            // 1. Regra da Manhã (Preguiça)
            if (inicioDoDia) {
                score -= PENALIDADE_MANHA_OCIOSA;
            }

            // 2. Regra de Ociosidade contínua
            // CPU: if (folgasSeguidas <= 2) score += 500; else score -= 5000;
            if (folgasSeguidas <= 2) score += BONUS_DESCANSO_CURTO;
            else score -= PENALIDADE_OCIOSIDADE;

            continue; 
        }

        // --- CASO 2: VOO REAL ---
        folgasSeguidas = 0;
        voosSeguidos++;
        
        // Proteção de acesso
        if (idx >= 0 && idx < MAX_CATALOGO_GPU) {
            VooGPU v = catalogo[idx];
            
            // 1. Contagem de Repetição (Variedade)
            freqVoos[idx]++;
            if (freqVoos[idx] > LIMITE_REPETICAO_VOO) {
                score -= PENALIDADE_REPETICAO;
            }

            // 2. Continuidade Geográfica
            if (v.origem == localizacaoAtual) {
                // Conexão Perfeita
                score += (long long)(v.duracao * BONUS_HORA_VOO);
                localizacaoAtual = v.destino;
            } else {
                // Quebra de Rota
                score -= PENALIDADE_QUEBRA;
                score -= CUSTO_DEADHEAD_FIXO;
                // CPU: score -= (180 * BONUS_HORA_VOO);
                score -= (180 * BONUS_HORA_VOO); 
                localizacaoAtual = v.destino;
            }
        }

        // 3. Fadiga
        if (voosSeguidos >= 3) {
            score -= PENALIDADE_FADIGA;
        }
    }
    
    // Regra Final: Base
    if (localizacaoAtual != 0) score -= 10000;

    return score;
}

// --- KERNEL GLOBAL ---
__global__ void hillClimbingKernel(
    const VooGPU* catalogo, int tamCatalogo,
    long long int* outScores,   
    int* outEscalas,            
    int numSlots, int maxIter, unsigned long seed,
    int voosPorDia
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    curandState state;
    curand_init(seed, idx, 0, &state);

    int solucaoAtual[MAX_SLOTS_GPU];
    int solucaoVizinha[MAX_SLOTS_GPU];

    if (numSlots > MAX_SLOTS_GPU) return; 

    // 1. Solução Inicial (20% Folga / 80% Voo - Igual CPU)
    for(int i=0; i<numSlots; i++) {
        if(curand_uniform(&state) < 0.2f) solucaoAtual[i] = -1;
        else solucaoAtual[i] = curand(&state) % tamCatalogo;
        solucaoVizinha[i] = solucaoAtual[i];
    }

    long long int scoreAtual = calcularPontuacaoGPU(solucaoAtual, catalogo, numSlots, voosPorDia);

    // 2. Loop de Otimização
    for(int k=0; k<maxIter; k++) {
        int tipo = curand(&state) % 100;
        int slot = curand(&state) % numSlots;
        
        // Mutação (Probabilidades iguais ao Escala.cpp)
        // 0-49: Troca Voo (50%)
        // 50-69: Cria Folga (20%)
        // 70-84: Remove Folga (15%)
        // 85-99: Swap (15%)
        
        if(tipo < 50) { 
            solucaoVizinha[slot] = curand(&state) % tamCatalogo;
        } else if(tipo < 70) { 
            solucaoVizinha[slot] = -1;
        } else if(tipo < 85) { 
             // Tenta remover folga se o slot for folga
             if (solucaoVizinha[slot] == -1) 
                 solucaoVizinha[slot] = curand(&state) % tamCatalogo;
        } else { 
            int slot2 = curand(&state) % numSlots;
            int temp = solucaoVizinha[slot];
            solucaoVizinha[slot] = solucaoVizinha[slot2];
            solucaoVizinha[slot2] = temp;
        }

        long long int scoreVizinho = calcularPontuacaoGPU(solucaoVizinha, catalogo, numSlots, voosPorDia);

        if(scoreVizinho >= scoreAtual) {
            scoreAtual = scoreVizinho;
            for(int i=0; i<numSlots; i++) solucaoAtual[i] = solucaoVizinha[i];
        } else {
            // Revert
            for(int i=0; i<numSlots; i++) solucaoVizinha[i] = solucaoAtual[i];
        }
    }

    // 3. Salva Resultado
    outScores[idx] = scoreAtual;
    int offset = idx * numSlots;
    for(int i=0; i<numSlots; i++) {
        outEscalas[offset + i] = solucaoAtual[i];
    }
}


long long int rodarOtimizacaoCUDA(
    const std::vector<Voo>& catalogoHost,
    int numTentativas, 
    int numSlots, 
    int maxIter,
    std::vector<int>& melhorEscalaIndices,
    int voosPorDia
) {
    if (numSlots > MAX_SLOTS_GPU) {
        std::cerr << "ERRO CUDA: NumSlots > MAX_SLOTS_GPU" << std::endl;
        return -1;
    }
    
    // Check extra para não estourar array de repetições
    if (catalogoHost.size() > MAX_CATALOGO_GPU) {
        std::cerr << "ERRO CUDA: Catalogo muito grande (" << catalogoHost.size() 
                  << "). Aumente MAX_CATALOGO_GPU no .cu" << std::endl;
        return -1;
    }

    // Prepara dados
    std::vector<VooGPU> flatCat;
    flatCat.reserve(catalogoHost.size());
    for(const auto& v : catalogoHost) {
        flatCat.push_back({ (int)v.getId(), v.getOrigem(), v.getDestino(), v.getDuracao() });
    }

    int threads = 256;
    int blocks = (numTentativas + threads - 1) / threads;
    int totalThreads = blocks * threads;

    VooGPU* d_cat;
    long long int* d_scores;
    int* d_escalas;

    size_t szCat = flatCat.size() * sizeof(VooGPU);
    size_t szScores = totalThreads * sizeof(long long int);
    size_t szEscalas = totalThreads * numSlots * sizeof(int);

    cudaMalloc(&d_cat, szCat);
    cudaMalloc(&d_scores, szScores);
    cudaMalloc(&d_escalas, szEscalas);

    cudaMemcpy(d_cat, flatCat.data(), szCat, cudaMemcpyHostToDevice);
    cudaMemset(d_scores, 0, szScores); 

    unsigned long seed = (unsigned long)time(NULL) + (unsigned long)clock();

    // Lança Kernel
    hillClimbingKernel<<<blocks, threads>>>(
        d_cat, (int)flatCat.size(), 
        d_scores, d_escalas, 
        numSlots, maxIter, seed,
        voosPorDia
    );

    // Recupera
    std::vector<long long int> h_scores(totalThreads);
    std::vector<int> h_escalas(totalThreads * numSlots);

    cudaMemcpy(h_scores.data(), d_scores, szScores, cudaMemcpyDeviceToHost);
    cudaMemcpy(h_escalas.data(), d_escalas, szEscalas, cudaMemcpyDeviceToHost);

    // Redução CPU
    long long int melhorScore = -9223372036854775800LL;
    int indiceVencedor = -1;

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
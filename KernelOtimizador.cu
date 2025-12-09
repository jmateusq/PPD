#include "KernelOtimizador.cuh"
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <iostream>
#include <ctime>
#include <cstdio>
#include <vector>

// --- DEFINIÇÕES E CONSTANTES ---

#define MAX_SLOTS_GPU  250
#define MAX_CATALOGO_GPU 100
#define LOCK_UNLOCKED 0
#define LOCK_LOCKED 1

#define BONUS_HORA_VOO          20
#define BONUS_DESCANSO_CURTO    500
#define PENALIDADE_QUEBRA       100000
#define PENALIDADE_FADIGA       15000
#define PENALIDADE_OCIOSIDADE   5000
#define PENALIDADE_MANHA_OCIOSA 5000
#define PENALIDADE_REPETICAO    8000
#define CUSTO_DEADHEAD_FIXO     2000
#define LIMITE_REPETICAO_VOO    3

// --- DEVICE FUNCTION (Mantida igual) ---
__device__ long long int calcularPontuacaoGPU(
    int* slots,
    const VooGPU* catalogo,
    int numSlots,
    int voosPorDia
) {
    long long int score = 0;
    int localizacaoAtual = 0;
    int voosSeguidos = 0;
    int folgasSeguidas = 0;

    unsigned char freqVoos[MAX_CATALOGO_GPU];
    for(int i=0; i<MAX_CATALOGO_GPU; i++) freqVoos[i] = 0;

    for (int i = 0; i < numSlots; i++) {
        int idx = slots[i];
        bool inicioDoDia = (i % voosPorDia == 0);

        if (idx == -1) {
            voosSeguidos = 0;
            folgasSeguidas++;
            if (inicioDoDia) score -= PENALIDADE_MANHA_OCIOSA;
            if (folgasSeguidas <= 2) score += BONUS_DESCANSO_CURTO;
            else score -= PENALIDADE_OCIOSIDADE;
            continue;
        }

        folgasSeguidas = 0;
        voosSeguidos++;

        if (idx >= 0 && idx < MAX_CATALOGO_GPU) {
            VooGPU v = catalogo[idx];
            freqVoos[idx]++;
            if (freqVoos[idx] > LIMITE_REPETICAO_VOO) score -= PENALIDADE_REPETICAO;

            if (v.origem == localizacaoAtual) {
                score += (long long)(v.duracao * BONUS_HORA_VOO);
                localizacaoAtual = v.destino;
            } else {
                score -= PENALIDADE_QUEBRA;
                score -= CUSTO_DEADHEAD_FIXO;
                score -= (180 * BONUS_HORA_VOO);
                localizacaoAtual = v.destino;
            }
        }
        if (voosSeguidos >= 3) score -= PENALIDADE_FADIGA;
    }
    if (localizacaoAtual != 0) score -= 10000;
    return score;
}

// --- KERNEL GLOBAL (Limpo) ---
__global__ void hillClimbingKernel(
    const VooGPU* catalogo, int tamCatalogo,
    int* outEscalas, // Removemos outScores
    int numSlots, int maxIter, unsigned long seed,
    int voosPorDia,
    long long int* d_globalBestScore,
    int* d_globalBestIndex,
    int* d_mutex
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;

    curandState state;
    curand_init(seed, idx, 0, &state);

    int solucaoAtual[MAX_SLOTS_GPU];
    int solucaoVizinha[MAX_SLOTS_GPU];

    if (numSlots > MAX_SLOTS_GPU) return;

    // 1. Solução Inicial
    for(int i=0; i<numSlots; i++) {
        if(curand_uniform(&state) < 0.2f) solucaoAtual[i] = -1;
        else solucaoAtual[i] = curand(&state) % tamCatalogo;
        solucaoVizinha[i] = solucaoAtual[i];
    }

    long long int scoreAtual = calcularPontuacaoGPU(solucaoAtual, catalogo, numSlots, voosPorDia);

    // 2. Loop de Otimização (Hill Climbing)
    for(int k=0; k<maxIter; k++) {
        int tipo = curand(&state) % 100;
        int slot = curand(&state) % numSlots;

        if(tipo < 50) {
            solucaoVizinha[slot] = curand(&state) % tamCatalogo;
        } else if(tipo < 70) {
            solucaoVizinha[slot] = -1;
        } else if(tipo < 85) {
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
            for(int i=0; i<numSlots; i++) solucaoVizinha[i] = solucaoAtual[i];
        }
    }

    int offset = idx * numSlots;
    for(int i=0; i<numSlots; i++) {
        outEscalas[offset + i] = solucaoAtual[i];
    }

    __shared__ long long int s_scores[256];
    __shared__ int s_indices[256];

    s_scores[tid] = scoreAtual;
    s_indices[tid] = idx;
    __syncthreads();

    for (unsigned int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            if (s_scores[tid + s] > s_scores[tid]) {
                s_scores[tid] = s_scores[tid + s];
                s_indices[tid] = s_indices[tid + s];
            }
        }
        __syncthreads();
    }

    if (tid == 0) {
        long long int blocoBestScore = s_scores[0];
        int blocoBestIndex = s_indices[0];

        if (blocoBestScore > *d_globalBestScore) {
            bool leave = false;
            while (!leave) {
                if (atomicCAS(d_mutex, LOCK_UNLOCKED, LOCK_LOCKED) == LOCK_UNLOCKED) {
                    if (blocoBestScore > *d_globalBestScore) {
                        *d_globalBestScore = blocoBestScore;
                        *d_globalBestIndex = blocoBestIndex;
                    }
                    atomicExch(d_mutex, LOCK_UNLOCKED);
                    leave = true;
                }
            }
        }
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
    if (numSlots > MAX_SLOTS_GPU || catalogoHost.size() > MAX_CATALOGO_GPU) {
        return -1;
    }

    std::vector<VooGPU> flatCat;
    flatCat.reserve(catalogoHost.size());
    for(const auto& v : catalogoHost) {
        flatCat.push_back({ (int)v.getId(), v.getOrigem(), v.getDestino(), v.getDuracao() });
    }

    int threads = 256;
    int blocks = (numTentativas + threads - 1) / threads;
    int totalThreads = blocks * threads;

    VooGPU* d_cat;
    int* d_escalas;
    long long int* d_globalBestScore;
    int* d_globalBestIndex;
    int* d_mutex;

    size_t szCat = flatCat.size() * sizeof(VooGPU);
    size_t szEscalas = totalThreads * numSlots * sizeof(int);

    cudaMalloc(&d_cat, szCat);
    cudaMalloc(&d_escalas, szEscalas);
    cudaMalloc(&d_globalBestScore, sizeof(long long int));
    cudaMalloc(&d_globalBestIndex, sizeof(int));
    cudaMalloc(&d_mutex, sizeof(int));

    long long int worstScore = -9223372036854775800LL;
    int initialIdx = -1;
    int initialMutex = LOCK_UNLOCKED;

    cudaMemcpy(d_cat, flatCat.data(), szCat, cudaMemcpyHostToDevice);
    cudaMemcpy(d_globalBestScore, &worstScore, sizeof(long long int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_globalBestIndex, &initialIdx, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_mutex, &initialMutex, sizeof(int), cudaMemcpyHostToDevice);

    unsigned long seed = (unsigned long)time(NULL) + (unsigned long)clock();

    hillClimbingKernel<<<blocks, threads>>>(
        d_cat, (int)flatCat.size(),
        d_escalas,
        numSlots, maxIter, seed,
        voosPorDia,
        d_globalBestScore, d_globalBestIndex, d_mutex
    );

    cudaDeviceSynchronize();

    long long int h_bestScore;
    int h_bestIndex;

    cudaMemcpy(&h_bestScore, d_globalBestScore, sizeof(long long int), cudaMemcpyDeviceToHost);
    cudaMemcpy(&h_bestIndex, d_globalBestIndex, sizeof(int), cudaMemcpyDeviceToHost);

    melhorEscalaIndices.resize(numSlots);
    int* d_bestEscalaPtr = d_escalas + (h_bestIndex * numSlots);
    cudaMemcpy(melhorEscalaIndices.data(), d_bestEscalaPtr, numSlots * sizeof(int), cudaMemcpyDeviceToHost);

    cudaFree(d_cat);
    cudaFree(d_escalas);
    cudaFree(d_globalBestScore);
    cudaFree(d_globalBestIndex);
    cudaFree(d_mutex);

    return h_bestScore;
}

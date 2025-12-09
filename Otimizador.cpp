#include "Otimizador.h"
#include <vector>
#include <iostream>
#include <algorithm> 
#include <ctime>     
#include <random>
#include <omp.h> // Necessário para OpenMP

// Importante: Só inclui o header CUDA se for compilar com suporte a GPU
#include "KernelOtimizador.cuh" 

using namespace std;

Otimizador::Otimizador(std::vector<Voo> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {
}

Escala Otimizador::rodarHillClimbingLocal(std::mt19937& rng) {
    Escala atual(catalogo, config, rng); 
    unsigned long int maxIter = config.getMaxIteracao();

    for (unsigned long int i = 0; i < maxIter; i++) {
        Escala vizinho = atual.gerarVizinho(catalogo, rng);
        if (vizinho.getPontuacao() > atual.getPontuacao()) {
            atual = vizinho; 
        }
    }
    return atual;
}

void Otimizador::executar() {
    unsigned long int nTentativas = config.getTentativas();
    
    std::mt19937 rngDummy(1234);
    Escala melhorGlobal(catalogo, config, rngDummy);
    
    bool primeiroUpdate = true;
    long long int scoreGlobal = -999999999; 

    // --- MODO 1: GPU CUDA ---
    if (config.getModoExecucao() == GPU_CUDA) {
        cout << "----------------------------------------------------" << endl;
        cout << ">> MODO: ACELERACAO POR GPU (CUDA)" << endl;
        cout << "----------------------------------------------------" << endl;

        std::vector<int> indicesVencedores;
        
        long long int scoreFinalGPU = rodarOtimizacaoCUDA(
            catalogo, 
            config.getTentativas(), 
            config.getTotalSlots(), 
            config.getMaxIteracao(), 
            indicesVencedores,
            config.getVoosPorDia()
        );

        cout << ">> GPU Finalizada. Score Estimado: " << scoreFinalGPU << endl;

        Escala escalaFinal(catalogo, config, rngDummy); 
        escalaFinal.carregarDeIndices(indicesVencedores, catalogo);
        
        cout << "----------------------------------------------------" << endl;
        cout << "MELHOR RESULTADO (GPU):" << endl;
        escalaFinal.imprimir();
        return; 
    }

    // --- MODO 2: CPU MULTICORE (OPENMP) ---
    else if (config.getModoExecucao() == CPU_OPENMP) {
        cout << "----------------------------------------------------" << endl;
        cout << ">> MODO: CPU MULTICORE (OpenMP)" << endl;
        
        #ifdef _OPENMP
            cout << ">> Threads ativas: " << omp_get_max_threads() << endl;
        #else
            cout << ">> AVISO: OpenMP nao detectado (Rodando single-core)!" << endl;
        #endif
        
        cout << "----------------------------------------------------" << endl;

        // Variável compartilhada para contar o progresso total entre todas as threads
        int progressoGlobal = 0;

        #pragma omp parallel 
        {
            unsigned int seed = (unsigned int)time(NULL) + (unsigned int)(omp_get_thread_num() * 1000);
            std::mt19937 rngLocal(seed);

            Escala melhorDaThread(catalogo, config, rngLocal);
            long long int melhorScoreThread = -99999999999;
            bool threadRodouPeloMenosUma = false;

            #pragma omp for
            for (unsigned long int t = 0; t < nTentativas; t++) {
                
                Escala resultadoTentativa = rodarHillClimbingLocal(rngLocal);
                long long int scoreAtual = resultadoTentativa.getPontuacao();

                // Atualiza o melhor local desta thread
                if (!threadRodouPeloMenosUma || scoreAtual > melhorScoreThread) {
                    melhorScoreThread = scoreAtual;
                    melhorDaThread = resultadoTentativa;
                    threadRodouPeloMenosUma = true;
                }

                // --- LOGICA DE PROGRESSO (NOVO) ---
                int meuProgresso;
                // Incrementa atomicamente para saber quantos jobs já foram feitos no total
                #pragma omp atomic capture
                meuProgresso = ++progressoGlobal;

                // A cada 50 tentativas globais, imprime um status
                if (meuProgresso % 50 == 0 || meuProgresso == nTentativas) {
                    #pragma omp critical (imprimir_cout) 
                    {
                        cout << "[OpenMP] Progresso: " << meuProgresso << "/" << nTentativas 
                             << " concluidos..." << endl;
                    }
                }
            }

            // --- ATUALIZACAO DO GLOBAL ---
            #pragma omp critical (atualizar_global)
            {
                if (threadRodouPeloMenosUma) {
                    // Verifica se essa thread tem algo melhor que o recorde global atual
                    if (primeiroUpdate || melhorScoreThread > scoreGlobal) {
                        melhorGlobal = melhorDaThread;
                        scoreGlobal = melhorScoreThread;
                        primeiroUpdate = false;
                        
                        // Imprime sempre que achar um recorde novo
                        cout << "[OpenMP] !!! NOVO RECORDE !!! (Thread " << omp_get_thread_num() 
                             << ") Score: " << scoreGlobal << endl;
                    }
                }
            }
        } 
    }

    // --- MODO 3: CPU SEQUENCIAL (SINGLE CORE) ---
    else {
        cout << "----------------------------------------------------" << endl;
        cout << ">> MODO: CPU SEQUENCIAL (Single Core)" << endl;
        cout << "----------------------------------------------------" << endl;

        std::mt19937 rng((unsigned int)time(NULL));

        for (unsigned long int t = 0; t < nTentativas; t++) {
            
            Escala melhorLocal = rodarHillClimbingLocal(rng);
            long long int scoreLocal = melhorLocal.getPontuacao();

            if (primeiroUpdate || scoreLocal > melhorGlobal.getPontuacao()) {
                melhorGlobal = melhorLocal;
                scoreGlobal = scoreLocal;
                primeiroUpdate = false;
                cout << "[Seq] !!! NOVO RECORDE !!! Tentativa " << (t+1) << " | Score: " << scoreLocal << endl;
            }
            
            // "Heartbeat" a cada 50 iterações
            if ((t + 1) % 50 == 0) { 
                cout << "[Seq] Processando... " << (t+1) << "/" << nTentativas << " concluidas." << endl;
            }
        }
    }

    cout << "----------------------------------------------------" << endl;
    cout << "MELHOR RESULTADO FINAL:" << endl;
    melhorGlobal.imprimir();
}
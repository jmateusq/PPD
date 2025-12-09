#include "Otimizador.h"
#include <vector>
#include <iostream>
#include <algorithm> 
#include <ctime>     
#include <random>

// Importante: Só inclui o header CUDA se for compilar com suporte a GPU
// Se estiver usando apenas g++, o makefile deve tratar isso ou usamos #ifdef
#include "KernelOtimizador.cuh" 

using namespace std;

Otimizador::Otimizador(std::vector<Voo> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {
}

// Método auxiliar executado por uma thread (seja main ou worker do OpenMP)
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
    
    // Gerador dummy apenas para iniciar a variável de melhor escala
    std::mt19937 rngDummy(1234);
    Escala melhorGlobal(catalogo, config, rngDummy);
    
    // Flag para garantir a primeira atualização
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

        // Reconstrói o objeto na CPU para impressão bonita
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
        cout << ">> Threads ativas: " << omp_get_max_threads() << endl;
        cout << "----------------------------------------------------" << endl;

        // Diretiva OpenMP Parallel
        #pragma omp parallel 
        {
            // Semente única por thread baseada no tempo + ID da thread
            unsigned int seed = (unsigned int)time(NULL) + (unsigned int)(omp_get_thread_num() * 1000);
            std::mt19937 rngLocal(seed);

            // Cada thread mantém seu "melhor local" para evitar travar a região crítica a toda hora
            Escala melhorDaThread(catalogo, config, rngLocal);
            long long int melhorScoreThread = -99999999999;
            bool threadRodouPeloMenosUma = false;

            // Divide o loop de tentativas entre as threads
            #pragma omp for
            for (unsigned long int t = 0; t < nTentativas; t++) {
                
                Escala resultadoTentativa = rodarHillClimbingLocal(rngLocal);
                long long int scoreAtual = resultadoTentativa.getPontuacao();

                if (!threadRodouPeloMenosUma || scoreAtual > melhorScoreThread) {
                    melhorScoreThread = scoreAtual;
                    melhorDaThread = resultadoTentativa;
                    threadRodouPeloMenosUma = true;
                }
            }

            // Região Crítica: Apenas uma thread por vez atualiza o global
            #pragma omp critical
            {
                if (threadRodouPeloMenosUma) {
                    if (primeiroUpdate || melhorScoreThread > scoreGlobal) {
                        melhorGlobal = melhorDaThread;
                        scoreGlobal = melhorScoreThread;
                        primeiroUpdate = false;
                        // Opcional: cout pode bagunçar se muitas threads imprimirem, mas ajuda no debug
                        // cout << "[Thread " << omp_get_thread_num() << "] Novo Recorde: " << scoreGlobal << endl;
                    }
                }
            }
        } // Fim parallel
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

            // Lógica de Atualização do Melhor Global
            if (primeiroUpdate || scoreLocal > melhorGlobal.getPontuacao()) {
                melhorGlobal = melhorLocal;
                scoreGlobal = scoreLocal;
                primeiroUpdate = false;
                
                // IMPRIME SEMPRE QUE MELHORAR
                cout << "[Seq] !!! NOVO RECORDE !!! Tentativa " << (t+1) << " | Score: " << scoreLocal << endl;
            }
            
            // Lógica de "Heartbeat" (Sinal de vida) a cada 10% ou 100 iterações
            // Para você ver que o programa não travou se ficar muito tempo sem melhorar
            if ((t + 1) % 50 == 0) { 
                cout << "[Seq] Processando... " << (t+1) << "/" << nTentativas << " concluidas." << endl;
            }
        }
    }

    // Impressão Final Comum (CPU e OpenMP)
    cout << "----------------------------------------------------" << endl;
    cout << "MELHOR RESULTADO FINAL:" << endl;
    melhorGlobal.imprimir();
}


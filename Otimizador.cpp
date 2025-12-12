#include "Otimizador.h"
#include <vector>
#include <iostream>
#include <algorithm> 
#include <ctime>     
#include <random>
#include <omp.h> // Necessário para OpenMP
#include <mpi.h> // Necessário para MPI

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
    
    // Configura o RNG Global (apenas para inicialização de variáveis, não usado no loop paralelo)
    std::mt19937 rngDummy(1234);
    Escala melhorGlobal(catalogo, config, rngDummy);
    
    bool primeiroUpdate = true;
    long long int scoreGlobal = -999999999; 

    // --- VARIÁVEIS MPI ---
    int rank = 0, size = 1;
    // Verifica se o MPI foi inicializado antes de chamar as funções
    int mpi_initialized = 0;
    MPI_Initialized(&mpi_initialized);
    if(mpi_initialized) {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }

    // --- MODO 1: GPU CUDA ---
    if (config.getModoExecucao() == GPU_CUDA) {
        // Se estamos em MPI, apenas o Rank 0 deve rodar GPU ou todos? 
        // Por padrão, para manter compatibilidade, deixamos Rank 0.
        if (rank != 0) return; 

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

    // --- MODO 4: CPU CLUSTER (MPI) ---
    else if (config.getModoExecucao() == CPU_MPI) {
        if (!mpi_initialized) {
            if(rank == 0) cerr << "ERRO: MPI nao inicializado!" << endl;
            return;
        }

        if(rank == 0) {
            cout << "----------------------------------------------------" << endl;
            cout << ">> MODO: CPU CLUSTER (MPI)" << endl;
            cout << ">> Processos (Ranks): " << size << endl;
            cout << "----------------------------------------------------" << endl;
        }

        // Divide o trabalho
        unsigned long int tentativasLocais = nTentativas / size;
        if (rank < (int)(nTentativas % size)) {
            tentativasLocais++; // Distribui o resto
        }

        // Seed única por processo
        unsigned int seed = (unsigned int)time(NULL) + (unsigned int)(rank * 10000);
        std::mt19937 rngLocal(seed);

        Escala melhorLocal(catalogo, config, rngLocal);
        long long int melhorScoreLocal = -99999999999LL;
        bool localRodou = false;

        // Execução Local
        for(unsigned long int t = 0; t < tentativasLocais; t++) {
            Escala resultado = rodarHillClimbingLocal(rngLocal);
            long long int s = resultado.getPontuacao();

            if(!localRodou || s > melhorScoreLocal) {
                melhorScoreLocal = s;
                melhorLocal = resultado;
                localRodou = true;
            }

            // Feedback visual apenas no Rank 0 para não poluir
            if(rank == 0 && (t % 50 == 0)) {
                 cout << "[MPI Rank 0] Processando localmente... " << t << "/" << tentativasLocais << endl;
            }
        }

        // --- REDUÇÃO (Encontrar o melhor score global) ---
        long long int globalMaxScore;
        MPI_Allreduce(&melhorScoreLocal, &globalMaxScore, 1, MPI_LONG_LONG, MPI_MAX, MPI_COMM_WORLD);

        // Quem venceu?
        // Nota: Pode haver empate. O primeiro rank que tiver o score igual ao global envia.
        int souVencedor = (melhorScoreLocal == globalMaxScore) ? 1 : 0;
        
        // Precisamos eleger UM único vencedor para imprimir, caso haja empate
        // Usamos EXSCAN ou lógica simples: Rank mais baixo vence empates
        // Vamos fazer o rank 0 receber de quem tiver o ID.
        
        // Estratégia simples:
        // Se eu sou vencedor, mando meus dados para o Rank 0.
        // Se Rank 0 for vencedor, ele já tem os dados.
        // Se multiplos vencerem, todos mandam? Isso daria colisão.
        // Solução: Gather de (Score, Rank) e Rank 0 decide.
        
        struct {
            long long val;
            int rank;
        } in, out;

        in.val = melhorScoreLocal;
        in.rank = rank;

        MPI_Allreduce(&in, &out, 1, MPI_LONG_LONG_INT, MPI_MAXLOC, MPI_COMM_WORLD);
        
        int rankVencedor = out.rank;
        
        if (rank == 0) {
            cout << "----------------------------------------------------" << endl;
            cout << ">> MPI Finalizado. Melhor Score: " << out.val << " (Encontrado pelo Rank " << rankVencedor << ")" << endl;
        }

        // Recuperar a Escala Vencedora
        std::vector<int> indicesVoos;
        int numSlots = config.getTotalSlots();
        indicesVoos.resize(numSlots);

        if (rank == rankVencedor) {
            indicesVoos = melhorLocal.exportarIndices();
            // Se eu não sou o rank 0, envio para ele
            if (rank != 0) {
                MPI_Send(indicesVoos.data(), numSlots, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }

        if (rank == 0) {
            if (rankVencedor != 0) {
                MPI_Recv(indicesVoos.data(), numSlots, MPI_INT, rankVencedor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // Reconstrói a escala localmente
                melhorGlobal.carregarDeIndices(indicesVoos, catalogo);
            } else {
                melhorGlobal = melhorLocal;
            }
            
            cout << "----------------------------------------------------" << endl;
            cout << "MELHOR RESULTADO GLOBAL (MPI):" << endl;
            melhorGlobal.imprimir();
        }
        
        // Barreira para garantir que ninguém saia antes do Rank 0 imprimir
        MPI_Barrier(MPI_COMM_WORLD);
        return;
    }

    // --- MODO 2: CPU MULTICORE (OPENMP) ---
    else if (config.getModoExecucao() == CPU_OPENMP) {
        // Se estivermos em ambiente MPI, apenas Rank 0 deve executar OpenMP (híbrido simples)
        // Ou se MPI não foi inicializado, roda normal.
        if (rank != 0) return; 

        cout << "----------------------------------------------------" << endl;
        cout << ">> MODO: CPU MULTICORE (OpenMP)" << endl;
        
        #ifdef _OPENMP
            cout << ">> Threads ativas: " << omp_get_max_threads() << endl;
        #else
            cout << ">> AVISO: OpenMP nao detectado (Rodando single-core)!" << endl;
        #endif
        
        cout << "----------------------------------------------------" << endl;

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

                if (!threadRodouPeloMenosUma || scoreAtual > melhorScoreThread) {
                    melhorScoreThread = scoreAtual;
                    melhorDaThread = resultadoTentativa;
                    threadRodouPeloMenosUma = true;
                }

                int meuProgresso;
                #pragma omp atomic capture
                meuProgresso = ++progressoGlobal;

                if (meuProgresso % 50 == 0 || meuProgresso == nTentativas) {
                    #pragma omp critical (imprimir_cout) 
                    {
                        cout << "[OpenMP] Progresso: " << meuProgresso << "/" << nTentativas 
                             << " concluidos..." << endl;
                    }
                }
            }

            #pragma omp critical (atualizar_global)
            {
                if (threadRodouPeloMenosUma) {
                    if (primeiroUpdate || melhorScoreThread > scoreGlobal) {
                        melhorGlobal = melhorDaThread;
                        scoreGlobal = melhorScoreThread;
                        primeiroUpdate = false;
                        cout << "[OpenMP] !!! NOVO RECORDE !!! (Thread " << omp_get_thread_num() 
                             << ") Score: " << scoreGlobal << endl;
                    }
                }
            }
        } 
    }

    // --- MODO 3: CPU SEQUENCIAL (SINGLE CORE) ---
    else {
        // Se estiver em ambiente MPI, apenas Rank 0 executa
        if (rank != 0) return;

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
            
            if ((t + 1) % 50 == 0) { 
                cout << "[Seq] Processando... " << (t+1) << "/" << nTentativas << " concluidas." << endl;
            }
        }
    }

    // Apenas Rank 0 imprime o resultado final nos modos não-MPI
    if (rank == 0) {
        cout << "----------------------------------------------------" << endl;
        cout << "MELHOR RESULTADO FINAL:" << endl;
        melhorGlobal.imprimir();
    }
}

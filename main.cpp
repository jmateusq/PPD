#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <fstream> 
#include <iomanip> 
#include <chrono> 
#include <mpi.h> // ADICIONADO: MPI

#include "Voo.h" 
#include "Otimizador.h"
#include "Configuracao.h" 

using namespace std;
using namespace std::chrono;

const string NOME_ARQUIVO = "catalogo_voos.txt";

// --- Funções Auxiliares de Sistema ---
void limparBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void salvarDados(const vector<Voo>& catalogo) {
    ofstream arquivo(NOME_ARQUIVO);
    if (arquivo.is_open()) {
        for (const auto& v : catalogo) {
            arquivo << v.getId() << " " << v.getOrigem() << " " << v.getDestino() << " " << v.getDuracao() << endl;
        }
        arquivo.close();
    } else {
        cerr << ">> ERRO CRITICO: Nao foi possivel escrever no arquivo " << NOME_ARQUIVO << endl;
    }
}

void carregarDados(vector<Voo>& catalogo) {
    ifstream arquivo(NOME_ARQUIVO);
    if (arquivo.is_open()) {
        catalogo.clear();
        int id, orig, dest, dur;
        while (arquivo >> id >> orig >> dest >> dur) {
            catalogo.push_back(Voo(id, orig, dest, dur));
        }
        arquivo.close();
    }
}

void reindexarESalvar(vector<Voo>& catalogo) {
    vector<Voo> novoCatalogo;
    novoCatalogo.reserve(catalogo.size());
    for (size_t i = 0; i < catalogo.size(); i++) {
        novoCatalogo.push_back(Voo((int)i, catalogo[i].getOrigem(), catalogo[i].getDestino(), catalogo[i].getDuracao()));
    }
    catalogo = novoCatalogo;
    salvarDados(catalogo);
}

// Helper para sincronizar o catálogo entre processos MPI
// Serializa em vetor de int e envia via Broadcast
void sincronizarCatalogoMPI(vector<Voo>& catalogo, int rank) {
    int tamanho = (int)catalogo.size();
    
    // 1. Broadcast do tamanho
    MPI_Bcast(&tamanho, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2. Prepara buffer (4 ints por Voo: id, orig, dest, dur)
    vector<int> buffer;
    if (rank == 0) {
        buffer.reserve(tamanho * 4);
        for(const auto& v : catalogo) {
            buffer.push_back(v.getId());
            buffer.push_back(v.getOrigem());
            buffer.push_back(v.getDestino());
            buffer.push_back(v.getDuracao());
        }
    } else {
        buffer.resize(tamanho * 4);
    }

    // 3. Broadcast dos dados
    MPI_Bcast(buffer.data(), tamanho * 4, MPI_INT, 0, MPI_COMM_WORLD);

    // 4. Reconstrói no destino
    if (rank != 0) {
        catalogo.clear();
        for(int i=0; i<tamanho; i++) {
            catalogo.push_back(Voo(
                buffer[i*4], 
                buffer[i*4+1], 
                buffer[i*4+2], 
                buffer[i*4+3]
            ));
        }
    }
}

// --- MAIN ---
int main(int argc, char** argv) {
    // Inicialização MPI
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand((unsigned int)time(NULL) + rank * 999);

    // Loop principal infinito para manter os processos vivos
    while (true) {
        Configuracao config; 
        std::vector<Voo> catalogo;

        // Rank 0 carrega do arquivo, outros receberão via Sync
        if(rank == 0) {
            carregarDados(catalogo);
        }

        int opcao = 0; 
        bool menuAtivo = true;

        while (menuAtivo) {
            // Apenas Rank 0 mostra o menu e lê a opção
            if (rank == 0) {
                cout << "\n=============================================" << endl;
                cout << "      AIRLINE CREW ROSTERING (OTIMIZADOR)    " << endl;
                cout << "=============================================" << endl;
                cout << " [1] CATÁLOGO: " << catalogo.size() << " voos." << endl;
                cout << " [2] ESCALA  : " << (config.getDias() > 0 ? to_string(config.getDias()) + " Dias" : "Nao def.") << endl;
                
                cout << " [3] ALGORITMO: ";
                if (config.getModoExecucao() == CPU_SEQUENCIAL) cout << "CPU SEQUENCIAL (Single-Core)";
                else if (config.getModoExecucao() == CPU_OPENMP) cout << "CPU PARALELO (OpenMP)";
                else if (config.getModoExecucao() == CPU_MPI) cout << "CPU CLUSTER (MPI - " << size << " Procs)";
                else cout << "GPU ACELERADO (CUDA)";
                cout << endl;
                
                cout << "---------------------------------------------" << endl;
                cout << " 1 - Cadastrar Novo Voo" << endl;
                cout << " 2 - Configurar Tamanho da Escala" << endl;
                cout << " 3 - Configurar Algoritmo (Modo/Iteracoes)" << endl;
                cout << " 4 - EXECUTAR OTIMIZACAO (MEDIR TEMPO)" << endl;
                cout << " 5 - Listar Voos" << endl;
                cout << " 6 - Remover Voo" << endl;
                cout << " 7 - ZERAR Base de Dados" << endl;
                cout << " 0 - Sair" << endl;
                cout << "Escolha: ";
                
                if (!(cin >> opcao)) {
                    cout << "\n>> Entrada invalida!" << endl;
                    limparBuffer();
                    opcao = -1; // Força reloop
                }
            }

            // Sincroniza a decisão do usuário com todos os processos
            MPI_Bcast(&opcao, 1, MPI_INT, 0, MPI_COMM_WORLD);

            if (opcao == -1 && rank == 0) continue;

            switch (opcao) {
                case 1: { 
                    // Add Voo
                    int dados[3]; // Orig, Dest, Dur
                    if (rank == 0) {
                        cout << "Origem: "; cin >> dados[0];
                        cout << "Destino: "; cin >> dados[1];
                        cout << "Duracao: "; cin >> dados[2];
                    }
                    // Envia dados do voo para todos
                    MPI_Bcast(dados, 3, MPI_INT, 0, MPI_COMM_WORLD);
                    
                    // Todos adicionam para manter consistência interna
                    if (rank == 0) {
                        catalogo.push_back(Voo((int)catalogo.size(), dados[0], dados[1], dados[2])); 
                        salvarDados(catalogo);
                    break;
                }
                case 2: {
                    int dados[2]; // Dias, VoosDia
                    if(rank == 0) {
                        cout << "Dias: "; cin >> dados[0];
                        cout << "Voos/Dia: "; cin >> dados[1];
                    }
                    MPI_Bcast(dados, 2, MPI_INT, 0, MPI_COMM_WORLD);
                    config.setDias(dados[0]);
                    config.setVoosPorDia(dados[1]);
                    break;
                }
                case 3: {
                    unsigned long int dadosUL[2]; // Tent, Iter
                    int modo;
                    if(rank == 0) {
                        cout << "Tentativas (Restarts): "; cin >> dadosUL[0];
                        cout << "Iteracoes/Tentativa: "; cin >> dadosUL[1];
                        cout << "Modo (0=CPU Seq, 1=OpenMP, 2=CUDA, 3=MPI): "; cin >> modo;
                    }
                    MPI_Bcast(dadosUL, 2, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
                    MPI_Bcast(&modo, 1, MPI_INT, 0, MPI_COMM_WORLD);

                    config.setTentativas(dadosUL[0]);
                    config.setMaxIteracoes(dadosUL[1]);
                    try { config.setModoExecucao(modo); } catch (...) {}
                    break;
                }
                case 4: {
                    // Executar
                    // 1. Sincronizar Catálogo Completo (Garante que workers tenham dados atualizados do arquivo)
                    if (rank == 0 && catalogo.empty()) { cout << "Catalogo vazio!" << endl; }
                    
                    // Verifica se está vazio no Rank 0 e avisa (skip execution)
                    int catalogoVazio = (rank == 0 && catalogo.empty()) ? 1 : 0;
                    MPI_Bcast(&catalogoVazio, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    if(catalogoVazio) break;

                    // Sincroniza o vetor de voos
                    sincronizarCatalogoMPI(catalogo, rank);

                    // Defaults
                    if (config.getDias() <= 0) { config.setDias(5); config.setVoosPorDia(4); }
                    if (config.getMaxIteracao() == 0) { config.setMaxIteracoes(2000); }

                    if(rank == 0) cout << "\n>>> INICIANDO EXECUCAO... AGUARDE <<<" << endl;

                    try {
                        Otimizador solver(catalogo, config);

                        // --- MEDIÇÃO DE TEMPO ---
                        // Barreira para todos começarem juntos
                        MPI_Barrier(MPI_COMM_WORLD);
                        auto inicio = high_resolution_clock::now();
                        
                        solver.executar();
                        
                        // Barreira para todos terminarem juntos
                        MPI_Barrier(MPI_COMM_WORLD);
                        auto fim = high_resolution_clock::now();
                        auto duracao = duration_cast<milliseconds>(fim - inicio);
                        // ------------------------

                        if(rank == 0) {
                            cout << "\n=============================================" << endl;
                            cout << " TEMPO DE EXECUCAO TOTAL: " << duracao.count() << " ms" << endl;
                            cout << "=============================================" << endl;
                            
                            cout << "Pressione ENTER para continuar...";
                            limparBuffer(); cin.get();
                        }
                        
                        menuAtivo = false; 

                    } catch (const std::exception& e) {
                        if(rank == 0) cerr << "Erro: " << e.what() << endl;
                    }
                    break;
                }
                case 5: {
                    if (rank == 0) {
                        for(const auto& v : catalogo) cout << v.getId() << ": " << Voo::getNomeAeroporto(v.getOrigem()) << "->" << Voo::getNomeAeroporto(v.getDestino()) << endl;
                        limparBuffer(); cin.get();
                    }
                    break;
                }
                case 6: {
                    // Remover
                    int id;
                    if(rank == 0) { cout << "ID para remover: "; cin >> id; }
                    MPI_Bcast(&id, 1, MPI_INT, 0, MPI_COMM_WORLD);
                    
                    // Logica de remoção apenas no Rank 0 (arquivo), sync ocorre no passo 4
                    if(rank == 0) {
                        bool achou = false;
                        for(auto it=catalogo.begin(); it!=catalogo.end(); ++it) if(it->getId()==id) { catalogo.erase(it); achou=true; break; }
                        if(achou) reindexarESalvar(catalogo);
                    }
                    break;
                }
                case 7: {
                    if(rank == 0) { catalogo.clear(); salvarDados(catalogo); }
                    break;
                }
                case 0: {
                    MPI_Finalize();
                    return 0;
                }
                default: if(rank == 0) cout << "Invalido" << endl;
            } 
        } 
        
    }
    
    MPI_Finalize();
    return 0;
    }
}

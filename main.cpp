#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <fstream> 
#include <iomanip> 
#include <chrono> // ADICIONADO: Para medição de tempo

#include "Voo.h" 
#include "Otimizador.h"
#include "Configuracao.h" 

using namespace std;
using namespace std::chrono; // Namespace para simplificar o tempo

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

// --- MAIN ---
int main() {
    srand((unsigned int)time(NULL));

    while (true) {
        Configuracao config; 
        std::vector<Voo> catalogo;
        carregarDados(catalogo);

        int opcao = 0; 
        bool menuAtivo = true;

        while (menuAtivo) {
            cout << "\n=============================================" << endl;
            cout << "      AIRLINE CREW ROSTERING (OTIMIZADOR)    " << endl;
            cout << "=============================================" << endl;
            cout << " [1] CATÁLOGO: " << catalogo.size() << " voos." << endl;
            cout << " [2] ESCALA  : " << (config.getDias() > 0 ? to_string(config.getDias()) + " Dias" : "Nao def.") << endl;
            
            cout << " [3] ALGORITMO: ";
            if (config.getModoExecucao() == CPU_SEQUENCIAL) cout << "CPU SEQUENCIAL (Single-Core)";
            else if (config.getModoExecucao() == CPU_OPENMP) cout << "CPU PARALELO (OpenMP)";
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
                continue;
            }

            switch (opcao) {
                case 1: { 
                    int orig, dest, dur;
                    cout << "Origem: "; cin >> orig;
                    cout << "Destino: "; cin >> dest;
                    cout << "Duracao: "; cin >> dur;
                    catalogo.push_back(Voo((int)catalogo.size(), orig, dest, dur)); 
                    salvarDados(catalogo);
                    break;
                }
                case 2: {
                    int d, v;
                    cout << "Dias: "; cin >> d; config.setDias(d);
                    cout << "Voos/Dia: "; cin >> v; config.setVoosPorDia(v);
                    break;
                }
                case 3: {
                    int t, i, m;
                    cout << "Tentativas (Restarts): "; cin >> t; config.setTentativas(t);
                    cout << "Iteracoes/Tentativa: "; cin >> i; config.setMaxIteracoes(i);
                    cout << "Modo (0=CPU Single, 1=CPU OpenMP, 2=GPU CUDA): "; cin >> m;
                    try { config.setModoExecucao(m); } catch (exception& e) { cout << "Erro: " << e.what() << endl; }
                    break;
                }
                case 4: {
                    if (catalogo.empty()) { cout << "Catalogo vazio!" << endl; break; }
                    
                    // Defaults se o user nao configurar
                    if (config.getDias() <= 0) { config.setDias(5); config.setVoosPorDia(4); }
                    if (config.getMaxIteracao() == 0) { config.setMaxIteracoes(2000); }

                    cout << "\n>>> INICIANDO EXECUCAO... AGUARDE <<<" << endl;

                    try {
                        Otimizador solver(catalogo, config);

                        // --- MEDIÇÃO DE TEMPO ---
                        auto inicio = high_resolution_clock::now();
                        
                        solver.executar();
                        
                        auto fim = high_resolution_clock::now();
                        auto duracao = duration_cast<milliseconds>(fim - inicio);
                        // ------------------------

                        cout << "\n=============================================" << endl;
                        cout << " TEMPO DE EXECUCAO TOTAL: " << duracao.count() << " ms" << endl;
                        cout << "=============================================" << endl;
                        
                        cout << "Pressione ENTER para continuar...";
                        limparBuffer(); cin.get();
                        menuAtivo = false; 
                    } catch (const std::exception& e) {
                        cerr << "Erro: " << e.what() << endl;
                    }
                    break;
                }
                case 5: {
                    for(const auto& v : catalogo) cout << v.getId() << ": " << Voo::getNomeAeroporto(v.getOrigem()) << "->" << Voo::getNomeAeroporto(v.getDestino()) << endl;
                    limparBuffer(); cin.get();
                    break;
                }
                case 6: {
                    int id; cout << "ID para remover: "; cin >> id;
                    bool achou = false;
                    for(auto it=catalogo.begin(); it!=catalogo.end(); ++it) if(it->getId()==id) { catalogo.erase(it); achou=true; break; }
                    if(achou) reindexarESalvar(catalogo);
                    break;
                }
                case 7: {
                    catalogo.clear(); salvarDados(catalogo); break;
                }
                case 0: return 0;
                default: cout << "Invalido" << endl;
            } 
        } 
    }
    return 0;
}
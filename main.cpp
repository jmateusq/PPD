#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <fstream> 
#include "Materia.h" 
#include "Otimizador.h"
#include "Configuracao.h" 

using namespace std;

const string NOME_ARQUIVO = "catalogo_materias.txt";

// --- Funções Auxiliares ---

void limparBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string traduzirDificuldade(int dif) {
    switch (dif) {
        case 0: return "Livre";
        case 1: return "Leve";
        case 2: return "Media";
        case 3: return "Dificil";
        default: return to_string(dif);
    }
}

// Salva o vetor atual no arquivo
void salvarDados(const vector<Materia>& catalogo) {
    ofstream arquivo(NOME_ARQUIVO);
    if (arquivo.is_open()) {
        for (const auto& mat : catalogo) {
            // Formato: ID Nome Dificuldade
            arquivo << mat.getId() << " " << mat.getNome() << " " << mat.getDificuldade() << endl;
        }
        arquivo.close();
    } else {
        cout << ">> ERRO: Nao foi possivel salvar no arquivo!" << endl;
    }
}

// Carrega do arquivo para o vetor
void carregarDados(vector<Materia>& catalogo) {
    ifstream arquivo(NOME_ARQUIVO);
    if (arquivo.is_open()) {
        catalogo.clear();
        unsigned long int id;
        string nome;
        int dif;
        while (arquivo >> id >> nome >> dif) {
            catalogo.push_back(Materia(id, nome, dif));
        }
        arquivo.close();
    }
}

// Reorganiza IDs e salva
void reindexarESalvar(vector<Materia>& catalogo) {
    vector<Materia> novoCatalogo;
    for (size_t i = 0; i < catalogo.size(); i++) {
        novoCatalogo.push_back(Materia(i, catalogo[i].getNome(), catalogo[i].getDificuldade()));
    }
    catalogo = novoCatalogo;
    salvarDados(catalogo);
}

// --- MAIN ---

int main() {
    srand((unsigned int)time(NULL));

    // --- LOOP DA SESSÃO (Reinicia o programa mantendo os dados salvos) ---
    while (true) {
        
        // 1. Instancia Configuração ZERADA a cada reinício
        Configuracao config; 
        try { config.setDias(0); } catch(...) {} 
        try { config.setMaxIteracoes(0); } catch(...) {}

        // 2. Carrega o catálogo (Persistência)
        std::vector<Materia> catalogo;
        carregarDados(catalogo);

        int opcao = 0; 
        bool sessaoAtiva = true; // Controla o loop do menu

        // --- LOOP DO MENU ---
        while (sessaoAtiva) {
            cout << "\n=============================================" << endl;
            cout << "      SISTEMA DE OTIMIZACAO DE ESTUDOS      " << endl;
            cout << "=============================================" << endl;
            
            cout << " [1] MATERIAS: " << catalogo.size() << " cadastradas." << endl;
            
            cout << " [2] TEMPO   : ";
            if (config.getDias() <= 0) cout << "Pendente";
            else cout << config.getDias() << " Dias | " << config.getTurnos() << " Turnos";
            cout << endl;

            cout << " [3] PRECISAO: ";
            if (config.getMaxIteracao() <= 0) cout << "Padrao (1x 1000)";
            else cout << config.getTentativas() << " Restarts x " << config.getMaxIteracao() << " Iteracoes";
            cout << endl;
            
            cout << "---------------------------------------------" << endl;
            cout << " 1 - Cadastrar Nova Materia" << endl;
            cout << " 2 - Configurar Dias, Turnos e Aulas" << endl;
            cout << " 3 - Configurar Precisao" << endl;
            cout << " 4 - EXECUTAR OTIMIZACAO" << endl;
            cout << " 5 - Listar Materias" << endl;
            cout << " 6 - Remover uma Materia (Por ID)" << endl;
            cout << " 7 - ZERAR todo o catalogo" << endl;
            cout << " 0 - Sair do Programa" << endl;
            cout << "Escolha: ";
            
            if (!(cin >> opcao)) {
                cout << "\n>> Entrada invalida!" << endl;
                limparBuffer();
                continue;
            }

            switch (opcao) {
                case 1: { 
                    string nomeMat;
                    int difMat;
                    cout << "\n--- CADASTRAR ---" << endl;
                    cout << "Nome (sem espacos): ";
                    cin >> nomeMat;
                    
                    bool difValida = false;
                    while (!difValida) {
                        cout << "Dificuldade (0=Livre, 1=Leve, 3=Dificil): ";
                        if (cin >> difMat && difMat >= 0 && difMat <= 3) difValida = true;
                        else { cout << ">> Valor invalido (0-3)." << endl; limparBuffer(); }
                    }

                    unsigned long int novoID = (unsigned long int)catalogo.size();
                    catalogo.push_back(Materia(novoID, nomeMat, difMat)); 
                    salvarDados(catalogo);
                    cout << ">> Salvo!" << endl;
                    break;
                }

                case 2: {
                    int val;
                    cout << "\n--- CONFIGURAR TEMPO ---" << endl;
                    while (true) {
                        cout << "Dias (1-7): ";
                        if (cin >> val) { try { config.setDias(val); break; } catch (exception& e) { cout << e.what() << endl; } } 
                        else limparBuffer();
                    }
                    while (true) {
                        cout << "Turnos (1-3): ";
                        if (cin >> val) { try { config.setTurnos(val); break; } catch (exception& e) { cout << e.what() << endl; } } 
                        else limparBuffer();
                    }
                    while (true) {
                        cout << "Aulas/Turno: ";
                        if (cin >> val) { try { config.setAulaTurno(val); break; } catch (exception& e) { cout << e.what() << endl; } } 
                        else limparBuffer();
                    }
                    break;
                }

                case 3: {
                    cout << "\n--- CONFIGURACAO DE PRECISAO ---" << endl;
                    unsigned long int tentativas, iteracoes;

                    // Configurar REINICIALIZAÇÕES (Restarts)
                    while (true) {
                        cout << "1. Numero de Tentativas (Quantas vezes reiniciar do zero?): ";
                        if (cin >> tentativas) {
                            try {
                                config.setTentativas(tentativas);
                                break;
                            } catch (const std::exception& e) {
                                cout << ">> ERRO: " << e.what() << endl;
                            }
                        } else {
                            limparBuffer();
                        }
                    }

                    // Configurar ITERAÇÕES (Steps)
                    while (true) {
                        cout << "2. Iteracoes por Tentativa (Passos de melhoria interna): ";
                        if (cin >> iteracoes) {
                            try {
                                config.setMaxIteracoes(iteracoes);
                                break;
                            } catch (const std::exception& e) {
                                cout << ">> ERRO: " << e.what() << endl;
                            }
                        } else {
                            limparBuffer();
                        }
                    }
                    
                    cout << ">> Configurado: Rodara " << config.getTentativas() 
                        << " vezes, com " << config.getMaxIteracao() << " passos cada." << endl;
                    cout << ">> Total de grades geradas/analisadas: " 
                        << (long long unsigned)config.getTentativas() * config.getMaxIteracao() << endl;
                    break;
                }


                case 4: {
                    // --- EXECUÇÃO DO ALGORITMO ---
                    if (catalogo.empty()) {
                        cout << "\n>> [ERRO] Catalogo vazio! Adicione materias primeiro." << endl;
                        break; 
                    }

                    if (config.getTentativas() <= 0) config.setTentativas(1);

                    // Aplica padrões se o usuário não configurou
                    if (config.getDias() <= 0) config.setDias(5);
                    if (config.getTurnos() <= 0) config.setTurnos(2);
                    if (config.getAulaTurno() <= 0) config.setAulaTurno(4);
                    if (config.getMaxIteracao() <= 0) config.setMaxIteracoes(1000);

                    cout << "\n=============================================" << endl;
                    cout << "          RESULTADO DA OTIMIZACAO           " << endl;
                    cout << "=============================================" << endl;

                    try {
                        Otimizador hillClimbing(catalogo, config);
                        hillClimbing.executar(); // Assume-se que este método imprime a grade na tela
                        
                        cout << "\n=============================================" << endl;
                        cout << ">> Processo concluido!" << endl;
                        cout << ">> Pressione ENTER para reiniciar o sistema (limpar config)...";
                        
                        limparBuffer(); 
                        cin.get(); // Pausa para leitura

                        sessaoAtiva = false; // <--- ISSO QUEBRA O LOOP INTERNO E REINICIA O PROGRAMA
                    } catch (const std::exception& e) {
                        cerr << "\n[ERRO]: " << e.what() << endl;
                    }
                    break;
                }

                case 5: {
                    cout << "\n--- LISTA ATUAL ---" << endl;
                    if (catalogo.empty()) cout << "Vazio." << endl;
                    else {
                        cout << "ID\t| DIF.\t| NOME" << endl;
                        for (const auto& m : catalogo) {
                            cout << m.getId() << "\t| " << m.getDificuldade() << "\t| " << m.getNome() << endl;
                        }
                    }
                    cout << "\nEnter p/ voltar...";
                    limparBuffer(); cin.get();
                    break;
                }

                case 6: { 
                    if (catalogo.empty()) { cout << "Vazio." << endl; break; }
                    cout << "\n--- REMOVER ---" << endl;
                    for (const auto& m : catalogo) cout << "[" << m.getId() << "] " << m.getNome() << endl;
                    unsigned long int idRemover;
                    cout << "ID para apagar: ";
                    if (cin >> idRemover) {
                        bool achou = false;
                        for (auto it = catalogo.begin(); it != catalogo.end(); ++it) {
                            if (it->getId() == idRemover) { catalogo.erase(it); achou = true; break; }
                        }
                        if (achou) { reindexarESalvar(catalogo); cout << ">> Removido." << endl; }
                        else cout << ">> ID nao encontrado." << endl;
                    } else limparBuffer();
                    break;
                }

                case 7: {
                    char c; cout << "APAGAR TUDO? (s/n): "; cin >> c;
                    if (c == 's' || c == 'S') { catalogo.clear(); salvarDados(catalogo); cout << ">> Resetado." << endl; }
                    break;
                }

                case 0:
                    cout << "Encerrando sistema..." << endl;
                    return 0; // Sai do programa completamente (encerra o while(true) principal)

                default:
                    cout << "Opcao invalida!" << endl;
            } // Fim Switch
        } // Fim Loop Menu (sessaoAtiva)

        // Ao sair do loop acima (após case 4), o código volta para o início do while(true),
        // recriando 'Configuracao config' do zero.
        cout << "\n\n... Reiniciando sistema ...\n\n";

    } // Fim Loop Sessão

    return 0;
}
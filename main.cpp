#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <fstream> 
#include <iomanip> // Para std::setw (formatação de tabelas)

// Nossos módulos refatorados
#include "Voo.h" 
#include "Otimizador.h"
#include "Configuracao.h" 

using namespace std;

const string NOME_ARQUIVO = "catalogo_voos.txt";

// --- Funções Auxiliares de Sistema ---

// Limpa o buffer de entrada (cin) para evitar bugs de leitura
void limparBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// Salva o vetor atual no arquivo de texto
void salvarDados(const vector<Voo>& catalogo) {
    ofstream arquivo(NOME_ARQUIVO);
    if (arquivo.is_open()) {
        for (const auto& v : catalogo) {
            // Formato: ID ORIGEM DESTINO DURACAO
            arquivo << v.getId() << " " 
                    << v.getOrigem() << " " 
                    << v.getDestino() << " " 
                    << v.getDuracao() << endl;
        }
        arquivo.close();
    } else {
        cerr << ">> ERRO CRITICO: Nao foi possivel escrever no arquivo " << NOME_ARQUIVO << endl;
    }
}

// Carrega do arquivo para o vetor em memória
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
    // Se o arquivo não existir, inicia com catálogo vazio sem dar erro
}

// Reorganiza IDs sequencialmente após uma remoção e salva
void reindexarESalvar(vector<Voo>& catalogo) {
    vector<Voo> novoCatalogo;
    novoCatalogo.reserve(catalogo.size());
    
    for (size_t i = 0; i < catalogo.size(); i++) {
        // Recria o voo mantendo dados originais, mas atualizando o ID para o índice 'i'
        novoCatalogo.push_back(Voo((int)i, catalogo[i].getOrigem(), catalogo[i].getDestino(), catalogo[i].getDuracao()));
    }
    catalogo = novoCatalogo;
    salvarDados(catalogo);
}

// --- MAIN (Ponto de Entrada) ---

int main() {
    // Inicializa semente aleatória para resultados variados
    srand((unsigned int)time(NULL));

    // Loop da Sessão (permite reiniciar o sistema sem fechar o executável)
    while (true) {
        
        // 1. Instancia Configuração (Reseta as configurações do usuário)
        Configuracao config; 
        
        // 2. Carrega os dados persistentes
        std::vector<Voo> catalogo;
        carregarDados(catalogo);

        int opcao = 0; 
        bool menuAtivo = true;

        // Loop do Menu Interativo
        while (menuAtivo) {
            cout << "\n=============================================" << endl;
            cout << "      AIRLINE CREW ROSTERING (OTIMIZADOR)    " << endl;
            cout << "=============================================" << endl;
            
            cout << " [1] CATÁLOGO: " << catalogo.size() << " voos disponiveis." << endl;
            
            cout << " [2] ESCALA  : ";
            if (config.getDias() <= 0) cout << "Nao configurada (Defina Dias/Voos)";
            else cout << config.getDias() << " Dias | Max " << config.getVoosPorDia() << " Voos/Dia";
            cout << endl;

            cout << " [3] ALGORITMO: ";
            if (config.getMaxIteracao() <= 0) cout << "Padrao (1x 1000)";
            else cout << config.getTentativas() << " Restarts x " << config.getMaxIteracao() << " Steps";
            cout << endl;
            
            cout << "---------------------------------------------" << endl;
            cout << " 1 - Cadastrar Novo Voo" << endl;
            cout << " 2 - Configurar Tamanho da Escala" << endl;
            cout << " 3 - Configurar Precisao (Iteracoes)" << endl;
            cout << " 4 - EXECUTAR OTIMIZACAO" << endl;
            cout << " 5 - Listar Voos (Tabela)" << endl;
            cout << " 6 - Remover Voo" << endl;
            cout << " 7 - ZERAR Base de Dados" << endl;
            cout << " 0 - Sair" << endl;
            cout << "Escolha: ";
            
            if (!(cin >> opcao)) {
                cout << "\n>> Entrada invalida! Digite um numero." << endl;
                limparBuffer();
                continue;
            }

            switch (opcao) {
                case 1: { 
                    int orig, dest, dur;
                    cout << "\n--- CADASTRAR VOO ---" << endl;
                    cout << "Use IDs de aeroportos (0=GRU, 1=MIA, 2=JFK...)" << endl;
                    
                    cout << "ID Origem: "; 
                    while(!(cin >> orig)) { cout << "Numero invalido. Origem: "; limparBuffer(); }
                    
                    cout << "ID Destino: "; 
                    while(!(cin >> dest)) { cout << "Numero invalido. Destino: "; limparBuffer(); }
                    
                    cout << "Duracao (min): "; 
                    while(!(cin >> dur)) { cout << "Numero invalido. Duracao: "; limparBuffer(); }

                    int novoID = (int)catalogo.size();
                    catalogo.push_back(Voo(novoID, orig, dest, dur)); 
                    salvarDados(catalogo);
                    cout << ">> Voo salvo com sucesso!" << endl;
                    break;
                }

                case 2: {
                    int val;
                    cout << "\n--- CONFIGURAR ESCALA ---" << endl;
                    while (true) {
                        cout << "Horizonte de Dias (ex: 30): ";
                        if (cin >> val) { try { config.setDias(val); break; } catch (exception& e) { cout << e.what() << endl; } } 
                        else limparBuffer();
                    }
                    while (true) {
                        cout << "Max Voos por Dia (ex: 4): ";
                        if (cin >> val) { try { config.setVoosPorDia(val); break; } catch (exception& e) { cout << e.what() << endl; } } 
                        else limparBuffer();
                    }
                    break;
                }

                case 3: {
                    cout << "\n--- CONFIGURACAO DE PRECISAO ---" << endl;
                    unsigned long int tentativas, iteracoes;

                    cout << "1. Tentativas (Restarts) - Quantas escalas gerar do zero? ";
                    cin >> tentativas;
                    try { config.setTentativas(tentativas); } catch(...) {}

                    cout << "2. Iteracoes por Tentativa - Quantas trocas testar? ";
                    cin >> iteracoes;
                    try { config.setMaxIteracoes(iteracoes); } catch(...) {}
                    
                    cout << ">> Configurado: " << config.getTentativas() * config.getMaxIteracao() << " avaliacoes totais." << endl;
                    break;
                }

                case 4: {
                    if (catalogo.empty()) {
                        cout << "\n>> [ERRO] Catalogo vazio! Impossivel gerar escala." << endl;
                        break; 
                    }

                    // Define padrões razoáveis se o usuário esqueceu de configurar
                    if (config.getTentativas() <= 0) config.setTentativas(1);
                    if (config.getDias() <= 0) config.setDias(5);
                    if (config.getVoosPorDia() <= 0) config.setVoosPorDia(4);
                    if (config.getMaxIteracao() <= 0) config.setMaxIteracoes(2000);

                    cout << "\n=============================================" << endl;
                    cout << "          INICIANDO OTIMIZACAO...           " << endl;
                    cout << "=============================================" << endl;

                    try {
                        Otimizador solver(catalogo, config);
                        solver.executar();
                        
                        cout << "\n>> Processo concluido!" << endl;
                        cout << ">> Pressione ENTER para reiniciar e limpar configuracoes...";
                        limparBuffer(); cin.get();
                        
                        menuAtivo = false; // Sai do menu para reiniciar a sessão
                    } catch (const std::exception& e) {
                        cerr << "\n[ERRO DE EXECUCAO]: " << e.what() << endl;
                    }
                    break;
                }

                case 5: {
                    cout << "\n--- LISTA DE VOOS CADASTRADOS ---" << endl;
                    if (catalogo.empty()) cout << "Nenhum voo cadastrado." << endl;
                    else {
                        cout << std::left << std::setw(6) << "ID" 
                             << "| " << std::setw(15) << "ORIGEM" 
                             << "-> " << std::setw(15) << "DESTINO" 
                             << "| DURACAO" << endl;
                        cout << "---------------------------------------------------------" << endl;
                        
                        for (const auto& v : catalogo) {
                            // CHAMADA ESTÁTICA PARA Voo::getNomeAeroporto
                            cout << std::left << std::setw(6) << v.getId() 
                                 << "| " << std::setw(15) << Voo::getNomeAeroporto(v.getOrigem())
                                 << "-> " << std::setw(15) << Voo::getNomeAeroporto(v.getDestino())
                                 << "| " << v.getDuracao() << " min" << endl;
                        }
                    }
                    cout << "\nPressione ENTER para voltar...";
                    limparBuffer(); cin.get();
                    break;
                }

                case 6: { 
                    if (catalogo.empty()) break;
                    int idRemover;
                    cout << "Digite o ID do voo para remover: ";
                    if (cin >> idRemover) {
                        bool achou = false;
                        for (auto it = catalogo.begin(); it != catalogo.end(); ++it) {
                            if (it->getId() == idRemover) { 
                                catalogo.erase(it); 
                                achou = true; 
                                break; 
                            }
                        }
                        if (achou) { 
                            reindexarESalvar(catalogo); 
                            cout << ">> Voo removido e IDs reordenados." << endl; 
                        } else {
                            cout << ">> ID nao encontrado." << endl;
                        }
                    } else limparBuffer();
                    break;
                }

                case 7: {
                    char c; cout << "TEM CERTEZA? Isso apagara todos os voos (s/n): "; cin >> c;
                    if (c == 's' || c == 'S') { 
                        catalogo.clear(); 
                        salvarDados(catalogo); 
                        cout << ">> Base de dados resetada." << endl; 
                    }
                    break;
                }

                case 0:
                    cout << "Saindo do sistema..." << endl;
                    return 0; // Encerra o programa

                default:
                    cout << "Opcao invalida!" << endl;
            } 
        } 
        
        cout << "\n\n... Reiniciando sistema ...\n\n";

    } // Fim do While Principal
    return 0;
}
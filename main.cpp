#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <limits>
#include <fstream> // Necessário para arquivos
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

// Carrega do arquivo para o vetor ao iniciar
void carregarDados(vector<Materia>& catalogo) {
    ifstream arquivo(NOME_ARQUIVO);
    if (arquivo.is_open()) {
        catalogo.clear();
        unsigned long int id;
        string nome;
        int dif;
        
        // Lê enquanto houver dados
        while (arquivo >> id >> nome >> dif) {
            catalogo.push_back(Materia(id, nome, dif));
        }
        arquivo.close();
    }
    // Se o arquivo não existir, apenas inicia vazio (sem erro)
}

// Reorganiza os IDs para ficarem sequenciais (0, 1, 2...) após remover algo
void reindexarESalvar(vector<Materia>& catalogo) {
    // Cria um novo vetor temporário para reconstruir com IDs certos
    vector<Materia> novoCatalogo;
    for (size_t i = 0; i < catalogo.size(); i++) {
        // Recria a materia mantendo nome e dificuldade, mas atualizando ID para 'i'
        novoCatalogo.push_back(Materia(i, catalogo[i].getNome(), catalogo[i].getDificuldade()));
    }
    catalogo = novoCatalogo;
    salvarDados(catalogo);
}

// --- MAIN ---

int main() {
    srand((unsigned int)time(NULL));
    Configuracao config; 
    
    // Inicializa config zerada para controle visual
    try { config.setDias(0); } catch(...) {} 
    try { config.setMaxIteracoes(0); } catch(...) {}

    std::vector<Materia> catalogo;

    // >> CARREGAMENTO AUTOMÁTICO <<
    carregarDados(catalogo);

    int opcao = 0; 

    do {
        // --- DASHBOARD ---
        cout << "\n=============================================" << endl;
        cout << "      SISTEMA DE OTIMIZACAO DE ESTUDOS      " << endl;
        cout << "=============================================" << endl;
        
        cout << " [1] MATERIAS: " << catalogo.size() << " cadastradas." << endl;
        
        cout << " [2] TEMPO   : ";
        if (config.getDias() <= 0) cout << "Pendente";
        else cout << config.getDias() << " Dias | " << config.getTurnos() << " Turnos";
        cout << endl;

        cout << " [3] PRECISAO: ";
        if (config.getMaxIteracao() <= 0) cout << "Padrao (1000)";
        else cout << config.getMaxIteracao() << " Iteracoes";
        cout << endl;
        
        cout << "---------------------------------------------" << endl;
        cout << " 1 - Cadastrar Nova Materia" << endl;
        cout << " 2 - Configurar Dias, Turnos e Aulas" << endl;
        cout << " 3 - Configurar Precisao" << endl;
        cout << " 4 - EXECUTAR OTIMIZACAO" << endl;
        cout << " 5 - Listar Materias" << endl;
        cout << " 6 - Remover uma Materia (Por ID)" << endl; // NOVA
        cout << " 7 - ZERAR todo o catalogo" << endl;       // NOVA
        cout << " 0 - Sair" << endl;
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
                cout << "\n--- CADASTRAR MATERIA ---" << endl;
                cout << "Nome (sem espacos, use underline_): ";
                cin >> nomeMat; // Obs: cin lê até o primeiro espaço. Para nomes compostos precisa de getline, mas simplificamos aqui.

                bool difValida = false;
                while (!difValida) {
                    cout << "Dificuldade (0=Livre, 1=Leve, 3=Dificil): ";
                    if (cin >> difMat) {
                        if (difMat >= 0 && difMat <= 3) difValida = true;
                        else cout << ">> Valor deve ser entre 0 e 3." << endl;
                    } else {
                        cout << ">> Digite apenas numeros." << endl;
                        limparBuffer();
                    }
                }

                // Adiciona e Salva
                unsigned long int novoID = (unsigned long int)catalogo.size();
                catalogo.push_back(Materia(novoID, nomeMat, difMat)); 
                salvarDados(catalogo); // <--- SALVA NO ARQUIVO

                cout << ">> Materia salva com sucesso!" << endl;
                break;
            }

            case 2: {
                int val;
                cout << "\n--- TEMPO ---" << endl;
                // Configura DIAS
                while (true) {
                    cout << "Dias (1-7): ";
                    if (cin >> val) {
                        try { config.setDias(val); break; } 
                        catch (exception& e) { cout << "Erro: " << e.what() << endl; }
                    } else { limparBuffer(); }
                }
                // Configura TURNOS
                while (true) {
                    cout << "Turnos (1-3): ";
                    if (cin >> val) {
                        try { config.setTurnos(val); break; } 
                        catch (exception& e) { cout << "Erro: " << e.what() << endl; }
                    } else { limparBuffer(); }
                }
                // Configura AULAS
                while (true) {
                    cout << "Aulas/Turno: ";
                    if (cin >> val) {
                        try { config.setAulaTurno(val); break; } 
                        catch (exception& e) { cout << "Erro: " << e.what() << endl; }
                    } else { limparBuffer(); }
                }
                break;
            }

            case 3: {
                int iter;
                cout << "Iteracoes: ";
                if(cin >> iter) {
                    try { config.setMaxIteracoes(iter); } catch(...) {}
                } else limparBuffer();
                break;
            }

            case 4:
                cout << "\nVerificando dados..." << endl;
                break; // Sai do switch e vai para execução

            case 5: {
                cout << "\n--- LISTA ---" << endl;
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

            case 6: { // --- REMOVER MATÉRIA ---
                if (catalogo.empty()) {
                    cout << ">> Nada para remover." << endl;
                } else {
                    cout << "\n--- REMOVER MATERIA ---" << endl;
                    // Lista primeiro para facilitar
                    for (const auto& m : catalogo) 
                        cout << "[" << m.getId() << "] " << m.getNome() << endl;
                    
                    unsigned long int idRemover;
                    cout << "Digite o ID para apagar: ";
                    if (cin >> idRemover) {
                        bool encontrado = false;
                        // Procura e remove
                        for (auto it = catalogo.begin(); it != catalogo.end(); ++it) {
                            if (it->getId() == idRemover) {
                                catalogo.erase(it);
                                encontrado = true;
                                cout << ">> Materia removida." << endl;
                                break;
                            }
                        }
                        if (encontrado) {
                            reindexarESalvar(catalogo); // Reorganiza IDs e Salva
                        } else {
                            cout << ">> ID nao encontrado." << endl;
                        }
                    } else {
                        limparBuffer();
                        cout << ">> Entrada invalida." << endl;
                    }
                }
                break;
            }

            case 7: { // --- ZERAR TUDO ---
                char conf;
                cout << "\nTem certeza que deseja APAGAR TODO O BANCO DE DADOS? (s/n): ";
                cin >> conf;
                if (conf == 's' || conf == 'S') {
                    catalogo.clear();
                    salvarDados(catalogo); // Salva vetor vazio (sobrescreve arquivo)
                    cout << ">> Banco de dados resetado com sucesso." << endl;
                } else {
                    cout << ">> Operacao cancelada." << endl;
                }
                break;
            }

            case 0:
                cout << "Saindo..." << endl;
                return 0;

            default:
                cout << "Opcao invalida!" << endl;
        }

    } while (opcao != 4); 

    // --- EXECUÇÃO FINAL ---
    if (catalogo.empty()) {
        cout << "\n[ERRO] Catalogo vazio! Adicione materias." << endl;
        return 1;
    }

    // Configurações padrão se o usuário pulou etapas
    if (config.getDias() <= 0) config.setDias(5);
    if (config.getTurnos() <= 0) config.setTurnos(2);
    if (config.getAulaTurno() <= 0) config.setAulaTurno(4);
    if (config.getMaxIteracao() <= 0) config.setMaxIteracoes(1000);

    try {
        cout << "\n>> Otimizando..." << endl;
        Otimizador hillClimbing(catalogo, config);
        hillClimbing.executar();
    } catch (const std::exception& e) {
        cerr << "\n[ERRO]: " << e.what() << endl;
    }

    return 0;
}
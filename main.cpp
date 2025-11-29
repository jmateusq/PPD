#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include "Materia.h"
#include "Otimizador.h"
#include "Configuracao.h" 

using namespace std;

int main() {
    srand((unsigned int)time(NULL));

    // Instancia as configurações padrão
    Contexto config; 
    
    // Cria o catálogo
    std::vector<Materia> catalogo;
    
    int opcao = 0;

    // Loop do Menu
    do {
        cout << "\n=============================================" << endl;
        cout << "      SISTEMA DE OTIMIZACAO DE ESTUDOS      " << endl;
        cout << "=============================================" << endl;
        cout << " 1 - Cadastrar uma materia " << endl;
        cout << " 2 - Configurar Dias, Turnos e Aulas" << endl;
        cout << " 3 - Configurar precisao (Iteracoes)" << endl;
        cout << " 4 - EXECUTAR OTIMIZACAO (Sair e Rodar)" << endl;
        cout << " 0 - Sair sem rodar" << endl;
        cout << "Escolha uma opcao: ";
        cin >> opcao;

        switch (opcao) {
            case 1: { 
                string nomeMat;
                int difMat;
                
                cout << "\n--- ADICIONAR NOVA MATERIA ---" << endl;
                cout << "Nome da materia: ";
                cin >> nomeMat;

                // 
                do {
                    cout << "Grau de dificuldade (0-3): ";
                    cin >> difMat;
                    if (difMat < 0 || difMat > 3) {
                        cout << "ERRO: Digite entre 0 e 3." << endl;
                    }
                } while (difMat < 0 || difMat > 3);

                int novoID = catalogo.size();
                
                // Assumindo que sua struct Materia aceita {id, nome, dificuldade}
                catalogo.push_back({novoID, nomeMat, difMat});

                cout << "Materia '" << nomeMat << "' adicionada com sucesso!" << endl;
                break;
            }

            case 2:
                cout << "\n--- CONFIGURACAO DE TEMPO ---" << endl;
                
                // Configurar DIAS
                cout << "Quantos dias voce pretende estudar? (1-7): ";
                cin >> config.dias;
                while (config.dias <= 0 || config.dias > 7) {
                    cout << "Invalido. Tente novamente: ";
                    cin >> config.dias;
                }

                // Configurar TURNOS
                cout << "Quantos turnos por dia? (1-6): ";
                cin >> config.turnos;
                while (config.turnos <= 0 || config.turnos > 6) {
                    cout << "Invalido. Tente novamente: ";
                    cin >> config.turnos;
                }

                // Configurar AULAS POR MATÉRIA (Se sua struct Contexto tiver esse campo, senão crie)
                // Assumindo que você adicione 'int aulasPorTurno' no Contexto ou use fixo
                // Por enquanto, apenas exibimos o total calculado pela struct
                cout << "\nCONFIGURACAO ATUALIZADA:" << endl;
                cout << "Dias: " << config.dias << " | Turnos: " << config.turnos << endl;
                cout << "Total de Slots (Aulas na semana): " << config.getTotalSlots() << endl;
                break;

            case 3:
                cout << "\n--- PRECISAO DO ALGORITMO ---" << endl;
                cout << "Atual: " << config.maxIteracoes << endl;
                cout << "Digite o novo numero de iteracoes: ";
                cin >> config.maxIteracoes;

                while (config.maxIteracoes <= 0) {
                    cout << "Valor invalido! Digite maior que 0: ";
                    cin >> config.maxIteracoes;
                }
                cout << "Configurado para rodar " << config.maxIteracoes << " vezes." << endl;
                break;

            case 4:
                cout << "Iniciando o otimizador..." << endl;
                break;
            
            case 0:
                cout << "Saindo..." << endl;
                return 0;

            default:
                cout << "Opcao invalida!" << endl;
        }

    } while (opcao != 4); // Sai do loop apenas se escolher rodar (4)

    // --- EXECUÇÃO ---
    // Verifica se tem matérias antes de rodar
    if (catalogo.empty()) {
        cout << "ERRO: O catalogo esta vazio! Adicione materias antes de rodar." << endl;
        return 1;
    }

    // Instancia o otimizador passando o Catálogo E as Configurações (Contexto)
    // Você precisa ajustar o construtor do Otimizador para aceitar (vector, Contexto)
    Otimizador hillClimbing(catalogo, config);
    
    hillClimbing.executar();

    return 0;
}
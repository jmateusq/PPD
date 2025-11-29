#include <iostream>
#include <vector>
#include <ctime>
#include "Materia.h"
#include "Otimizador.h"
using namespace std;

int DIAS = 5; // PADRÃO
int TURNOS = 6; // PADRÃO {(8H-10), (10H-12H), (14H-16H), (16H-18H), (19H-21H), (21H-23H)}
int AULAS_MAT = 2 // qdd de aulas por turno
int SLOTS_TOTAL = 0; // depende do usuário SLOTS_TOTAL = DIAS * TURNOS

int MAX_ITERACAO == 5000; // PADRÃO

int main() {
    int opcao;
    srand((unsigned int)time(NULL));

    cout << "Escolha uma opção do catálogo" << endl;
    cout << " 1 - Cadastrar uma matéria " << endl;
    cout << " 2 - Escolher quantos dias/turnos estudar e quantas aulas dessa matéria" << endl;
    cout << " 3 - Número de testes do algoritmo" << endl;
    cout << " 4 - Sair" << endl;

    switch (opcao){
      case 1:
        string nomeMat;
        int difMat;
        int novoID;

        // Adicionar matéria
        cout << "--- ADICIONAR NOVA MATÉRIA ---" << endl;
        cout << "--- DIGITE O NOME DA MATÉRIA ---" << endl;
        cin >> nomeMat;

        // Adicionar dificuldade
        cout << "--- DIGITE O GRAU DE DIFICULDADE (0-3) ---" << endl;
            if(difMat < 0 || difMat > 3){
                cin >> difMat;
            }

        // Geração de um ID automático -> tamanho do vetor
        novoID = catalogo.size();

        // Adiciona no vetor
        catalogo.push_back({novoID, nomeMat, difMat});

        cout << "MATERIA " << nomeMat << "ADICIONADA COM SUCESSO" << endl;
      break;
      case 2:
            cout << "Quantos dias você pretende estudar? (seg-sex): ";
                if(DIAS < 0 || DIAS > 5){
                    cin >> DIAS;
                }
            cout << "Quantos turnos você pretende estudar? (manhã:8h-12h/2 turnos) (terça:14h-18h/2 turnos) (noite:19h-23h/2 turnos): ";
                if (TURNOS < 0 || TURNOS > 6){
                    cin >> TURNOS;
                }
            cout << "Quantas aulas dessa matéria você quer ter na semana?: ";
                if (AULAS_MAT < 0 || AULAS_MAT > 6) {
                    cin >> AULAS_MAT;
                }
            
            // Atualiza os valores que foram definidos lá em cima.
            SLOTS_TOTAL = DIAS * TURNOS * AULAS_MAT;
            cout << "CONFIGURAÇÃO DEFINIDA: " << DIAS << " dias, " << TURNOS << " turnos.\n";
            cout << "TOTAL DE AULAS NA SEMANA: " << SLOTS_TOTAL << "\n\n";
      break;
      case 3:
            cout << "DEFINIR PRECISÃO DO ALGORITMO (ITERAÇÕES)" << endl;
            cout << "PADRAO: 5000. RECOMENDADO ENTRE 1000 E 5000." << endl;

            cout << "DIGITE O NÚMERO DE ITERAÇÕES: ";
            cin >> MAX_ITERACAO;

            while (MAX < 0){
                cout << "VALOR INVÁLIDO! DIGITE UM NÚMERO MAIOR QUE 0: ";
                cin >> MAX_ITERACAO;
            }

            cout << "CONFIGURADO PARA RODAR " << MAX_ITERACAO << " VEZES" << endl;
      break;
      case 4:
            return 0;
      break;
    }

    // Definição do Catálogo de Matérias
    // ID, Nome, Dificuldade (0=Livre, 1=Leve, 2=Médio, 3=Dificil)
    std::vector<Materia> catalogo;

    for(auto& mat : catalogo){

    }


    catalogo.push_back(Materia(0, "Livre", 0));
    catalogo.push_back(Materia(1, "Matemat", 3));
    catalogo.push_back(Materia(2, "Hist",    1));
    catalogo.push_back(Materia(3, "Fisica",  3));
    catalogo.push_back(Materia(4, "Ingles",  1));

    // Instancia o otimizador com o catálogo e nº de iterações
    Otimizador hillClimbing(catalogo, 1000);
    
    hillClimbing.executar();

    return 0;
}
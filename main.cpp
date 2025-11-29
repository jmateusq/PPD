#include <iostream>
#include <vector>
#include <ctime>
#include "Materia.h"
#include "Otimizador.h"
using namespace std;

int DIAS = 5; // PADRÃO
int TURNOS = 6; // PADRÃO {(8H-10), (10H-12H), (14H-16H), (16H-18H), (19H-21H), (21H-23H)}
int SLOTS_TOTAL = 0; // depende do usuário SLOTS_TOTAL = DIAS * TURNOS

int main() {
    int opcao;
    srand((unsigned int)time(NULL));

    cout << "Escolha uma opção do catálogo" << endl;
    cout << " 1 - Cadastrar uma matéria " << endl;
    cout << " 2 - Escolher quantos dias e quantos turnos estudar" << endl;
    cout << " 4 - Mostrar grade" << endl;
    cout << " 5 - Sair" << endl;

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
            int diasEst;
            int turnoEst;

            cout << "Quantos dias você pretende estudar? (seg-sex): ";
                if(diasEst < 0 || diasEst > 5){
                    cin >> diasEst;
                }
            cout << "Quantos turnos você pretende estudar? (manhã:8h-12h/2 turnos) (terça:14h-18h/2 turnos) (noite:19h-23h/2 turnos): ";
                if (turnoEst < 0 || turnoEst > 6){
                    cin >> turnoEst;
                }

      break;
      case 3:
      break;
      case 4:
      break;
      case 5:
      break;  
    }

    // Definição do Catálogo de Matérias
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
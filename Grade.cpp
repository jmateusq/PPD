#include "Grade.h"
#include "Configuracao.h"
#include <cstdlib> // rand
#include <algorithm> // swap

#define SLOTS_TOTAL 10
#define DIAS 5
#define TURNOS 2



Grade::Grade(const std::vector<Materia>& catalogoMaterias) {
    slots.reserve(SLOTS_TOTAL); // Otimização de memória

    // PASSO 1: Garantir pelo menos uma ocorrência de cada matéria
    // (Desde que haja espaço na grade)
    for (const auto& mat : catalogoMaterias) {
        if (slots.size() < SLOTS_TOTAL) {
            slots.push_back(mat);
        }
    }

    // PASSO 2: Preencher o restante dos slots aleatoriamente
    while (slots.size() < SLOTS_TOTAL) {
        unsigned long indiceAleatorio = (unsigned long)rand() % catalogoMaterias.size();
        slots.push_back(catalogoMaterias[indiceAleatorio]);
    }

    // PASSO 3: Embaralhar a grade inicial
    // Como inserimos em ordem, precisamos misturar para gerar aleatoriedade real
    for (size_t i = 0; i < slots.size(); i++) {
        size_t j = (unsigned long)rand() % slots.size();
        std::swap(slots[i], slots[j]);
    }

    atualizarPontuacao();
}

Grade::Grade(const Grade& outra) {
    this->slots = outra.slots;
    this->pontuacao = outra.pontuacao;
}

int Grade::getPontuacao() const {
    return pontuacao;
}

void Grade::atualizarPontuacao() {
    int score = 0;

    for (unsigned long dia = 0; dia < DIAS; dia++) {
        for (unsigned long turno = 0; turno < TURNOS; turno++) {
            unsigned long index = dia * TURNOS + turno;
            Materia atual = slots[index];

            // REGRA 1: Premia matérias difíceis (Dif=3) de manhã (Turno 0)
            if (turno == 0) {
                if (atual.getDificuldade() == 3) score += 20;
                else if (atual.getId() == 0) score -= 10; // ID 0 é Livre
            }

            // REGRA 2: Penaliza matérias difíceis à noite (Turno 2)
            if (turno == 2) { 
                if (atual.getDificuldade() == 3) score -= 20;
                else if (atual.getId() == 0) score += 10; // Descanso à noite
            }

            // REGRA 3: Penaliza repetição no mesmo dia
            if (turno > 0) {
                if (slots[index] == slots[index - 1] && atual.getId() != 0) {
                    score -= 50; 
                }
            }
        }
    }
    this->pontuacao = score;
}

Grade Grade::gerarVizinho() const {
    Grade vizinho(*this); // Cria cópia
    
    unsigned long i = (unsigned long)rand() % SLOTS_TOTAL;
    unsigned long j = (unsigned long)rand() % SLOTS_TOTAL;

    std::swap(vizinho.slots[i], vizinho.slots[j]);
    
    vizinho.atualizarPontuacao(); // Recalcula score do vizinho
    return vizinho;
}

Grade& Grade::operator=(const Grade& other) {
    // 1. Verificação de auto-atribuição (ex: a = a)
    if (this != &other) {
        // 2. Copia os dados
        this->slots = other.slots;
        this->pontuacao = other.pontuacao;
    }
    // 3. Retorna a referência do próprio objeto
    return *this;
}

void Grade::imprimir() const {
    std::cout << "---------------------------------------------------\n";
    std::cout << "      | Seg    | Ter    | Qua    | Qui    | Sex    |\n";
    std::cout << "---------------------------------------------------\n";
    
    const std::string turnosNome[] = {"Manha", "Tarde", "Noite"};
    
    for (unsigned long t = 0; t < TURNOS; t++) {
        std::cout << turnosNome[t] << " | ";
        for (unsigned long d = 0; d < DIAS; d++) {
            unsigned long index = d * TURNOS + t;
            // Formatação simples para manter alinhamento
            std::string nome = slots[index].getNome();
            while(nome.length() < 7) nome += " "; 
            std::cout << nome << "| ";
        }
        std::cout << "\n";
    }
    std::cout << "---------------------------------------------------\n";
    std::cout << "PONTUACAO TOTAL: " << pontuacao << "\n\n";
}
#include "Configuracao.h"
#include <stdexcept> // Necessário para std::invalid_argument
#include <string>    // Para mensagens de texto

// Construtor Padrão
Configuracao::Configuracao() : dias(0), turnos(0), maxIteracoes(0), aulasPorTurno(0) {}

// Construtor com parâmetros (reutiliza os setters para garantir a validação também na construção)
Configuracao::Configuracao(int Dias, int Turnos, int MaxIteracoes, int AulasTurno) {
    setDias(Dias);
    setTurnos(Turnos);
    setMaxIteracoes(MaxIteracoes);
    setAulaTurno(AulasTurno);
}

// --- SETTERS COM VALIDAÇÃO ---

void Configuracao::setDias(int Dias) {
    // Exemplo: Dias deve ser entre 1 (segunda) e 7 (domingo)
    if (Dias <= 0 || Dias > 7) {
        throw std::invalid_argument("Erro: Dias deve ser um valor entre 1 e 7.");
    }
    this->dias = Dias;
}

void Configuracao::setTurnos(int Turnos) {
    // Exemplo: Pelo menos 1 turno, no máximo 3 (Manhã, Tarde, Noite)
    if (Turnos <= 0 || Turnos > 3) {
        throw std::invalid_argument("Erro: Turnos deve ser entre 1 e 3.");
    }
    this->turnos = Turnos;
}

void Configuracao::setMaxIteracoes(int MaxIteracoes) {
    // Exemplo: Deve haver pelo menos 1 iteração
    if (MaxIteracoes <= 0) {
        throw std::invalid_argument("Erro: MaxIteracoes deve ser maior que 0.");
    }
    this->maxIteracoes = MaxIteracoes;
}

void Configuracao::setAulaTurno(int AulasTurno) {
    // Exemplo: Pelo menos 1 aula, e vamos supor um limite de 10 aulas seguidas
    if (AulasTurno <= 0 || AulasTurno > 10) {
        throw std::invalid_argument("Erro: Aulas por turno deve ser entre 1 e 10.");
    }
    this->aulasPorTurno = AulasTurno;
}

// --- GETTERS ---
// (Implementação padrão dos getters...)
int Configuracao::getDias() const { return dias; }
int Configuracao::getTurnos() const { return turnos; }
int Configuracao::getMaxIteracao() const { return maxIteracoes; }
int Configuracao::getAulaTurno() const { return aulasPorTurno; }

int Configuracao::getTotalSlots() const {
    return dias * turnos * aulasPorTurno;
}
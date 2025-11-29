#include "Configuracao.h"

Contexto::Contexto() : dias(5), turnos(6), maxIteracoes(5000), aulasPorTurno(2) {}

Contexto::Contexto(int Dias, int Turnos, int MaxInteracao, int AulasTurno)
    : dias(Dias), turnos(Turnos), maxIteracoes(MaxInteracao), aulasPorTurno(AulasTurno)    {}

int Contexto::getDias() const {
    return this->dias;
}
    
int Contexto::getTurnos() const {
    return this->turnos;
}

int Contexto::getMaxIteracao() const {
    return this->maxIteracoes;
}

int Contexto::getAulaTurno() const{
    return this->aulasPorTurno;
}

int Contexto::getTotalSlots() const {
    return dias * turnos * aulasPorTurno; 
}
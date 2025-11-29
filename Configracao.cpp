#include "Configuracao.h"

Contexto:Contexto() : dias(5), turnos(6), maxIteracao(5000), aulasTurno(2) {}

Contexto::Contexto(int dias, int turnos, int maxInteracao, int aulasTurno)
    : dias(), turnos(), maxInteracao(), aulasTurno()    {}

int getDias () const {
        return this->dias;
    }
    int getTurnos () const {
        return this->turnos;
    }

    int getMaxIteracao () const {
        return this->maxIteracoes;
    }

    int getAulaTurno() const{
        return this->aulasPorTurno;
    }
    int getTotalSlots() const {
        return dias * turnos * aulasPorTurno; 
    }
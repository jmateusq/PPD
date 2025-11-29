#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

class Contexto {
    private:
    int dias = 5;
    int turnos = 3;
    int maxIteracoes = 5000; 
    int aulasPorTurno = 2; 

    // Função auxiliar para calcular quantidade de slots
    public(int dias, int turnos, int maxIteracoes, int aulasPorTurno);
        : dias(), turnos(), maxIteracoes(), aulasPorTurno() {}
    
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
};

#endif
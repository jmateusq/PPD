#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

struct Contexto {
    int dias = 5;
    int turnos = 3;
    int maxIteracoes = 5000; 
    int aulasPorTurno = 2; 

    // Função auxiliar para calcular quantidade de slots
    int getTotalSlots() const {
        return dias * turnos * aulasPorTurno; 
    }
};

#endif
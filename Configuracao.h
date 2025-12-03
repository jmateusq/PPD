#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <stdexcept> // Necessário para as exceções, se for implementar no .h, senão apenas no .cpp

class Configuracao {
private:
    int dias;
    int turnos;
    int maxIteracoes; 
    int aulasPorTurno; 

public:
    Configuracao();
    Configuracao(int dias, int turnos, int maxIteracoes, int aulasTurno);
    
    // Getters
    int getDias() const;
    int getTurnos() const;
    int getMaxIteracao() const;
    int getAulaTurno() const;
    int getTotalSlots() const;

    // Setters com validação (PÚBLICOS)
    void setDias(int dias);
    void setTurnos(int turnos);
    void setMaxIteracoes(int maxIteracoes);
    void setAulaTurno(int aulasTurno);
};

#endif
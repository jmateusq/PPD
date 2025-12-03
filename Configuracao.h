#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <stdexcept> // Necessário para as exceções, se for implementar no .h, senão apenas no .cpp

class Configuracao {
private:
    int dias;
    int turnos;
    unsigned long int maxIteracoes; 
    int aulasPorTurno; 
    unsigned long int tentativas;

public:
    Configuracao();
    Configuracao(int dias, int turnos, unsigned long int maxIteracoes, int aulasTurno);
    
    // Getters
    int getDias() const;
    int getTurnos() const;
    unsigned long int getMaxIteracao() const;
    int getAulaTurno() const;
    int getTotalSlots() const;
    unsigned long int getTentativas() const;

    // Setters com validação (PÚBLICOS)
    void setDias(int dias);
    void setTurnos(int turnos);
    void setMaxIteracoes(unsigned long int maxIteracoes);
    void setAulaTurno(int aulasTurno);
    void setTentativas(unsigned long int t);
};
#endif
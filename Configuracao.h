#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

class Contexto {
private:
    int dias;
    int turnos;
    int maxIteracoes; 
    int aulasPorTurno; 

    // Função auxiliar para calcular quantidade de slots
public:
    Contexto();
    Contexto(int dias, int turnos, int maxIteracoes, int aulasTurno);
    
    int getDias () const;
    int getTurnos () const;

    int getMaxIteracao () const;

    int getAulaTurno() const;

    int getTotalSlots() const;

};

#endif
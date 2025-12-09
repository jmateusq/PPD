#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <stdexcept>

class Configuracao {
private:
    int numDias;             // Quantos dias a escala cobre (horizonte de planejamento)
    int voosPorDia;          // Máximo de slots de voo por dia (normalmente 1 a 4)
    unsigned long int maxIteracoes; 
    unsigned long int tentativas;
    bool usarGPU;  

public:
    Configuracao();
    Configuracao(int dias, int voosDia, unsigned long int maxIter, unsigned long int tent);
    
    // Getters
    int getDias() const;
    int getVoosPorDia() const;
    unsigned long int getMaxIteracao() const;
    unsigned long int getTentativas() const;
    bool getUsarGPU() const;
    
    // Helper para saber o tamanho total do vetor da escala
    int getTotalSlots() const;

    
        

    // Setters com validação
    void setDias(int dias);
    void setVoosPorDia(int vpd);
    void setMaxIteracoes(unsigned long int maxIteracoes);
    void setTentativas(unsigned long int t);
    void setUsarGPU(bool usar); 
};

#endif
#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

#include <stdexcept>

// Enum para facilitar a leitura do modo de execução
enum ModoExecucao {
    CPU_SEQUENCIAL = 0,
    CPU_OPENMP = 1,
    GPU_CUDA = 2,
    CPU_MPI = 3
};

class Configuracao {
private:
    int numDias;             
    int voosPorDia;          
    unsigned long int maxIteracoes; 
    unsigned long int tentativas;
    ModoExecucao modo; // Novo campo para controlar os 3 modos

public:
    Configuracao();
    Configuracao(int dias, int voosDia, unsigned long int maxIter, unsigned long int tent);
    
    // Getters
    int getDias() const;
    int getVoosPorDia() const;
    unsigned long int getMaxIteracao() const;
    unsigned long int getTentativas() const;
    ModoExecucao getModoExecucao() const;
    
    int getTotalSlots() const;

    // Setters
    void setDias(int dias);
    void setVoosPorDia(int vpd);
    void setMaxIteracoes(unsigned long int maxIteracoes);
    void setTentativas(unsigned long int t);
    void setModoExecucao(int m); // 0, 1 ou 2
};

#endif
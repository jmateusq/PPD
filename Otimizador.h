#ifndef OTIMIZADOR_H
#define OTIMIZADOR_H

#include "Voo.h"
#include "Escala.h"
#include "Configuracao.h"
#include <omp.h> 
#include <vector>
#include <random> // <--- OBRIGATÓRIO: Adicionado para std::mt19937

class Otimizador {
private:
    std::vector<Voo> catalogo;
    Configuracao config;

    // --- ATUALIZADO: Agora recebe o RNG ---
    // Isso permite que cada thread tenha seu próprio gerador sem conflito
    Escala rodarHillClimbingLocal(std::mt19937& rng);

public:
    Otimizador(std::vector<Voo> &catalogo, Configuracao &config);
    
    void executar();
};

#endif
#ifndef OTIMIZADOR_H
#define OTIMIZADOR_H

#include "Voo.h"
#include "Escala.h"
#include "Configuracao.h"
#include <vector>

class Otimizador {
private:
    std::vector<Voo> catalogo;
    Configuracao config;

    // Método auxiliar que roda UMA tentativa completa de otimização (Hill Climbing)
    // Este método será executado em paralelo por várias threads no futuro
    Escala rodarHillClimbingLocal(); 

public:
    Otimizador(std::vector<Voo> &catalogo, Configuracao &config);
    
    // Método principal que gerencia as tentativas (Random Restarts)
    void executar();
};

#endif
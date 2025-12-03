#ifndef OTIMIZADOR_H
#define OTIMIZADOR_H

#include "Grade.h"
#include "Configuracao.h"
#include <vector>

class Otimizador {
private:
    std::vector<Materia> catalogo;
    Configuracao config; // Armazena uma cópia ou referência da config

    // Método auxiliar privado
    Grade rodarHillClimbingLocal(); 

public:
    Otimizador(std::vector<Materia> &catalogo, Configuracao &config);
    
    void executar();
};

#endif
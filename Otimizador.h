#ifndef OTIMIZADOR_H
#define OTIMIZADOR_H

#include "Grade.h"
#include "Materia.h"
#include "Configuracao.h"
#include <vector>

class Otimizador {
private:
    std::vector<Materia> catalogo;
    Configuracao configuracao;

public:
    Otimizador(std::vector<Materia> &catalogo, Configuracao &configuracao);
    
    // Método principal que roda o algoritmo
    void executar();
};

#endif
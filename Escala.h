#ifndef ESCALA_H
#define ESCALA_H

#include <vector>
#include <iostream>
#include "Voo.h"
#include "Configuracao.h"

class Escala {
private:
    std::vector<Voo> slots; // Vetor linear: Dia 1 (voo 1, voo 2...), Dia 2...
    long long int pontuacao; // long long pois a penalidade pode ser alta
    Configuracao config;

    // Calcula se a escala faz sentido físico e logístico [!!!]
    void atualizarPontuacao();

public:
    // Construtor: Gera uma escala inicial (pode ser aleatória ou semi-gulosa) [!!!]
    Escala(const std::vector<Voo>& catalogoVoos, Configuracao Config);
    
    // Construtor de Cópia
    Escala(const Escala& outra);

    long long int getPontuacao() const;
    
    // Gera uma nova escala modificando levemente a atual
    Escala gerarVizinho(const std::vector<Voo>& catalogoVoos) const;

    Escala& operator=(const Escala& other); 

    // Imprime a escala formatada
    void imprimir() const;

    // Carrega a escala diretamente de um vetor de índices (usado para reconstruir após GPU)
    void carregarDeIndices(const std::vector<int>& indices, const std::vector<Voo>& catalogo);
};

#endif

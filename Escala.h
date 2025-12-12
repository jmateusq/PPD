#ifndef ESCALA_H
#define ESCALA_H

#include <vector>
#include <iostream>
#include <random> // <--- OBRIGATÓRIO: Adicionado para std::mt19937
#include "Voo.h"
#include "Configuracao.h"

class Escala {
private:
    std::vector<Voo> slots; 
    long long int pontuacao; 
    Configuracao config;

    void atualizarPontuacao();

public:
    // --- ATUALIZADO: Agora recebe o RNG ---
    Escala(const std::vector<Voo>& catalogoVoos, Configuracao Config, std::mt19937& rng);
    
    // Construtor de Cópia
    Escala(const Escala& outra);

    long long int getPontuacao() const;
    
    // --- ATUALIZADO: Agora recebe o RNG ---
    Escala gerarVizinho(const std::vector<Voo>& catalogoVoos, std::mt19937& rng) const;

    Escala& operator=(const Escala& other); 

    void imprimir() const;

    // Carrega a escala diretamente de um vetor de índices (usado pela GPU)
    void carregarDeIndices(const std::vector<int>& indices, const std::vector<Voo>& catalogo);

    std::vector<int> exportarIndices() const;
};

#endif
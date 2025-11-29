#include "Otimizador.h"
#include <iomanip>

Otimizador::Otimizador(std::vector<Materia> Catalogo, int MaxIteracoes) 
    : catalogo(Catalogo), maxIteracoes(MaxIteracoes) {}

void Otimizador::executar() {
    // 1. Gera estado inicial
    Grade atual(catalogo);
    
    std::cout << "=== ESTADO INICIAL (ALEATORIO) ===\n";
    atual.imprimir();

    std::cout << "Iniciando otimizacao...\n";
    std::cout << "Iteracao | Score Atual\n";

    int iteracoes = 0;

    // Loop Hill Climbing
    while (iteracoes < maxIteracoes) {
        // 2. Gera vizinho
        Grade vizinho = atual.gerarVizinho();

        // 3. Verifica se melhorou
        if (vizinho.getPontuacao() > atual.getPontuacao()) {
            atual = vizinho;
            std::cout << std::setw(8) << iteracoes << " | " << atual.getPontuacao() << " (Subindo...)\n";
        }
        
        iteracoes++;
    }

    std::cout << "\n=== ESTADO FINAL (OTIMIZADO) ===\n";
    atual.imprimir();
}
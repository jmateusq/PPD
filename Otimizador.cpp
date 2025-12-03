// Otimizador.cpp
#include "Otimizador.h"
#include <vector>
#include <iostream>
#include <algorithm> // para std::max_element

using namespace std;

Otimizador::Otimizador(std::vector<Materia> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {
}

// Função auxiliar interna: Roda UMA VEZ o Hill Climbing e retorna o Máximo Local encontrado
Grade Otimizador::rodarHillClimbingLocal() {
    // 1. Gera uma solução inicial aleatória
    Grade atual(catalogo, config); 
    
    unsigned long int maxIter = config.getMaxIteracao();

    // 2. Loop de melhoria (Escalada)
    for (unsigned long int i = 0; i < maxIter; i++) {
        Grade vizinho = atual.gerarVizinho();

        if (vizinho.getPontuacao() > atual.getPontuacao()) {
            atual = vizinho; // Aceita a melhora
        }
        // Se for igual ou pior, mantém a atual (Hill Climbing padrão)
    }

    return atual; // Retorna o "Máximo Local" desta tentativa
}

void Otimizador::executar() {
    unsigned long int nTentativas = config.getTentativas(); // Ex: 1000 vezes
    std::vector<Grade> listaMaximosLocais;
    
    // Reserva memória para evitar realocações
    listaMaximosLocais.reserve(nTentativas);

    cout << "Iniciando Otimizacao com Random Restarts..." << endl;
    cout << "Tentativas (Restarts): " << nTentativas << endl;
    cout << "Iteracoes por tentativa: " << config.getMaxIteracao() << endl;
    cout << "----------------------------------------------------" << endl;

    // --- FASE 1: Coletar Máximos Locais ---
    for (unsigned long int t = 0; t < nTentativas; t++) {
        // Roda o algoritmo completo uma vez
        Grade melhorDestaRodada = rodarHillClimbingLocal();
        
        // Adiciona na lista
        listaMaximosLocais.push_back(melhorDestaRodada);

        // Feedback visual de progresso (a cada 10% ou a cada 1 se for pouco)
        if (nTentativas < 20 || (t+1) % (nTentativas/10) == 0) {
            cout << ">> Tentativa " << (t+1) << "/" << nTentativas 
                 << " concluida. Melhor Score local: " << melhorDestaRodada.getPontuacao() << endl;
        }
    }

    // --- FASE 2: Comparar a lista e pegar o Grid com maior pontuação ---
    cout << "----------------------------------------------------" << endl;
    cout << "Comparando " << listaMaximosLocais.size() << " resultados..." << endl;

    if (listaMaximosLocais.empty()) return;

    // Encontra o iterador para o elemento com maior pontuação
    // Usamos uma lambda function para comparar
    auto melhorGlobalIt = std::max_element(
        listaMaximosLocais.begin(), 
        listaMaximosLocais.end(),
        [](const Grade& a, const Grade& b) {
            return a.getPontuacao() < b.getPontuacao();
        }
    );

    // --- FASE 3: Exibir o Resultado Final ---
    cout << "MELHOR RESULTADO ENCONTRADO (GLOBAL):" << endl;
    melhorGlobalIt->imprimir(); // Imprime a grade vencedora
}
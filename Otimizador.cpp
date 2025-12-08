#include "Otimizador.h"
#include <vector>
#include <iostream>
#include <algorithm> // para std::max_element

using namespace std;

Otimizador::Otimizador(std::vector<Voo> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {
}

// Roda UMA VEZ o Hill Climbing e retorna o Máximo Local encontrado
// (Essa função é thread-safe se o rand() for tratado corretamente ou substituído por gerador local)
Escala Otimizador::rodarHillClimbingLocal() {
    // 1. Gera uma solução inicial (Geralmente ruim/aleatória)
    Escala atual(catalogo, config); 
    
    unsigned long int maxIter = config.getMaxIteracao();

    // 2. Loop de melhoria (Escalada da Encosta)
    for (unsigned long int i = 0; i < maxIter; i++) {
        
        // Gera uma pequena mutação na escala atual
        Escala vizinho = atual.gerarVizinho(catalogo);

        // Se o vizinho for melhor (pontuação maior), aceitamos a mudança.
        // Como nossas penalidades são negativas, "maior" significa "mais perto de zero ou positivo".
        if (vizinho.getPontuacao() > atual.getPontuacao()) {
            atual = vizinho; 
        }
        // Se for pior ou igual, mantemos a 'atual' e tentamos outro vizinho na próxima iteração
    }

    return atual; // Retorna a melhor escala encontrada nesta tentativa
}

void Otimizador::executar() {
    unsigned long int nTentativas = config.getTentativas(); // Ex: 1000 restarts
    
    // Armazenará a melhor escala encontrada entre TODAS as tentativas
    // Inicializamos com uma escala vazia/padrão
    Escala melhorGlobal(catalogo, config);
    bool primeiroUpdate = true;

    cout << ">> Iniciando Otimizacao de Escala (Airline Crew Rostering)..." << endl;
    cout << ">> Restarts: " << nTentativas << " | Iteracoes/Restart: " << config.getMaxIteracao() << endl;
    cout << "----------------------------------------------------" << endl;

    // ==============================================================================
    // PONTO DE PARALELISMO (OpenMP vai aqui depois)
    // Cada iteração 't' é independente das outras.
    // ==============================================================================
    for (unsigned long int t = 0; t < nTentativas; t++) {
        
        // Executa a busca local (Processo pesado)
        Escala melhorLocal = rodarHillClimbingLocal();
        
        // Verifica se essa tentativa encontrou um resultado melhor que o recorde atual
        long long int scoreLocal = melhorLocal.getPontuacao();

        // (Futuramente: Critical Section aqui)
        if (primeiroUpdate || scoreLocal > melhorGlobal.getPontuacao()) {
            melhorGlobal = melhorLocal;
            primeiroUpdate = false;
            
            // Log de progresso (Opcional: reduzir frequência se for muito rápido)
            cout << "[Tentativa " << (t+1) << "] Novo Recorde: " << scoreLocal << endl;
        }
    }

    // --- Exibir o Resultado Final ---
    cout << "----------------------------------------------------" << endl;
    cout << "MELHOR RESULTADO ENCONTRADO (GLOBAL):" << endl;
    melhorGlobal.imprimir(); 
}

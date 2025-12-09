#include "Otimizador.h"
#include "KernelOtimizador.cuh" // Interface CUDA
#include <iostream>
#include <algorithm> // Necessário para max_element se usar vetores, ou lógica manual

using namespace std;

Otimizador::Otimizador(std::vector<Voo> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {}

// --- MÉTODO RESTAURADO DA VERSÃO SEQUENCIAL ---
Escala Otimizador::rodarHillClimbingLocal() {
    Escala atual(catalogo, config); 
    unsigned long int maxIter = config.getMaxIteracao();

    for (unsigned long int i = 0; i < maxIter; i++) {
        Escala vizinho = atual.gerarVizinho(catalogo);
        if (vizinho.getPontuacao() > atual.getPontuacao()) {
            atual = vizinho; 
        }
    }
    return atual;
}

void Otimizador::executar() {
    
    // === LÓGICA DE DECISÃO ===
    if (config.getUsarGPU()) {
        
        // --- MODO GPU (O que já existia no PPD-master) ---
        cout << "----------------------------------------------------" << endl;
        cout << ">> MODO EXCLUSIVO GPU (CUDA) INICIADO" << endl;
        cout << "----------------------------------------------------" << endl;

        std::vector<int> indicesVencedores;
        
        long long int scoreFinalGPU = rodarOtimizacaoCUDA(
            catalogo, 
            config.getTentativas(), 
            config.getTotalSlots(), 
            config.getMaxIteracao(), 
            indicesVencedores,
            config.getVoosPorDia()
        );

        cout << ">> GPU Finalizada. Melhor Score Estimado: " << scoreFinalGPU << endl;

        // Reconstrói o objeto para impressão
        Escala escalaFinal(catalogo, config); 
        escalaFinal.carregarDeIndices(indicesVencedores, catalogo);
        
        cout << "----------------------------------------------------" << endl;
        cout << "MELHOR RESULTADO (GPU):" << endl;
        escalaFinal.imprimir();
        // Validação do resultado pela CPU
        long long int scoreCPU = escalaFinal.getPontuacao();
    
        cout << ">> Score Validado pela CPU: " << scoreCPU << endl;

    } else {

        // --- MODO CPU (Restaurado da versão sequencial) ---
        cout << "----------------------------------------------------" << endl;
        cout << ">> MODO SEQUENCIAL CPU INICIADO" << endl;
        cout << "----------------------------------------------------" << endl;

        unsigned long int nTentativas = config.getTentativas();
        Escala melhorGlobal(catalogo, config);
        bool primeiroUpdate = true;

        for (unsigned long int t = 0; t < nTentativas; t++) {
            
            Escala melhorLocal = rodarHillClimbingLocal();
            long long int scoreLocal = melhorLocal.getPontuacao();

            if (primeiroUpdate || scoreLocal > melhorGlobal.getPontuacao()) {
                melhorGlobal = melhorLocal;
                primeiroUpdate = false;
                cout << "[CPU] Tentativa " << (t+1) << "/" << nTentativas << " | Novo Recorde: " << scoreLocal << endl;
            }
        }

        cout << "----------------------------------------------------" << endl;
        cout << "MELHOR RESULTADO (CPU):" << endl;
        melhorGlobal.imprimir();
    }
}
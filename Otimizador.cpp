#include "Otimizador.h"
#include "KernelOtimizador.cuh"
#include <iostream>

using namespace std;

Otimizador::Otimizador(std::vector<Voo> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {}

// AVISO: rodarHillClimbingLocal foi removido pois a lógica agora é interna da GPU

void Otimizador::executar() {
    cout << "----------------------------------------------------" << endl;
    cout << ">> MODO EXCLUSIVO GPU (CUDA) INICIADO" << endl;
    cout << "----------------------------------------------------" << endl;

    std::vector<int> indicesVencedores;
    
    // CHAMADA PARALELA
    // Todas as tentativas rodam simultaneamente na placa de vídeo
    long long int scoreFinal = rodarOtimizacaoCUDA(
        catalogo, 
        config.getTentativas(), 
        config.getTotalSlots(), 
        config.getMaxIteracao(), 
        indicesVencedores
    );

    cout << ">> GPU Finalizada. Melhor Score Global: " << scoreFinal << endl;

    // RECONSTRUÇÃO DO OBJETO PARA IMPRESSÃO
    // Criamos uma escala vazia e forçamos os slots vencedores nela
    Escala escalaFinal(catalogo, config); // (Cria random, mas vamos sobrescrever)
    
    // Método auxiliar que você precisará adicionar em Escala.h / Escala.cpp
    // para injetar os dados puros vindos da GPU
    escalaFinal.carregarDeIndices(indicesVencedores, catalogo);

    cout << "----------------------------------------------------" << endl;
    cout << "MELHOR RESULTADO ENCONTRADO:" << endl;
    escalaFinal.imprimir();
}
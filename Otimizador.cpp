#include "Otimizador.h"
#include "KernelOtimizador.cuh" // Interface CUDA
#include <iostream>

using namespace std;

Otimizador::Otimizador(std::vector<Voo> &Catalogo, Configuracao &Config) 
    : catalogo(Catalogo), config(Config) {}

// AVISO: A função 'rodarHillClimbingLocal' antiga foi removida 
// pois agora delegamos tudo para 'rodarOtimizacaoCUDA'.

void Otimizador::executar() {
    cout << "----------------------------------------------------" << endl;
    cout << ">> MODO EXCLUSIVO GPU (CUDA) INICIADO" << endl;
    cout << "----------------------------------------------------" << endl;

    std::vector<int> indicesVencedores;
    
    // CHAMADA AO KERNEL (WRAPPER)
    long long int scoreFinalGPU = rodarOtimizacaoCUDA(
        catalogo, 
        config.getTentativas(), 
        config.getTotalSlots(), 
        config.getMaxIteracao(), 
        indicesVencedores,
        config.getVoosPorDia() // IMPORTANTE: Passar este parâmetro!
    );

    cout << ">> GPU Finalizada. Melhor Score Estimado: " << scoreFinalGPU << endl;

    // RECONSTRUÇÃO DO OBJETO NA CPU
    // Criamos uma escala "vazia" e injetamos os índices que a GPU escolheu
    Escala escalaFinal(catalogo, config); 
    escalaFinal.carregarDeIndices(indicesVencedores, catalogo);

    // Recalcula o score na CPU para ter certeza absoluta (validação cruzada)
    // O construtor/carregarDeIndices já chama atualizarPontuacao(), 
    // mas é bom conferir se bate com o da GPU.
    long long int scoreCPU = escalaFinal.getPontuacao();
    
    cout << ">> Score Validado pela CPU: " << scoreCPU << endl;

    cout << "----------------------------------------------------" << endl;
    cout << "MELHOR RESULTADO ENCONTRADO:" << endl;
    escalaFinal.imprimir();
}
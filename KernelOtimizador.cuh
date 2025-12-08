#ifndef OTIMIZADOR_KERNEL_CUH
#define OTIMIZADOR_KERNEL_CUH

#include <vector>
#include "Voo.h"

// Estrutura leve para passar dados para a GPU
struct VooGPU {
    int id, origem, destino, duracao;
};

// Função Wrapper que será chamada pelo C++
// Retorna: O score vencedor e preenche o vetor 'melhorEscalaIndices'
long long int rodarOtimizacaoCUDA(
    const std::vector<Voo>& catalogoHost,
    int numTentativas,
    int numSlots,
    int maxIter,
    std::vector<int>& melhorEscalaIndices
);

#endif
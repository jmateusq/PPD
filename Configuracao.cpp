#include "Configuracao.h"
#include <stdexcept>

Configuracao::Configuracao() : numDias(0), voosPorDia(0), maxIteracoes(0), tentativas(1), modo(CPU_SEQUENCIAL) {}

Configuracao::Configuracao(int Dias, int VoosDia, unsigned long int MaxIter, unsigned long int Tent) {
    setDias(Dias);
    setVoosPorDia(VoosDia);
    setMaxIteracoes(MaxIter);
    setTentativas(Tent);
    this->modo = CPU_SEQUENCIAL;
}

void Configuracao::setDias(int Dias) {
    if (Dias <= 0) throw std::invalid_argument("Dias deve ser > 0");
    this->numDias = Dias;
}

void Configuracao::setVoosPorDia(int vpd) {
    if (vpd <= 0) throw std::invalid_argument("Voos por dia deve ser > 0");
    this->voosPorDia = vpd;
}

void Configuracao::setMaxIteracoes(unsigned long int MaxIteracoes) {
    if (MaxIteracoes == 0) throw std::invalid_argument("MaxIteracoes deve ser > 0");
    this->maxIteracoes = MaxIteracoes;
}

void Configuracao::setTentativas(unsigned long int t) { 
    if(t < 1) throw std::invalid_argument("Tentativas deve ser >= 1");
    this->tentativas = t; 
}

void Configuracao::setModoExecucao(int m) {
    if (m < 0 || m > 2) throw std::invalid_argument("Modo invalido (0=Seq, 1=OpenMP, 2=CUDA)");
    this->modo = static_cast<ModoExecucao>(m);
}

int Configuracao::getDias() const { return numDias; }
int Configuracao::getVoosPorDia() const { return voosPorDia; }
unsigned long int Configuracao::getMaxIteracao() const { return maxIteracoes; }
unsigned long int Configuracao::getTentativas() const { return tentativas; }
ModoExecucao Configuracao::getModoExecucao() const { return modo; }

int Configuracao::getTotalSlots() const {
    return numDias * voosPorDia;
}
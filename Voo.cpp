#include "Voo.h"

// Construtor padrão: Cria um "buraco" na escala (Folga ou espera)
// Definimos origem/destino como -1 para indicar que não é voo real
Voo::Voo() : id(-1), origem(-1), destino(-1), duracao(0), custo(0) {}

Voo::Voo(int Id, int Origem, int Destino, int Duracao) 
    : id(Id), origem(Origem), destino(Destino), duracao(Duracao), custo(0) {}

int Voo::getId() const { return id; }
int Voo::getOrigem() const { return origem; }
int Voo::getDestino() const { return destino; }
int Voo::getDuracao() const { return duracao; }

bool Voo::operator==(const Voo& other) const {
    return this->id == other.id;
}

bool Voo::operator!=(const Voo& other) const {
    return !(*this == other);
}

std::string Voo::getNomeAeroporto(int id) {
    switch(id) {
        // Sudeste
        case 0: return "GRU (Guarulhos)";
        case 1: return "CGH (Congonhas)";
        case 2: return "VCP (Viracopos)";
        case 3: return "GIG (Galeao)";
        case 4: return "SDU (Santos Dumont)";
        case 5: return "CNF (Confins)";
        case 6: return "VIX (Vitoria)";
        // Sul
        case 7: return "CWB (Curitiba)";
        case 8: return "POA (Porto Alegre)";
        case 9: return "FLN (Florianopolis)";
        case 10: return "NVT (Navegantes)";
        // Centro-Oeste
        case 11: return "BSB (Brasilia)";
        case 12: return "GYN (Goiania)";
        case 13: return "CGB (Cuiaba)";
        case 14: return "CGR (Campo Grande)";
        // Nordeste
        case 15: return "SSA (Salvador)";
        case 16: return "REC (Recife)";
        case 17: return "FOR (Fortaleza)";
        case 18: return "NAT (Natal)";
        case 19: return "MCZ (Maceio)";
        case 20: return "SLZ (Sao Luis)";
        // Norte
        case 21: return "MAO (Manaus)";
        case 22: return "BEL (Belem)";
        case 23: return "PMW (Palmas)";
        case 24: return "PVH (Porto Velho)";
        default: return "Aero_" + std::to_string(id);
    }
}

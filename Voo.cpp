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
        case 0: return "GRU";
        case 1: return "MIA";
        case 2: return "JFK";
        case 3: return "LHR";
        case 4: return "BSB";
        case 5: return "SCL";
        case 6: return "CDG";
        case 7: return "DXB";
        default: return "Aero_" + std::to_string(id);
    }
}

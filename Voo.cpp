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

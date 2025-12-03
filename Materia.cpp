#include "Materia.h"

Materia::Materia() : id(0), nome("Livre"), dificuldade(0) {}

Materia::Materia(unsigned long int Id, std::string Nome, int Dificuldade) 
    : id(Id), nome(Nome), dificuldade(Dificuldade) {}

unsigned long int Materia::getId() const {
    return id;
}

std::string Materia::getNome() const {
    return nome;
}

int Materia::getDificuldade() const {
    return dificuldade;
}

bool Materia::operator==(const Materia& other) const {
    return this->id == other.id;
}

bool Materia::operator!=(const Materia& other) const {
    return !(*this == other);
}
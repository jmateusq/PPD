//No catálogo_voos.txt temos:
//ID_VOO ID_ORIGEM ID_DESTINO DURACAO_MIN
#ifndef VOO_H
#define VOO_H

#include <string> // Necessário agora

class Voo {
private:
    int id;
    int origem;
    int destino;
    int duracao;
    int custo;

public:
    Voo(); 
    Voo(int id, int origem, int destino, int duracao);

    int getId() const;
    int getOrigem() const;
    int getDestino() const;
    int getDuracao() const;

    bool operator==(const Voo& other) const;
    bool operator!=(const Voo& other) const;

    // --- NOVO: Método Estático Centralizado ---
    static std::string getNomeAeroporto(int id);
};

#endif
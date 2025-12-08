//No catálogo_voos.txt temos:
//ID_VOO ID_ORIGEM ID_DESTINO DURACAO_MIN
#ifndef VOO_H
#define VOO_H

#include <string>
#include <iostream>


class Voo {
private:
    int id;           // ID único do voo
    int origem;       // ID do aeroporto (0, 1, 2...)
    int destino;      // ID do aeroporto (0, 1, 2...)
    int duracao;      // Em minutos (peso para o score)
    int custo;        // Opcional: Custo operacional

public:
    Voo(); // Construtor padrão (Voo vazio/folga)
    Voo(int id, int origem, int destino, int duracao);

    int getId() const;
    int getOrigem() const;
    int getDestino() const;
    int getDuracao() const;

    // Sobrecarga para comparação rápida
    bool operator==(const Voo& other) const;
    bool operator!=(const Voo& other) const;
};

#endif
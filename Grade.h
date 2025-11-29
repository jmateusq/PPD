#ifndef GRADE_H
#define GRADE_H

#include <vector>
#include <iostream>
#include "Materia.h"

class Grade {
private:
    std::vector<Materia> slots;
    int pontuacao;

    // Método interno para atualizar a pontuação baseada nas regras
    void atualizarPontuacao();

public:
    // Construtor cria grade aleatória baseada numa lista de matérias disponíveis
    Grade(const std::vector<Materia>& catalogoMaterias);
    
    // Construtor de cópia (útil para gerar vizinhos)
    Grade(const Grade& outra);

    int getPontuacao() const;
    
    // Gera uma nova grade trocando dois horários
    Grade gerarVizinho() const;

    Grade& operator=(const Grade& other); 

    void imprimir() const;
};

#endif
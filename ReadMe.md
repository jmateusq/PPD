# Otimização de Horários de Estudo (Hill Climbing)

Este projeto é uma implementação em **C++ Orientado a Objetos** de um sistema para organização automática de grades de horários. Baseado no trabalho de *Gabrielly Maria da S. Barbosa* e *José Mateus Freitas Queiroz* (2025).

O objetivo é utilizar o algoritmo de busca local **Hill Climbing** (Subida de Encosta) para encontrar a distribuição de aulas que maximize a produtividade do estudante e minimize o cansaço mental.

## 🧠 Heurística (Regras de Pontuação)

O algoritmo avalia a qualidade da grade com base nos seguintes critérios pedagógicos:

1.  **Produtividade Matinal (+20):** Prioriza matérias de alta dificuldade (ex: Matemática, Física) no turno da manhã.
2.  **Preservação Noturna (-20 / +10):** Penaliza matérias difíceis à noite e bonifica tempo livre para descanso.
3.  **Variedade (-50):** Aplica penalidade severa caso a mesma disciplina se repita consecutivamente no mesmo dia (evita fadiga).

## 📂 Estrutura do Projeto

O código original foi refatorado para seguir padrões de Orientação a Objetos:

*   **`Materia`**: Classe que representa a disciplina (ID, Nome e Nível de Dificuldade).
*   **`Grade`**: Representa o estado da solução (vetor de slots) e calcula sua própria pontuação.
*   **`Otimizador`**: Contém a lógica do *Hill Climbing* (gera vizinhos e decide trocas).
*   **`Constantes`**: Configurações globais (Dias da semana, Turnos).

## 🚀 Como Compilar e Executar

Certifique-se de ter um compilador C++ (como g++) instalado. No terminal, execute:

```bash
# Compilar todos os arquivos .cpp
g++ main.cpp Grade.cpp Materia.cpp Otimizador.cpp -o otimizador

# Executar
./otimizador
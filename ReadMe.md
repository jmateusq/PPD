# Airline Crew Rostering (Otimização de Escala de Tripulação)

Este projeto é uma implementação em **C++** de um sistema para otimização de escalas de tripulação aérea (Crew Rostering). O objetivo é resolver um problema NP-Difícil de alocação de recursos, garantindo que voos sejam cobertos respeitando restrições geográficas e regulatórias.

O projeto foi desenvolvido para a disciplina de **Programação Paralela e Distribuída (PPD)**, demonstrando estratégias de escalabilidade utilizando **OpenMP (CPU Multicore)**, **CUDA (GPU)** e **MPI (Cluster/Distribuído)**.

## ✈️ O Problema (Domínio)

Diferente de uma grade escolar estática, a escala de tripulação possui **Continuidade Geográfica**:
1.  Se um piloto pousa em Miami (MIA), seu próximo voo *obrigatoriamente* deve partir de Miami.
2.  Existem restrições rígidas de descanso e horas de voo.
3.  O objetivo é maximizar as horas de voo produtivas e minimizar custos (como estadias em hotéis não planejadas ou "deadheads").

## 🧠 Algoritmo e Heurística

Utilizamos o algoritmo de busca local **Hill Climbing** com **Random Restarts**:
1.  **Geração Inicial:** Cria uma escala aleatória (respeitando minimamente as restrições ou totalmente caótica).
2.  **Avaliação (Score):**
    *   **+ Pontos:** Voos de alta prioridade cobertos.
    *   **- Penalidade Infinita:** Quebra de rota (ex: GRU->MIA seguido de JFK->LHR).
    *   **- Penalidade:** Excesso de jornada ou pouco descanso.
3.  **Vizinhança:** Troca voos de tripulantes ou horários para tentar melhorar o score.

## 📂 Estrutura do Projeto (Refatorado)

*   **`Configuracao`**: Define o tamanho do problema (Número de Tripulantes, Voos disponíveis, Máximo de Iterações).
*   **`Voo` (Antiga Materia)**: Representa um trecho aéreo. Contém:
    *   `ID`, `Origem` (Aeroporto), `Destino` (Aeroporto), `Duração`.
*   **`Escala` (Antiga Grade)**: Representa a linha de trabalho de um ou mais tripulantes. É responsável por validar a continuidade geográfica dos voos alocados.
*   **`Otimizador`**: O motor de busca. Implementa as versões Sequencial, OpenMP e CUDA para encontrar a melhor escala.

## 🚀 Como Compilar e Executar

### Pré-requisitos
*   Docker (recomendado) ou GCC com suporte a OpenMP e NVCC (para CUDA).

### Compilação (Sequencial/OpenMP)
```bash
# Compilar
g++ -o airline_opt *.cpp -fopenmp -O3

# Executar
./airline_opt
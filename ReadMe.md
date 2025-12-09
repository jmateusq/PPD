# Airline Crew Rostering (Otimização de Escala de Tripulação)

Este projeto é uma implementação em **C++** de um sistema para otimização de escalas de tripulação aérea (Crew Rostering). O objetivo é resolver um problema NP-Difícil de alocação de recursos, garantindo que voos sejam cobertos respeitando restrições geográficas e regulatórias. 

O projeto foi desenvolvido para a disciplina de **Programação Paralela e Distribuída (PPD)**, demonstrando estratégias de escalabilidade utilizando **OpenMP (CPU Multicore)** e **CUDA (GPU)**.

O projeto completo, com todos os códigos na íntegra, pode ser acessado [aqui](https://github.com/jmateusq/PPD).

## ✈️ O Problema

A complexidade fundamental reside na explosão combinatória. Imagine um cenário simples com 30 dias, 6 turnos possíveis e 52 voos disponíveis. O número de combinações possíveis de escalas é astronômico, superando a capacidade de verificação manual ou até mesmo a força bruta de computadores convencionais em tempo hábil. À medida que se adicionam mais voos ou períodos mais longos, o espaço de busca por uma solução viável cresce exponencialmente, tornando impossível testar todas as possibilidades para encontrar a "melhor".

Dado o cenário anterior, se a tripulação pode ter apenas 2 alternativas de deslocamento dado o lugar que está, são em torno de $2^{30*6*52}$ ou aproximadamente $10^{2817}$ combinações possíveis. Para se ter uma ideia, o número de átomos no universo observável é de "apenas" $10^{80}$

No entanto, a dificuldade não é apenas numérica; ela é estrutural, devido à natureza das restrições que precisam ser satisfeitas simultaneamente. O problema lida com dois tipos de regras antagônicas: as restrições rígidas (hard constraints) e as restrições suaves (soft constraints). As restrições rígidas são mandatórias e invioláveis, geralmente ditadas por legislação trabalhista ou necessidades críticas do hospital

## 🧠 Algoritmo e Heurística

Utilizamos o algoritmo de busca local **Hill Climbing** com **Random Restarts**:
1.  **Geração Inicial:** Cria uma escala aleatória (respeitando minimamente as restrições ou totalmente caótica).
2.  **Avaliação (Score):**
    *   **+ Pontos:** Situações desejáveis.
    *   **- Pontos:** Situações indesejáveis.
3.  **Vizinhança:** Troca voos de tripulantes ou horários para tentar melhorar o score.

## 📂 Estrutura do Projeto

*   **`Configuracao`**: Define o tamanho do problema.
*   **`Voo`**: Representa um trecho aéreo. Contém:
    *   `ID`, `Origem` (Aeroporto), `Destino` (Aeroporto), `Duração`.
*   **`Escala`**: Representa a linha de trabalho de um ou mais tripulantes. É responsável por validar a continuidade geográfica dos voos alocados.
*   **`Otimizador`**: O motor de busca. Implementa as versões Sequencial, OpenMP e CUDA para encontrar a melhor escala.

## 🚀 Como Compilar e Executar

### Pré-requisitos
*   Docker (recomendado) e CUDA Container ToolKit, ou GCC com suporte a OpenMP e NVCC (para CUDA).

### Compilação no container
```bash
sudo docker compose --project-directory ./PPD/docker run --rm otimizador
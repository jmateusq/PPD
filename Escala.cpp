#include "Escala.h"
#include <cstdlib> 
#include <algorithm> 
#include <iomanip> 

// Auxiliar para imprimir nomes baseados no ID (Apenas visual)
std::string getNomeAeroporto(int id) {
    switch(id) {
        case 0: return "GRU";
        case 1: return "MIA";
        case 2: return "JFK";
        case 3: return "LHR";
        case 4: return "BSB";
        case 5: return "SCL";
        default: return "---";
    }
}

// Construtor
Escala::Escala(const std::vector<Voo>& catalogoVoos, Configuracao Config) : config(Config) {
    int totalSlots = config.getTotalSlots(); // Dias * VoosPorDia
    slots.reserve(totalSlots); 

    // PREENCHIMENTO INICIAL
    // Estratégia: Preencher com voos aleatórios e alguns "buracos" (folgas)
    // Isso cria uma solução inicial ruim, que o Hill Climbing vai consertar.
    for (int i = 0; i < totalSlots; i++) {
        // 30% de chance de ser folga (Voo vazio), 70% de ser voo
        if ((rand() % 100) < 30) {
            slots.push_back(Voo()); // Voo vazio (construtor padrão)
        } else {
            int idx = rand() % catalogoVoos.size();
            slots.push_back(catalogoVoos[idx]);
        }
    }

    atualizarPontuacao();
}

// Construtor de Cópia
Escala::Escala(const Escala& outra) : config(outra.config) {
    this->slots = outra.slots;
    this->pontuacao = outra.pontuacao;
}

long long int Escala::getPontuacao() const {
    return pontuacao;
}

// --- CORAÇÃO DO ALGORITMO ---
void Escala::atualizarPontuacao() {
    long long int score = 0;
    
    // Assumimos que todo tripulante começa na BASE (Aeroporto 0 - ex: GRU)
    int localizacaoAtual = 0; 

    // Penalidades e Bônus configuráveis
    const int BONUS_HORA_VOO = 10;    // Ganha pontos por produzir
    const int PENALIDADE_QUEBRA = 50000; // Perde muito se teleportar
    const int CUSTO_DEADHEAD = 2000;     // Custo se precisar mover o piloto como passageiro

    for (const auto& voo : slots) {
        // Se for slot vazio (Folga/Espera)
        if (voo.getId() == -1) {
            // Folga é neutra, mas o piloto continua no mesmo lugar.
            // Poderíamos dar um pequeno bônus por descanso se quiséssemos.
            continue; 
        }

        // Verifica Continuidade: O voo sai de onde o piloto está?
        if (voo.getOrigem() == localizacaoAtual) {
            // Conexão Perfeita!
            score += (voo.getDuracao() * BONUS_HORA_VOO); // Quanto mais longo, mais produtivo
            
            // Atualiza a posição do piloto para o destino
            localizacaoAtual = voo.getDestino();
        } 
        else {
            // ERRO DE ROTA: O piloto está em A, mas o voo sai de B.
            // Penalidade severa (solução inválida ou muito custosa)
            score -= PENALIDADE_QUEBRA;
            
            // Para o algoritmo não se perder totalmente, assumimos que a empresa
            // pagou um transporte de emergência (Deadhead) para levar ele até a origem do voo.
            score -= CUSTO_DEADHEAD;
            
            // Agora ele está no destino desse voo "forçado"
            localizacaoAtual = voo.getDestino();
        }
    }
    
    // Regra Extra: O piloto deve voltar para a Base (0) no final da escala?
    // Se não terminar em 0, penaliza.
    if (localizacaoAtual != 0) {
        score -= 5000; // Penalidade por não dormir em casa no final do mês
    }

    this->pontuacao = score;
}

// Gera Vizinho: Faz uma mutação na escala
Escala Escala::gerarVizinho(const std::vector<Voo>& catalogoVoos) const {
    Escala vizinho(*this); // Copia
    
    int tipoMutacao = rand() % 3;
    int idx = rand() % vizinho.slots.size();

    if (tipoMutacao == 0) {
        // TROCA: Troca um voo por outro aleatório do catálogo
        int idxVoo = rand() % catalogoVoos.size();
        vizinho.slots[idx] = catalogoVoos[idxVoo];
    } 
    else if (tipoMutacao == 1) {
        // LIMPEZA: Transforma um voo em folga (remove conflito)
        vizinho.slots[idx] = Voo(); // Voo vazio
    }
    else {
        // SWAP: Troca dois slots de lugar na mesma escala
        int idx2 = rand() % vizinho.slots.size();
        std::swap(vizinho.slots[idx], vizinho.slots[idx2]);
    }
    
    vizinho.atualizarPontuacao();
    return vizinho;
}

Escala& Escala::operator=(const Escala& other) {
    if (this != &other) {
        this->slots = other.slots;
        this->pontuacao = other.pontuacao;
        this->config = other.config;
    }
    return *this;
}

void Escala::imprimir() const {
    std::cout << "\n=== ESCALA DO TRIPULANTE (Score: " << pontuacao << ") ===\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << "DIA  | ORIGEM -> DESTINO | DURACAO | STATUS\n";
    std::cout << "---------------------------------------------------------\n";

    int dias = config.getDias();
    int voosPorDia = config.getVoosPorDia();
    
    int currentDay = 1;
    for (size_t i = 0; i < slots.size(); i++) {
        // Calcula o dia atual baseado no índice
        if (i > 0 && i % voosPorDia == 0) currentDay++;

        Voo v = slots[i];
        
        std::cout << std::setw(4) << currentDay << " | ";
        
        if (v.getId() == -1) {
            std::cout << "      ---          |    ---  | FOLGA/ESPERA";
        } else {
            std::string rota = getNomeAeroporto(v.getOrigem()) + " -> " + getNomeAeroporto(v.getDestino());
            std::cout << std::setw(17) << rota << " | " 
                      << std::setw(6) << v.getDuracao() << "m | VOO " << v.getId();
        }
        std::cout << "\n";
    }
    std::cout << "---------------------------------------------------------\n";
}
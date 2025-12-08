#include "Escala.h"
#include <cstdlib> 
#include <algorithm> 
#include <iomanip> 
#include <map> 

// Construtor
Escala::Escala(const std::vector<Voo>& catalogoVoos, Configuracao Config) : config(Config) {
    int totalSlots = (int)config.getTotalSlots(); 
    slots.reserve((unsigned long int)totalSlots); 

    for (int i = 0; i < totalSlots; i++) {
        // 20% Folga, 80% Voo (Começa bem preenchido)
        if ((rand() % 100) < 20) {
            slots.push_back(Voo()); 
        } else {
            int idx = rand() % catalogoVoos.size();
            slots.push_back(catalogoVoos[idx]);
        }
    }
    atualizarPontuacao();
}

Escala::Escala(const Escala& outra) : config(outra.config) {
    this->slots = outra.slots;
    this->pontuacao = outra.pontuacao;
}

long long int Escala::getPontuacao() const {
    return pontuacao;
}

// --- LÓGICA DE PONTUAÇÃO (Constraints) ---
void Escala::atualizarPontuacao() {
    long long int score = 0;
    int localizacaoAtual = 0; // Base: GRU
    
    // Contadores Sequenciais
    int voosSeguidos = 0;
    int folgasSeguidas = 0;

    // Mapa para contar repetição global de voos (Variedade)
    std::map<int, int> frequenciaVoos;
    
    // Configurações do tempo
    int voosPorDia = config.getVoosPorDia();

    // --- PESOS E PENALIDADES ---
    const int BONUS_HORA_VOO          = 20;   // Mantém (Base do cálculo)
    const int BONUS_DESCANSO_CURTO    = 500;  // Aumentaria um pouco para incentivar recargas estratégicas

    // --- HARD CONSTRAINTS (Coisas impossíveis ou ilegais) ---
    // Devem ser ordens de magnitude maiores que qualquer ganho
    const int PENALIDADE_QUEBRA       = 100000; // Aumentar! Teletransporte é inaceitável.
    
    // --- SAFETY CONSTRAINTS (Regras de segurança) ---
    // Devem ser maiores que o ganho do voo mais longo (ex: > 12.000)
    const int PENALIDADE_FADIGA       = 15000;  // Aumentar! Jamais vale a pena voar cansado.
    
    // --- SOFT CONSTRAINTS (Qualidade de vida / Preferência da empresa) ---
    // Podem ser menores, negociáveis com a produtividade
    const int PENALIDADE_REPETICAO    = 8000;   // Aumentar. Repetir 4x é muito chato.
    const int PENALIDADE_OCIOSIDADE   = 5000;   // Bom valor.
    const int PENALIDADE_MANHA_OCIOSA = 5000;   // Bom valor (equivalente a perder um voo médio).
    
    const int CUSTO_DEADHEAD_FIXO     = 2000;   // Ok.

    // Mudei para loop com índice (i) para saber quando é o início do dia
    for (size_t i = 0; i < slots.size(); i++) {
        
        Voo voo = slots[i];
        
        // Verifica se é o primeiro slot do dia atual
        bool inicioDoDia = (i % voosPorDia == 0);

        // --- CASO 1: FOLGA ---
        if (voo.getId() == -1) {
            voosSeguidos = 0; 
            folgasSeguidas++; 

            // 1. Regra da Manhã Ociosa (NOVO)
            if (inicioDoDia) {
                score -= PENALIDADE_MANHA_OCIOSA;
            }

            // 2. Regra de Ociosidade contínua
            if (folgasSeguidas <= 2) score += BONUS_DESCANSO_CURTO;
            else score -= PENALIDADE_OCIOSIDADE; 
            
            continue; 
        }

        // --- CASO 2: VOO REAL ---
        folgasSeguidas = 0; 
        voosSeguidos++;

        // 1. Contagem de Repetição (Variedade)
        frequenciaVoos[voo.getId()]++;
        if (frequenciaVoos[voo.getId()] > LIMITE_REPETICAO_VOO) {
            score -= PENALIDADE_REPETICAO;
        }

        // 2. Continuidade Geográfica
        if (voo.getOrigem() == localizacaoAtual) {
            score += (voo.getDuracao() * BONUS_HORA_VOO); 
        } 
        else {
            score -= PENALIDADE_QUEBRA;
            score -= CUSTO_DEADHEAD_FIXO;
            score -= (180 * BONUS_HORA_VOO); 
        }

        // 3. Fadiga
        if (voosSeguidos >= 3) {
            score -= PENALIDADE_FADIGA;
        }

        localizacaoAtual = voo.getDestino();
    }
    
    // Regra Final: Base
    if (localizacaoAtual != 0) {
        score -= 10000; 
    }

    this->pontuacao = score;
}

Escala Escala::gerarVizinho(const std::vector<Voo>& catalogoVoos) const {
    Escala vizinho(*this); 
    
    int tipoMutacao = rand() % 100; 
    unsigned long idx = rand() % vizinho.slots.size();

    // Ajustei as probabilidades para favorecer a troca de voos (ajuda na variedade)
    if (tipoMutacao < 50) { // 50% chance: Trocar Voo
        int idxVoo = rand() % catalogoVoos.size();
        vizinho.slots[idx] = catalogoVoos[idxVoo];
    } 
    else if (tipoMutacao < 70) { // 20%: Criar Folga
        vizinho.slots[idx] = Voo(); 
    }
    else if (tipoMutacao < 85) { // 15%: Remover Folga
        if (vizinho.slots[idx].getId() == -1) {
            int idxVoo = rand() % catalogoVoos.size();
            vizinho.slots[idx] = catalogoVoos[idxVoo];
        }
    }
    else { // 15%: Swap
        unsigned long idx2 = rand() % vizinho.slots.size();
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
    std::cout << "\n=== ESCALA OTIMIZADA (Score: " << pontuacao << ") ===\n";
    std::cout << "-----------------------------------------------------------------------------\n";
    std::cout << "DIA | ORIGEM -> DESTINO               | DURACAO | STATUS       | OBS\n";
    std::cout << "-----------------------------------------------------------------------------\n";

    int voosPorDia = config.getVoosPorDia();
    int currentDay = 1;
    
    int voosSeguidos = 0;
    int folgasSeguidas = 0;
    
    // Mapa auxiliar apenas para visualização de repetidos na impressão
    std::map<int, int> countView;

    for (size_t i = 0; i < slots.size(); i++) {
        
        bool inicioDoDia = (i % voosPorDia == 0);

        if (i > 0 && inicioDoDia) {
            currentDay++;
            std::cout << "----|---------------------------------|---------|--------------|-----\n";
        }

        Voo v = slots[i];
        
        std::cout << std::setw(3) << currentDay << " | ";
        
        if (v.getId() == -1) {
            folgasSeguidas++;
            voosSeguidos = 0;
            std::cout << "      ---   REPOUSO   ---         |   ---   | FOLGA        |";
            
            // Visualização das flags
            bool obs = false;
            if (inicioDoDia) { std::cout << " ! PREGUICA"; obs = true; }
            if (folgasSeguidas > 2) { std::cout << " ! OCIO"; obs = true; }
            if (!obs) std::cout << " OK";

        } else {
            folgasSeguidas = 0;
            voosSeguidos++;
            countView[v.getId()]++; 

            std::string rota = Voo::getNomeAeroporto(v.getOrigem()) + " -> " + Voo::getNomeAeroporto(v.getDestino());
            if (rota.length() > 31) rota = rota.substr(0, 31);

            std::cout << std::left << std::setw(32) << rota << "| " 
                      << std::right << std::setw(3) << v.getDuracao() << " min | VOO " << std::setw(4) << v.getId() << " |";
            
            bool obs = false;
            if (voosSeguidos >= 3) { std::cout << " ! FADIGA"; obs=true; }
            if (countView[v.getId()] > 3) { std::cout << " ! REPETIDO"; obs=true; } 
            if (!obs) std::cout << " OK";
        }
        std::cout << "\n";
    }
    std::cout << "-----------------------------------------------------------------------------\n";
}
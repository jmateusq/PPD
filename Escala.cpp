#include "Escala.h"
#include <algorithm> 
#include <iomanip> 
#include <map> 
#include <random> // Necessário para as distribuições

// Construtor: Agora recebe o RNG (Random Number Generator)
Escala::Escala(const std::vector<Voo>& catalogoVoos, Configuracao Config, std::mt19937& rng) : config(Config) {
    int totalSlots = (int)config.getTotalSlots(); 
    slots.reserve((unsigned long int)totalSlots); 

    // Cria distribuições para gerar números no intervalo desejado
    std::uniform_int_distribution<int> distPorcentagem(0, 99);
    
    // Se o catálogo estiver vazio, evita erro de divisão por zero na distribuição
    int maxCatalogo = catalogoVoos.empty() ? 0 : (int)catalogoVoos.size() - 1;
    std::uniform_int_distribution<int> distCatalogo(0, maxCatalogo);

    for (int i = 0; i < totalSlots; i++) {
        // 20% de chance de ser Folga (buraco), 80% de ser Voo
        if (distPorcentagem(rng) < 20 || catalogoVoos.empty()) {
            slots.push_back(Voo()); // Voo vazio (Folga)
        } else {
            int idx = distCatalogo(rng);
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
    const int BONUS_HORA_VOO          = 20;   
    const int BONUS_DESCANSO_CURTO    = 500; 

    // Hard Constraints
    const int PENALIDADE_QUEBRA       = 100000; 
    
    // Safety Constraints
    const int PENALIDADE_FADIGA       = 15000;  
    
    // Soft Constraints
    const int PENALIDADE_REPETICAO    = 8000;   
    const int PENALIDADE_OCIOSIDADE   = 5000;   
    const int PENALIDADE_MANHA_OCIOSA = 5000;   
    
    const int CUSTO_DEADHEAD_FIXO     = 2000;   

    const int LIMITE_REPETICAO_VOO = 3; 

    for (size_t i = 0; i < slots.size(); i++) {
        
        Voo voo = slots[i];
        
        // Verifica se é o primeiro slot do dia atual
        bool inicioDoDia = (i % voosPorDia == 0);

        // --- CASO 1: FOLGA ---
        if (voo.getId() == -1) {
            voosSeguidos = 0; 
            folgasSeguidas++; 

            // 1. Regra da Manhã Ociosa
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
    
    // Regra Final: Base (deve terminar em GRU = 0)
    if (localizacaoAtual != 0) {
        score -= 10000; 
    }

    this->pontuacao = score;
}

// Carrega a escala diretamente de um vetor de índices (usado para reconstruir após GPU)
void Escala::carregarDeIndices(const std::vector<int>& indices, const std::vector<Voo>& catalogo) {
    this->slots.clear();
    for (int idx : indices) {
        if (idx == -1) {
            this->slots.push_back(Voo()); // Folga
        } else {
            // Cuidado: idx deve ser válido
            if(idx >= 0 && idx < (int)catalogo.size()) {
                this->slots.push_back(catalogo[idx]);
            }
        }
    }
    atualizarPontuacao(); 
}

// Gera Vizinho: Faz uma mutação na escala usando o RNG passado
Escala Escala::gerarVizinho(const std::vector<Voo>& catalogoVoos, std::mt19937& rng) const {
    Escala vizinho(*this); 
    
    if (catalogoVoos.empty()) return vizinho;

    // Distribuições
    std::uniform_int_distribution<int> distMutacao(0, 99);
    std::uniform_int_distribution<int> distSlot(0, (int)vizinho.slots.size() - 1);
    std::uniform_int_distribution<int> distCatalogo(0, (int)catalogoVoos.size() - 1);

    int tipoMutacao = distMutacao(rng); 
    unsigned long idx = distSlot(rng);

    if (tipoMutacao < 50) { // 50% chance: Trocar Voo
        int idxVoo = distCatalogo(rng);
        vizinho.slots[idx] = catalogoVoos[idxVoo];
    } 
    else if (tipoMutacao < 70) { // 20%: Criar Folga
        vizinho.slots[idx] = Voo(); 
    }
    else if (tipoMutacao < 85) { // 15%: Remover Folga
        if (vizinho.slots[idx].getId() == -1) {
            int idxVoo = distCatalogo(rng);
            vizinho.slots[idx] = catalogoVoos[idxVoo];
        }
    }
    else { // 15%: Swap (Troca dois slots de lugar)
        unsigned long idx2 = distSlot(rng);
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
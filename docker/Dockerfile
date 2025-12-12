# Usa a imagem oficial da NVIDIA com toolkit de desenvolvimento (nvcc incluído)
# Ubuntu 22.04 e CUDA 12.3 (compatível com a maioria das placas modernas)
FROM nvidia/cuda:12.3.1-devel-ubuntu22.04

# Instala o G++ (para o código C++ host) e Make (para compilar)
RUN apt-get update && apt-get install -y \
    g++ \
    build-essential \
    make \
    openmpi-bin \
    libopenmpi-dev \
    && rm -rf /var/lib/apt/lists/*

# Define o diretório de trabalho
WORKDIR /usr/src/app

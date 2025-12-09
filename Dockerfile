version: '3.8'

services:
  otimizador:
    build: .
    image: otimizador-cuda
    container_name: otimizador_cuda_container
    
    # Mapeia a pasta atual
    volumes:
      - .:/usr/src/app
    
    stdin_open: true 
    tty: true

    # --- CONFIGURAÇÃO DA GPU ---
    deploy:
      resources:
        reservations:
          devices:
            - driver: nvidia
              count: 1          
              capabilities: [gpu]

    # --- COMANDO DE COMPILAÇÃO E EXECUÇÃO ---
    # Removido -O3 de todas as etapas.
    # Mantido -fopenmp no g++ e -lgomp no link final (nvcc)
    command: >
      /bin/bash -c "
      rm -f *.o otimizador_gpu && 
      nvcc -c KernelOtimizador.cu -o KernelOtimizador.o && 
      g++ -fopenmp -c *.cpp && 
      nvcc *.o -o otimizador_gpu -lgomp && 
      ./otimizador_gpu"
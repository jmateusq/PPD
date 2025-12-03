# Usa a imagem oficial do GCC (Compilador C++)
FROM gcc:latest

# Define a pasta de trabalho dentro do container
WORKDIR /usr/src/app

# Copia os arquivos atuais para dentro (apenas para build inicial)
COPY . .

# O comando padrão será sobrescrito pelo docker-compose, 
# mas deixamos um padrão aqui por boas práticas.
CMD ["./otimizador"]
CC = gcc

all: produtor consumidor

produtor: produtor.c
	$(CC) produtor.c -o produtor

consumidor: consumidor.c
	$(CC) consumidor.c -o consumidor

# Limpa tudo (executáveis)
clean:
	rm -f produtor consumidor

.PHONY: all clean

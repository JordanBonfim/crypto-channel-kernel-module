#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/cryptochannel"
#define READ_SIZE 4096 

int main() {
    int fd;
    char buffer[READ_SIZE];
    ssize_t bytes_read;

    printf("--- CONSUMIDOR CRYPTOCHANNEL ---\n");
    printf("Abrindo %s para leitura...\n", DEVICE_PATH);

    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("ERRO ao abrir dispositivo");
        return errno;
    }

    printf("Aguardando mensagens (Ctrl+C para sair)...\n");

    while(1) {
        // O processo dorme se não tiver dados no buffer do kernel
        bytes_read = read(fd, buffer, READ_SIZE - 1);

        if (bytes_read < 0) {
            perror("Erro na leitura");
            // Se foi interrompido (Ctrl+C) ou erro , sai do loop
            break; 
        }

        // Se leu 0 bytes volta pro inicio
        if (bytes_read == 0) continue;

        buffer[bytes_read] = '\0';
        printf("Consumidor < Recebido (%zd bytes): %s\n", bytes_read, buffer);
            
        memset(buffer, 0, READ_SIZE);
    }

    close(fd);
    printf("Consumidor encerrado.\n");
    return 0;
}
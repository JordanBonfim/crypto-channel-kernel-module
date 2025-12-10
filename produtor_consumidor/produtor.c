#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  // Para open
#include <unistd.h> // Para write, close
#include <errno.h>

#define DEVICE_PATH "/dev/cryptochannel"
#define BUFFER_SIZE 256

int main() {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t ret;

    printf("--- PRODUTOR CRYPTOCHANNEL ---\n");
    printf("Abrindo %s para escrita...\n", DEVICE_PATH);

    // Abre somente para escrita
    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("ERRO ao abrir dispositivo");
        return errno;
    }

    printf("Digite suas mensagens (digite 'sair' para encerrar):\n");

    while (1) {
        printf("Produtor > ");
        
        // Lê do teclado
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        // Remove o \n do final da string 
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "sair") == 0) break;
        if (strlen(buffer) == 0) continue; // Ignora linhas vazias

        // Tenta escrever no Kernel
        // se o buffer do kernel estiver cheio, o processo dorme até ter espaço
        ret = write(fd, buffer, strlen(buffer));

        if (ret < 0) {
            perror("Erro na escrita");
        } else {
            printf("   [Kernel] Recebeu %zd bytes, criptografou e guardou.\n", ret);
        }
    }

    close(fd);
    printf("Produtor encerrado.\n");
    return 0;
}
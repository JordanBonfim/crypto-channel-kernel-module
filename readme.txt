CryptoChannel - Linux Kernel Module

Este é um módulo de kernel Linux (cryptochannel) desenvolvido como base para um canal criptográfico.

Pré-requisitos

Para compilar este módulo, você precisa dos headers do kernel instalados na sua máquina.

Debian/Ubuntu/Kali:

sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)


Estrutura de Arquivos

Certifique-se de que seu diretório contém:

cryptochannel.c: O código fonte C.

Makefile: O arquivo de script de compilação.



Como Usar

1. Compilar

Execute o comando make no diretório raiz do projeto:

make


Isso irá gerar o arquivo binário do módulo: cryptochannel.ko.

2. Carregar o Módulo via script

Execute:
sudo sh ./run.sh

3. Verificar Logs

Para confirmar se o módulo foi carregado e ver a mensagem "STARTED":

sudo dmesg | tail
# Ou
sudo journalctl -k -n 10


4. Descarregar o Módulo

Para remover o módulo do kernel:

sudo rmmod cryptochannel


Verifique novamente o dmesg para ver a mensagem "ENDED".

Limpeza

Para remover os arquivos temporários de compilação:

make clean

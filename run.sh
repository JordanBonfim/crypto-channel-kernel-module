#!/bin/bash

# Remove silenciosamente
sudo rmmod cryptochannel 2>/dev/null

# Insere o módulo
sudo insmod cryptochannel.ko

# Espera 1 segundo para o udev criar o arquivo /dev/cryptochannel
echo "Aguardando criação do dispositivo..."
sleep 1 

# Dá permissão
if [ -e /dev/cryptochannel ]; then
    sudo chmod 666 /dev/cryptochannel
    echo "Permissões concedidas e módulo carregado."
else
    echo "ERRO: O arquivo /dev/cryptochannel não foi criado."
    echo "Verifique se o init_module possui device_create e class_create."
fi
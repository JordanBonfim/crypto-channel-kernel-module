# CryptoChannel

Um módulo de kernel Linux simples (`cryptochannel`) que implementa um canal criptográfico educativo para leitura/escrita de mensagens através de um dispositivo de caractere e um pseudo-fs `/proc` para estatísticas.

Principais objetivos:
- Expor um dispositivo em `/dev/cryptochannel` para enviar/receber mensagens (fixas em 16 bytes, com padding se necessário).
- Expor estatísticas em `/proc/cryptochannel/stats`.

---

## Pré-requisitos

O sistema precisa ter os headers do kernel e ferramentas de compilação instaladas.

Debian / Ubuntu / Kali:

```bash
sudo apt update
sudo apt install build-essential linux-headers-$(uname -r)
```

---

## Estrutura de arquivos

Certifique-se de que seu diretório contém, pelo menos:

- `cryptochannel.c` — código fonte do módulo
- `Makefile` — script de compilação
- `run.sh` — script auxiliar para carregar/configurar o módulo

---

## Como compilar

No diretório do projeto, execute:

```bash
make
```

Isso gera o binário do módulo: `cryptochannel.ko`.

---

## Carregar o módulo

Use o script de execução incluído:

```bash
sudo sh ./run.sh
```

Verifique o dmesg para ver mensagens do módulo (por exemplo, `STARTED`):

```bash
sudo dmesg | tail
# ou
sudo journalctl -k -n 10
```

---

## Uso

- Para enviar uma mensagem (o módulo espera mensagens de 16 bytes — mensagens menores são preenchidas):

```bash
echo -n "mensagem de 16 bytes" > /dev/cryptochannel
```

Observações:
- O `-n` no `echo` evita adicionar `\n`. Se uma nova linha for enviada, o módulo deve tratar essa terminação, deve ser feito.
- Mensagens menores que 16 bytes devem ser preenchidas (padding) internamente, deve ser feito.

- Para ler a mensagem armazenada/recebida:

```bash
cat /dev/cryptochannel
```

- Para ver estatísticas do canal:

```bash
cat /proc/cryptochannel/stats
```

---

## Produtor / Consumidor

Há um pequeno par de programas em `produtor_consumidor/` que demonstram como escrever (produtor) e ler (consumidor) mensagens do dispositivo `/dev/cryptochannel`.

- **Compilar**:

```bash
cd produtor_consumidor
make
```

- **Pré-requisito**: o módulo deve estar carregado e o dispositivo `/dev/cryptochannel` criado com permissões de leitura/escrita. Use o script do projeto para isso:

```bash
sudo sh ../run.sh
```

- **Executar** (recomendado em dois terminais separados):

Terminal A (consumidor - fica aguardando mensagens):

```bash
cd produtor_consumidor
./consumidor
```

Terminal B (produtor - envia mensagens interativas):

```bash
cd produtor_consumidor
./produtor
```

- **Alternativa**: executar o `consumidor` em background no mesmo terminal:

```bash
cd produtor_consumidor
./consumidor &
./produtor
```

- **Controles**:
	- No `produtor`, digite mensagens e pressione Enter para enviar. Digite `sair` para encerrar o produtor.
	- No `consumidor`, use `Ctrl+C` para parar o consumidor.

- **Limpeza dos binários**:

```bash
cd produtor_consumidor
make clean
```


## Logs e diagnóstico

Mensagens de inicialização/finalização e outros logs ficam no buffer do kernel. Use:

```bash
sudo dmesg | tail
```

ou

```bash
sudo journalctl -k -n 50
```

---

## Descarregar o módulo

Para remover o módulo do kernel:

```bash
sudo rmmod cryptochannel
```

Verifique o dmesg para ver a mensagem de encerramento (por exemplo, `ENDED`).

---

## Limpeza

Para remover arquivos temporários e binários compilados:

```bash
make clean
```

---

## Contribuições

Este repositório é destinado a fins educacionais. Se quiser contribuir com melhorias (melhor tratamento de tamanho de mensagem, uso de mecanismos de criptografia, testes, documentação), abra uma issue ou pull request.

---

## Licença

Este projeto está licenciado sob a [Licença MIT](LICENSE). Consulte o arquivo `LICENSE` para mais detalhes.

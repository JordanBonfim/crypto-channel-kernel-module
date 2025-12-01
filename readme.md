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
- O `-n` no `echo` evita adicionar `\n`. Se uma nova linha for enviada, o módulo deve tratar essa terminação (comportamento do módulo descrito no código).
- Mensagens menores que 16 bytes são preenchidas (padding) internamente.

- Para ler a mensagem armazenada/recebida:

```bash
cat /dev/cryptochannel
```

- Para ver estatísticas do canal:

```bash
cat /proc/cryptochannel/stats
```

---

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

Coloque aqui a licença do seu projeto, se aplicável (por exemplo, MIT). Se preferir, mantenha sem licença até decidir.



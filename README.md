
# IHM Digital via DVI com Raspberry Pi Pico

Projeto de Interface Homem-Máquina (IHM) utilizando o **Raspberry Pi Pico W (RP2040)**, capaz de ler sinais analógicos e exibi-los em tempo real em um monitor via saída digital DVI, totalmente gerada por software.

---

## Sumário

- [Visão Geral](#visão-geral)
- [Arquitetura e Funcionamento](#arquitetura-e-funcionamento)
- [Especificações de Vídeo](#especificações-de-vídeo)
- [Funcionalidades](#funcionalidades)
- [Principais Características](#principais-características)
- [Bibliotecas Utilizadas](#bibliotecas-utilizadas)
- [Como Compilar e Executar](#como-compilar-e-executar)
- [Estrutura do Projeto](#estrutura-do-projeto)

---

## Visão Geral

Este projeto demonstra como gerar uma interface gráfica responsiva em monitores HDMI/DVI utilizando apenas recursos do microcontrolador RP2040, sem hardware gráfico dedicado. O sistema lê valores analógicos (ex: sensores) e os apresenta em tempo real, com interface customizada e escalonamento de fonte para melhor legibilidade.

---

## Arquitetura e Funcionamento

O projeto explora o paralelismo do RP2040, dividindo tarefas entre os dois núcleos:

- **Core 0 (Lógica e Aquisição):**
 	- Inicializa periféricos e hardware
 	- Realiza leitura do ADC (canal 1, GPIO 27)
 	- Gerencia buffers de caracteres (`charbuf`) e cores (`colourbuf`)

- **Core 1 (Renderização em Tempo Real):**
 	- Responsável pela geração do sinal DVI
 	- Executa a codificação TMDS e transmite os dados para o monitor

O uso de máquinas de estado PIO e rotinas otimizadas em Assembly permite a geração de vídeo digital estável a 60Hz, mesmo sem controlador gráfico dedicado.

---

## Especificações de Vídeo

- **Resolução:** 640x480p @ 60Hz
- **Escalonamento Vertical:** Fonte 8x8 ampliada para 8x24 pixels (3x), melhorando a leitura em telas LCD
- **Codificação TMDS:** Implementada em Assembly (`tmds_encode_font_2bpp.S`) para conversão rápida de RGB222 para o protocolo digital

---

## Funcionalidades

- **Leitura Analógica (ADC):** Monitoramento contínuo de sinais (0 a 4095), exibidos dinamicamente
- **Interface Visual:** Moldura personalizada (`draw_border`), texto centralizado, cores distintas para valores e prompts

---

## Principais Características

- **DVI via Software:** Sinal TMDS gerado por software usando a biblioteca `libdvi` e máquinas de estado PIO
- **Paralelismo Real:** Stack do Core 1 otimizado para garantir processamento de vídeo sem travamentos
- **Rotinas em Assembly:** Codificação TMDS ultra-rápida para exibição de dados em tempo real
- **Baixa Latência:** Ideal para aplicações que exigem resposta visual imediata

---

## Bibliotecas Utilizadas

O executável final integra:

1. **pico_stdlib & pico_multicore:** Base do sistema e suporte ao processamento paralelo
2. **libdvi:** Camada de transporte do sinal de vídeo digital (DVI/TMDS)
3. **libsprite:** Motor para gestão de elementos visuais na tela
4. **hardware_adc:** Leitura de dados analógicos para exibição na IHM

---

## Como Compilar e Executar

1. **Pré-requisitos:**
  - [Pico SDK](https://github.com/raspberrypi/pico-sdk) instalado
  - Ferramentas: CMake, Ninja, OpenOCD, Picotool

2. **Compilação:**
  - Execute o comando de build:
   ```sh
   ninja -C build
   ```

	 - Ou utilize a task "Compile Project" no VS Code

3. **Gravação/Execução:**
  - Use a task "Run Project" ou "Flash" para gravar o firmware na placa

---

## Estrutura do Projeto

```
├── main.c                  # Código principal
├── main_tx_uart.c          # Variante com transmissão UART
├── tmds_encode_font_2bpp.S # Rotinas Assembly para TMDS
├── assets/                 # Fontes e recursos visuais
├── lib/                    # Bibliotecas auxiliares (ADC, display, sensor, terminal)
├── libdvi/                 # Implementação do protocolo DVI/TMDS
├── libsprite/              # Motor de sprites e elementos gráficos
├── include/                # Headers compartilhados
├── build/                  # Saída de build
└── README.md               # Este arquivo
```

---

## Créditos

Desenvolvido por Matheus Silva para fins didáticos e demonstração de técnicas avançadas de vídeo digital embarcado.

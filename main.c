#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"
#include "terminal.h"
#include "display.h"
#include "./include/common_dvi_pin_configs.h"

#define VREG_VSEL VREG_VOLTAGE_1_20

#define BOTAO_A 5
#define BOTAO_B 6

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Variáveis de "Heartbeat" Multicore
volatile bool core0_alive = false;
volatile bool core1_alive = true; // Assumimos que a libdvi no Core 1 está rodando

volatile bool botao_a_pressed = false;

// Protótipos
bool uart_read_line(char *buffer, size_t max_len, uint32_t timeout_ms);
int16_t parse_lux_value(const char *message);
static void gpio_irq_handler(uint gpio, uint32_t events);

// Handler unificado para botões

int __not_in_flash_func(main)()
{
    // 1. Inicializa Display
    display_init(VREG_VSEL, &picodvi_dvi_cfg);

    // 2. Inicializa Terminal
    terminal_init();
    terminal_draw_border(COLOR_GRAY, COLOR_GREEN);

    // === DIAGNÓSTICO DE RESET ===
    const uint center_y = (CHAR_ROWS / 2) - 1;
    bool wdt_reset = watchdog_caused_reboot();

    if (wdt_reset)
    {
        // Se entrou aqui, foi o Watchdog que resetou o sistema (ex: cabo solto)
        // Incrementa contador de falhas no registrador SCRATCH[0]
        uint32_t fail_count = watchdog_hw->scratch[0] + 1;
        watchdog_hw->scratch[0] = fail_count;

        terminal_write_centered(center_y + 4, "ALERTA: Recuperado de Falha!", COLOR_RED, COLOR_BLACK);
        char fail_msg[30];
        sprintf(fail_msg, "Total Falhas: %d", fail_count);
        terminal_write_centered(center_y + 5, fail_msg, COLOR_YELLOW, COLOR_BLACK);
        sleep_ms(2000); // Delay para visualizar mensagens de erro
    }
    else
    {
        // Reset normal (energia ou upload de código)
        watchdog_hw->scratch[0] = 0;
        terminal_write_centered(center_y + 4, "Inicializacao Normal (Power On)", COLOR_GREEN, COLOR_BLACK);
        sleep_ms(2000); // Delay para visualizar mensagem de inicialização
    }

    // 3. Inicializa UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);

    // 4a. Configura botão A (GPIO 5)
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // 4b. Configura botão B (GPIO 6)
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // 5. Inicia renderização (Core 1)
    display_start_rendering();

    // === HABILITA WATCHDOG ===
    // Timeout de 2000ms (2 segundos).
    // Se não alimentar em 2s, o sistema reinicia.
    watchdog_enable(2000, 1);

    // 6. Loop principal
    char buffer[128];
    char uart_buffer[128];

    while (true)
    {
        // Reseta a flag do Core 0 no início do ciclo
        core0_alive = false;

        // Tenta ler linha via UART (timeout 1000ms)
        if (uart_read_line(uart_buffer, sizeof(uart_buffer), 1000))
        {
            // Se chegou aqui, recebemos dados
            core0_alive = true;

            // Processa e exibe os dados
            int16_t lux_value = parse_lux_value(uart_buffer);
            if (lux_value >= 0)
            {
                terminal_clear_line(center_y, COLOR_BLACK);
                sprintf(buffer, "Luminosidade: %d LUX", lux_value);
                terminal_write_centered(center_y, buffer, COLOR_WHITE, COLOR_BLUE);
            }
            else
            {
                terminal_clear_line(center_y, COLOR_BLACK);
                sprintf(buffer, "RX: %s", uart_buffer);
                terminal_write_centered(center_y, buffer, COLOR_YELLOW, COLOR_RED);
            }
        }
        else
        {
            // Timeout - Nenhum dado recebido (Cabo desconectado?)
            // Não setamos core0_alive = true aqui.
            terminal_clear_line(center_y, COLOR_BLACK);
            terminal_write_centered(center_y, "ERRO: Sem Sinal UART!", COLOR_RED, COLOR_BLACK);
        }

        // === ALIMENTAÇÃO DO WATCHDOG ===
        // Só alimenta se:
        // 1. Core 0 recebeu dados (core0_alive == true)
        // 2. Core 1 está renderizando (core1_alive == true)
        // 3. Botão A NÃO está pressionado
        if (core0_alive && core1_alive && !botao_a_pressed)
        {
            watchdog_update();
        }
        // SE core0_alive for false (timeout UART), o update NÃO acontece.
        // Após 2 ciclos de timeout (aprox 2000ms), o sistema vai reiniciar.

        sleep_ms(50);
    }
}

// Função para ler uma linha via UART com timeout
bool uart_read_line(char *buffer, size_t max_len, uint32_t timeout_ms)
{
    size_t pos = 0;
    absolute_time_t timeout = make_timeout_time_ms(timeout_ms);

    while (pos < max_len - 1)
    {
        if (time_reached(timeout))
        {
            buffer[pos] = '\0';
            return false; // Timeout
        }

        if (uart_is_readable(UART_ID))
        {
            char c = uart_getc(UART_ID);

            if (c == '\n')
            {
                buffer[pos] = '\0';
                return true; // Linha completa recebida
            }
            else if (c != '\r') // Ignora \r
            {
                buffer[pos++] = c;
            }
        }
        else
        {
            sleep_us(100); // Pequeno delay para não saturar CPU
        }
    }

    buffer[pos] = '\0';
    return false; // Buffer cheio
}

// Extrai valor de LUX da mensagem "LUX: 2450"
int16_t parse_lux_value(const char *message)
{
    const char *lux_ptr = strstr(message, "LUX:");
    if (lux_ptr)
    {
        int value;
        if (sscanf(lux_ptr, "LUX: %d", &value) == 1)
        {
            return (int16_t)value;
        }
    }
    return -1; // Erro no parse
}

// Callback unificado para botões BOOTSEL, A e B
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BOTAO_A)
    {
        if (gpio_get(BOTAO_A) == 0)
        {
            // Botão A pressionado
            botao_a_pressed = true;
        }
        else
        {
            // Botão A solto
            botao_a_pressed = false;
        }
    }
    else if (gpio == BOTAO_B)
    {
        reset_usb_boot(0, 0);
    }
}
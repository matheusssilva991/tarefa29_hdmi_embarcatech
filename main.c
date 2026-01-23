#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "hardware/uart.h"
#include "terminal.h"
#include "display.h"
#include "./include/common_dvi_pin_configs.h"

#define VREG_VSEL VREG_VOLTAGE_1_20
#define BOTAO_BOOTSEL 6

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0 // GP0 (SDA no header I2C0) -> Ligar no TX do outro
#define UART_RX_PIN 1 // GP1 (SCL no header I2C0) -> Ligar no RX do outro

// Protótipos
bool uart_read_line(char *buffer, size_t max_len, uint32_t timeout_ms);
int16_t parse_adc_value(const char *message);

// Callback para botão BOOTSEL
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

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

int __not_in_flash_func(main)()
{
    // 1. Inicializa Display
    display_init(VREG_VSEL, &picodvi_dvi_cfg);

    // 2. Inicializa Terminal
    terminal_init();
    terminal_draw_border(COLOR_GRAY, COLOR_GREEN);

    // 3. Inicializa UART para recepção
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART_ID, true);

    // 4. Configura botão BOOTSEL
    gpio_init(BOTAO_BOOTSEL);
    gpio_set_dir(BOTAO_BOOTSEL, GPIO_IN);
    gpio_pull_up(BOTAO_BOOTSEL);
    gpio_set_irq_enabled_with_callback(BOTAO_BOOTSEL, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // 5. Inicia renderização (Core 1)
    display_start_rendering();

    // 6. Loop principal - Recebe dados via UART
    const uint center_y = (CHAR_ROWS / 2) - 1;
    char buffer[128];
    char uart_buffer[128];

    while (true)
    {
        // Tenta ler linha via UART (timeout 1000ms)
        if (uart_read_line(uart_buffer, sizeof(uart_buffer), 1000))
        {
            // Parse do valor ADC
            int16_t adc_value = parse_adc_value(uart_buffer);

            if (adc_value >= 0)
            {
                // Limpa linha central
                terminal_clear_line(center_y, COLOR_BLACK);

                // Monta mensagem para exibir
                sprintf(buffer, "UART RX - ADC Recebido: %d", adc_value);

                // Escreve centralizado
                terminal_write_centered(center_y, buffer, COLOR_WHITE, COLOR_BLUE);
            }
            else
            {
                // Erro no parse - exibe mensagem recebida
                terminal_clear_line(center_y, COLOR_BLACK);
                sprintf(buffer, "RX: %s", uart_buffer);
                terminal_write_centered(center_y, buffer, COLOR_YELLOW, COLOR_RED);
            }
        }
        else
        {
            // Timeout - nenhum dado recebido
            terminal_clear_line(center_y, COLOR_BLACK);
            terminal_write_centered(center_y, "Aguardando dados UART...", COLOR_GRAY, COLOR_BLACK);
        }

        sleep_ms(50);
    }
}

// Função para ler linha completa via UART (até \n)
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

// Extrai valor do ADC da mensagem "MSG#123 | ADC: 4095"
int16_t parse_adc_value(const char *message)
{
    const char *adc_ptr = strstr(message, "ADC:");
    if (adc_ptr)
    {
        int value;
        if (sscanf(adc_ptr, "ADC: %d", &value) == 1)
        {
            return (int16_t)value;
        }
    }
    return -1; // Erro no parse
}
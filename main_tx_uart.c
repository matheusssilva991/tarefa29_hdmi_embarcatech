#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "adc_reader.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0 // GP0 (SDA no header I2C0) -> Ligar no RX do outro
#define UART_RX_PIN 1 // GP1 (SCL no header I2C0) -> Ligar no TX do outro

// Função para enviar string via UART
void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_putc_raw(UART_ID, *str++);
    }
}

int main()
{
    // Inicializa stdio (para printf via USB)
    stdio_init_all();

    // Inicializa UART
    uart_init(UART_ID, BAUD_RATE);

    // Configura pinos GPIO para UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Configura formato da comunicação: 8 bits de dados, 1 stop bit, sem paridade
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // Habilita FIFO da UART
    uart_set_fifo_enabled(UART_ID, true);

    // Inicializa ADC para leitura de dados
    adc_reader_init();

    printf("UART TX Transmissor Inicializado!\n");
    printf("Configuração: %d baud, 8N1\n", BAUD_RATE);
    printf("TX Pin: GP%d | RX Pin: GP%d\n", UART_TX_PIN, UART_RX_PIN);

    char buffer[128];
    uint32_t contador = 0;

    while (true)
    {
        // Lê valor do ADC
        uint16_t adc_value = adc_reader_read();

        // Monta mensagem para enviar
        sprintf(buffer, "MSG#%lu | ADC: %d\r\n", contador, adc_value);

        // Envia via UART
        uart_send_string(buffer);

        // Log no console USB
        printf("Enviado: %s", buffer);

        contador++;
        sleep_ms(500); // Envia a cada 500ms
    }

    return 0;
}

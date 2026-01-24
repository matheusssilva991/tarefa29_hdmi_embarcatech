#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "bh1750.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0 // GP0 (SDA no header I2C0) -> Ligar no RX do outro
#define UART_RX_PIN 1 // GP1 (SCL no header I2C0) -> Ligar no TX do outro

// Para I2C (sensor de luz BH1750)
#define I2C_PORT i2c0
#define SDA_PIN 4   // GP4 para I2C0
#define SCL_PIN 5   // GP5 para I2C0

// SDA: GPIO 4 (I2C0)
// SCL: GPIO 5 (I2C0)
// Endereço BH1750: 0x23

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

    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    bh1750_power_on(I2C_PORT);

    printf("UART TX Transmissor Inicializado!\n");
    printf("Configuração: %d baud, 8N1\n", BAUD_RATE);
    printf("TX Pin: GP%d | RX Pin: GP%d\n", UART_TX_PIN, UART_RX_PIN);

    char buffer[128];

    while (true)
    {
        // Lê valor do sensor de luz BH1750
        uint16_t luz_value = bh1750_read_measurement(I2C_PORT);

        // Monta mensagem para enviar
        sprintf(buffer, "LUX: %d\r\n", luz_value);

        // Envia via UART
        uart_send_string(buffer);

        // Log no console USB
        printf("Enviado: %s", buffer);

        sleep_ms(500); // Envia a cada 500ms
    }

    return 0;
}

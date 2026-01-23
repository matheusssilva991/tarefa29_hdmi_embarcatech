#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "terminal.h"
#include "display.h"
#include "adc_reader.h"
#include "./include/common_dvi_pin_configs.h"

#define VREG_VSEL VREG_VOLTAGE_1_20
#define BOTAO_BOOTSEL 6

// Callback para botão BOOTSEL
static void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

int __not_in_flash_func(main)()
{
    // 1. Inicializa Display
    display_init(VREG_VSEL, &picodvi_dvi_cfg);

    // 2. Inicializa Terminal
    terminal_init();
    terminal_draw_border(COLOR_GRAY, COLOR_GREEN);

    // 3. Inicializa ADC
    adc_reader_init();

    // 4. Configura botão BOOTSEL
    gpio_init(BOTAO_BOOTSEL);
    gpio_set_dir(BOTAO_BOOTSEL, GPIO_IN);
    gpio_pull_up(BOTAO_BOOTSEL);
    gpio_set_irq_enabled_with_callback(BOTAO_BOOTSEL, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // 5. Inicia renderização (Core 1)
    display_start_rendering();

    // 6. Loop principal
    const uint center_y = (CHAR_ROWS / 2) - 1;
    char buffer[80];

    while (true)
    {
        uint16_t adc_value = adc_reader_read();

        // Limpa linha central
        terminal_clear_line(center_y, COLOR_BLACK);

        // Monta mensagem
        sprintf(buffer, "A Entrada ANALOGICA Lida Pelo ADC Foi: %d", adc_value);

        // Escreve centralizado
        terminal_write_centered(center_y, buffer, COLOR_WHITE, COLOR_BLUE);

        sleep_ms(100);
    }
}
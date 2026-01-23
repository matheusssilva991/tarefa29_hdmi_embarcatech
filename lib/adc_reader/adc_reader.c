#include "adc_reader.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

void adc_reader_init(void) {
    adc_init();
    adc_gpio_init(ADC_GPIO);
    adc_select_input(ADC_CHANNEL);
}

uint16_t adc_reader_read(void) {
    return adc_read();
}
#ifndef ADC_READER_H
#define ADC_READER_H

#include <stdint.h>

#define ADC_GPIO 27
#define ADC_CHANNEL 1

/**
 * @brief Inicializa o ADC
 */
void adc_reader_init(void);

/**
 * @brief Lê o valor do ADC
 * @return Valor de 0 a 4095
 */
uint16_t adc_reader_read(void);

#endif // ADC_READER_H
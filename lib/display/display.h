#ifndef DISPLAY_H
#define DISPLAY_H

#include "dvi.h"
#include "terminal.h"

/**
 * @brief Inicializa o sistema de display DVI
 * @param voltage Voltagem do VREG
 * @param dvi_pins Configuração dos pinos DVI
 */
void display_init(uint voltage, const struct dvi_serialiser_cfg *dvi_pins);

/**
 * @brief Inicia a renderização no Core 1
 */
void display_start_rendering(void);

/**
 * @brief Obtém ponteiro para a instância DVI
 * @return Ponteiro para struct dvi_inst
 */
struct dvi_inst* display_get_dvi_instance(void);

#endif // DISPLAY_H
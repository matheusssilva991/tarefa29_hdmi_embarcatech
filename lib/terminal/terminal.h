#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include "pico/types.h" // Inclui definição de 'uint' do Pico SDK

// Configurações do terminal
#define FONT_CHAR_WIDTH 8
#define FONT_CHAR_HEIGHT 24
#define FONT_ORIGINAL_HEIGHT 8
#define FONT_SCALE_FACTOR 3

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480

#define CHAR_COLS (FRAME_WIDTH / FONT_CHAR_WIDTH)   // 80
#define CHAR_ROWS (FRAME_HEIGHT / FONT_CHAR_HEIGHT) // 20

#define COLOUR_PLANE_SIZE_WORDS (CHAR_ROWS * CHAR_COLS * 4 / 32)

// Cores pré-definidas (RGB222)
#define COLOR_BLACK 0x00
#define COLOR_RED 0x30
#define COLOR_GREEN 0x0C
#define COLOR_BLUE 0x03
#define COLOR_YELLOW 0x3C
#define COLOR_WHITE 0x3F
#define COLOR_GRAY 0x15

// Buffers globais
extern char charbuf[CHAR_ROWS * CHAR_COLS];
extern uint32_t colourbuf[3 * COLOUR_PLANE_SIZE_WORDS];

/**
 * @brief Inicializa o terminal (limpa buffers)
 */
void terminal_init(void);

/**
 * @brief Define um caractere em uma posição
 */
void terminal_set_char(uint x, uint y, char c);

/**
 * @brief Define a cor de um caractere
 */
void terminal_set_colour(uint x, uint y, uint8_t fg, uint8_t bg);

/**
 * @brief Limpa toda a tela
 */
void terminal_clear(uint8_t bg);
/**
 * @brief Limpa uma linha específica
 */
void terminal_clear_line(uint y, uint8_t bg);

/**
 * @brief Desenha uma borda ao redor da tela
 */
void terminal_draw_border(uint8_t fg, uint8_t bg);

/**
 * @brief Escreve uma string centralizada
 */
void terminal_write_centered(uint y, const char *text, uint8_t fg, uint8_t bg);

/**
 * @brief Escreve uma string em uma posição
 */
void terminal_write(uint x, uint y, const char *text, uint8_t fg, uint8_t bg);

#endif // TERMINAL_H
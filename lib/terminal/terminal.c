#include "terminal.h"
#include <string.h>

// Buffers
char charbuf[CHAR_ROWS * CHAR_COLS];
uint32_t colourbuf[3 * COLOUR_PLANE_SIZE_WORDS];

void terminal_init(void)
{
    terminal_clear(COLOR_BLACK);
}

void terminal_set_char(uint x, uint y, char c)
{
    if (x >= CHAR_COLS || y >= CHAR_ROWS)
        return;
    charbuf[x + y * CHAR_COLS] = c;
}

void terminal_set_colour(uint x, uint y, uint8_t fg, uint8_t bg)
{
    if (x >= CHAR_COLS || y >= CHAR_ROWS)
        return;

    uint char_index = x + y * CHAR_COLS;
    uint bit_index = char_index % 8 * 4;
    uint word_index = char_index / 8;

    for (int plane = 0; plane < 3; ++plane)
    {
        uint32_t fg_bg_combined = (fg & 0x3) | (bg << 2 & 0xc);
        colourbuf[word_index] = (colourbuf[word_index] & ~(0xfu << bit_index)) |
                                (fg_bg_combined << bit_index);
        fg >>= 2;
        bg >>= 2;
        word_index += COLOUR_PLANE_SIZE_WORDS;
    }
}

void terminal_clear(uint8_t bg)
{
    for (uint y = 0; y < CHAR_ROWS; ++y)
    {
        for (uint x = 0; x < CHAR_COLS; ++x)
        {
            terminal_set_char(x, y, ' ');
            terminal_set_colour(x, y, COLOR_BLACK, bg);
        }
    }
}

void terminal_clear_line(uint y, uint8_t bg)
{
    for (uint x = 0; x < CHAR_COLS; ++x)
    {
        terminal_set_char(x, y, ' ');
        terminal_set_colour(x, y, COLOR_BLACK, bg);
    }
}

void terminal_draw_border(uint8_t fg, uint8_t bg)
{
    // Cantos
    terminal_set_char(0, 0, '+');
    terminal_set_colour(0, 0, fg, bg);
    terminal_set_char(CHAR_COLS - 1, 0, '+');
    terminal_set_colour(CHAR_COLS - 1, 0, fg, bg);
    terminal_set_char(0, CHAR_ROWS - 1, '+');
    terminal_set_colour(0, CHAR_ROWS - 1, fg, bg);
    terminal_set_char(CHAR_COLS - 1, CHAR_ROWS - 1, '+');
    terminal_set_colour(CHAR_COLS - 1, CHAR_ROWS - 1, fg, bg);

    // Linhas horizontais
    for (uint x = 1; x < CHAR_COLS - 1; ++x)
    {
        terminal_set_char(x, 0, '-');
        terminal_set_colour(x, 0, fg, bg);
        terminal_set_char(x, CHAR_ROWS - 1, '-');
        terminal_set_colour(x, CHAR_ROWS - 1, fg, bg);
    }

    // Linhas verticais
    for (uint y = 1; y < CHAR_ROWS - 1; ++y)
    {
        terminal_set_char(0, y, '|');
        terminal_set_colour(0, y, fg, bg);
        terminal_set_char(CHAR_COLS - 1, y, '|');
        terminal_set_colour(CHAR_COLS - 1, y, fg, bg);
    }
}

void terminal_write_centered(uint y, const char *text, uint8_t fg, uint8_t bg)
{
    int len = strlen(text);
    int start_x = (CHAR_COLS / 2) - (len / 2);
    terminal_write(start_x, y, text, fg, bg);
}

void terminal_write(uint x, uint y, const char *text, uint8_t fg, uint8_t bg)
{
    int len = strlen(text);
    for (int i = 0; i < len && (x + i) < CHAR_COLS; ++i)
    {
        terminal_set_char(x + i, y, text[i]);
        terminal_set_colour(x + i, y, fg, bg);
    }
}
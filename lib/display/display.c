#include "display.h"
#include "pico/multicore.h"
#include "hardware/structs/bus_ctrl.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "dvi_serialiser.h"
#include "tmds_encode_font_2bpp.h"
#include "./assets/font_teste.h"

#define FONT_N_CHARS 95
#define FONT_FIRST_ASCII 32
#define DVI_TIMING dvi_timing_640x480p_60hz

static struct dvi_inst dvi0;

// Função do Core 1
static void core1_main(void) {
    dvi_register_irqs_this_core(&dvi0, DMA_IRQ_0);
    dvi_start(&dvi0);

    while (true) {
        for (uint y = 0; y < FRAME_HEIGHT; ++y) {
            uint font_row = (y % FONT_CHAR_HEIGHT) / FONT_SCALE_FACTOR;

            uint32_t *tmdsbuf;
            queue_remove_blocking(&dvi0.q_tmds_free, &tmdsbuf);

            for (int plane = 0; plane < 3; ++plane) {
                tmds_encode_font_2bpp(
                    (const uint8_t*)&charbuf[y / FONT_CHAR_HEIGHT * CHAR_COLS],
                    &colourbuf[y / FONT_CHAR_HEIGHT * (COLOUR_PLANE_SIZE_WORDS / CHAR_ROWS) +
                               plane * COLOUR_PLANE_SIZE_WORDS],
                    tmdsbuf + plane * (FRAME_WIDTH / DVI_SYMBOLS_PER_WORD),
                    FRAME_WIDTH,
                    (const uint8_t*)&font_8x8[font_row * FONT_N_CHARS] - FONT_FIRST_ASCII
                );
            }
            queue_add_blocking(&dvi0.q_tmds_valid, &tmdsbuf);
        }
    }
}

void display_init(uint voltage, const struct dvi_serialiser_cfg *dvi_pins) {
    vreg_set_voltage(voltage);
    sleep_ms(10);
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    dvi0.timing = &DVI_TIMING;
    dvi0.ser_cfg = *dvi_pins;
    dvi_init(&dvi0, next_striped_spin_lock_num(), next_striped_spin_lock_num());
}

void display_start_rendering(void) {
    hw_set_bits(&bus_ctrl_hw->priority, BUSCTRL_BUS_PRIORITY_PROC1_BITS);
    multicore_launch_core1(core1_main);
}

struct dvi_inst* display_get_dvi_instance(void) {
    return &dvi0;
}
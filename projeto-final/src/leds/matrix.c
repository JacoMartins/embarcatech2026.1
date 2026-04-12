/**
 * @file    matrix.c
 * @brief   Driver da matriz de LEDs WS2812B 5×5 (25 LEDs, pino GP7)
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/stdlib.h"
#include "ws2812.pio.h"
#include "matrix.h"

// Pino de dados da matriz WS2812B (conforme esquemático BitDogLab v5.3)
#define MATRIX_PIN      7
#define MATRIX_NUM_LEDS 25
#define MATRIX_PIO      pio0
#define MATRIX_SM       0

// Brilho reduzido para não machucar os olhos (40 / 255 ≈ 15%)
#define DIM 40

// Converte componentes R, G, B para o formato de 32 bits GRB do WS2812B.
// O WS2812B espera os dados na ordem Verde → Vermelho → Azul.
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

// Envia um pixel para a state machine PIO.
// Desloca 8 bits para alinhar os 24 bits de cor nos MSBs do registrador de 32 bits.
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(MATRIX_PIO, MATRIX_SM, pixel_grb << 8u);
}

void matrix_init(void) {
    uint offset = pio_add_program(MATRIX_PIO, &ws2812_program);
    ws2812_program_init(MATRIX_PIO, MATRIX_SM, offset, MATRIX_PIN, 800000.0f, false);

    // apaga todos os LEDs (envia zero para cada um dos 25 LEDs)
    for (int i = 0; i < MATRIX_NUM_LEDS; i++) put_pixel(0);
    sleep_ms(1);  // pulso de reset do WS2812B (mínimo 50 µs)
}

// Atualiza os 25 LEDs da matriz com base no estado atual dos três relés.
// Posições escolhidas para separação visual:
//   LED  0 → canto superior esquerdo → RELAY_LUZ     (amarelo)
//   LED 12 → centro da matriz        → RELAY_ENTRADA_1 (verde)
//   LED 24 → canto inferior direito  → RELAY_ENTRADA_2 (azul)
void matrix_update_relays(bool luz, bool entrada1, bool entrada2) {
    for (int i = 0; i < MATRIX_NUM_LEDS; i++) {
        uint32_t color = 0;

        if (i == 0  && luz)     color = urgb_u32(DIM, DIM, 0);    // amarelo
        if (i == 12 && entrada1) color = urgb_u32(0,   DIM, 0);   // verde
        if (i == 24 && entrada2) color = urgb_u32(0,   0,   DIM); // azul

        put_pixel(color);
    }
    // sem sleep_us() — o gap natural entre chamadas supera os 50 µs de reset
}

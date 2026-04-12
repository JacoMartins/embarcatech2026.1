/**
 * @file    matrix.h
 * @brief   Interface pública do driver da matriz de LEDs WS2812B (5x5, GP7)
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <stdbool.h>

// Inicializa o PIO e apaga todos os LEDs da matriz
void matrix_init(void);

// Atualiza a matriz refletindo o estado dos três relés.
// Seguro para chamar de qualquer contexto (usa pio_sm_put_blocking).
void matrix_update_relays(bool luz, bool entrada1, bool entrada2);

#endif

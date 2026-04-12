/**
 * @file    buttons.h
 * @brief   Interface pública do módulo de botões com interrupção e debounce
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdbool.h>

// Flag definida como true pelo IRQ quando um botão é pressionado.
// O loop principal deve verificar e imprimir o estado no OLED quando estiver verdadeira.
extern volatile bool button_oled_dirty;

// Flag definida como true pelo IRQ do joystick click (GP22).
// O loop principal deve disparar o buzzer quando estiver verdadeira.
extern volatile bool buzzer_pending;

// Configura GP5, GP6 e GP22 como entradas com pull-up e registra o callback de IRQ
void buttons_init(void);

#endif

/**
 * @file    buzzer.h
 * @brief   Interface do buzzer passivo via PWM (GP21)
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

// Pino do buzzer passivo na BitDogLab V6
#define BUZZER_PIN 21

// Inicializa o PWM no pino do buzzer (saída silenciosa até buzzer_beep)
void buzzer_init(void);

// Emite um beep com a frequência e duração especificadas, depois silencia.
// freq_hz : frequência em Hz (ex: 750)
// duration_ms : duração em milissegundos
void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms);

#endif

/**
 * @file    microphone.h
 * @brief   Interface pública do módulo de captura do microfone
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <stdint.h>

// Pino ADC do microfone no BitDogLab V6
#define MICROPHONE_PIN      28  // ADC2

// Taxa de amostragem e duração da captura
#define AUDIO_SAMPLE_RATE   8000    // 8 kHz — qualidade de voz
#define AUDIO_DURATION_S    5       // segundos
#define AUDIO_BUFFER_SIZE   (AUDIO_SAMPLE_RATE * AUDIO_DURATION_S)  // 40.000 amostras

// Inicializa o pino ADC do microfone
void microphone_init();

// Captura AUDIO_BUFFER_SIZE amostras via DMA e armazena em buffer.
// Bloqueia até a captura completar (5 segundos).
void microphone_capture(uint8_t *buffer);

#endif

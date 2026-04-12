/**
 * @file    encoder.h
 * @brief   Interface pública do codificador de áudio — empacotamento WAV
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "sensors/microphone.h"

// Tamanho fixo do cabeçalho WAV padrão (RIFF/WAVE/fmt/data)
#define WAV_HEADER_SIZE  44
// Tamanho total do arquivo: cabeçalho + amostras de áudio (~40 KB)
#define WAV_TOTAL_SIZE   (WAV_HEADER_SIZE + AUDIO_BUFFER_SIZE)

// Captura AUDIO_DURATION_S segundos do microfone e monta o arquivo WAV
// completo no buffer interno. Bloqueia durante a captura (5 segundos).
void audio_capture_wav();

// Retorna ponteiro para o buffer WAV interno.
// Válido somente após chamada a audio_capture_wav().
const uint8_t *audio_get_buffer();

#endif

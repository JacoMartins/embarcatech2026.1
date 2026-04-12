/**
 * @file    encoder.c
 * @brief   Captura de áudio do microfone e empacotamento em formato WAV
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <string.h>
#include "encoder.h"
#include "sensors/microphone.h"

// Representa os 44 bytes do cabeçalho WAV padrão (RIFF/WAVE PCM não comprimido).
// __attribute__((packed)) garante que o compilador não insira bytes de padding.
typedef struct __attribute__((packed)) {
    char     riff_id[4];        // "RIFF"
    uint32_t riff_size;         // tamanho total do arquivo - 8 bytes
    char     wave_id[4];        // "WAVE"
    char     fmt_id[4];         // "fmt "
    uint32_t fmt_size;          // 16 para PCM não comprimido
    uint16_t audio_format;      // 1 = PCM sem compressão
    uint16_t num_channels;      // 1 = mono
    uint32_t sample_rate;       // 8000 Hz
    uint32_t byte_rate;         // bytes por segundo (sample_rate × canais × bits/8)
    uint16_t block_align;       // bytes por amostra completa (canais × bits/8)
    uint16_t bits_per_sample;   // 8 bits por amostra
    char     data_id[4];        // "data"
    uint32_t data_size;         // tamanho dos dados de áudio em bytes
} WavHeader;

// Buffer estático que contém o arquivo WAV completo (cabeçalho + amostras).
// Declarado estático para não ocupar a pilha (~40 KB no segmento BSS).
static uint8_t wav_buffer[WAV_TOTAL_SIZE];

// Preenche os primeiros 44 bytes do wav_buffer com um cabeçalho WAV válido
static void build_wav_header(uint32_t data_size)
{
    WavHeader h;
    memcpy(h.riff_id,  "RIFF", 4);
    h.riff_size       = 36 + data_size;
    memcpy(h.wave_id,  "WAVE", 4);
    memcpy(h.fmt_id,   "fmt ", 4);
    h.fmt_size        = 16;
    h.audio_format    = 1;
    h.num_channels    = 1;
    h.sample_rate     = AUDIO_SAMPLE_RATE;
    h.byte_rate       = AUDIO_SAMPLE_RATE;  // 8000 Hz × 1 canal × 1 byte
    h.block_align     = 1;
    h.bits_per_sample = 8;
    memcpy(h.data_id,  "data", 4);
    h.data_size       = data_size;
    memcpy(wav_buffer, &h, WAV_HEADER_SIZE);
}

void audio_capture_wav()
{
    // escreve o cabeçalho WAV nos primeiros 44 bytes
    build_wav_header(AUDIO_BUFFER_SIZE);
    // captura as amostras via DMA diretamente após o cabeçalho
    microphone_capture(wav_buffer + WAV_HEADER_SIZE);
}

const uint8_t *audio_get_buffer()
{
    return wav_buffer;
}

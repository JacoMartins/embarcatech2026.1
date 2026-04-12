/**
 * @file    microphone.c
 * @brief   Captura de áudio do microfone analógico via ADC + DMA
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "microphone.h"
#include "display.h"


// Inicializa o ADC no pino do microfone (GP28 → ADC2).
// Configura o ADC para operar em modo FIFO, necessário para uso com DMA.
void microphone_init()
{
    adc_init();
    adc_gpio_init(MICROPHONE_PIN);
    adc_select_input(2);

    // habilita o FIFO do ADC: cada leitura gera uma entrada no FIFO
    // shift=true reduz os 12 bits para 8 bits (economiza memória no buffer)
    adc_fifo_setup(
        true,   // habilita FIFO
        true,   // habilita requisição DMA
        1,      // dispara DMA a cada 1 amostra
        false,  // não sinaliza erro de overflow
        true    // shift para 8 bits (descarta os 4 bits menos significativos)
    );

    // define a taxa de amostragem: clock do ADC é 48 MHz
    // divisor = 48.000.000 / 8.000 = 6.000
    adc_set_clkdiv(6000.0f);
    print_oled("[OK] Mic: GP28+DMA.");
}


// Captura AUDIO_BUFFER_SIZE amostras (5 segundos a 8 kHz) via DMA.
// O DMA transfere as amostras diretamente do FIFO do ADC para o buffer
// sem ocupar o processador durante a captura.
// Esta função bloqueia até a transferência completar.
void microphone_capture(uint8_t *buffer)
{
    print_oled("[...] Capturando 5s.");

    // requisita um canal DMA livre
    int dma_chan = dma_claim_unused_channel(true);

    // configura a transferência DMA
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);   // 8 bits por amostra
    channel_config_set_read_increment(&cfg, false);             // origem fixa (FIFO do ADC)
    channel_config_set_write_increment(&cfg, true);             // destino avança no buffer
    channel_config_set_dreq(&cfg, DREQ_ADC);                    // disparo sincronizado com o ADC

    dma_channel_configure(
        dma_chan,
        &cfg,
        buffer,             // destino: buffer fornecido pelo chamador
        &adc_hw->fifo,      // origem: FIFO do ADC
        AUDIO_BUFFER_SIZE,  // número de amostras a capturar
        true                // inicia imediatamente
    );

    // inicia o ADC em modo contínuo e aguarda o DMA terminar
    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_chan);
    adc_run(false);
    adc_fifo_drain();

    // libera o canal DMA para outros usos
    dma_channel_unclaim(dma_chan);
    print_oled("[OK] Audio capturado.");
}

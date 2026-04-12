/**
 * @file    temperature.c
 * @brief   Leitura do sensor de temperatura interno do RP2040
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include "hardware/adc.h"
#include "temperature.h"
#include "display.h"


// Habilita o ADC e o sensor de temperatura interno do chip.
// O sensor está ligado ao canal ADC4 (sem pino físico externo).
void temperature_init()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    print_oled("[OK] Temp: ADC4.");
}


// Lê o canal ADC4 e converte para graus Celsius usando a fórmula
// oficial do datasheet do RP2040:
//   T = 27 - (tensão - 0.706) / 0.001721
float temperature_read_celsius()
{
    adc_select_input(4);
    uint16_t raw = adc_read();

    // converte a leitura de 12 bits para tensão (referência de 3.3V)
    float voltage = raw * 3.3f / 4095.0f;

    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

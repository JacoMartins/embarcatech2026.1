/**
 * @file    joystick.c
 * @brief   Leitura do joystick analógico via ADC (simula sensor ACS71240)
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include "hardware/adc.h"
#include "joystick.h"
#include "display.h"


// Inicializa o ADC e os dois pinos do joystick.
// GP26 → eixo X (ADC0), GP27 → eixo Y (ADC1)
void joystick_init()
{
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    print_oled("[OK] Joystick GP26/27");
}


// Lê os dois eixos do joystick em sequência e retorna as leituras brutas.
// Valores entre 0 e 4095 (resolução de 12 bits do ADC).
JoystickReading joystick_read()
{
    JoystickReading reading;

    // seleciona o canal ADC0 para o eixo X e faz a leitura
    adc_select_input(0);
    reading.x = adc_read();

    // seleciona o canal ADC1 para o eixo Y e faz a leitura
    adc_select_input(1);
    reading.y = adc_read();

    return reading;
}

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pwm.h"


const uint LED_R = 13;
const uint LED_G = 11;
const uint LED_B = 12;
const uint16_t PERIOD = 1000;    // periodo PWM (valor maximo do contador)
const float DIVIDER_PWM = 12.5; // divisor fracional do clock para o PWM
const uint16_t LED_B_STEP = 50;  // passo de incremento/decremento para o duty cycle do LED
uint16_t led_b_level = 50;       // nivel inicial do duty cycle


bool timer_callback(struct repeating_timer *t) {
    // codigo que rodará toda vida que o timer estiver off, importante no uso de interruptores.
    bool reached_max_brightness = led_b_level == PERIOD;
    pwm_set_gpio_level(LED_B, led_b_level);
    
    if (!reached_max_brightness)
    {
        led_b_level += LED_B_STEP;
    }
    else
    {
        led_b_level = 50;
    }
    
    return true;
}


void setup_pwm()
{
    uint slice;
    gpio_set_function(LED_B, GPIO_FUNC_PWM); // configura o pino do LED para a função PWM
    slice = pwm_gpio_to_slice_num(LED_B);    // obtem o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, DIVIDER_PWM);      // define o valor do divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD);             // configura o valor maximo do contador (periodo do PWM)
    pwm_set_gpio_level(LED_B, led_b_level);  // define o nivel inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);            // habilita o PWM no slice correspondente
}


int main()
{
    stdio_init_all();
    setup_pwm();
    struct repeating_timer timer;

    add_repeating_timer_ms(2000, timer_callback, NULL, &timer);
    
    while (true)
    {
    }
}

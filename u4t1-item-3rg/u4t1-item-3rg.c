#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pwm.h"


const uint LED_R = 13;
const uint LED_G = 11;

const uint16_t PERIOD = 1000;    // periodo PWM (valor maximo do contador)
const float DIVIDER_PWM_R = 125.0; // divisor fracional do clock para o PWM
const float DIVIDER_PWM_G = 12.5; // divisor fracional do clock para o PWM
const uint16_t LED_STEP = 50;  // passo de incremento/decremento para o duty cycle do LED

uint16_t led_r_level = 50;       // nivel inicial do duty cycle
uint16_t led_g_level = 0;       // nivel inicial do duty cycle


bool timer_callback(struct repeating_timer *t) {
    // codigo que rodará toda vida que o timer estiver off, importante no uso de interruptores.
    bool led_r_reached_max_brightness = led_r_level == PERIOD;
    bool led_g_reached_max_brightness = led_g_level == PERIOD;
    pwm_set_gpio_level(LED_R, led_r_level);
    pwm_set_gpio_level(LED_G, led_g_level);

    if (!led_r_reached_max_brightness)
    {
        led_r_level += LED_STEP;
    }
    else
    {
        led_r_level = 50;
    }
    
    if (!led_g_reached_max_brightness)
    {
        led_g_level += LED_STEP;
    }
    else
    {
        led_g_level = 0;
    }
    
    return true;
}


void setup_pwm_led_r()
{
    uint slice;
    gpio_set_function(LED_R, GPIO_FUNC_PWM); // configura o pino do LED para a função PWM
    slice = pwm_gpio_to_slice_num(LED_R);    // obtem o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, DIVIDER_PWM_R);      // define o valor do divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD);             // configura o valor maximo do contador (periodo do PWM)
    pwm_set_gpio_level(LED_R, led_r_level);  // define o nivel inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);            // habilita o PWM no slice correspondente
}


void setup_pwm_led_g()
{
    uint slice;
    gpio_set_function(LED_G, GPIO_FUNC_PWM); // configura o pino do LED para a função PWM
    slice = pwm_gpio_to_slice_num(LED_G);    // obtem o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, DIVIDER_PWM_G);      // define o valor do divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD);             // configura o valor maximo do contador (periodo do PWM)
    pwm_set_gpio_level(LED_G, led_g_level);  // define o nivel inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true);            // habilita o PWM no slice correspondente
}


int main()
{
    stdio_init_all();
    setup_pwm_led_r();
    setup_pwm_led_g();
    struct repeating_timer timer;

    add_repeating_timer_ms(2000, timer_callback, NULL, &timer);
    
    while (true)
    {
    }
}

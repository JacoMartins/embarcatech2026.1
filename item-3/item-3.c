#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

const uint LED = 12;
const uint16_t PERIOD = 2000; // periodo PWM (valor maximo do contador)
const float DIVIDER_PWM = 16.0; // divisor fracional do clock para o PWM
const uint16_t LED_STEP = 100; // passo de incremento/decremento para o duty cycle do LED
uint16_t led_level = 100; // nivel inicial do duty cycle

void setup_pwm()
{
    uint slice;
    gpio_set_function(LED, GPIO_FUNC_PWM); // configura o pino do LED para a função PWM
    slice = pwm_gpio_to_slice_num(LED); // obtem o slice do PWM associado ao pino do LED
    pwm_set_clkdiv(slice, DIVIDER_PWM); // define o valor do divisor de clock do PWM
    pwm_set_wrap(slice, PERIOD); // configura o valor maximo do contador (periodo do PWM)
    pwm_set_gpio_level(LED, led_level); // define o nivel inicial do PWM para o pino do LED
    pwm_set_enabled(slice, true); // habilita o PWM no slice correspondente
}

int main()
{
    uint up_down = 1;
    stdio_init_all();
    setup_pwm();

    while (true) {
        pwm_set_gpio_level(LED, led_level);
        sleep_ms(1000);

        if(up_down)
        {
            led_level += LED_STEP;
            
            if (led_level >= PERIOD)
                up_down = 0;
        } else {
            led_level -= LED_STEP;
            
            if (led_level <= PERIOD)
                up_down = 1;
        }
    }
}

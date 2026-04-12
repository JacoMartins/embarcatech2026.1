/**
 * @file    buzzer.c
 * @brief   Buzzer passivo via PWM — GP21, BitDogLab V6
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include "buzzer.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

void buzzer_init(void)
{
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Inicia com PWM desligado (duty cycle 0)
    pwm_set_enabled(slice, false);
}

void buzzer_beep(uint32_t freq_hz, uint32_t duration_ms)
{
    if (freq_hz == 0) return;

    uint slice   = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN);

    // O contador PWM é 16 bits (wrap máx = 65535).
    // Com clkdiv=1 qualquer frequência < ~1907 Hz gera wrap > 65535 →
    // truncamento → duty > wrap → saída sempre HIGH → apenas cliques, sem tom.
    // clkdiv=125 reduz o tick para 1 MHz: wrap = 1_000_000/freq − 1,
    // o que cabe em 16 bits para toda a faixa audível (20 Hz – 20 kHz).
    const float   clkdiv = 125.0f;
    uint16_t wrap = (uint16_t)(1000000u / freq_hz - 1);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, wrap);
    pwm_config_set_clkdiv(&cfg, clkdiv);
    pwm_init(slice, &cfg, false);

    // 50 % de duty cycle — soa melhor em buzzers passivos
    pwm_set_chan_level(slice, channel, (uint16_t)((wrap + 1) / 2));

    pwm_set_enabled(slice, true);
    sleep_ms(duration_ms);
    pwm_set_enabled(slice, false);

    // Deixa o pino em baixo para não zumbir
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);
    // Restaura função PWM para próxima chamada
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
}

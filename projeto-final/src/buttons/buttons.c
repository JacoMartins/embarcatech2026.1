/**
 * @file    buttons.c
 * @brief   Interrupção de hardware nos botões A e B do BitDogLab com debounce por software
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include "hardware/gpio.h"
#include "pico/time.h"
#include "buttons.h"
#include "relays/relays.h"
#include "leds/matrix.h"

// Botão A (GP5)       → alterna RELAY_ENTRADA_1
// Botão B (GP6)       → alterna RELAY_ENTRADA_2
// Joystick click (GP22) → alterna RELAY_LUZ
// Pinos confirmados pelo esquemático BitDogLab SMD v5.3
#define BUTTON_A_PIN      5
#define BUTTON_B_PIN      6
#define JOYSTICK_BTN_PIN  22

// Janela de debounce: ignora pressões em menos de 200 ms após a última válida
#define DEBOUNCE_MS  200

// Marca o instante da última pressão válida de cada botão (em ms desde o boot)
static uint32_t last_press_a   = 0;
static uint32_t last_press_b   = 0;
static uint32_t last_press_joy = 0;

// Sinaliza para o loop principal que o OLED precisa ser atualizado.
// Não chamamos print_oled() direto do IRQ pois ele usa I2C, que não é reentrante.
volatile bool button_oled_dirty = false;

// Sinaliza para o loop principal que deve disparar o buzzer (750 Hz).
// buzzer_beep usa sleep_ms, que não pode ser chamado de dentro de um IRQ.
volatile bool buzzer_pending = false;

// Callback único para toda a GPIO bank — o RP2040 permite apenas um por core.
// O parâmetro gpio identifica qual pino disparou o evento.
static void gpio_irq_callback(uint gpio, uint32_t events) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (gpio == BUTTON_A_PIN && (now - last_press_a) > DEBOUNCE_MS) {
        last_press_a = now;

        // alterna estado do relé — versão IRQ-safe (sem I2C / sem print_oled)
        relay_toggle_irq_safe(RELAY_ENTRADA_1);

        // atualiza a matriz de LEDs imediatamente (PIO é seguro em IRQ)
        matrix_update_relays(
            relay_get(RELAY_LUZ),
            relay_get(RELAY_ENTRADA_1),
            relay_get(RELAY_ENTRADA_2)
        );

        button_oled_dirty = true;
        buzzer_pending    = true;  // disparo adiado: main loop chama buzzer_beep
    }

    if (gpio == BUTTON_B_PIN && (now - last_press_b) > DEBOUNCE_MS) {
        last_press_b = now;

        relay_toggle_irq_safe(RELAY_ENTRADA_2);

        matrix_update_relays(
            relay_get(RELAY_LUZ),
            relay_get(RELAY_ENTRADA_1),
            relay_get(RELAY_ENTRADA_2)
        );

        button_oled_dirty = true;
        buzzer_pending    = true;  // disparo adiado: main loop chama buzzer_beep
    }

    if (gpio == JOYSTICK_BTN_PIN && (now - last_press_joy) > DEBOUNCE_MS) {
        last_press_joy = now;

        // alterna o relé de luz — único relé sem botão dedicado
        relay_toggle_irq_safe(RELAY_LUZ);

        matrix_update_relays(
            relay_get(RELAY_LUZ),
            relay_get(RELAY_ENTRADA_1),
            relay_get(RELAY_ENTRADA_2)
        );

        button_oled_dirty = true;
        buzzer_pending    = true;  // disparo adiado: main loop chama buzzer_beep
    }
}

void buttons_init(void) {
    // Botão A — GP5, entrada com pull-up (nível baixo quando pressionado)
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    // Botão B — GP6, entrada com pull-up
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Joystick click — GP22, entrada com pull-up (nível baixo quando pressionado)
    gpio_init(JOYSTICK_BTN_PIN);
    gpio_set_dir(JOYSTICK_BTN_PIN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN_PIN);

    // Registra o callback e habilita IRQ na borda de descida (botão pressionado)
    // gpio_set_irq_enabled_with_callback registra o callback global para a GPIO bank;
    // os demais pinos reutilizam o mesmo callback via gpio_set_irq_enabled.
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN,      GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
    gpio_set_irq_enabled(BUTTON_B_PIN,      GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(JOYSTICK_BTN_PIN,  GPIO_IRQ_EDGE_FALL, true);
}

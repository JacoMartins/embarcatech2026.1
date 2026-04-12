/**
 * @file    relays.c
 * @brief   Controle de relés simulados por LEDs GPIO no BitDogLab V6
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <stdio.h>
#include "hardware/gpio.h"
#include "relays.h"
#include "display.h"

// Nomes legíveis de cada relé — usados nas mensagens do display
static const char *relay_names[RELAY_COUNT] = { "Luz", "Entrada1", "Entrada2" };

// Mapeamento de RelayId para pino GPIO
static const uint relay_pins[RELAY_COUNT] = {
    RELAY_LUZ_PIN,
    RELAY_ENTRADA_1_PIN,
    RELAY_ENTRADA_2_PIN
};

// Estado atual de cada relé (false = desligado, true = ligado)
static bool relay_states[RELAY_COUNT] = { false, false, false };

// Configura todos os pinos como saída digital e garante estado inicial desligado
void relays_init(void) {
    for (int i = 0; i < RELAY_COUNT; i++) {
        gpio_init(relay_pins[i]);
        gpio_set_dir(relay_pins[i], GPIO_OUT);
        gpio_put(relay_pins[i], 0);
    }
    print_oled("[OK] Reles: GPIO.");
}

// Liga (on=true) ou desliga (on=false) o relé, atualiza estado interno e OLED.
// Não chamar de IRQ — usa I2C via print_oled.
void relay_set(RelayId id, bool on) {
    relay_states[id] = on;
    gpio_put(relay_pins[id], on ? 1 : 0);

    char msg[22];
    snprintf(msg, sizeof(msg), "[Rele %s] %s",
        relay_names[id], on ? "ON" : "OFF");
    print_oled(msg);
}

// Alterna o estado do relé — seguro para chamar de IRQ.
// Atualiza apenas estado interno e GPIO; não chama print_oled (I2C).
void relay_toggle_irq_safe(RelayId id) {
    relay_states[id] = !relay_states[id];
    gpio_put(relay_pins[id], relay_states[id] ? 1 : 0);
}

// Retorna o último estado registrado do relé sem acessar o hardware
bool relay_get(RelayId id) {
    return relay_states[id];
}

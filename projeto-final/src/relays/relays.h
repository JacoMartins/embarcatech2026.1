/**
 * @file    relays.h
 * @brief   Interface pública do módulo de relés — simulados por LEDs GPIO
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef RELAYS_H
#define RELAYS_H

#include <stdbool.h>

// Pinos dos LEDs onboard que simulam os relés no BitDogLab V6
// GP11 → LED Azul  (Luz)
// GP12 → LED Vermelho (Entrada 1)
// GP13 → LED Verde    (Entrada 2)
#define RELAY_LUZ_PIN       11
#define RELAY_ENTRADA_1_PIN 12
#define RELAY_ENTRADA_2_PIN 13

#define RELAY_COUNT  3

// Identificadores dos relés
typedef enum {
    RELAY_LUZ       = 0,
    RELAY_ENTRADA_1 = 1,
    RELAY_ENTRADA_2 = 2
} RelayId;

// Inicializa os pinos GPIO de todos os relés como saída e os desliga
void relays_init(void);

// Liga ou desliga o relé — atualiza GPIO e imprime no OLED
// NÃO chamar de dentro de um IRQ (usa I2C)
void relay_set(RelayId id, bool on);

// Alterna o estado do relé — versão segura para IRQ
// Atualiza apenas o estado interno e o GPIO, sem usar I2C
void relay_toggle_irq_safe(RelayId id);

// Retorna o estado atual do relé sem acessar o hardware
bool relay_get(RelayId id);

#endif

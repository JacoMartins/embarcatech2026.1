/**
 * @file    joystick.h
 * @brief   Interface pública do módulo do joystick analógico
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>

// Pinos ADC do joystick no BitDogLab V6
#define JOYSTICK_X_PIN  26  // ADC0
#define JOYSTICK_Y_PIN  27  // ADC1

// Leitura bruta do ADC (0 a 4095 para 12 bits)
typedef struct {
    uint16_t x;
    uint16_t y;
} JoystickReading;

// Inicializa os pinos ADC do joystick
void joystick_init();

// Retorna a leitura atual dos dois eixos
JoystickReading joystick_read();

#endif

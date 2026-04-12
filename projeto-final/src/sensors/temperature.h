/**
 * @file    temperature.h
 * @brief   Interface pública do sensor de temperatura interno do RP2040
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

// Inicializa o sensor de temperatura interno do RP2040
void temperature_init();

// Retorna a temperatura em graus Celsius
float temperature_read_celsius();

#endif

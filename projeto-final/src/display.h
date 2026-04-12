/**
 * @file    display.h
 * @brief   Interface pública do gerenciador de display OLED
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef DISPLAY_H
#define DISPLAY_H

// Inicializa o display OLED e o gerenciador de linhas de texto
int initialize_display_hardware();

// Exibe uma linha de texto no display, com scroll automático quando cheio
void print_oled(const char *str);

#endif

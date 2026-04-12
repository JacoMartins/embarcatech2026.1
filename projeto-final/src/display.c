/**
 * @file    display.c
 * @brief   Gerenciador de texto para display OLED SSD1306 128x64
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <string.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "display.h"

// Altura de cada linha em pixels (8px de fonte + 2px de espaçamento)
#define DISPLAY_LINE_SIZE       10
// Número máximo de linhas visíveis no display ao mesmo tempo
#define DISPLAY_LINE_LIMIT      6

// Estrutura que representa uma linha de texto no display
typedef struct {
    int  id;        // índice da linha
    char text[50];  // conteúdo da linha
} TextLine;

// Contador de linhas atualmente exibidas
static int      line_count = 0;
// Buffer interno com todas as linhas visíveis
static TextLine lines[DISPLAY_LINE_LIMIT];


// Zera o buffer de linhas e reinicia o contador.
// Chamada internamente durante a inicialização do hardware.
static void initialize_text_manager()
{
    line_count = 0;
    memset(lines, 0, sizeof(lines));
}


// Redesenha todas as linhas do buffer no display e envia para o hardware.
// Chamada automaticamente toda vez que uma nova linha é adicionada.
static void display_update()
{
    ssd1306_clear();
    for (int i = 0; i < line_count; i++)
        ssd1306_draw_string(0, i * DISPLAY_LINE_SIZE, lines[i].text);
    ssd1306_show();
}


// Inicializa o hardware I2C e o display OLED, e prepara o gerenciador de texto.
// Deve ser chamada uma única vez no início do programa, antes de qualquer print_oled.
int initialize_display_hardware()
{
    ssd1306_init();
    initialize_text_manager();
    return 0;
}


// Exibe uma nova linha de texto no display.
// Se o display estiver cheio, as linhas sobem (scroll) e a nova entra no final.
void print_oled(const char *str)
{
    if (line_count < DISPLAY_LINE_LIMIT) {
        // ainda há espaço — adiciona a linha na próxima posição disponível
        lines[line_count].id = line_count;
        strncpy(lines[line_count].text, str, sizeof(lines[0].text) - 1);
        lines[line_count].text[sizeof(lines[0].text) - 1] = '\0';
        line_count++;
    } else {
        // display cheio — descarta a linha mais antiga e sobe todas uma posição
        for (int i = 0; i < DISPLAY_LINE_LIMIT - 1; i++)
            lines[i] = lines[i + 1];
        strncpy(lines[DISPLAY_LINE_LIMIT - 1].text, str, sizeof(lines[0].text) - 1);
        lines[DISPLAY_LINE_LIMIT - 1].text[sizeof(lines[0].text) - 1] = '\0';
    }
    display_update();
}

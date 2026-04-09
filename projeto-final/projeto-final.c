// Projeto Final - Jacó Alves Martins Barros

#include <stdio.h>
#include "hardware/i2c.h"
#include "libs/ssd1306.c"
#include "pico/stdlib.h"

#define UART0_PORT 0

const int linha_px = 14;

void print_oled(int x, int y, const char *str)
{
    ssd1306_draw_string(x, y, str);
    ssd1306_show();
}

int main()
{
    stdio_init_all();
    ssd1306_init();

    int linha = 0;
    int linha_exibivel = 0;
    int px_linha_y = 0;
    
    ssd1306_clear();
    ssd1306_draw_string(0, 0, "====Projeto Final====");
    ssd1306_show();
    
    linha++;
    linha_exibivel++;
    
    while (true)
    {

        if (linha_exibivel > 4)
        {
            px_linha_y = 0;
            linha_exibivel = 0;
            ssd1306_clear();
        }
        else
        {
            px_linha_y += linha_px;
        }

        char line_text[20];
        sprintf(line_text, "Linha %d", linha);

        print_oled(0, px_linha_y, line_text);
        
        linha++;
        linha_exibivel++;
        
        sleep_ms(1000);
    }
}

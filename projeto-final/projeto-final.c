// Projeto Final - Jacó Alves Martins Barros

#include <stdio.h>
#include "hardware/i2c.h"
#include "libs/ssd1306.c"
#include "libs/lwipopts.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#define UART0_PORT 0

#define WIFI_SSID "OI_FIBRA_Casa Nova_5G"
#define WIFI_PASSWORD "c4s4n0v4"

const int linha_px = 14;


void print_oled(int x, int y, const char *str)
{
    ssd1306_draw_string(x, y, str);
    ssd1306_show();
}


int main()
{
    int linha = 0;
    int linha_exibivel = 0;
    int px_linha_y = 0;
    
    stdio_init_all();
    ssd1306_init();

    if (cyw43_arch_init())
    {
        print_oled(0, 0, "Erro no driver Wi-Fi!");

        linha++;
        linha_exibivel++;

        return 1;
    }

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 15000)) {
        print_oled(0, 0, "Erro ao tentar Wi-Fi!");
        
        linha++;
        linha_exibivel++;
        
        return 1;
    }
    
    ssd1306_clear();
    print_oled(0, px_linha_y, "Wifi Conectado");
    
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

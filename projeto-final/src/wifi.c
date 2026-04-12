/**
 * @file    wifi.c
 * @brief   Inicialização e conexão Wi-Fi via CYW43 + lwIP
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <stdio.h>
#include "libs/lwipopts.h"
#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "pico/cyw43_arch.h"
#include "display.h"


// Inicializa o chip Wi-Fi CYW43 da Pico W.
// Retorna 0 em sucesso, 1 se o hardware não responder.
int initialize_wifi_hardware()
{
    if (cyw43_arch_init())
    {
        print_oled("[ERRO] Wi-Fi HW.");
        return 1;
    }

    print_oled("[OK] Wi-Fi: Iniciado.");
    return 0;
}


// Ativa o modo estação (STA) e tenta conectar à rede definida por
// WIFI_SSID e WIFI_PASSWORD (injetados pelo CMake em tempo de compilação).
// Aguarda até 15 segundos pela conexão.
// Retorna 0 em sucesso, 1 se o tempo esgotar.
int connect_to_network()
{
    // modo estação: a placa se conecta a um roteador existente
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 15000))
    {
        print_oled("[ERRO] Timeout.");
        return 1;
    }

    print_oled("[OK] Wi-Fi: Conectado");
    return 0;
}


// Lê o endereço IP atribuído pela rede (via DHCP) e exibe no display.
// Deve ser chamada somente após connect_to_network() ter retornado 0.
void show_ip_address()
{
    char msg[50];
    // netif_default é a interface de rede ativa gerenciada pelo lwIP
    snprintf(msg, sizeof(msg), "IP: %s", ipaddr_ntoa(&netif_default->ip_addr));
    print_oled(msg);
}

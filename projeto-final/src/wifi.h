/**
 * @file    wifi.h
 * @brief   Interface pública do módulo Wi-Fi
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef WIFI_H
#define WIFI_H

// Inicializa o hardware Wi-Fi da placa
int initialize_wifi_hardware();

// Conecta à rede usando as credenciais definidas em tempo de compilação
int connect_to_network();

// Exibe o endereço IP atual no display OLED
void show_ip_address();

#endif

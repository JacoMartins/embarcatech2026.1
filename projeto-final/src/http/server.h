/**
 * @file    server.h
 * @brief   Interface pública do servidor HTTP — ciclo de vida TCP via lwIP
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <stdint.h>
#include <stdbool.h>
#include "lwip/tcp.h"

// Tamanho máximo do buffer que acumula bytes da requisição HTTP recebida
#define HTTP_REQ_BUF_SIZE  512

// Estado associado a cada conexão TCP ativa.
// Compartilhado entre server.c (TCP) e routes.c (lógica).
typedef struct {
    char           req_buf[HTTP_REQ_BUF_SIZE]; // bytes da requisição acumulados
    uint16_t       req_len;                     // quantos bytes foram recebidos
    bool           dispatched;                  // impede despacho duplo da mesma requisição
    const uint8_t *send_ptr;                    // posição atual no buffer de envio
    uint32_t       send_remaining;              // bytes ainda pendentes de envio
} HttpConn;

// Inicializa o servidor HTTP na porta 80.
// Deve ser chamada após connect_to_network() retornar 0.
void http_server_init();

#endif

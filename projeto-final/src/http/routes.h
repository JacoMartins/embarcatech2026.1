/**
 * @file    routes.h
 * @brief   Interface pública dos handlers de rota HTTP
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#ifndef HTTP_ROUTES_H
#define HTTP_ROUTES_H

#include "lwip/tcp.h"
#include "server.h"

// Analisa a requisição acumulada em conn->req_buf e chama o handler correto:
//   GET  /status → JSON com leituras dos sensores
//   GET  /audio  → solicita captura (flag) e retorna; envio acontece via route_audio_send
//   POST /relay  → altera estado de um relé (corpo: {"id":0,"state":true})
void http_dispatch(struct tcp_pcb *pcb, HttpConn *conn);

// Definida como 1 por route_audio quando uma captura de áudio é solicitada.
// O loop principal deve chamar audio_capture_wav() e depois route_audio_send()
// dentro de cyw43_arch_lwip_begin()/end() quando este flag estiver ativo.
extern volatile int audio_capture_requested;

// Envia o WAV já capturado para o cliente pendente.
// Deve ser chamada dentro de cyw43_arch_lwip_begin()/end().
// Se a conexão tiver sido encerrada durante a captura, retorna sem fazer nada.
void route_audio_send(void);

// Chamada pelo http_error_cb para limpar o estado pendente caso a conexão
// de áudio seja encerrada com erro durante a captura.
void route_audio_conn_error(HttpConn *conn);

#endif

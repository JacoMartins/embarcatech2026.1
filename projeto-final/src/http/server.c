/**
 * @file    server.c
 * @brief   Servidor HTTP — ciclo de vida TCP via lwIP raw API
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <stdlib.h>
#include <string.h>
#include "lwip/tcp.h"
#include "server.h"
#include "routes.h"
#include "display.h"

#define HTTP_PORT 80

// Desregistra todos os callbacks, libera o estado da conexão e fecha o socket TCP
static void http_close_conn(struct tcp_pcb *pcb, HttpConn *conn)
{
    tcp_arg(pcb,  NULL);
    tcp_recv(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_err(pcb,  NULL);
    tcp_poll(pcb, NULL, 0);
    free(conn);
    tcp_close(pcb);
}

// Chamado pelo lwIP após cada bloco de dados ser reconhecido (ACK) pelo cliente.
// Continua o envio do buffer pendente em chunks até esgotá-lo.
static err_t http_sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    HttpConn *conn = (HttpConn *)arg;

    if (conn->send_remaining == 0) {
        // todos os dados foram enviados — encerra a conexão
        http_close_conn(pcb, conn);
        return ERR_OK;
    }

    // calcula o próximo chunk respeitando o espaço disponível no buffer TCP
    uint16_t chunk = conn->send_remaining;
    if (chunk > tcp_sndbuf(pcb)) chunk = tcp_sndbuf(pcb);

    tcp_write(pcb, conn->send_ptr, chunk, TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    conn->send_ptr       += chunk;
    conn->send_remaining -= chunk;

    return ERR_OK;
}

// Chamado pelo lwIP quando chegam dados numa conexão ativa.
// Acumula os bytes no buffer e despacha a requisição quando os headers estiverem completos.
static err_t http_recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    HttpConn *conn = (HttpConn *)arg;

    // pbuf NULL indica que o cliente fechou a conexão
    if (p == NULL) {
        http_close_conn(pcb, conn);
        return ERR_OK;
    }

    // informa ao lwIP que os dados foram consumidos (libera a janela TCP)
    tcp_recved(pcb, p->tot_len);

    // copia os bytes recebidos para o buffer da conexão sem ultrapassar o limite
    uint16_t copy_len = p->tot_len;
    if (conn->req_len + copy_len >= HTTP_REQ_BUF_SIZE)
        copy_len = HTTP_REQ_BUF_SIZE - conn->req_len - 1;

    pbuf_copy_partial(p, conn->req_buf + conn->req_len, copy_len, 0);
    conn->req_len += copy_len;
    conn->req_buf[conn->req_len] = '\0';
    pbuf_free(p);

    // \r\n\r\n marca o fim dos headers HTTP
    // dispatched impede que um segundo pacote acione o handler duas vezes
    if (strstr(conn->req_buf, "\r\n\r\n") && !conn->dispatched) {

        // Para POST: aguarda o corpo chegar antes de despachar.
        // Postman e browsers frequentemente enviam headers e corpo em pacotes separados.
        if (strncmp(conn->req_buf, "POST", 4) == 0) {
            char *cl_ptr = strstr(conn->req_buf, "Content-Length:");
            if (!cl_ptr) cl_ptr = strstr(conn->req_buf, "content-length:");
            if (cl_ptr) {
                int expected     = atoi(cl_ptr + 15);
                char *body_start = strstr(conn->req_buf, "\r\n\r\n") + 4;
                int  received    = (int)(conn->req_buf + conn->req_len - body_start);
                if (received < expected) {
                    print_oled("[HTTP] Aguard body.");
                    return ERR_OK; // ainda aguardando o corpo
                }
            }
        }

        conn->dispatched = true;
        http_dispatch(pcb, conn);
    }

    return ERR_OK;
}

// Chamado pelo lwIP em caso de erro fatal na conexão (ex: reset pelo cliente)
static void http_error_cb(void *arg, err_t err)
{
    print_oled("[HTTP] Erro TCP.");
    route_audio_conn_error((HttpConn *)arg);  // limpa estado de áudio pendente se necessário
    if (arg) free(arg);
}

// Chamado pelo lwIP quando uma nova conexão TCP é aceita na porta 80.
// Aloca o estado da conexão e registra os callbacks para ela.
static err_t http_accept_cb(void *arg, struct tcp_pcb *client_pcb, err_t err)
{
    if (err != ERR_OK || client_pcb == NULL) return ERR_VAL;

    HttpConn *conn = calloc(1, sizeof(HttpConn));
    if (!conn) { tcp_abort(client_pcb); return ERR_MEM; }

    tcp_arg(client_pcb,  conn);
    tcp_recv(client_pcb, http_recv_cb);
    tcp_sent(client_pcb, http_sent_cb);
    tcp_err(client_pcb,  http_error_cb);

    print_oled("[HTTP] Conectado.");
    return ERR_OK;
}

// Cria o socket TCP, faz bind na porta 80 e começa a escutar conexões.
// A partir daqui o lwIP gerencia tudo em background via callbacks.
void http_server_init()
{
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_bind(pcb, IP_ADDR_ANY, HTTP_PORT);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, http_accept_cb);
    print_oled("[OK] HTTP: porta 80.");
}

/**
 * @file    routes.c
 * @brief   Handlers das rotas HTTP: /status, /audio, /relay
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lwip/tcp.h"
#include "routes.h"
#include "server.h"
#include "encoder.h"
#include "display.h"
#include "sensors/joystick.h"
#include "sensors/temperature.h"
#include "relays/relays.h"
#include "leds/matrix.h"

// Envia os headers HTTP imediatamente e configura o corpo para envio
// chunked via http_sent_cb. Passa NULL em body para respostas sem corpo.
static void send_response(struct tcp_pcb *pcb, HttpConn *conn,
                          const char *headers,
                          const uint8_t *body, uint32_t body_len)
{
    // 1. Envia apenas o cabeçalho
    tcp_write(pcb, headers, strlen(headers), TCP_WRITE_FLAG_COPY);

    // 2. Apenas aponta para o áudio, mas NÃO manda enviar o corpo aqui!
    conn->send_ptr       = body;
    conn->send_remaining = body_len;

    // 3. Dispara o cabeçalho
    tcp_output(pcb);
    
    // Assim que o cabeçalho chegar no navegador, o navegador vai responder "Recebi!".
    // Isso vai acionar o seu http_sent_cb lá no server.c de forma perfeita,
    // e o envio do áudio em chunks vai começar no ritmo exato do Wi-Fi.
}

// GET /status
// Lê o joystick (simula tensão e potência) e o sensor de temperatura interno.
// Retorna JSON com campos nomeados para facilitar leitura:
//   {"tensao_V":220.0,"temperatura_C":25.3,"potencia_W":264.0,
//    "reles":{"luz":false,"tomada1":false,"tomada2":false}}
static void route_status(struct tcp_pcb *pcb, HttpConn *conn)
{
    print_oled("[HTTP] GET /status");
    JoystickReading joy  = joystick_read();
    float           temp = temperature_read_celsius();

    // escala os eixos do joystick (0–4095) para grandezas simuladas
    float voltage = (joy.x / 4095.0f) * 330.0f;  // 0–330 V
    float power   = (joy.y / 4095.0f) * 500.0f;  // 0–500 W

    static char body[160];
    snprintf(body, sizeof(body),
        "{\"tensao_V\":%.1f,\"temperatura_C\":%.1f,\"potencia_W\":%.1f,"
        "\"reles\":{\"luz\":%s,\"entrada1\":%s,\"entrada2\":%s}}",
        voltage, temp, power,
        relay_get(RELAY_LUZ)       ? "true" : "false",
        relay_get(RELAY_ENTRADA_1) ? "true" : "false",
        relay_get(RELAY_ENTRADA_2) ? "true" : "false"
    );

    static char headers[128];
    snprintf(headers, sizeof(headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        (int)strlen(body)
    );

    send_response(pcb, conn, headers, (const uint8_t *)body, strlen(body));
}

// Estado pendente para captura de áudio assíncrona.
// route_audio guarda o pcb/conn e sinaliza o loop principal,
// que faz a captura fora do contexto lwIP e chama route_audio_send().
static struct tcp_pcb *audio_pcb  = NULL;
static HttpConn       *audio_conn = NULL;
volatile int           audio_capture_requested = 0;

// GET /audio
// NÃO bloqueia: salva pcb/conn e sinaliza o loop principal via audio_capture_requested.
// Bloquear 5 s dentro de um callback lwIP seguraria o mutex lwIP e impediria
// o driver Wi-Fi de processar eventos, corrompendo a pilha TCP.
static void route_audio(struct tcp_pcb *pcb, HttpConn *conn)
{
    print_oled("[HTTP] GET /audio");
    audio_pcb  = pcb;
    audio_conn = conn;
    audio_capture_requested = 1;
    // resposta enviada pelo loop principal após a captura completar
}

// Chamada pelo loop principal (dentro de cyw43_arch_lwip_begin/end)
// após audio_capture_wav() ter sido concluída.
void route_audio_send(void)
{
    struct tcp_pcb *pcb  = audio_pcb;
    HttpConn       *conn = audio_conn;
    audio_pcb  = NULL;
    audio_conn = NULL;

    if (!pcb || !conn) {
        print_oled("[HTTP] Audio: sem conn.");
        return;   // conexão encerrada durante a captura
    }

    print_oled("[HTTP] Enviando WAV.");
    const uint8_t *wav = audio_get_buffer();

    static char headers[192];
    snprintf(headers, sizeof(headers),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: audio/wav\r\n"
        "Content-Disposition: attachment; filename=\"audio.wav\"\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        WAV_TOTAL_SIZE
    );

    send_response(pcb, conn, headers, wav, WAV_TOTAL_SIZE);
}

// Limpa o estado pendente se a conexão de áudio morrer durante a captura.
void route_audio_conn_error(HttpConn *conn)
{
    if (conn != NULL && conn == audio_conn) {
        audio_pcb  = NULL;
        audio_conn = NULL;
    }
}

// POST /relay
// Espera corpo JSON no formato: {"id":0,"state":true}
// Liga ou desliga o relé identificado por "id" (0=luz, 1=tomada1, 2=tomada2).
static void route_relay(struct tcp_pcb *pcb, HttpConn *conn)
{
    print_oled("[HTTP] POST /relay");

    static const char *bad_request =
        "HTTP/1.1 400 Bad Request\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";

    // o corpo HTTP vem após os headers, separado por \r\n\r\n
    char *body = strstr(conn->req_buf, "\r\n\r\n");
    if (!body) {
        print_oled("[HTTP] 400: sem body.");
        send_response(pcb, conn, bad_request, NULL, 0); return;
    }
    body += 4;

    // extrai o ID do relé usando busca de substring no JSON
    char *id_ptr = strstr(body, "\"id\":");
    if (!id_ptr) {
        print_oled("[HTTP] 400: sem id.");
        send_response(pcb, conn, bad_request, NULL, 0); return;
    }
    int id = atoi(id_ptr + 5);

    // extrai o estado: presença de "true" após "state" indica ligado
    char *state_ptr = strstr(body, "\"state\":");
    if (!state_ptr) {
        print_oled("[HTTP] 400: sem state");
        send_response(pcb, conn, bad_request, NULL, 0); return;
    }
    bool state = strstr(state_ptr, "true") != NULL;

    if (id >= 0 && id < RELAY_COUNT) {
        relay_set((RelayId)id, state);
        // atualiza a matriz de LEDs para refletir o novo estado do relé
        matrix_update_relays(
            relay_get(RELAY_LUZ),
            relay_get(RELAY_ENTRADA_1),
            relay_get(RELAY_ENTRADA_2)
        );
    }

    const char *ok =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";

    send_response(pcb, conn, ok, NULL, 0);
}

// Lê a primeira linha da requisição e chama o handler correspondente.
// Rotas não mapeadas recebem resposta 404.
void http_dispatch(struct tcp_pcb *pcb, HttpConn *conn)
{
    const char *req = conn->req_buf;

    if      (strncmp(req, "GET /status", 11) == 0) route_status(pcb, conn);
    else if (strncmp(req, "GET /audio",  10) == 0) route_audio(pcb, conn);
    else if (strncmp(req, "POST /relay", 11) == 0) route_relay(pcb, conn);
    else {
        const char *not_found =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
        print_oled("[HTTP] 404.");
        send_response(pcb, conn, not_found, NULL, 0);
    }
}

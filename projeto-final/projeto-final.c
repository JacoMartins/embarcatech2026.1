/**
 * @file    projeto-final.c
 * @brief   Ponto de entrada do sistema — inicialização e loop principal
 * @author  Jacó Alves Martins Barros
 * @version 1.0.0
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "wifi.h"
#include "display.h"
#include "sensors/joystick.h"
#include "sensors/temperature.h"
#include "sensors/microphone.h"
#include "audio/encoder.h"
#include "relays/relays.h"
#include "leds/matrix.h"
#include "buttons/buttons.h"
#include "buzzer/buzzer.h"
#include "http/routes.h"
#include "server.h"
#include "pico/cyw43_arch.h"


int main()
{
    // Inicializa a comunicação serial (USB/UART) para debug via printf
    stdio_init_all();

    // Display deve ser o primeiro — os módulos seguintes já usam print_oled
    initialize_display_hardware();
    print_oled("Sistema iniciando...");
    

    // Sensores
    joystick_init();
    temperature_init();
    microphone_init();
    print_oled("[OK] Sensores.");

    // Relés (LEDs GPIO simulados)
    relays_init();
    print_oled("[OK] Reles prontos.");

    // Matriz de LEDs WS2812B — começa com todos os LEDs apagados
    matrix_init();
    print_oled("[OK] Matriz LEDs.");

    // Botões A (GP5) e B (GP6) com interrupção e debounce
    buttons_init();
    print_oled("[OK] Botoes: IRQ.");

    // Buzzer passivo (GP21)
    buzzer_init();
    print_oled("[OK] Buzzer.");

    // Wi-Fi — em caso de falha reinicia automaticamente via watchdog
    if (initialize_wifi_hardware() != 0 || connect_to_network() != 0)
    {
        print_oled("[HALT] Reiniciando.");
        sleep_ms(3000);
        watchdog_reboot(0, 0, 0);
    }

    show_ip_address();

    // Servidor HTTP — só sobe após Wi-Fi conectado
    http_server_init();

    // Notifica que todos os drivers subiram com sucesso
    print_oled("[OK] Sistema pronto!");
    buzzer_beep(1000, 200);  // 1000 Hz por 200 ms — sinal de boot completo

    // Loop principal:
    //   - Captura áudio quando solicitado via GET /audio e envia a resposta.
    //     A captura ocorre FORA do lock lwIP para não travar o driver Wi-Fi.
    //   - Verifica botões pressionados e atualiza o OLED
    //   - Dispara buzzer quando sinalizado pelo IRQ do joystick
    while (true)
    {
        if (audio_capture_requested) {
            audio_capture_requested = 0;
            audio_capture_wav();            // bloqueia ~5 s via DMA (sem mutex lwIP)
            cyw43_arch_lwip_begin();        // adquire mutex lwIP para acessar a pilha TCP
            route_audio_send();             // envia o WAV para o cliente pendente
            cyw43_arch_lwip_end();
        }

        if (button_oled_dirty) {
            char msg[22];
            snprintf(msg, sizeof(msg), "L:%s E1:%s E2:%s",
                relay_get(RELAY_LUZ)       ? "ON " : "OFF",
                relay_get(RELAY_ENTRADA_1) ? "ON " : "OFF",
                relay_get(RELAY_ENTRADA_2) ? "ON"  : "OFF");
            print_oled(msg);
            button_oled_dirty = false;
        }

        if (buzzer_pending) {
            buzzer_pending = false;
            buzzer_beep(750, 150);
        }

        sleep_ms(100);
    }
}

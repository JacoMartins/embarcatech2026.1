# Estrutura Hardware / Software

## Diagrama de Arquitetura

```mermaid
flowchart TB

    subgraph HW["HARDWARE — BitDogLab V6 (RP2040 + CYW43439)"]
        direction TB

        subgraph INPUTS["Entradas"]
            JOY["Joystick\nGP26 ADC0 / GP27 ADC1\nGP22 click IRQ"]
            MIC["Microfone analógico\nGP28 ADC2 + DMA"]
            TEMP["Temperatura interna\nADC4"]
            BTNA["Botão A\nGP5 IRQ"]
            BTNB["Botão B\nGP6 IRQ"]
        end

        subgraph OUTPUTS["Saídas"]
            OLED["Display OLED\nSSD1306 128×64\nI2C GP14/GP15"]
            RELAYS["Relés simulados\nLED Azul GP11 — Luz\nLED Vermelho GP12 — Entrada1\nLED Verde GP13 — Entrada2"]
            MATRIX["Matriz WS2812B 5×5\n25 LEDs — GP7 PIO\n800 kHz GRB"]
            BUZZER["Buzzer passivo\nGP21 PWM\n1 MHz tick"]
        end

        subgraph COMMS["Comunicação"]
            WIFI["Wi-Fi CYW43439\n802.11 b/g/n\nDHCP"]
        end
    end

    subgraph SW["SOFTWARE — Módulos C"]
        direction TB

        subgraph APP["Aplicação"]
            MAIN["projeto-final.c\nBoot + Loop principal"]
        end

        subgraph DRIVERS["Drivers de Hardware"]
            D_DISPLAY["display.c\nprint_oled → I2C + SSD1306"]
            D_WIFI["wifi.c\nCYW43 init + DHCP"]
            D_JOY["joystick.c\nADC mux 0/1"]
            D_TEMP["temperature.c\nADC 4 → °C"]
            D_MIC["microphone.c\nADC 2 + DMA\n8 kHz 8-bit"]
            D_RELAY["relays.c\nGPIO + estado interno\nirq_safe toggle"]
            D_MATRIX["matrix.c\nPIO WS2812B\nGRB 25 LEDs"]
            D_BTN["buttons.c\nIRQ GPIO debounce 200ms\nflags para loop principal"]
            D_BUZZ["buzzer.c\nPWM clkdiv 125\n→ 1 MHz tick"]
        end

        subgraph AUDIO["Áudio"]
            D_ENC["encoder.c\nWAV header 44B\nbuffer estático 40 044B"]
        end

        subgraph HTTP["Servidor HTTP"]
            D_SRV["server.c\nlwIP raw TCP\naccept/recv/sent/poll"]
            D_RTE["routes.c\nGET /status\nGET /audio\nPOST /relay"]
        end

        subgraph LWIP["Pilha de Rede"]
            LWIPSTACK["lwIP threadsafe_background\nTCP_MSS 1460\nTCP_SND_BUF 11680\nMEM_SIZE 8000"]
        end
    end

    %% Hardware ↔ Driver connections
    JOY   --> D_JOY
    MIC   --> D_MIC
    TEMP  --> D_TEMP
    BTNA  --> D_BTN
    BTNB  --> D_BTN
    JOY   --> D_BTN
    D_DISPLAY --> OLED
    D_RELAY   --> RELAYS
    D_MATRIX  --> MATRIX
    D_BUZZ    --> BUZZER
    D_WIFI    --> WIFI

    %% Driver ↔ Application
    D_JOY   --> D_RTE
    D_TEMP  --> D_RTE
    D_MIC   --> D_ENC
    D_ENC   --> D_RTE
    D_RELAY --> D_RTE
    D_RELAY --> D_BTN
    D_MATRIX --> D_RTE
    D_MATRIX --> D_BTN
    D_BTN   --> MAIN
    D_BUZZ  --> MAIN
    D_DISPLAY --> MAIN

    %% HTTP stack
    D_WIFI    --> LWIPSTACK
    LWIPSTACK --> D_SRV
    D_SRV     --> D_RTE
    D_RTE     --> D_SRV

    %% Main orchestrates everything
    MAIN --> D_SRV
    MAIN --> D_ENC
    MAIN --> D_RTE
    MAIN --> D_DISPLAY
    MAIN --> D_WIFI
```

---

## Fluxo de dados por rota

```mermaid
flowchart LR

    CLIENT(["Cliente HTTP\nPostman / Browser / App"])

    subgraph STATUS["GET /status"]
        S1["Lê joystick\nADC 0+1"] --> S2["Escala\n0–330V / 0–500W"]
        S3["Lê temperatura\nADC 4"] --> S4["Converte para °C"]
        S5["Lê estado\n3 relés"]
        S2 & S4 & S5 --> S6["snprintf JSON\n~120 bytes"]
        S6 --> S7["tcp_write\nContent-Type: application/json"]
    end

    subgraph AUDIO["GET /audio"]
        A1["route_audio\nSalva pcb/conn\naudio_capture_requested=1"] --> A2["Loop principal\nDetecta flag"]
        A2 --> A3["microphone_capture\nDMA 40 000 amostras\n5 s @ 8 kHz ADC2"]
        A3 --> A4["build_wav_header\nRIFF/WAVE/PCM 8-bit"]
        A4 --> A5["cyw43_arch_lwip_begin\nRoute_audio_send"]
        A5 --> A6["http_sent_cb loop\nchunks 1460B sem cópia\naté send_remaining=0"]
        A6 --> A7["tcp_close → FIN\nDownload completo"]
    end

    subgraph RELAY["POST /relay"]
        R1["Parse JSON body\nid + state"] --> R2["relay_set\ngpio_put + print_oled"]
        R2 --> R3["matrix_update_relays\nPIO WS2812B"]
        R3 --> R4["200 OK\nContent-Length: 0"]
    end

    subgraph IRQ["Botões / Joystick (IRQ)"]
        I1["GPIO IRQ\nEdge Fall"] --> I2["Debounce 200ms"]
        I2 --> I3["relay_toggle_irq_safe\ngpio_put apenas"]
        I3 --> I4["matrix_update_relays\nPIO — seguro em IRQ"]
        I4 --> I5["Sinaliza flags\noled_dirty\nbuzzer_pending"]
        I5 --> I6["Loop principal\nprint_oled\nbuzzer_beep 750Hz"]
    end

    CLIENT -->|"GET /status"| STATUS
    CLIENT -->|"GET /audio"| AUDIO
    CLIENT -->|"POST /relay"| RELAY
    STATUS -->|"JSON 200 OK"| CLIENT
    AUDIO  -->|"WAV 40044B 200 OK"| CLIENT
    RELAY  -->|"200 OK"| CLIENT
```

---

## Módulos e responsabilidades

| Módulo | Arquivo | Responsabilidade |
|---|---|---|
| Boot + Loop | `projeto-final.c` | Sequência de init, loop principal, orquestra captura de áudio e flags de IRQ |
| Display | `display.c` | Abstração `print_oled()` sobre I2C + SSD1306 |
| Wi-Fi | `wifi.c` | Init CYW43, DHCP, exibe IP no OLED |
| Joystick | `sensors/joystick.c` | Leitura ADC multiplexado canais 0/1 |
| Temperatura | `sensors/temperature.c` | Leitura ADC canal 4, conversão datasheet RP2040 |
| Microfone | `sensors/microphone.c` | Config FIFO ADC canal 2, captura DMA 8-bit 8 kHz |
| Encoder WAV | `audio/encoder.c` | Cabeçalho RIFF/WAVE e buffer estático de 40 KB |
| Relés | `relays/relays.c` | GPIO dos LEDs simulados, toggle IRQ-safe, estado interno |
| Matriz LED | `leds/matrix.c` | PIO WS2812B 800 kHz, mapeamento GRB 25 LEDs |
| Botões | `buttons/buttons.c` | IRQ borda de descida GP5/6/22, debounce 200 ms, flags async |
| Buzzer | `buzzer/buzzer.c` | PWM clkdiv=125 (1 MHz tick), frequência e duração configuráveis |
| Servidor HTTP | `http/server.c` | lwIP raw TCP: accept, recv, sent (chunks 1 MSS), poll (retry ERR_MEM) |
| Rotas HTTP | `http/routes.c` | Handlers /status, /audio (assíncrono), /relay; despacho 404 |

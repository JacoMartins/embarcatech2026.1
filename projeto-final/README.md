# Projeto Final — Embarcatech 2026.1

Monitor residencial IoT baseado na **BitDogLab V6** (Raspberry Pi Pico W) com servidor HTTP embarcado, captura de áudio, controle de relés e feedback visual/sonoro em tempo real.

**Autor:** Jacó Alves Martins Barros  
**Curso:** Embarcatech 2026.1

---

## Funcionalidades

| Recurso | Descrição |
|---|---|
| `GET /status` | Retorna JSON com tensão simulada, temperatura interna e estado dos relés |
| `GET /audio` | Captura 5 s de áudio pelo microfone e devolve um arquivo WAV pronto para reprodução |
| `POST /relay` | Liga ou desliga um dos três relés via JSON `{"id":0,"state":true}` |
| Botão A (GP5) | Alterna Relé Entrada 1 com debounce por IRQ |
| Botão B (GP6) | Alterna Relé Entrada 2 com debounce por IRQ |
| Joystick click (GP22) | Alterna Relé Luz com debounce por IRQ + beep de confirmação |
| Matriz WS2812B | Reflete visualmente o estado dos três relés (amarelo / verde / azul) |
| Buzzer | Beep de boot (1000 Hz) e confirmação de ação (750 Hz) |
| Display OLED | Exibe IP, estado dos relés, eventos HTTP e debug em tempo real |

---

## Hardware

### Plataforma
- **BitDogLab V6** — Raspberry Pi Pico W (RP2040 + CYW43439 Wi-Fi)

### Mapeamento de pinos

| Periférico | Pino(s) | Interface |
|---|---|---|
| Display OLED SSD1306 | GP14 (SDA), GP15 (SCL) | I2C |
| Joystick eixo X | GP26 | ADC 0 |
| Joystick eixo Y | GP27 | ADC 1 |
| Joystick botão click | GP22 | GPIO / IRQ |
| Microfone analógico | GP28 | ADC 2 + DMA |
| Sensor de temperatura | interno | ADC 4 |
| Relé Luz (LED Azul) | GP11 | GPIO |
| Relé Entrada 1 (LED Vermelho) | GP12 | GPIO |
| Relé Entrada 2 (LED Verde) | GP13 | GPIO |
| Matriz LED WS2812B 5×5 | GP7 | PIO |
| Botão A | GP5 | GPIO / IRQ |
| Botão B | GP6 | GPIO / IRQ |
| Buzzer passivo | GP21 | PWM |

---

## Estrutura de Software

```
projeto-final/
├── projeto-final.c          # Ponto de entrada — boot e loop principal
├── CMakeLists.txt
├── libs/
│   ├── ssd1306.c/h          # Driver baixo nível do OLED
│   └── lwipopts.h           # Configuração da pilha lwIP
└── src/
    ├── display.c/h          # Abstração print_oled (I2C + SSD1306)
    ├── wifi.c/h             # Conexão Wi-Fi (CYW43)
    ├── sensors/
    │   ├── joystick.c/h     # Leitura ADC 0/1 (eixos X/Y)
    │   ├── temperature.c/h  # Leitura ADC 4 (sensor interno RP2040)
    │   └── microphone.c/h   # Captura DMA via ADC 2 (8 kHz, 8 bits)
    ├── audio/
    │   └── encoder.c/h      # Montagem do cabeçalho WAV + buffer estático
    ├── relays/
    │   └── relays.c/h       # Estado + GPIO dos três relés simulados
    ├── leds/
    │   ├── matrix.c/h       # Driver WS2812B via PIO (25 LEDs)
    │   └── ws2812.pio        # Programa PIO para protocolo 800 kHz
    ├── buttons/
    │   └── buttons.c/h      # IRQ GPIO com debounce por software (200 ms)
    ├── buzzer/
    │   └── buzzer.c/h       # PWM passivo (clkdiv 125 → 1 MHz tick)
    └── http/
        ├── server.c/h       # Servidor TCP lwIP raw — accept/recv/sent/poll
        └── routes.c/h       # Handlers: /status, /audio, /relay
```

---

## API HTTP

### `GET /status`
Retorna leituras dos sensores e estado dos relés.

```json
{
  "tensao_V": 220.5,
  "temperatura_C": 42.1,
  "potencia_W": 312.0,
  "reles": {
    "luz": false,
    "entrada1": true,
    "entrada2": false
  }
}
```

> `tensao_V` e `potencia_W` são simulados pelos eixos X/Y do joystick (0–330 V, 0–500 W).  
> `temperatura_C` é a temperatura interna do die do RP2040 (~42–92 °C em operação normal com Wi-Fi).

---

### `GET /audio`
Captura 5 segundos de áudio pelo microfone analógico (GP28) e retorna um arquivo WAV.

- **Formato:** PCM não comprimido, 8 bits, mono, 8000 Hz  
- **Tamanho:** 40 044 bytes (44 bytes cabeçalho + 40 000 amostras)  
- **Tempo de resposta:** ~5 s de captura + tempo de transferência

> A captura ocorre no loop principal, fora do mutex lwIP, para não bloquear o driver Wi-Fi.

---

### `POST /relay`
Liga ou desliga um relé.

**Corpo (JSON):**
```json
{ "id": 0, "state": true }
```

| `id` | Relé |
|---|---|
| `0` | Luz (LED Azul — GP11) |
| `1` | Entrada 1 (LED Vermelho — GP12) |
| `2` | Entrada 2 (LED Verde — GP13) |

---

## Como compilar

### Pré-requisitos
- [Pico SDK 1.5.1](https://github.com/raspberrypi/pico-sdk)
- CMake ≥ 3.13
- ARM GCC Toolchain 13.2

### Variáveis de ambiente
```bash
export WIFI_SSID="nome_da_rede"
export WIFI_PASSWORD="senha_da_rede"
```

### Build
```bash
mkdir build && cd build
cmake ..
make -j4
```

O arquivo `.uf2` gerado em `build/projeto-final.uf2` pode ser copiado diretamente para a BitDogLab em modo BOOTSEL.

---

## Decisões de arquitetura

### Captura de áudio fora do mutex lwIP
O driver Wi-Fi (`pico_cyw43_arch_lwip_threadsafe_background`) usa um mutex para proteger a pilha lwIP. Bloquear 5 s dentro de um callback TCP impede o driver de processar eventos Wi-Fi, corrompendo a pilha TCP. A solução adotada:
1. `route_audio` salva o `pcb`/`conn` e sinaliza `audio_capture_requested = 1`, retornando imediatamente.
2. O loop principal detecta o flag, chama `audio_capture_wav()` **sem** segurar o mutex.
3. Após a captura, adquire o mutex com `cyw43_arch_lwip_begin()` e envia a resposta.

### Envio TCP em chunks de 1 MSS
O envio do WAV (40 KB) é feito em segmentos de 1460 bytes (TCP_MSS) por chamada de `http_sent_cb`. Enviar mais de um segmento por vez esgota o pool `MEMP_PBUF` de pbufs sem cópia enquanto o driver Wi-Fi também os usa, gerando `ERR_MEM`. O callback `http_poll_cb` atua como mecanismo de retry com intervalo de 1 s.

### Buzzer e OLED fora dos IRQs
O buzzer usa `sleep_ms` e o display usa I2C — ambos proibidos em contexto de interrupção. O callback GPIO sinaliza `buzzer_pending` e `button_oled_dirty`; o loop principal executa as ações correspondentes de forma segura.

### Relés simulados por LEDs
Os três LEDs RGB onboard (GP11/12/13) simulam relés de luz e duas entradas. O estado é refletido simultaneamente na matriz WS2812B (LED 0 = amarelo/luz, LED 12 = verde/entrada1, LED 24 = azul/entrada2) a 15% de brilho.

---

## Licença

Este projeto é distribuído sob a licença **MIT**. Consulte o arquivo [LICENSE](LICENSE) para mais detalhes.

## Créditos e bibliotecas utilizadas

| Biblioteca / Ferramenta | Uso |
|---|---|
| [Raspberry Pi Pico SDK 1.5.1](https://github.com/raspberrypi/pico-sdk) | HAL, DMA, PIO, PWM, ADC, watchdog |
| [lwIP 2.x](https://savannah.nongnu.org/projects/lwip/) | Pilha TCP/IP embarcada (inclusa no Pico SDK) |
| [SSD1306 driver](https://github.com/daschr/pico-ssd1306) | Driver I2C para o display OLED (em `libs/ssd1306.c`) |
| [CYW43 / pico-w drivers](https://github.com/raspberrypi/pico-sdk) | Driver Wi-Fi CYW43439 (incluso no Pico SDK) |
| [Claude Code — Anthropic](https://claude.ai/code) | Assistente de IA utilizado no desenvolvimento do firmware e documentação |

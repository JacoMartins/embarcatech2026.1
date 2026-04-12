# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(projeto-final C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(projeto-final
    projeto-final.c
    src/display.c
    src/wifi.c
    libs/ssd1306.c
    src/sensors/joystick.c
    src/sensors/temperature.c
    src/sensors/microphone.c
    src/audio/encoder.c
    src/relays/relays.c
    src/leds/matrix.c
    src/buttons/buttons.c
    src/buzzer/buzzer.c
    src/http/server.c
    src/http/routes.c
)

# Gera o header C a partir do programa PIO do WS2812B
# Deve vir depois de add_executable
pico_generate_pio_header(projeto-final ${CMAKE_CURRENT_LIST_DIR}/src/leds/ws2812.pio)

pico_set_program_name(projeto-final "projeto-final")
pico_set_program_version(projeto-final "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(projeto-final 1)
pico_enable_stdio_usb(projeto-final 0)

# Add the standard include files to the build
target_include_directories(projeto-final PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/libs
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/sensors
    ${CMAKE_CURRENT_LIST_DIR}/src/relays
    ${CMAKE_CURRENT_LIST_DIR}/src/audio
    ${CMAKE_CURRENT_LIST_DIR}/src/http
    ${CMAKE_CURRENT_LIST_DIR}/src/leds
    ${CMAKE_CURRENT_LIST_DIR}/src/buttons
    ${CMAKE_CURRENT_LIST_DIR}/src/buzzer
)

# Add any user requested libraries
target_link_libraries(projeto-final
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_pwm
    hardware_adc
    hardware_dma
    hardware_i2c
    hardware_pio
    hardware_watchdog
)

target_compile_definitions(projeto-final PRIVATE
    WIFI_SSID=\"$ENV{WIFI_SSID}\"
    WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
)

pico_add_extra_outputs(projeto-final)

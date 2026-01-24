# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# =========================================================================
# PROJETO: IHM Digital via DVI com Raspberry Pi Pico
# =========================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =========================================================================
# CONFIGURAÇÃO DO PICO SDK (NÃO EDITAR)
# =========================================================================

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

# =========================================================================
# DECLARAÇÃO DO PROJETO
# =========================================================================

project(main
    LANGUAGES C CXX ASM
    VERSION 0.1
    DESCRIPTION "IHM Digital via DVI"
)

pico_sdk_init()

# =========================================================================
# BIBLIOTECAS EXTERNAS
# =========================================================================

add_subdirectory(libdvi)
add_subdirectory(libsprite)

# =========================================================================
# FUNÇÃO AUXILIAR: Criar biblioteca customizada
# =========================================================================
# Simplifica a criação de bibliotecas com configuração padrão

function(add_custom_library LIB_NAME SOURCE_FILE)
    add_library(${LIB_NAME} STATIC ${SOURCE_FILE})

    target_include_directories(${LIB_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/lib/${LIB_NAME}
    )

    # Passa bibliotecas adicionais como argumentos
    if(${ARGC} GREATER 2)
        target_link_libraries(${LIB_NAME} ${ARGN})
    endif()
endfunction()

# =========================================================================
# BIBLIOTECAS DO PROJETO
# =========================================================================

# Terminal: Gerencia texto e cores na tela
add_custom_library(terminal
    lib/terminal/terminal.c
    pico_stdlib
)

# Display: Gerencia vídeo DVI e Core 1
add_library(display STATIC
    lib/display/display.c
    tmds_encode_font_2bpp.S
)

target_include_directories(display PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/lib/display
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(display
    terminal
    libdvi
    pico_multicore
    hardware_vreg
    hardware_clocks
    hardware_structs
)

# ADC Reader: Lê valores analógicos
add_custom_library(adc_reader
    lib/adc_reader/adc_reader.c
    hardware_adc
    hardware_gpio
)

# Sensor de Luz: Lê intensidade luminosa via BH1750
add_custom_library(bh1750
    lib/sensor_luz/bh1750.c
    hardware_i2c
    pico_stdlib
)


# =========================================================================
# EXECUTÁVEL PRINCIPAL (parametrizável)
# =========================================================================

set(MAIN_SRC main.c CACHE STRING "Arquivo principal do projeto")
# Para usar main_tx_uart.c, troque para:
# set(MAIN_SRC main_tx_uart.c CACHE STRING "Arquivo principal do projeto")

add_executable(${PROJECT_NAME} ${MAIN_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/lib/sensor_luz
)

pico_set_program_name(${PROJECT_NAME} "IHM Digital DVI")
pico_set_program_version(${PROJECT_NAME} "0.1")

# Desabilita saída serial
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Configurações de compilação
target_compile_definitions(${PROJECT_NAME} PRIVATE
    DVI_VERTICAL_REPEAT=1
    DVI_DEFAULT_SERIAL_CONFIG=picodvi_dvi_cfg
    PICO_CORE1_STACK_SIZE=0x200
)

# Linkagem
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_multicore
    pico_bootrom
    hardware_gpio
    terminal
    display
    adc_reader
    bh1750
)

pico_add_extra_outputs(${PROJECT_NAME})

# =========================================================================
# COMPILAÇÃO: mkdir build && cd build && cmake .. && make -j4
# =========================================================================

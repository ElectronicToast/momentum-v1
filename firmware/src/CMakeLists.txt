cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(momentum C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        -Os
        )

add_executable(${PROJECT_NAME}
        main.c
        sys.c
        isr.c
        tick.c
        ledstrip.c
        button.c
        speaker.c
        imu.c
        utilities.c
        )

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# pull in common dependencies
target_link_libraries(${PROJECT_NAME} 
        pico_stdlib
        hardware_pio
        hardware_sync
        hardware_pwm
        hardware_dma
        hardware_timer
        hardware_clocks
        hardware_sleep
        hardware_i2c
        )

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

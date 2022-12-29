cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(rp2040_arducam)

pico_sdk_init()

add_executable(AR_racing
	arducam/arducam.c
	main.c
	lib/st7735.c
	lib/fonts.c
	lib/DEV_Config.c
	lib/draw.c
	lib/pio_i2c.c
	lib/lsm6ds3.c
)

pico_generate_pio_header(AR_racing ${CMAKE_CURRENT_LIST_DIR}/image.pio)
pico_generate_pio_header(AR_racing ${CMAKE_CURRENT_LIST_DIR}/lib/i2c.pio)

target_link_libraries(AR_racing
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pio
	hardware_pwm
	hardware_spi
	pico_multicore
)
pico_enable_stdio_usb(AR_racing 1)

pico_add_extra_outputs(AR_racing)

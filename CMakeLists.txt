cmake_minimum_required(VERSION 3.31)

if(NOT PICO_BOARD)
	message(FATAL_ERROR "PICO_BOARD not specified. Use -DPICO_BOARD=xxx (eg. pico, pico2, pico_w or pico2_w)")
endif()

if(NOT DEFINED ENV{PICO_SDK_PATH})
	message(FATAL_ERROR "PICO_SDK_PATH not set")
endif()

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Macropad C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# Find Pioasm as an alternative to building it since Windows + Ninja is a cross-compiling nightmare
find_program(PIOASM_PATH pioasm)
find_program(PICOTOOL_PATH picotool)

if(PIOASM_PATH)
	add_executable(pioasm IMPORTED)
	set_property(TARGET pioasm PROPERTY IMPORTED_LOCATION "${PIOASM_PATH}")

	set(pioasm_FOUND 1)
endif()

pico_sdk_init()

add_subdirectory(external)

add_executable(macropad
	source/main.cpp
	source/devices/analogstick.cpp
	source/devices/analogstick.h
	source/devices/display.cpp
	source/devices/display.h
	source/devices/keymatrix.cpp
	source/devices/keymatrix.h
	source/gui/drawing.cpp
	source/gui/drawing.h
	source/gui/font.h
	source/logic/keylayer.cpp
	source/logic/keylayer.h
	source/logic/flashfs.cpp
	source/logic/flashfs.h
	source/logic/application.cpp
	source/logic/application.h
	source/usb/usb_descriptor.cpp
	source/usb/usb_descriptor.h
	source/usb/usb_hid.cpp
	source/usb/usb_msc.cpp)

target_link_libraries(macropad pico_stdlib hardware_i2c hardware_adc tinyusb_device tinyusb_board fatfs tiny-json)
target_compile_definitions(macropad PUBLIC CFG_TUSB_CONFIG_FILE=<usb/tusb_config.h>)

target_include_directories(macropad PRIVATE SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/source)

pico_enable_stdio_usb(macropad 0)
pico_enable_stdio_uart(macropad 0)

pico_add_extra_outputs(macropad)

add_custom_target(macropad-Flash
	DEPENDS macropad
	COMMAND picotool load "$<TARGET_FILE:macropad>" -fx)


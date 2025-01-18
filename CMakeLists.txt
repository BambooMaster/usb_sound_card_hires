cmake_minimum_required(VERSION 3.12)

set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/lib/pico-sdk)
set(PICO_EXTRAS_PATH ${CMAKE_SOURCE_DIR}/lib/pico-extras)

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(pico_playground C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_executable(usb_sound_card_hires
        usb_sound_card.c
        )

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(usb_sound_card_hires 1)
pico_enable_stdio_usb(usb_sound_card_hires 0)

target_compile_definitions(usb_sound_card_hires PRIVATE
        AUDIO_FREQ_MAX=192000

        # ours are zero based, so say so
        PICO_USBDEV_USE_ZERO_BASED_INTERFACES=1

        # need large descriptor
        PICO_USBDEV_MAX_DESCRIPTOR_SIZE=256


        PICO_USBDEV_ISOCHRONOUS_BUFFER_STRIDE_TYPE=3
        PICO_USBDEV_ENABLE_DEBUG_TRAgCE

        PICO_AUDIO_I2S_MONO_OUTPUT=0
        PICO_AUDIO_I2S_MONO_INPUT=0
)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/pico-i2s-pio)

target_link_libraries(usb_sound_card_hires
        pico_stdlib
        usb_device
        pico_unique_id
        pico_multicore
        hardware_clocks
        pico-i2s-pio
)
pico_add_extra_outputs(usb_sound_card_hires)

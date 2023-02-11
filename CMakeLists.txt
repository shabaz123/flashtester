cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(flashtester)

pico_sdk_init()


# rest of your project
add_executable(flashtester
        main.cpp
        SPIFBlockDevice.cpp
        littlefs/lfs.c
        littlefs/lfs_util.c lfs_adapter.cpp lfs_adapter.h)

target_include_directories(flashtester PUBLIC
        littlefs
        )

target_link_libraries(flashtester pico_stdlib hardware_clocks
        hardware_spi
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(flashtester 1)
pico_enable_stdio_uart(flashtester 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(flashtester)

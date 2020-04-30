cmake_minimum_required(VERSION 3.17.2)

set(NAME "gsmbase")

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_GENERATOR "MinGW Makefiles")

project(${NAME})

enable_language(CXX C ASM)

set(F_CPU 48000000UL)
set(ARCH cortex-m0plus)

include_directories(
    ../include
    ..
    src/GSM
    src/peripherals
)

file(GLOB_RECURSE SRCS "src/*.c" "src/*.cpp" "src/*.s")

set(BUILD_PATH "build")

set(OBJCOPY arm-none-eabi-objcopy)
set(OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)

add_definitions(
    -DF_CPU=${F_CPU}
    -D__SAMD10D14AM__
    -DDONT_USE_CMSIS_INIT
)

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=${ARCH} -mthumb -Wl,--gc-sections -Wl,--script=../linker/samd10d14.ld --specs=nano.specs")

add_compile_options(
    #-W
    -Wall
    #-Wno-unused-parameter
    -Os
    -g2
    -fno-diagnostics-show-caret
    -fdata-sections
    -ffunction-sections
    -fno-exceptions
    -funsigned-char
    -funsigned-bitfields
    -mcpu=${ARCH}
    -mthumb
)

set(CMAKE_CXX_FLAGS "-fno-rtti")
set(CMAKE_C_FLAGS "-std=gnu99")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_executable(${NAME}.elf ${SRCS})

set_target_properties(
    ${NAME}.elf
    PROPERTIES
    OUTPUT_NAME ${BUILD_PATH}/${NAME}.elf
)

add_custom_target(hex ALL ${OBJCOPY} -O ihex ${BUILD_PATH}/${NAME}.elf ${BUILD_PATH}/${NAME}.hex)

add_custom_target(lss ALL ${OBJDUMP} -x -S ${BUILD_PATH}/${NAME}.elf > ${BUILD_PATH}/${NAME}.lss)
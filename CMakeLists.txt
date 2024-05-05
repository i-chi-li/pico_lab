cmake_minimum_required(VERSION 3.22)
# CMake が自動的に相対パスを絶対パスに変換するようにする設定
cmake_policy(SET CMP0076 NEW)
# CMake のロード時のテストでエラーがでないようにする
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(PICO_LAB_PATH ${CMAKE_CURRENT_LIST_DIR})

if (NOT CMAKE_TOOLCHAIN_FILE)
    # ツールチェイン定義を読み込む
    include(arm-none-eabi-gcc_toolchain.cmake)
endif ()

# pico-sdk 用
include(../pico-sdk/pico_sdk_init.cmake)
# pico-extras 用
include(../pico-extras/external/pico_extras_import.cmake)

project(pico_lab C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# pico-sdk を初期化
pico_sdk_init()

# 実行ファイルに含まれる URL 情報を設定するマクロを読み込む
include(auto_set_url.cmake)

#add_compile_options(
#        -Wall
#        -Wno-format
#        -Wno-unused-function
#)
#if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
#    add_compile_options(-Wno-maybe-uninitialized)
#endif()

add_subdirectory(hello)

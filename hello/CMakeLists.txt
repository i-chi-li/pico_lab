if (TARGET tinyusb_device)
    if (NOT PICO_NO_HARDWARE)
        add_executable(hello hello.c)
        # 依存ライブラリを追加
        target_link_libraries(hello pico_stdlib hardware_adc hardware_i2c)
        # USB コンソール出力設定
        pico_enable_stdio_usb(hello 1)
        # シリアル（UART）コンソール出力設定
        pico_enable_stdio_uart(hello 1)
        # .bin .hex .dis .uf2 ファイルを生成するように設定
        # .dis ファイルは、.elf ファイルのセクションヘッダー情報と、逆アセンブル情報が格納しているテキストファイル。
        pico_add_extra_outputs(hello)
        # uf2 に含める URL 情報を設定
        auto_set_url(hello)
    endif ()
elseif (PICO_ON_DEVICE)
    message(WARNING "not building hello because TinyUSB submodule is not initialized in the SDK")
endif ()

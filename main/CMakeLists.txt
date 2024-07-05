set(SOURCES
    main.cpp
    speech_recognition.cpp
    led_blinker.cpp
)

if (CONFIG_BSP_NOSSAT_BOX_LITE)
    message("Building for ESP32-S3-BOX-Lite")
    list(APPEND BOARD_SOURCES
        board/esp-box-lite/esp-box-lite-board.cpp
        board/esp-box-lite/gui/lvgl_ui.cpp
        board/esp-box-lite/gui/font/font_en_24.c
        board/esp-box-lite/gui/image/mic_logo.c
    )
    set_source_files_properties(${BOARD_SOURCES}
        PROPERTIES COMPILE_OPTIONS -DLV_LVGL_H_INCLUDE_SIMPLE)
elseif (CONFIG_BSP_NOSSAT_ONE)
    message("Building for ESP32-S3-DevKit C")
    list(APPEND BOARD_SOURCES
        board/nossat-one/esp32-s3-devkit-c-board.cpp
        board/nossat-one/lvgl_ui.c
    )
endif()

idf_component_register(SRCS ${SOURCES} ${BOARD_SOURCES} INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

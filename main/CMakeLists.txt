set(SOURCES
    main.cpp

    system/interrupt_manager.cpp
    system/event_loop.cpp
    system/task.cpp
    system/file_system.cpp

    speech_recognition/speech_recognition.cpp
)

if (CONFIG_BSP_NOSSAT_BOX_LITE)
    message(STATUS "Building for Nosyna Satellite Box Lite")

    list(APPEND BOARD_SOURCES
        board/nossat-box-lite/nossat-box-lite.cpp

        hal/audio_input/audio_input_bsp.cpp
        hal/audio_output/audio_output.cpp
        hal/audio_output/audio_output_bsp.cpp

        network/mqtt_manager.cpp
    )



    list(APPEND BOARD_SOURCES
        lvgl/speech/lvgl_ui.cpp
        lvgl/speech/font/font_en_24.c
        lvgl/speech/image/mic_logo.c

    )
    set_source_files_properties(${BOARD_SOURCES}
        PROPERTIES COMPILE_OPTIONS -DLV_LVGL_H_INCLUDE_SIMPLE)
elseif (CONFIG_BSP_NOSSAT_ONE)
    message(STATUS "Building for Nosyna Satellite One")
    list(APPEND BOARD_SOURCES
        board/nossat-one/nossat-one.cpp


        lvgl/lvgl_ui.c

        hal/audio_input/audio_input_inmp441.cpp
        hal/audio_output/audio_output.cpp
        hal/audio_output/audio_output_max98357a.cpp
        hal/encoder/encoder.cpp
        hal/led/led.cpp
        hal/led/led_blinker.cpp

        network/mqtt_manager.cpp
    )
endif()

idf_component_register(SRCS ${SOURCES} ${BOARD_SOURCES} INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

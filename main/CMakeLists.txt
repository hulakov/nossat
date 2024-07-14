set(SOURCES
    main.cpp

    system/interrupt_manager.cpp
    system/event_loop.cpp
    system/task.cpp
    system/file_system.cpp

    network/mqtt_manager.cpp

    hal/audio_output/audio_output.cpp
)

if (CONFIG_NOSSAT_SPEECH_RECOGNITION)
    list(APPEND SOURCES speech_recognition/speech_recognition.cpp)
endif ()

if (CONFIG_NOSSAT_BOX_LITE_BOARD)
    message(STATUS "Building for Nosyna Satellite Box Lite")

    list(APPEND BOARD_SOURCES
        board/nossat_box_lite/nossat_box_lite.cpp
        hal/audio_input/audio_input_bsp.cpp
        hal/audio_output/audio_output_bsp.cpp
    )

    list(APPEND LVGL_SOURCES
        lvgl/speech/lvgl_ui.cpp
        lvgl/speech/font/font_en_24.c
        lvgl/speech/image/mic_logo.c

    )
    set_source_files_properties(${LVGL_SOURCES}
        PROPERTIES COMPILE_OPTIONS -DLV_LVGL_H_INCLUDE_SIMPLE)
elseif (CONFIG_NOSSAT_ONE_BOARD)
    message(STATUS "Building for Nosyna Satellite One")
    list(APPEND BOARD_SOURCES
        board/nossat_one/nossat_one.cpp
        hal/audio_input/audio_input_inmp441.cpp
        hal/audio_output/audio_output_max98357a.cpp
        hal/encoder/encoder.cpp
        hal/led/led.cpp
        hal/led/led_blinker.cpp
    )

    list(APPEND LVGL_SOURCES
        lvgl/lvgl_ui.c
    )
endif()

idf_component_register(SRCS ${SOURCES} ${BOARD_SOURCES} ${LVGL_SOURCES} INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

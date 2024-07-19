set(SOURCES
    main.cpp

    system/interrupt_manager.cpp
    system/event_loop.cpp
    system/task.cpp

    hal/file_system.cpp

    sound/audio_data.cpp

    network/mqtt_manager.cpp
)

if (CONFIG_NOSSAT_SPEECH_RECOGNITION)
    list(APPEND SOURCES sound/speech_recognition.cpp)
endif ()

if (CONFIG_NOSSAT_BOX_LITE_BOARD)
    message(STATUS "Building for Nosyna Satellite Box Lite")

    list(APPEND SOURCES
        board/nossat_box_lite.cpp
        hal/audio_input_bsp.cpp
        hal/audio_output_bsp.cpp
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
    list(APPEND SOURCES
        board/nossat_one.cpp
        hal/audio_input_inmp441.cpp
        hal/audio_output_max98357a.cpp
        hal/encoder.cpp
        hal/led.cpp
        hal/led_blinker.cpp
    )

    list(APPEND LVGL_SOURCES
        lvgl/lvgl_ui.c
    )
endif()

idf_component_register(SRCS ${SOURCES} ${LVGL_SOURCES} INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

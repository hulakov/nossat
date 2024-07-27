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
        hal/display_bsp.cpp
    )

    list(APPEND LVGL_SOURCES
        gui/gui_box.cpp

        gui/lvgl/speech_recognition.cpp
        gui/lvgl/font/font_en_24.c
        gui/lvgl/image/mic_logo.c
    )

elseif (CONFIG_NOSSAT_ONE_BOARD)
    message(STATUS "Building for Nosyna Satellite One")

    list(APPEND SOURCES
        board/nossat_one.cpp
        hal/audio_input_inmp441.cpp
        hal/audio_output_max98357a.cpp
        hal/knob.cpp
        hal/lvgl_knob.cpp
        hal/led.cpp
        hal/led_blinker.cpp
        hal/display_st7735.cpp
        hal/display_st7735.c
    )

    list(APPEND LVGL_SOURCES
        gui/gui_one.cpp

        gui/nossat-one/src/ui/images.c
        gui/nossat-one/src/ui/screens.c
        gui/nossat-one/src/ui/styles.c
        gui/nossat-one/src/ui/ui.c
    )
endif()

# set_source_files_properties(${LVGL_SOURCES}
#     PROPERTIES COMPILE_OPTIONS -DLV_LVGL_H_INCLUDE_SIMPLE)

idf_component_register(SRCS ${SOURCES} ${LVGL_SOURCES} INCLUDE_DIRS ".")

spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)

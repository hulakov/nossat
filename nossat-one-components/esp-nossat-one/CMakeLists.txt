idf_component_register(SRCS "esp-nossat-one-display.c" "esp-nossat-one-led.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver spiffs
                       PRIV_REQUIRES "esp_lcd_st7735")

idf_component_register(SRCS "esp-nossat-one.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver spiffs
                       PRIV_REQUIRES "esp_lcd_st7735")

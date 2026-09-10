idf_component_register(SRCS "task1.c"
                       INCLUDE_DIRS "." "include"
                       REQUIRES esp_driver_gpio esp_driver_gptimer esp_adc esp_timer)
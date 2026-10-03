idf_component_register(SRCS "task1.c" "pwm_hw.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_ledc driver esp_driver_gpio esp_driver_gptimer esp_timer esp_driver_pcnt)
set(COMPONENT_SRCS "tictactoe.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_PRIV_REQUIRES lvgl lvgl_esp32_drivers esp_timer gui)

register_component()

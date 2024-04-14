set(COMPONENT_SRCS "my_wifi.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES nvs_flash esp_wifi)

register_component()

set(COMPONENT_SRCS "my_mqtt.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES driver mqtt nvs_flash esp_netif protocol_examples_common json temp_hum_sensor tictactoe) 

register_component()

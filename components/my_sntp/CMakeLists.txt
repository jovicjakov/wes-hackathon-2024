set(COMPONENT_SRCS "my_sntp.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES driver esp_event nvs_flash esp_netif protocol_examples_common json)

register_component()
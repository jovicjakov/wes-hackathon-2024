set(COMPONENT_SRCS "user_interface.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES gui led joystick my_wifi my_sntp temp_sensor)

register_component()

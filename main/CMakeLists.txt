# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES ble_conn)
set(COMPONENT_PRIV_REQUIRES nvs_flash)

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")


register_component()

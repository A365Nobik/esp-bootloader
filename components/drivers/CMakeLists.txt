idf_component_register(
    SRCS  "battery.cpp" "display.cpp" "storage.c"
    INCLUDE_DIRS "include"
    REQUIRES config LovyanGFX utils fatfs
)
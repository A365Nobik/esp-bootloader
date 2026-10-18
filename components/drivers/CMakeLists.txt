idf_component_register(
    SRCS "battery.cpp" "display.cpp"
    INCLUDE_DIRS "include"
    REQUIRES config LovyanGFX utils
)
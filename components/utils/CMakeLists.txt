idf_component_register(
    SRCS "board_info.c" "boot.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_hw_support spi_flash app_update config
)

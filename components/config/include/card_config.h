#pragma once

#define ENABLE_SD

#ifdef ENABLE_SD
#define STORAGE_ROOT "/sdcard"
#define HOME_DIR STORAGE_ROOT "/esp-bootloader"
#define SD_SPI_HOST SPI3_HOST
#define SD_SPI_SPEED 100000
#define SD_SPI_MISO GPIO_NUM_3
#define SD_SPI_MOSI GPIO_NUM_18
#define SD_SPI_SCLK GPIO_NUM_8
#define SD_SPI_CS GPIO_NUM_17
#endif

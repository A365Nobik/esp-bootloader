#include "board_info.h"
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_heap_caps.h>
#include <esp_clk_tree.h>
#include "esp_partition.h"

const char *get_chip_name(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    const char *chip_name = "Unknown";
    switch (chip_info.model)
    {
    case CHIP_ESP32:
        chip_name = "ESP32";
        break;
    case CHIP_ESP32S2:
        chip_name = "ESP32-S2";
        break;
    case CHIP_ESP32S3:
        chip_name = "ESP32-S3";
        break;
    case CHIP_ESP32C3:
        chip_name = "ESP32-C3";
        break;
    case CHIP_ESP32C2:
        chip_name = "ESP32-C2";
        break;
    case CHIP_ESP32C6:
        chip_name = "ESP32-C6";
        break;
    case CHIP_ESP32H2:
        chip_name = "ESP32-H2";
        break;
    default:
        chip_name = "?";
        break;
    }
    return chip_name;
}

uint32_t get_flash_size_bytes(void)
{
    uint32_t flash = 0;
    if (esp_flash_get_size(NULL, &flash) == ESP_OK)
    {
        return flash;
    }
    return 0;
}

size_t get_sram_size_bytes(void)
{
    return heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
}

size_t get_psram_size_bytes(void)
{
    return heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
}

uint32_t get_chip_freq_hz(void)
{
    uint32_t freq = 0;
    if (esp_clk_tree_src_get_freq_hz(SOC_MOD_CLK_CPU, ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &freq) == ESP_OK)
    {
        return freq;
    }
    return 0;
}

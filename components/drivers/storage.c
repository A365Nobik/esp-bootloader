#include "card_config.h"
#include "storage.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

static const char *TAG = "storage";

esp_err_t init_storage(void)
{
    ESP_LOGI(TAG, "Initializing storage...");

    esp_err_t res;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 1,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(TAG, "Initializing SPI bus...");

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_SPI_MOSI,
        .miso_io_num = SD_SPI_MISO,
        .sclk_io_num = SD_SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    res = spi_bus_initialize(SD_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI bus not initialized: %s", esp_err_to_name(res));
        return res;
    }
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SD_SPI_HOST;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SD_SPI_CS;
    slot_config.host_id = SD_SPI_HOST;

    ESP_LOGI(TAG, "Mounting FS to:%s", STORAGE_ROOT);

    sdmmc_card_t *card;

    res = esp_vfs_fat_sdspi_mount(STORAGE_ROOT, &host, &slot_config, &mount_config, &card);

    if (res != ESP_OK)
    {
        if (res == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Mounting FAT FS error, please ensure card is formatted by FAT32");
        }
        else
        {
            ESP_LOGE(TAG, "Card initializing error:%s", esp_err_to_name(res));
        }
        spi_bus_free(SD_SPI_HOST);
        card = NULL;
        return res;
    }
    ESP_LOGI(TAG, "Storage initialized");
    sdmmc_card_print_info(stdout, card);

    return ESP_OK;
}
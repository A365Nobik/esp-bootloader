#include "card_config.h"
#include "storage.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "esp_ota_ops.h"

static const char *TAG = "Storage";

esp_err_t storage_begin(void)
{
    ESP_LOGI(TAG, "Initializing storage...");

    esp_err_t res;

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
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

    struct stat st;

    if (stat(HOME_DIR, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        ESP_LOGW(TAG, "No home dir (%s), creating...", HOME_DIR);
        if (mkdir(HOME_DIR, 0) != 0)
        {
            ESP_LOGE(TAG, "mkdir failed: %s", strerror(errno));
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

uint8_t storage_init_apps(storage_app_t arr[], uint8_t max)
{
    DIR *d = opendir(HOME_DIR);
    if (!d)
    {
        ESP_LOGE(TAG, "opendir failed: %s", strerror(errno));
        return 0;
    }

    uint8_t n = 0;
    struct dirent *e;
    while (n < max && (e = readdir(d)) != NULL)
    {
        if (e->d_type == DT_DIR || e->d_name[0] == '.')
        {
            continue;
        }

        const char *ext = strrchr(e->d_name, '.');
        if (!ext || strcasecmp(ext, ".bin") != 0)
        {
            continue;
        }

        char path[300];
        snprintf(path, sizeof(path), "%s/%s", HOME_DIR, e->d_name);

        struct stat st;
        if (stat(path, &st) != 0)
        {
            continue;
        }

        strlcpy(arr[n].name, e->d_name, STORAGE_APP_NAME_LEN);
        arr[n].size = (uint32_t)st.st_size;
        n++;
    }
    closedir(d);
    return n;
}

esp_err_t storage_flash_app(const char *name, const esp_partition_t *part)
{
    ESP_LOGI(TAG, "Trying to flash...");
    char path[300];
    snprintf(path, sizeof(path), "%s/%s", HOME_DIR, name);
    struct stat st;
    if (stat(path, &st) != 0)
    {
        return ESP_ERR_NOT_FOUND;
    }
    if ((size_t)st.st_size > part->size)
    {
        return ESP_ERR_INVALID_SIZE;
    }

    FILE *f = fopen(path, "rb");
    if (!f)
    {
        return ESP_FAIL;
    }

    esp_ota_handle_t h;
    esp_err_t err = esp_ota_begin(part, st.st_size, &h);
    if (err != ESP_OK)
    {
        fclose(f);
        return err;
    }
    static uint8_t buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
    {
        err = esp_ota_write(h, buf, n);
        if (err != ESP_OK)
        {
            esp_ota_abort(h);
            fclose(f);
            return err;
        }
    }
    fclose(f);

    return esp_ota_end(h);
}
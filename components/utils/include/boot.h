#pragma once
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_app_format.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        const esp_partition_t *part;
        const char *label;
        uint32_t address;
        bool encrypted;
        bool read_only;
        uint32_t size;
    } boot_app_t;

    esp_err_t boot_load_app(const esp_partition_t *app);
    uint8_t boot_init_apps(boot_app_t arr[], uint8_t max);
    bool part_has_image(const esp_partition_t *part);
    const esp_partition_t *boot_has_storage_part(const char *name);

#ifdef __cplusplus
}
#endif

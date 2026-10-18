#pragma once
#include "esp_ota_ops.h"
#include "esp_partition.h"

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

    bool boot_app(const esp_partition_t *app);
    uint8_t init_apps(boot_app_t arr[], uint8_t max);

#ifdef __cplusplus
}
#endif
#pragma once
#include "esp_err.h"
#include "card_config.h"
#include "esp_partition.h"

#define STORAGE_APP_NAME_LEN 255

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        char name[STORAGE_APP_NAME_LEN];
        uint32_t size;
    } storage_app_t;

    esp_err_t storage_begin(void);
    uint8_t storage_init_apps(storage_app_t arr[], uint8_t max);
    esp_err_t storage_flash_app(const char *name, const esp_partition_t *part);

#ifdef __cplusplus
}
#endif

#include "boot.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "BOOT";

bool boot_app(const esp_partition_t *app)
{
    if (app == NULL)
    {
        ESP_LOGE(TAG, "App is null");
        return false;
    }
    if (app->type != ESP_PARTITION_TYPE_APP)
    {
        ESP_LOGE(TAG, "App type is not correct");
        return false;
    }
    if (esp_ota_set_boot_partition(app) != ESP_OK)
    {
        ESP_LOGE(TAG, "Cannot set boot partition for app");
        return false;
    }
    ESP_LOGI(TAG, "Booting app Name:%s,  Addres:%lu", app->label, app->address);
    esp_restart();
    return true;
}

const esp_partition_t *get_app_by_id(uint8_t id)
{
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_ANY,
        NULL);
    uint8_t curr_id = 1;
    while (it != NULL)
    {

        if (curr_id == id)
        {
            return esp_partition_get(it);
        }
        curr_id++;
        it = esp_partition_next(it);
    }

    esp_partition_iterator_release(it);
    it = NULL;
    return NULL;
}

const char *get_app_subtype_str(uint8_t subtype)
{
    if (subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY)
    {
        return "Factory";
    }
    if (subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_0 && subtype <= ESP_PARTITION_SUBTYPE_APP_OTA_15)
    {
        static char ota_str[16];
        snprintf(ota_str, sizeof(ota_str), "OTA %d", subtype - ESP_PARTITION_SUBTYPE_APP_OTA_0);
        return ota_str;
    }
    if (subtype == ESP_PARTITION_SUBTYPE_APP_TEST)
    {
        return "Test App";
    }
    return "Unknown";
}

uint8_t init_apps(boot_app_t arr[], uint8_t max)
{
    uint8_t n = 0;
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL && n < max; it = esp_partition_next(it))
    {
        const esp_partition_t *p = esp_partition_get(it);
        if (p->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY)
            continue;
        arr[n++] = (boot_app_t){
            .part = p,
            .label = p->label,
            .address = p->address,
            .encrypted = p->encrypted,
            .read_only = p->readonly,
            .size = p->size};
    }
    esp_partition_iterator_release(it);
    return n;
}
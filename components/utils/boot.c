#include "boot.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "Boot";

bool part_has_image(const esp_partition_t *part)
{
    uint8_t magic = 0;
    if (esp_partition_read(part, 0, &magic, 1) != ESP_OK)
        return false;
    return magic == ESP_IMAGE_HEADER_MAGIC;
}

uint8_t boot_init_apps(boot_app_t arr[], uint8_t max)
{
    uint8_t n = 0;
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);

    for (; it != NULL && n < max; it = esp_partition_next(it))
    {
        const esp_partition_t *p = esp_partition_get(it);
        if (p->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY)
        {
            continue;
        }
        if (!part_has_image(p))
        {
            continue;
        }
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

esp_err_t boot_load_app(const esp_partition_t *app)
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
    esp_err_t err = esp_ota_set_boot_partition(app);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Partition boot error: %s", esp_err_to_name(err));

        return err;
    }
    ESP_LOGI(TAG, "App: %s loaded", app->label);
    return ESP_OK;
}

const esp_partition_t *boot_has_storage_part(const char *name)
{
    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL)
    {
        const esp_partition_t *p = esp_partition_get(it);
        if (strcmp(p->label, name) == 0 && p->type == ESP_PARTITION_TYPE_APP)
        {
            esp_partition_iterator_release(it);
            return p;
        }
        it = esp_partition_next(it);
    }
    return NULL;
}

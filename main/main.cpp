#include "esp_log.h"
#include "display.h"
#include "esp_partition.h"
#include "buttons_config.h"
#include "globals.h"
#include "button.h"
#include "boot.h"
#include "storage.h"

static const char *TAG = "Main";

Display lcd;

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Initializing display");
    if (!lcd.init())
    {
        ESP_LOGE(TAG, "Failed to init display");
        return;
    }
    ESP_LOGI(TAG, "Initialized display");
    lcd.intro();

    ESP_LOGI(TAG, "Initializing buttons");
    Button key_up;
    Button key_down;
    Button key_select;
    Button key_storage;

    key_up.init(KEY_UP_PIN);
    key_down.init(KEY_DOWN_PIN);
    key_select.init(KEY_SELECT_PIN);
    key_storage.init(KEY_STORAGE_PIN);
    ESP_LOGI(TAG, "Initialized buttons");
    vTaskDelay(pdMS_TO_TICKS(100));

#ifdef ENABLE_SD
    esp_err_t card_err = storage_begin();
    bool card_ok = card_err == ESP_OK;

    if (!card_ok)
    {
        char info[256];
        snprintf(info, sizeof(info), "Storage error: %s", esp_err_to_name(card_err));
        lcd.drawPanic(info);
        btn_wait_click(key_select);
    }
#else
    bool card_ok = false;
    uint8_t storage_length = 0;
#endif

    static menu_layout layout;
    layout.update(LCD_WIDTH, LCD_HEIGHT);

    boot_app_t apps[APPS_AMOUNT];
    uint8_t length = boot_init_apps(apps, APPS_AMOUNT);

#ifdef ENABLE_SD
    storage_app_t storage_apps[STORAGE_APPS_AMOUNT];
    uint8_t storage_length = storage_init_apps(storage_apps, STORAGE_APPS_AMOUNT);
#endif

    uint8_t selected = 0;
    bool is_flash = true;

    lcd.drawAll(layout, apps, length, selected, card_ok);

    while (1)
    {
#ifdef ENABLE_SD
        if (key_storage.is_clicked() && card_ok)
        {
            is_flash = !is_flash;
            lcd.changeAppsType(layout, is_flash);
            selected = 0;
            if (is_flash)
            {
                lcd.highlightApp(layout, apps, length, selected);
            }
            else
            {
                lcd.highlightStorageApp(layout, storage_apps, storage_length, selected);
            }
        }
#endif
        if (length != 0 || storage_length != 0)
        {
            if (key_up.is_clicked())
            {
                selected = (selected + length - 1) % length;
                if (is_flash)
                {
                    lcd.highlightApp(layout, apps, length, selected);
                }
#ifdef ENABLE_SD
                else
                {
                    lcd.highlightStorageApp(layout, storage_apps, storage_length, selected);
                }
#endif
            }
            if (key_down.is_clicked())
            {
                selected = (selected + 1) % length;
                if (is_flash)
                {
                    lcd.highlightApp(layout, apps, length, selected);
                }
#ifdef ENABLE_SD
                else
                {
                    lcd.highlightStorageApp(layout, storage_apps, storage_length, selected);
                }
#endif
            }
            if (key_select.is_clicked())
            {
                if (is_flash)
                {
                    esp_err_t boot_err = boot_load_app(apps[selected].part);
                    if (boot_err != ESP_OK)
                    {
                        char info[256];
                        snprintf(info, sizeof(info), "Load error: %s\n Check monitor for details", esp_err_to_name(boot_err));
                        lcd.drawPanic(info);
                        btn_wait_click(key_select);
                        lcd.drawAll(layout, apps, length, selected, card_ok);
                    }
                    else
                    {
                        ESP_LOGI(TAG, "Booting app Name:%s,  Addres:%lu", apps[selected].label, apps[selected].address);
                        esp_restart();
                    }
                }
#ifdef ENABLE_SD
                else
                {
                    const esp_partition_t *target = esp_ota_get_next_update_partition(NULL);
                    esp_err_t err = target ? storage_flash_app(storage_apps[selected].name, target)
                                           : ESP_ERR_NOT_FOUND;
                    if (err == ESP_OK)
                    {
                        err = boot_load_app(target);
                    }
                    if (err != ESP_OK)
                    {
                        char info[256];
                        snprintf(info, sizeof(info), "Flash error: %s\n Check monitor for details", esp_err_to_name(err));
                        lcd.drawPanic(info);
                        btn_wait_click(key_select);
                        lcd.drawAll(layout, apps, length, selected, card_ok);
                    }
                    else
                    {
                        esp_restart();
                    }
                }
#endif
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
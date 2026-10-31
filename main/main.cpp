#include "esp_log.h"
#include "display.h"
#include "esp_partition.h"
#include "globals.h"
#ifndef USE_SENSOR
#include "buttons_config.h"
#include "button.h"
#endif
#include "boot.h"
#include "storage.h"

static const char *TAG = "Main";
static menu_layout layout;

Display lcd;

void wait_click()
{
#ifndef USE_SENSOR
    btn_wait_click(key_select);
#else
    lcd.waitTouch();
#endif
}

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
    layout.update(LCD_WIDTH, LCD_HEIGHT);

#ifndef USE_SENSOR
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
#endif
#ifdef ENABLE_SD
    esp_err_t card_err = storage_begin();
    bool card_ok = card_err == ESP_OK;

    if (!card_ok)
    {
        char info[256];
        snprintf(info, sizeof(info), "Storage error: %s", esp_err_to_name(card_err));
        lcd.drawPanic(info);
        wait_click();
    }
#else
    bool card_ok = false;
    uint8_t storage_length = 0;
#endif

    layout.update(LCD_WIDTH, LCD_HEIGHT);

    boot_app_t apps[APPS_AMOUNT];
    uint8_t length = boot_init_apps(apps, APPS_AMOUNT);
    ESP_LOGI(TAG, "Flash len:%d", length);

#ifdef ENABLE_SD
    storage_app_t storage_apps[STORAGE_APPS_AMOUNT];
    uint8_t storage_length = storage_init_apps(storage_apps, STORAGE_APPS_AMOUNT);
    ESP_LOGI(TAG, "Storage len:%d", storage_length);
#endif

    uint8_t selected = 0;
    bool is_flash = true;

    lcd.drawAll(layout, apps, length, selected, card_ok);

    while (1)
    {
#ifdef USE_SENSOR
        const char *s_btn = lcd.proceedSensor(layout, card_ok);
#endif
#ifdef ENABLE_SD
#ifdef USE_SENSOR
        if (s_btn && strcmp(s_btn, SYM_TYPE) == 0 && card_ok)
#else
        if (key_storage.is_clicked() && card_ok)
#endif
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
#ifdef USE_SENSOR
            if (s_btn && strcmp(s_btn, SYM_UP) == 0)
#else
            if (key_up.is_clicked())
#endif
            {
                if (is_flash)
                {
                    selected = (selected + length - 1) % length;
                    lcd.highlightApp(layout, apps, length, selected);
                }
#ifdef ENABLE_SD
                else
                {
                    selected = (selected + storage_length - 1) % storage_length;
                    lcd.highlightStorageApp(layout, storage_apps, storage_length, selected);
                }
#endif
            }
#ifdef USE_SENSOR
            if (s_btn && strcmp(s_btn, SYM_DOWN) == 0)
#else
            if (key_down.is_clicked())
#endif
            {
                if (is_flash)
                {
                    selected = (selected + 1) % length;
                    lcd.highlightApp(layout, apps, length, selected);
                }
#ifdef ENABLE_SD
                else
                {
                    selected = (selected + 1) % storage_length;
                    lcd.highlightStorageApp(layout, storage_apps, storage_length, selected);
                }
#endif
            }
#ifdef USE_SENSOR
            if (s_btn && strcmp(s_btn, SYM_OK) == 0)
#else
            if (key_select.is_clicked())
#endif
            {
                if (is_flash)
                {
                    esp_err_t boot_err = boot_load_app(apps[selected].part);
                    if (boot_err != ESP_OK)
                    {
                        char info[256];
                        snprintf(info, sizeof(info), "Load error: %s\n Check monitor for details", esp_err_to_name(boot_err));
                        lcd.drawPanic(info);
                        wait_click();
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
                    const esp_partition_t *target = boot_has_storage_part(STORAGE_PART_NAME);
                    if (target == NULL)
                    {
                        char info[256];
                        snprintf(info, sizeof(info), "You have to create '%s' part for using storage apps", STORAGE_PART_NAME);
                        lcd.drawPanic(info);
                        wait_click();
                        lcd.drawAll(layout, apps, length, selected, card_ok);
                        continue;
                    }
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
                        wait_click();
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

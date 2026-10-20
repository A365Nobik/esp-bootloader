#include "esp_log.h"
#include "display.h"
#include "esp_partition.h"
#include "buttons_config.h"
#include "globals.h"
#include "button.h"
#include "boot.h"
#include "storage.h"

static const char *TAG = "MAIN";

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
    Button key_a;
    // Button key_menu;

    key_up.init(KEY_UP_PIN);
    key_down.init(KEY_DOWN_PIN);
    key_a.init(KEY_SELECT_PIN);
    ESP_LOGI(TAG, "Initialized buttons");
    vTaskDelay(pdMS_TO_TICKS(100));

    esp_err_t card_err = init_storage();
    if (card_err != ESP_OK)
    {
        char info[256];
        snprintf(info, sizeof(info), "Storage error: %s", esp_err_to_name(card_err));
        lcd.drawPanic(info);
        while (1)
        {
            if (key_a.isClicked())
            {
                break;
            }
            vTaskDelay(1);
        }
    }
    static menu_layout layout;
    layout.update(LCD_WIDTH, LCD_HEIGHT);
    lcd.drawMenuLayout(layout);
    lcd.printMenuInfo(layout,card_err==ESP_OK);

    boot_app_t apps[APPS_AMOUNT];
    uint8_t length = init_apps(apps, APPS_AMOUNT);

    uint8_t selected = 0;
    lcd.highlightApp(layout, apps, length, selected);

    while (1)
    {
        if (length != 0)
        {
            if (key_up.isClicked())
            {
                selected = (selected + length - 1) % length;
                lcd.highlightApp(layout, apps, length, selected);
            }
            if (key_down.isClicked())
            {
                selected = (selected + 1) % length;
                lcd.highlightApp(layout, apps, length, selected);
            }
            if (key_a.isClicked())
            {
                boot_app(apps[selected].part);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
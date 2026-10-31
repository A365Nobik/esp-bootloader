#pragma once
#include "lgfx.hpp"
#include "freertos/FreeRTOS.h"
#include "esp_partition.h"
#include "boot.h"
#include "storage.h"

#ifdef USE_SENSOR
#define SYM_OK "OK"
#define SYM_UP "UP"
#define SYM_DOWN "DW"
#define SYM_TYPE "T"
#endif

struct menu_layout
{
    static constexpr uint8_t padding = 4;
    float header_pers = 0.05;
    float footer_pers = 0.05;
    uint16_t screen_w;
    uint16_t screen_h;

    uint16_t header_h;
    uint16_t header_y;
    uint16_t footer_h;
    uint16_t footer_y;
    // uint8_t controls_h;

    uint16_t table_h;
    uint16_t table_y;
    uint16_t table_n_x;
    uint16_t table_n_y;
    uint16_t table_name_x;

    uint16_t apps_y;
    uint16_t apps_h;
    uint16_t app_sidebar_x;
    uint16_t app_sidebar_y;

    uint16_t app_info_x;
    uint16_t app_info_y;
    uint16_t app_info_w;
    uint16_t app_info_h;

    uint16_t sensor_x;
    uint16_t sensor_y;
    uint16_t sensor_w;
    uint16_t sensor_h;

    void update(uint16_t w, uint16_t h)
    {
        screen_w = w;
        screen_h = h;

        header_h = screen_h * header_pers;
        header_y = header_h / 4;
        footer_h = screen_h * footer_pers;
        footer_y = screen_h - footer_h;
        table_h = header_h * 2;

        apps_y = header_h;
        apps_h = screen_h - (header_h + footer_h);
        table_n_x = padding * 4;
        table_n_y = apps_h;

        app_sidebar_x = screen_w / 2;
        app_sidebar_y = header_h;

        app_info_x = app_sidebar_x + padding;
        app_info_y = table_h + padding;
        app_info_h = (screen_h - 2 * (footer_h + table_h)) / 2;
        app_info_w = screen_w - app_sidebar_x;

        sensor_x = app_sidebar_x;
        sensor_y = screen_h / 2;
        sensor_w = app_info_w;
        sensor_h = app_info_h;
    }
};

class Display
{
private:
    inline static LGFX _lcd;
    const char *_intro_title = "ESP-BOOTLOADER";
    uint8_t _prev_brg = LCD_START_BRG;
    const char *_TAG = "Display";
    bool inside(int32_t x, int32_t y,
                int32_t rx, int32_t ry,
                int32_t w, int32_t h)
    {
        return x >= rx && x <= rx + w &&
               y >= ry && y <= ry + h;
    }

public:
    bool init();
    void intro();
    void drawMenuLayout(const menu_layout &layout);
    void printMenuInfo(const menu_layout &layout, const bool cardState);
    void drawPanic(const char *msg);

    void highlightApp(const menu_layout &layout, boot_app_t apps[], uint8_t length, uint8_t id);
    void drawAppInfo(const menu_layout &layout, boot_app_t app);
    void drawAll(const menu_layout &layout, boot_app_t apps[], uint8_t length, uint8_t id, const bool cardState);
    void changeAppsType(const menu_layout &layout, const bool is_flash);
    void highlightStorageApp(const menu_layout &layout, storage_app_t apps[], uint8_t length, uint8_t id);
    void drawStorageAppInfo(const menu_layout &layout, storage_app_t app);
    void drawSensorButtons(const menu_layout &layout, const bool card_state);
    const char *proceedSensor(const menu_layout &layout, bool card_state);
    void waitTouch();
};

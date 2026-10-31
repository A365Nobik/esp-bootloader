#include "display.h"
#include <cstring>
#include "board_info.h"
#include "esp_log.h"
#include "globals.h"

bool Display::init()
{
    if (_lcd.init())
    {
        _lcd.setRotation(LCD_ROTATE);
#ifdef USE_SENSOR
        if (!_lcd.touch()->init())
        {
            return false;
        }
#endif
        return true;
    }
    return false;
}

void Display::intro()
{
    _lcd.clear();
    _lcd.setBrightness(0);
    _lcd.fillScreen(TFT_BLUE);
    _lcd.setColor(TFT_WHITE);
    if (_lcd.width() < _lcd.height())
    {
        _lcd.setTextSize(2);
    }
    else
    {
        _lcd.setTextSize(3);
    }
    _lcd.setTextDatum(textdatum_t::middle_center);

    const size_t len = strlen(_intro_title);
    std::string current;
    current.reserve(len);

    int16_t cx = _lcd.width() / 2;
    int16_t cy = _lcd.height() / 2;
    _lcd.setBrightness(LCD_START_BRG);
    for (size_t i = 0; i < len; i++)
    {
        current += _intro_title[i];

        _lcd.drawString(current.c_str(), cx, cy);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    _lcd.clear();
}

void Display::drawMenuLayout(const menu_layout &layout)
{
    _lcd.fillScreen(TFT_BLUE);

    _lcd.drawFastHLine(0, layout.header_h, layout.screen_w, TFT_WHITE);
    _lcd.drawFastHLine(0, layout.screen_h - layout.footer_h, layout.screen_w, TFT_WHITE);
    _lcd.drawFastHLine(0, layout.table_h, layout.screen_w, TFT_WHITE);

    _lcd.drawFastVLine(layout.app_sidebar_x, layout.app_sidebar_y, layout.apps_h);
}

void Display::printMenuInfo(const menu_layout &layout, const bool cardState)
{
    _lcd.setFont(&fonts::Font0);
    _lcd.setTextSize(1);
    _lcd.setTextColor(TFT_WHITE);
    _lcd.setTextDatum(textdatum_t::top_left);

    // header
    const char *title = "Esp-Bootloader";
    _lcd.drawString(title, layout.padding, layout.header_y);
    int16_t h_x = _lcd.textWidth(title) + layout.padding;
    _lcd.drawFastVLine(h_x, 0, layout.header_h);

    h_x += layout.padding;
    char stateBuf[6];
    sniprintf(stateBuf, sizeof(stateBuf), "SD:%s", cardState ? "OK" : "NO");
    _lcd.drawString(stateBuf, h_x, layout.header_y);
    h_x += _lcd.textWidth(stateBuf) + layout.padding;
    _lcd.drawFastVLine(h_x, 0, layout.header_h);

    // table

    _lcd.setTextDatum(textdatum_t::middle_left);
    uint16_t t_cy = 1 + (layout.header_h + layout.header_h / 2);
    _lcd.drawString("App info", layout.app_sidebar_x + layout.padding, t_cy);

    // footer
    char footerBuf[64];
    snprintf(footerBuf, sizeof(footerBuf), "V:%s", VERSION);
    uint16_t f_cy = 1 + (layout.footer_y + layout.footer_h / 2);
    uint16_t cur_f_x = layout.padding;
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
    cur_f_x += _lcd.textWidth(footerBuf) + layout.padding;
    _lcd.drawFastVLine(cur_f_x, layout.footer_y, layout.footer_h);

    if (layout.screen_h > layout.screen_w)
    {
        snprintf(footerBuf, sizeof(footerBuf), "C:%s", get_chip_name(true));
    }
    else
    {
        snprintf(footerBuf, sizeof(footerBuf), "Chip:%s", get_chip_name(false));
    }
    cur_f_x += layout.padding;
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
    cur_f_x += _lcd.textWidth(footerBuf) + layout.padding;
    _lcd.drawFastVLine(cur_f_x, layout.footer_y, layout.footer_h);

    cur_f_x += layout.padding;
    snprintf(footerBuf, sizeof(footerBuf), "F:%dMb", int(get_flash_size_bytes() / (1024 * 1024)));
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
    cur_f_x += _lcd.textWidth(footerBuf) + layout.padding;
    _lcd.drawFastVLine(cur_f_x, layout.footer_y, layout.footer_h);

    cur_f_x += layout.padding;
    snprintf(footerBuf, sizeof(footerBuf), "P:%dMb", int(get_psram_size_bytes() / (1024 * 1024)));
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
    cur_f_x += _lcd.textWidth(footerBuf) + layout.padding;
    _lcd.drawFastVLine(cur_f_x, layout.footer_y, layout.footer_h);

    cur_f_x += layout.padding;
    snprintf(footerBuf, sizeof(footerBuf), "S:%dKb", int(get_sram_size_bytes() / 1024));
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
    cur_f_x += _lcd.textWidth(footerBuf) + layout.padding;
    _lcd.drawFastVLine(cur_f_x, layout.footer_y, layout.footer_h);

    cur_f_x += layout.padding;
    if (layout.screen_h > layout.screen_w)
    {
        snprintf(footerBuf, sizeof(footerBuf), "%dMHz", int(get_chip_freq_hz() / (1000000)));
    }
    else
    {
        snprintf(footerBuf, sizeof(footerBuf), "Fr:%dMHz", int(get_chip_freq_hz() / (1000000)));
    }
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
}

void Display::drawAppInfo(const menu_layout &layout, boot_app_t app)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(1);
    const int32_t font_h = _lcd.fontHeight(font);

    ESP_LOGI(_TAG, "Current app: %s", app.label);

    uint16_t cur_y = layout.app_info_y;
    uint16_t cur_x = layout.app_info_x + 1;

    _lcd.fillRect(cur_x, cur_y, layout.app_info_w, layout.app_info_h, TFT_BLUE);
    cur_y += layout.padding;
    _lcd.setTextDatum(textdatum_t::middle_left);

    char infoBuf[64];
    snprintf(infoBuf, sizeof(infoBuf), "Addres:%lu", app.address);
    _lcd.drawString(infoBuf, cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;

    snprintf(infoBuf, sizeof(infoBuf), "Encrypted:%s", app.encrypted ? "True" : "False");
    _lcd.drawString(infoBuf, cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;

    snprintf(infoBuf, sizeof(infoBuf), "Read only:%s", app.read_only ? "True" : "False");
    _lcd.drawString(infoBuf, cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;

    snprintf(infoBuf, sizeof(infoBuf), "Size:%lu KB", app.size / 1024);
    _lcd.drawString(infoBuf, cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;

    snprintf(infoBuf, sizeof(infoBuf), "Sub type: OTA %d", app.part->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_0);
    _lcd.drawString(infoBuf, cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;

    _lcd.drawString("Type: App", cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;
}

void Display::changeAppsType(const menu_layout &layout, const bool is_flash)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(1);
    _lcd.setTextDatum(textdatum_t::middle_left);

    uint16_t y = 1 + (layout.header_h + layout.header_h / 2);
    uint16_t x = 0;
    _lcd.fillRect(x, layout.header_h + 1, layout.app_sidebar_x, layout.header_h, TFT_BLUE);
    _lcd.drawFastHLine(0, layout.table_h, layout.app_sidebar_x, TFT_WHITE);

    x += layout.padding;
    char label[20];
    snprintf(label, sizeof(label), "Apps: %s", is_flash ? "Flash" : "Storage");
    _lcd.drawString(label, x, y);
    ESP_LOGI(_TAG, "App type changed to: %s", is_flash ? "Flash" : "Storage");
}

void Display::highlightApp(const menu_layout &layout, boot_app_t apps[], uint8_t length, uint8_t id)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(1);
    const int32_t row_h = _lcd.fontHeight(font) + layout.padding * 2 - 2;
    const uint16_t id_x = layout.table_n_x / 2;
    const uint16_t name_x = layout.padding + layout.table_n_x;

    _lcd.fillRect(0, layout.table_h + 1, layout.app_sidebar_x, layout.footer_y - layout.table_h - 1, TFT_BLUE);

    if (length == 0)
    {
        _lcd.drawString("No apps found", layout.padding, layout.table_h + row_h);
        return;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        const bool sel = (i == id);
        const uint16_t y = layout.table_h + i * row_h + layout.padding;

        _lcd.fillRect(0, y, layout.app_sidebar_x, row_h, sel ? TFT_WHITE : TFT_BLUE);
        _lcd.setTextColor(sel ? TFT_BLUE : TFT_WHITE);
        _lcd.setTextDatum(textdatum_t::middle_center);
        _lcd.drawNumber(i, id_x, y + row_h / 2);
        _lcd.setTextDatum(textdatum_t::middle_left);
        _lcd.drawString(apps[i].label, name_x, y + row_h / 2);
    }
    _lcd.setTextColor(TFT_WHITE);
    drawAppInfo(layout, apps[id]);
}
void Display::drawPanic(const char *msg)
{
    _lcd.clear();
    _lcd.fillScreen(TFT_RED);
    _lcd.setTextColor(TFT_WHITE);
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(2);
    _lcd.setTextDatum(top_left);
    _lcd.setCursor(0, 0);
    _lcd.setTextWrap(true);
    _lcd.print(msg);

    // _lcd.setTextDatum(middle_center);
    // int32_t y = _lcd.height() - _lcd.fontHeight(font);
    // int32_t x = (_lcd.width() - _lcd.textWidth("OK")) / 2;
    // _lcd.fillRect(0, y, _lcd.width(), _lcd.height() - y, TFT_WHITE);
    // // _lcd.setTextColor(TFT_RED);
    // // _lcd.setTextDatum(bottom_center);
    // // _lcd.drawCenterString("OK", _lcd.width() / 2, y + (_lcd.height() - y) / 2);
}

void Display::drawAll(const menu_layout &layout, boot_app_t apps[], uint8_t length, uint8_t id, const bool cardState)
{
    _lcd.clear();
    drawMenuLayout(layout);
    printMenuInfo(layout, cardState);
    highlightApp(layout, apps, length, id);
    changeAppsType(layout, true);
#ifdef USE_SENSOR
    drawSensorButtons(layout, cardState);
#endif
}

void Display::highlightStorageApp(const menu_layout &layout, storage_app_t apps[], uint8_t length, uint8_t id)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(1);
    const int32_t row_h = _lcd.fontHeight(font) + layout.padding * 2 - 2;
    const uint16_t id_x = layout.table_n_x / 2;
    const uint16_t name_x = layout.padding + layout.table_n_x;

    _lcd.fillRect(0, layout.table_h + 1, layout.app_sidebar_x, layout.footer_y - layout.table_h - 1, TFT_BLUE);

    if (length == 0)
    {
        _lcd.drawString("No storage apps found", layout.padding, layout.table_h + row_h);
        return;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        const bool sel = (i == id);
        const uint16_t y = layout.table_h + i * row_h + layout.padding;
        _lcd.fillRect(0, y, layout.app_sidebar_x, row_h, sel ? TFT_WHITE : TFT_BLUE);
        _lcd.setTextColor(sel ? TFT_BLUE : TFT_WHITE);
        _lcd.setTextDatum(textdatum_t::middle_center);
        _lcd.drawNumber(i, id_x, y + row_h / 2);
        _lcd.setTextDatum(textdatum_t::middle_left);
        _lcd.drawString(apps[i].name, name_x, y + row_h / 2);
    }
    _lcd.setTextColor(TFT_WHITE);
    drawStorageAppInfo(layout, apps[id]);
}

void Display::drawStorageAppInfo(const menu_layout &layout, storage_app_t app)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(1);
    const int32_t font_h = _lcd.fontHeight(font);

    ESP_LOGI(_TAG, "Current storage app: %s", app.name);
    uint16_t cur_y = layout.app_info_y;
    uint16_t cur_x = layout.app_info_x + 1;
    _lcd.fillRect(cur_x, cur_y, layout.app_info_w, layout.app_info_h, TFT_BLUE);
    cur_y += layout.padding;
    _lcd.setTextDatum(textdatum_t::middle_left);

    char infoBuf[64];
    snprintf(infoBuf, sizeof(infoBuf), "Size:%lu KB", app.size / 1024);
    _lcd.drawString(infoBuf, cur_x, cur_y);
    cur_y += (font_h / 2) + layout.padding + 3;
}

void Display::drawSensorButtons(const menu_layout &layout, const bool card_state)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    _lcd.setTextSize(2);
    _lcd.setTextDatum(baseline_center);

    const uint8_t w = 48;
    const uint8_t h = 48;

    int32_t x = layout.sensor_x;
    int32_t y = layout.sensor_y;

    int32_t t_x = layout.sensor_x + layout.padding;
    int32_t t_y = layout.sensor_y + layout.padding;
    t_y += h / 2 + layout.padding;

    _lcd.drawFastHLine(x, y, layout.screen_h);

    y += layout.padding;
    x += layout.padding;

    t_x += w / 2;
    _lcd.drawRect(x, y, w, h);
    _lcd.drawString(SYM_UP, t_x, t_y);

    x += w + layout.padding;
    t_x = x + w / 2;
    _lcd.drawRect(x, y, w, h);
    _lcd.drawString(SYM_OK, t_x, t_y);

    if (card_state)
    {
        if (layout.screen_h > layout.screen_w)
        {
            _lcd.drawRect(x, y + h + layout.padding, w, h);
            _lcd.drawString(SYM_TYPE, t_x, y + h + (2 * layout.padding) + (h / 2));
        }
        else
        {
            x += w + layout.padding;
            t_x = x + w / 2;
            _lcd.drawRect(x, y, w, h);
            _lcd.drawString(SYM_TYPE, t_x, t_y);
        }
    }

    y += h + layout.padding;
    x = layout.sensor_x + layout.padding;
    _lcd.drawRect(x, y, w, h);
    t_x = x + w / 2;
    t_y = y + h / 2 + layout.padding;
    _lcd.drawString(SYM_DOWN, t_x, t_y);
}

const char *Display::proceedSensor(const menu_layout &layout, bool card_state)
{
    static bool was_touched = false;

    lgfx::touch_point_t tp;

    if (!_lcd.getTouch(&tp))
    {
        was_touched = false;
        return nullptr;
    }

    if (was_touched)
    {
        return nullptr;
    }

    was_touched = true;

    const int32_t w = 48;
    const int32_t h = 48;
    const int32_t p = layout.padding;

    const int32_t x = layout.sensor_x + p;
    const int32_t y = layout.sensor_y + p;

    const char *button = nullptr;

    if (inside(tp.x, tp.y, x, y, w, h))
    {
        button = SYM_UP;
    }
    else if (inside(tp.x, tp.y, x + w + p, y, w, h))
    {
        button = SYM_OK;
    }
    else if (inside(tp.x, tp.y, x, y + h + p, w, h))
    {
        button = SYM_DOWN;
    }
    else if (card_state)
    {
        if (layout.screen_h > layout.screen_w)
        {
            if (inside(tp.x, tp.y, x + w + p, y + h + p, w, h))
            {
                button = SYM_TYPE;
            }
        }
        else
        {
            if (inside(tp.x, tp.y, x + 2 * (w + p), y, w, h))
            {
                button = SYM_TYPE;
            }
        }
    }

    ESP_LOGI(
        _TAG, "Touch X:%d Y:%d Button:%s",
        tp.x, tp.y, button ? button : "NONE");
    return button;
}

void Display::waitTouch()
{
    lgfx::touch_point_t tp;

    while (!_lcd.getTouch(&tp))
        vTaskDelay(pdMS_TO_TICKS(10));
}

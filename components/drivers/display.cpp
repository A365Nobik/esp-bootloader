#include "display.h"
#include <cstring>
#include "board_info.h"
#include "esp_log.h"

bool Display::init()
{
    if (_lcd.init())
    {
        _lcd.setRotation(LCD_ROTATE);
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
    _lcd.setTextSize(3);
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
    // uint16_t cur_t_x = layout.padding;
    _lcd.drawString("Apps", layout.table_n_x + layout.padding, t_cy);

    _lcd.drawString("App info", layout.app_sidebar_x + layout.padding, t_cy);

    // footer
    const char *version = "V:0.0";
    uint16_t f_cy = 1 + (layout.footer_y + layout.footer_h / 2);
    uint16_t cur_f_x = layout.padding;
    _lcd.drawString(version, cur_f_x, f_cy);
    cur_f_x += _lcd.textWidth(version) + layout.padding;
    _lcd.drawFastVLine(cur_f_x, layout.footer_y, layout.footer_h);

    char footerBuf[64];
    snprintf(footerBuf, sizeof(footerBuf), "Chip: %s", get_chip_name());
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
    snprintf(footerBuf, sizeof(footerBuf), "Fr:%dMHz", int(get_chip_freq_hz() / (1000000)));
    _lcd.drawString(footerBuf, cur_f_x, f_cy);
}

void Display::drawAppInfo(const menu_layout &layout, boot_app_t app)
{
    ESP_LOGI(_TAG, "Drawing app info");
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    const int32_t font_h = _lcd.fontHeight(font);

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

void Display::highlightApp(const menu_layout &layout, boot_app_t apps[], uint8_t length, uint8_t id)
{
    auto font = &fonts::Font0;
    _lcd.setFont(font);
    const int32_t row_h = _lcd.fontHeight(font) + layout.padding * 2 - 2;
    const uint16_t id_x = layout.table_n_x / 2;
    const uint16_t name_x = layout.padding + layout.table_n_x;

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

    _lcd.setTextDatum(middle_center);
    int32_t y = _lcd.height() - _lcd.fontHeight(&fonts::Font0);
    int32_t x = (_lcd.width() / 2) - _lcd.textWidth("OK") + 1;
    _lcd.fillRect(0, y, _lcd.width(), _lcd.height() - y, TFT_WHITE);
    _lcd.setCursor(x, y + (_lcd.fontHeight(&fonts::Font0) / 2));
    _lcd.setTextColor(TFT_RED);
    _lcd.print("OK");
}

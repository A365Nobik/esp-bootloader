#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "display_config.h"

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
#ifdef USE_SENSOR
    lgfx::Touch_XPT2046 _sensor_instance;
#endif
public:
    LGFX(void)
    {
        // spi
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = LCD_SPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = LCD_SPI_WRITE_SPEED;
#ifdef USE_SENSOR
            cfg.freq_read = LCD_SPI_READ_SPEED;
#endif
            cfg.pin_sclk = LCD_PIN_SCLK;
            cfg.pin_mosi = LCD_PIN_MOSI;
            cfg.pin_miso = LCD_PIN_MISO;
            cfg.pin_dc = LCD_PIN_DS;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        // display
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = LCD_PIN_CS;
            cfg.pin_rst = LCD_PIN_RST;
            cfg.pin_busy = -1;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            if (LCD_ROTATE % 2 != 0)
            {
                cfg.panel_width = LCD_HEIGHT;
                cfg.panel_height = LCD_WIDTH;
                cfg.memory_width = LCD_HEIGHT;
                cfg.memory_height = LCD_WIDTH;
            }
            else
            {
                cfg.panel_width = LCD_WIDTH;
                cfg.panel_height = LCD_HEIGHT;
                cfg.memory_width = LCD_WIDTH;
                cfg.memory_height = LCD_HEIGHT;
            }
            _panel_instance.config(cfg);
        }
        // led
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = LCD_PIN_BL;
            cfg.invert = false;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
// sensor
#ifdef USE_SENSOR
        {
            auto cfg = _sensor_instance.config();
            cfg.bus_shared = true;
            cfg.freq = LCD_SPI_READ_SPEED;
            cfg.pin_sclk = LCD_PIN_SCLK;
            cfg.pin_mosi = LCD_PIN_MOSI;
            cfg.pin_miso = LCD_PIN_MISO;
            cfg.pin_cs = LCD_TOUCH_PIN_CS;
            cfg.pin_int = LCD_TOUCH_PIN_INT;
            cfg.spi_host = LCD_SPI_HOST;
            cfg.x_min = 3800;
            cfg.x_max = 300;
            cfg.y_min = 200;
            cfg.y_max = 3800;
            cfg.offset_rotation = 1;
            _sensor_instance.config(cfg);
            _panel_instance.setTouch(&_sensor_instance);
        }
#endif
        setPanel(&_panel_instance);
    }
};

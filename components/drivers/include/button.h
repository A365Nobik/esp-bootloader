#pragma once

#include <driver/gpio.h>

struct Button
{
    gpio_num_t pin;
    const uint32_t delay = 50;
    bool last_state = true;
    uint32_t last_time = 0;

    void init(gpio_num_t pin)
    {
        this->pin = pin;
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
    }

    bool is_clicked(void)
    {
        bool reading = gpio_get_level(pin);
        uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
        bool clicked = false;

        if (reading != last_state)
        {
            if ((now - last_time) > delay)
            {
                if (reading == 1 && last_state == 0)
                {
                    clicked = true;
                }
                last_state = reading;
            }
            last_time = now;
        }

        return clicked;
    }
};

void btn_wait_click(Button &btn)
{
    while (!btn.is_clicked())
    {
        vTaskDelay(10);
    }
}

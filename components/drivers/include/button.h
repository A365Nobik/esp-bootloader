#pragma once

#include <driver/gpio.h>

struct Button
{
    gpio_num_t pin;
    const uint32_t delay = 50;
    bool lastState = true;
    uint32_t lastTime = 0;

    void init(gpio_num_t pin)
    {
        this->pin = pin;
        gpio_set_direction(pin, GPIO_MODE_INPUT);
        gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
    }

    bool isClicked(void)
    {
        bool reading = gpio_get_level(pin);
        uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
        bool clicked = false;

        if (reading != lastState)
        {
            if ((now - lastTime) > delay)
            {
                if (reading == 1 && lastState == 0)
                {
                    clicked = true;
                }
                lastState = reading;
            }
            lastTime = now;
        }

        return clicked;
    }
};

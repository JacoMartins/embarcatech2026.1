#include "stdio.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <hardware/gpio.h>

#define button_a 5
#define led_b 12

bool repeating_timer_callback(struct repeating_timer *t)
{
    return true;
}

int main()
{
    gpio_init(led_b);
    gpio_set_dir(led_b, GPIO_OUT);
    gpio_put(led_b, false);

    gpio_init(button_a);
    gpio_set_dir(button_a, GPIO_IN);
    gpio_pull_up(button_a);

    int count = 0;
    int seconds = 0;
    bool blinking = false;
    bool led_state = false;
    bool last_button_state = true;

    struct repeating_timer timer;

    while (true)
    {
        bool button_a_state = gpio_get(button_a);

        if (last_button_state == true && button_a_state == false)
        {
            count++;
        }

        last_button_state = button_a_state;

        if (count >= 5)
        {
            while (seconds < 10)
            {
                gpio_put(led_b, blinking);
                blinking = !blinking;
                seconds++;
            }
        }
    }
}

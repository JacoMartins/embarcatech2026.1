#include <stdio.h>
#include <hardware/gpio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#define button_a 5
#define button_b 6
#define led_b 12

int main()
{
    gpio_init(led_b);
    gpio_set_dir(led_b, GPIO_OUT);
    gpio_put(led_b, false);

    gpio_init(button_a);
    gpio_set_dir(button_a, GPIO_IN);
    gpio_pull_up(button_a);

    gpio_init(button_b);
    gpio_set_dir(button_b, GPIO_IN);
    gpio_pull_up(button_b);

    int count = 0;
    int toggle_interval_ms = 50;

    bool blinking = false;
    bool button_a_last_state = false;
    bool button_b_last_state = false;
    bool last_led_state = false;
    absolute_time_t end_blinking = delayed_by_ms(get_absolute_time(), 50);
    absolute_time_t toggle_interval = delayed_by_ms(get_absolute_time(), 50);
    absolute_time_t button_a_debouncer = get_absolute_time();
    absolute_time_t button_b_debouncer = get_absolute_time();

    while (true)
    {
        if (!blinking)
        {
            bool button_a_state = !gpio_get(button_a);

            if (button_a_last_state != button_a_state)
            {
                button_a_last_state = button_a_state;

                if (!button_a_state)
                {
                    button_a_debouncer = delayed_by_ms(get_absolute_time(), 50);
                }

                if (time_reached(button_a_debouncer))
                {
                    toggle_interval_ms = 50;
                    count++;
                }
            }

            if (count == 5)
            {
                end_blinking = delayed_by_ms(get_absolute_time(), 10000);
                toggle_interval = delayed_by_ms(get_absolute_time(), toggle_interval_ms);
                blinking = true;
                count = 0;
            }
        }
        else
        {
            bool button_b_state = !gpio_get(button_b);

            if (button_b_last_state != button_b_state)
            {
                button_b_last_state = button_b_state;

                if (!button_b_state)
                {
                    button_b_debouncer = delayed_by_ms(get_absolute_time(), 50);
                }

                if (time_reached(button_b_debouncer))
                {
                    toggle_interval_ms = 500;
                }
            }

            if (time_reached(toggle_interval))
            {
                gpio_put(led_b, !last_led_state);
                last_led_state = !last_led_state;
                toggle_interval = delayed_by_ms(get_absolute_time(), toggle_interval_ms);
            }

            if (time_reached(end_blinking))
            {
                blinking = false;
                gpio_put(led_b, false);
            }
        }
    }
}

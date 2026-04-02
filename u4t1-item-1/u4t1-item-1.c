#include <stdio.h>
#include <hardware/gpio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#define button_a 5
#define led_b 12

int main()
{
    gpio_init(led_b);
    gpio_set_dir(led_b, GPIO_OUT);
    gpio_put(led_b, false);

    gpio_init(button_a);
    gpio_set_dir(button_a, GPIO_IN);
    gpio_pull_up(button_a);
    
    int count = 0;
    
    bool blinking = false;
    bool button_a_last_state = false;
    bool last_led_state = false;
    absolute_time_t end_blinking;
    absolute_time_t toggle_interval;
    absolute_time_t software_debouncer;
    
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
                    software_debouncer = delayed_by_ms(get_absolute_time(), 50);
                }
                
                if(time_reached(software_debouncer)) 
                {
                    count++;
                }
            }
            
            if (count == 5)
            {
                end_blinking = delayed_by_ms(get_absolute_time(), 10000);
                toggle_interval = delayed_by_ms(get_absolute_time(), 50);
                blinking = true;
                count = 0;
            }
        }
        else
        {
            if (time_reached(toggle_interval))
            {
                gpio_put(led_b, !last_led_state);
                last_led_state = !last_led_state;
                toggle_interval = delayed_by_ms(get_absolute_time(), 50);
            }
            
            if (time_reached(end_blinking))
            {
                blinking = false;
                gpio_put(led_b, false);
            }
        }
    }
}

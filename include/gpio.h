#pragma once
#include <zephyr/drivers/gpio.h>

class GpioPin {
private:
    const struct device* port_;
    gpio_pin_t pin_;
    gpio_flags_t flags_;
    bool initialized_;

public:
    GpioPin();
    
    int init(const struct device* port, gpio_pin_t pin, gpio_flags_t flags);
    int init(const struct gpio_dt_spec* spec, gpio_flags_t flags);
    
    int set(bool state);
    int toggle();
    int get(bool* state) const;
    
    bool is_initialized() const { return initialized_; }
};
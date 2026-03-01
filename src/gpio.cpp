#include "gpio.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gpio);

GpioPin::GpioPin() 
    : port_(nullptr), pin_(0), flags_(0), initialized_(false) {}

int GpioPin::init(const struct device* port, gpio_pin_t pin, gpio_flags_t flags) {
    if (!port) {
        LOG_ERR("GPIO port is NULL");
        return -1;
    }
    
    if (!device_is_ready(port)) {
        LOG_ERR("GPIO port not ready");
        return -2;
    }
    
    port_ = port;
    pin_ = pin;
    flags_ = flags;
    
    int ret = gpio_pin_configure(port_, pin_, flags_);
    if (ret != 0) {
        LOG_ERR("Failed to configure GPIO pin %d: %d", pin_, ret);
        return ret;
    }
    
    initialized_ = true;
    LOG_INF("GPIO pin %d initialized", pin_);
    return 0;
}

int GpioPin::init(const struct gpio_dt_spec* spec, gpio_flags_t flags) {
    if (!spec) {
        LOG_ERR("GPIO spec is NULL");
        return -1;
    }
    
    if (!gpio_is_ready_dt(spec)) {
        LOG_ERR("GPIO device not ready");
        return -2;
    }
    
    port_ = spec->port;
    pin_ = spec->pin;
    flags_ = flags;
    
    int ret = gpio_pin_configure_dt(spec, flags_);
    if (ret != 0) {
        LOG_ERR("Failed to configure GPIO pin %d: %d", pin_, ret);
        return ret;
    }
    
    initialized_ = true;
    LOG_INF("GPIO pin %d initialized", pin_);
    return 0;
}

int GpioPin::set(bool state) {
    if (!initialized_) {
        return -1;
    }
    
    return gpio_pin_set(port_, pin_, state ? 1 : 0);
}

int GpioPin::toggle() {
    if (!initialized_) {
        return -1;
    }
    
    return gpio_pin_toggle(port_, pin_);
}

int GpioPin::get(bool* state) const {
    if (!initialized_ || !state) {
        return -1;
    }
    
    int val = gpio_pin_get(port_, pin_);
    if (val < 0) {
        return val;
    }
    
    *state = (val != 0);
    return 0;
}
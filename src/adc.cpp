// zephyr_adc.cpp
#include "adc.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zephyr_adc);

AdcChannel::AdcChannel() : adc_dev_(nullptr), channel_id_(0), resolution_(12), vref_(3.3f)
{
    buffer_[0] = 0;
}

int AdcChannel::init(const struct device *adc_dev, uint8_t channel_id, int resolution, float vref)
{
    adc_dev_ = adc_dev;
    channel_id_ = channel_id;
    resolution_ = resolution;
    vref_ = vref;

    if (!device_is_ready(adc_dev_))
    {
        LOG_ERR("ADC device not ready");
        return -1;
    }

    channel_cfg_.gain = ADC_GAIN_1;
    channel_cfg_.reference = ADC_REF_INTERNAL;
    channel_cfg_.acquisition_time = ADC_ACQ_TIME_DEFAULT;
    channel_cfg_.channel_id = channel_id_;
    channel_cfg_.differential = 0;

    sequence_.channels = BIT(channel_id_);
    sequence_.buffer = buffer_;
    sequence_.buffer_size = sizeof(buffer_);
    sequence_.resolution = resolution_;
    sequence_.options = NULL;
    sequence_.oversampling = 0; // Add this - disable oversampling
    sequence_.calibrate = false;

    int ret = adc_channel_setup(adc_dev_, &channel_cfg_);
    [[unlikely]] if (ret != 0)
    {
        LOG_ERR("Failed to setup ADC channel %d: %d", channel_id_, ret);
        return ret;
    }

    LOG_INF("ADC channel %d initialized", channel_id_);
    return 0;
}

float AdcChannel::read_voltage()
{
    int16_t raw = read_raw();

    // Convert to voltage at ADC pin: raw_value * vref / (2^resolution)
    float adc_voltage = (raw * vref_) / (1 << resolution_);

    // Scale by voltage divider to get actual input voltage
    float actual_voltage = adc_voltage * DIVIDER_RATIO;

    return actual_voltage;
}

inline int16_t AdcChannel::read_raw()
{
    if (!adc_dev_)
    {
        LOG_ERR("ADC channel %d not initialized", channel_id_);
        return -1;
    }

    int ret = adc_read(adc_dev_, &sequence_);
    if (ret != 0)
    {
        LOG_ERR("ADC read failed on channel %d: %d", channel_id_, ret);
        return -1;
    }

    // Check for invalid reading
    if (buffer_[0] < 0)
    {
        LOG_ERR("Invalid ADC reading on channel %d: %d", channel_id_, buffer_[0]);
        return -1;
    }

    return buffer_[0];
}

void AdcChannel::set_test_voltage(float voltage)
{
    // For unit testing - set buffer to simulate actual input voltage
    // Reverse the divider calculation: ADC sees voltage / DIVIDER_RATIO
    float adc_voltage = voltage / DIVIDER_RATIO;
    buffer_[0] = (int16_t)((adc_voltage / vref_) * (1 << resolution_));
}

float AdcChannel::get_divider_ratio()
{
    return this->DIVIDER_RATIO;
}
// zephyr_adc.h
#pragma once
#include <zephyr/drivers/adc.h>

class AdcChannel
{
  private:
    const struct device *adc_dev_;
    struct adc_channel_cfg channel_cfg_;
    struct adc_sequence sequence_;
    int16_t buffer_[1];
    uint8_t channel_id_;
    int resolution_;
    float vref_;

    // Voltage divider: 3.3k top, 6.8k bottom
    static constexpr float DIVIDER_RATIO = (3.3f + 6.8f) / 6.8f; // 1.485
    static constexpr size_t BUFFER_SIZE = sizeof(buffer_);

  public:
    AdcChannel();

    int init(const struct device *adc_dev, uint8_t channel_id, int resolution = 12, float vref = 3.3f);

    float read_voltage();
    int16_t read_raw();
    float get_divider_ratio();

    // For testing
    void set_test_voltage(float voltage);
};
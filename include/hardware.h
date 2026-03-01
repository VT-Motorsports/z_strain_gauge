// hardware.h
#pragma once

#include "adc.h"
#include "can.h"
#include "gpio.h"
#include "vehicle_state.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>

class Hardware
{
  public:
    Hardware(VehicleState *state);
    // ADC Channels (8 total) - referenced as chan0..chan7
    AdcChannel adc_chan0; // PA0  -> ADC1_INP16
    AdcChannel adc_chan1; // PA1  -> ADC1_INP17
    AdcChannel adc_chan2; // PA2  -> ADC1_INP14
    AdcChannel adc_chan3; // PA3  -> ADC1_INP15
    AdcChannel adc_chan4; // PA4  -> ADC1_INP18
    AdcChannel adc_chan5; // PA5  -> ADC1_INP19
    AdcChannel adc_chan6; // PA6  -> ADC1_INP3
    AdcChannel adc_chan7; // PA7  -> ADC1_INP7

    // Error LEDs (PE2-PE6)
    GpioPin led_yellow; // PE2 - led0
    GpioPin led_orange; // PE3 - led1
    GpioPin led_red;    // PE4 - led2
    GpioPin led_blue;   // PE5 - led3
    GpioPin led_green;  // PE6 - led4

    // Control signals
    GpioPin horn_signal;  // PC8
    GpioPin drive_enable; // PC9
    GpioPin air_ctrl;     // PA8

    CanBus can1;
    CanBus can2;

    int init();
    uint16_t getADCValue(uint8_t channel);

  private:
    VehicleState *vehicle;
    const struct device *adc_dev_ = nullptr;
    const struct device *gpioe_ = nullptr;
    const struct device *gpioc_ = nullptr;
    const struct device *gpioa_ = nullptr;
    const struct device *can1_dev = nullptr;
    const struct device *can2_dev = nullptr;
    int initializeADCs();
    int initializeGPIOs();
    int initializeCANs();
};

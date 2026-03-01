#include "APPS.h"
#include "hardware.h"
#include "vehicle_state.h"
#include "zephyr/kernel.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(APPS, LOG_LEVEL_INF);

// Define diagnostics thread stack
K_THREAD_STACK_DEFINE(apps_stack, 1024);

APPS_THREAD::APPS_THREAD(VehicleState *vehicle)
{
    vehicle_ = vehicle;
}

[[nodiscard]] int APPS_THREAD::Initialize(Hardware *hw)
{
    LOG_INF("Initializing APPS Thread");

    if (!hw)
    {
        LOG_ERR("Hardware pointer is NULL");
        return -2;
    }

    hardware_ = hw;

    k_thread_create(&apps_thread_, apps_stack, APPS_THREAD_STACK_SIZE, apps_main_loop, this, NULL, NULL,
                    K_PRIO_PREEMPT(APPS_THREAD_PRIORITY), K_FP_REGS, K_MSEC(10));

    LOG_INF("APPS thread started");
    return 0;
}

void APPS_THREAD::apps_main_loop(void *p1, void *p2, void *p3)
{
    APPS_THREAD *self = static_cast<APPS_THREAD *>(p1);
    LOG_INF("ENTERED APPS THREAD");
    while (1)
    {
        self->update();
        k_msleep(100);
    }
}

void APPS_THREAD::update()
{
    APPS_data &apps = vehicle_->APPSIf;
    uint16_t &pedal1raw = vehicle_->analogIf.channels[vehicle_->APPSIf.pedal1_adc_channel_num];
    uint16_t &pedal2raw = vehicle_->analogIf.channels[vehicle_->APPSIf.pedal2_adc_channel_num];

    pedal1raw = hardware_->getADCValue(vehicle_->APPSIf.pedal1_adc_channel_num);
    pedal2raw = hardware_->getADCValue(vehicle_->APPSIf.pedal2_adc_channel_num);

    apps.errors[OPEN_CIRCUIT_P1] = checkOpenCircuit(pedal1raw, APPS_data::pedal1_low_threshold);
    apps.errors[OPEN_CIRCUIT_P2] = checkOpenCircuit(pedal2raw, APPS_data::pedal2_low_threshold);
    apps.errors[SHORT_CIRCUIT_P1] = checkShortCircuit(pedal1raw, APPS_data::pedal1_high_threshold);
    apps.errors[SHORT_CIRCUIT_P2] = checkShortCircuit(pedal2raw, APPS_data::pedal2_high_threshold);

    bool range_fault = apps.errors[OPEN_CIRCUIT_P1] || apps.errors[OPEN_CIRCUIT_P2] || apps.errors[SHORT_CIRCUIT_P1] ||
                       apps.errors[SHORT_CIRCUIT_P2];

    apps.pedal1_percent = readPedalPercent(pedal1raw, APPS_data::pedal1_low_threshold, APPS_data::pedal1_range_width,
                                           APPS_data::pedal1_slope_direction);
    apps.pedal2_percent = readPedalPercent(pedal2raw, APPS_data::pedal2_low_threshold, APPS_data::pedal2_range_width,
                                           APPS_data::pedal2_slope_direction);

    apps.errors[PEDAL_AGREEMENT] = checkPedalAgreement(apps.pedal1_percent, apps.pedal2_percent);

    float avg_pct = (apps.pedal1_percent + apps.pedal2_percent) / 2.0f;
    apps.errors[BRAKE_OVERLAP] = checkBrakeOverlap(avg_pct);

    apps.faulted = range_fault || apps.errors[PEDAL_AGREEMENT] || apps.errors[BRAKE_OVERLAP];

    apps.commandedTorquePercentage = apps.faulted ? 0.0 : (double)avg_pct;

    if (apps.faulted)
    {
        LOG_WRN("APPS fault active — torque command zeroed");
    }
}

float APPS_THREAD::readPedalPercent(uint16_t raw, uint16_t low, uint16_t range, PEDAL_SLOPE_DIRECTION slope)
{
    float pct;
    if (slope == POSITIVE)
    {
        pct = (double)(raw - low) / (double)range;
    }
    else
    {
        pct = (double)(low - raw) / (double)range;
    }

    if (pct < 0.0f)
    {
        pct = 0.0f;
    }
    if (pct > 1.0f)
    {
        pct = 1.0f;
    }

    return pct;
}

bool APPS_THREAD::checkOpenCircuit(uint16_t raw, uint16_t low_threshold)
{
    return raw < low_threshold;
}

bool APPS_THREAD::checkShortCircuit(uint16_t raw, uint16_t high_threshold)
{
    return raw > high_threshold;
}

bool APPS_THREAD::checkPedalAgreement(float p1_pct, float p2_pct)
{
    if (fabsf(p1_pct - p2_pct) > APPS_data::agreement_threshold)
    {
        if (agreement_fault_deadline_ == 0)
        {
            agreement_fault_deadline_ = k_uptime_get() + APPS_data::agreement_timeout_ms;
        }
        else if (k_uptime_get() >= agreement_fault_deadline_)
        {
            LOG_WRN("Pedal agreement fault: p1=%.2f p2=%.2f", (double)p1_pct, (double)p2_pct);
            return true;
        }
    }
    else
    {
        agreement_fault_deadline_ = 0;
    }
    return false;
}

bool APPS_THREAD::checkBrakeOverlap(float avg_pct)
{
    // TODO: wire up brake switch GPIO from hardware_
    bool brake_pressed = false; // hardware_->brake_switch.read();

    if (avg_pct > APPS_data::brake_on_threshold && brake_pressed)
    {
        brake_fault_latched_ = true;
        LOG_WRN("Brake overlap fault latched");
    }
    if (brake_fault_latched_ && avg_pct < APPS_data::brake_off_threshold)
    {
        brake_fault_latched_ = false;
        LOG_INF("Brake overlap fault cleared");
    }
    return brake_fault_latched_;
}

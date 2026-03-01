#pragma once
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>

#include "vehicle_state.h"
#include "dti_decoders.h"

using frame_handler_t = void (*)(const struct can_frame *frame, volatile VehicleState *vehicle);

class CanBus
{
  private:
    const struct device *dev_;
    uint32_t bitrate_;
    uint32_t sample_point_;
    bool initialized_;
    bool started_;
    VehicleState *vehicle_;
    uint32_t frames_rec;
    uint32_t frames_sent;

    static void can1_rx_isr(const struct device *dev, struct can_frame *frame, void *self_ptr);
    static void can2_rx_isr(const struct device *dev, struct can_frame *frame, void *self_ptr);
    void dispatch(const struct can_frame *frame);
    int register_handlers();

    frame_handler_t bus_handlers[2048]{nullptr};

  public:
    CanBus(VehicleState *vehicle);

    int init(const struct device *dev, uint32_t bitrate = 1000000, uint32_t sample_point = 875);
    int start();
    int stop();

    int send(const struct can_frame *frame, k_timeout_t timeout, can_tx_callback_t callback = nullptr,
             void *user_data = nullptr);

    int add_rx_filter_msgq(struct k_msgq *msgq, const struct can_filter *filter);
    void remove_rx_filter(int filter_id);

    int get_state(enum can_state *state) const;
    int set_mode(can_mode_t mode);
    can_mode_t get_mode() const;

    bool is_initialized() const
    {
        return initialized_;
    }
    bool is_started() const
    {
        return started_;
    }

    const struct device *get_device() const
    {
        return dev_;
    }
};
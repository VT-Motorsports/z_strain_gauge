#include "can.h"
#include "dti_decoders.h"

#include <zephyr/logging/log.h>

#include "dti_decoders.h"
#include "stm32h753xx.h"
#include "vehicle_state.h"
#include "zephyr/drivers/can.h"

LOG_MODULE_REGISTER(can);

// ============================================================================
// Helper macro for DTI standard CAN IDs: (packet_id << 5) | node_id
// ============================================================================
#define DTI_CAN_ID(pkt, node) (((pkt) << 5) | (node))

// DTI node IDs per corner
#define DTI_NODE_FL 22
#define DTI_NODE_FR 23
#define DTI_NODE_RL 24
#define DTI_NODE_RR 25

// ============================================================================
// Construction
// ============================================================================

CanBus::CanBus(VehicleState *vehicle)
    : dev_(nullptr), bitrate_(0), sample_point_(0), initialized_(false), started_(false), vehicle_(vehicle),
      frames_rec(0), frames_sent(0)
{
}

// ============================================================================
// ISR dispatch
// ============================================================================

void CanBus::dispatch(const struct can_frame *frame)
{
    uint16_t id = frame->id & CAN_STD_ID_MASK;

    if (id < 2048 && bus_handlers[id])
    {
        bus_handlers[id](frame, vehicle_);
    }

    frames_rec++;
}

void CanBus::can1_rx_isr(const struct device *dev, struct can_frame *frame, void *self_ptr)
{
    CanBus *bus = static_cast<CanBus *>(self_ptr);
    bus->dispatch(frame);
}

void CanBus::can2_rx_isr(const struct device *dev, struct can_frame *frame, void *self_ptr)
{
    CanBus *bus = static_cast<CanBus *>(self_ptr);
    bus->dispatch(frame);
}

// ============================================================================
// Handler registration — called internally at end of init()
// ============================================================================

int CanBus::register_handlers()
{
    if (dev_ == DEVICE_DT_GET(DT_NODELABEL(fdcan1)))
    {
        // ---- DTI Inverters on CAN1 ----

        // FL (node 22) — packets 0x1F-0x26
        bus_handlers[DTI_CAN_ID(0x1F, DTI_NODE_FL)] = decode_dti_fl_0x1F;
        bus_handlers[DTI_CAN_ID(0x20, DTI_NODE_FL)] = decode_dti_fl_0x20;
        bus_handlers[DTI_CAN_ID(0x21, DTI_NODE_FL)] = decode_dti_fl_0x21;
        bus_handlers[DTI_CAN_ID(0x22, DTI_NODE_FL)] = decode_dti_fl_0x22;
        bus_handlers[DTI_CAN_ID(0x23, DTI_NODE_FL)] = decode_dti_fl_0x23;
        bus_handlers[DTI_CAN_ID(0x24, DTI_NODE_FL)] = decode_dti_fl_0x24;
        bus_handlers[DTI_CAN_ID(0x25, DTI_NODE_FL)] = decode_dti_fl_0x25;
        bus_handlers[DTI_CAN_ID(0x26, DTI_NODE_FL)] = decode_dti_fl_0x26;

        // FR (node 23)
        bus_handlers[DTI_CAN_ID(0x1F, DTI_NODE_FR)] = decode_dti_fr_0x1F;
        bus_handlers[DTI_CAN_ID(0x20, DTI_NODE_FR)] = decode_dti_fr_0x20;
        bus_handlers[DTI_CAN_ID(0x21, DTI_NODE_FR)] = decode_dti_fr_0x21;
        bus_handlers[DTI_CAN_ID(0x22, DTI_NODE_FR)] = decode_dti_fr_0x22;
        bus_handlers[DTI_CAN_ID(0x23, DTI_NODE_FR)] = decode_dti_fr_0x23;
        bus_handlers[DTI_CAN_ID(0x24, DTI_NODE_FR)] = decode_dti_fr_0x24;
        bus_handlers[DTI_CAN_ID(0x25, DTI_NODE_FR)] = decode_dti_fr_0x25;
        bus_handlers[DTI_CAN_ID(0x26, DTI_NODE_FR)] = decode_dti_fr_0x26;

        // RL (node 24)
        bus_handlers[DTI_CAN_ID(0x1F, DTI_NODE_RL)] = decode_dti_rl_0x1F;
        bus_handlers[DTI_CAN_ID(0x20, DTI_NODE_RL)] = decode_dti_rl_0x20;
        bus_handlers[DTI_CAN_ID(0x21, DTI_NODE_RL)] = decode_dti_rl_0x21;
        bus_handlers[DTI_CAN_ID(0x22, DTI_NODE_RL)] = decode_dti_rl_0x22;
        bus_handlers[DTI_CAN_ID(0x23, DTI_NODE_RL)] = decode_dti_rl_0x23;
        bus_handlers[DTI_CAN_ID(0x24, DTI_NODE_RL)] = decode_dti_rl_0x24;
        bus_handlers[DTI_CAN_ID(0x25, DTI_NODE_RL)] = decode_dti_rl_0x25;
        bus_handlers[DTI_CAN_ID(0x26, DTI_NODE_RL)] = decode_dti_rl_0x26;

        // RR (node 25)
        bus_handlers[DTI_CAN_ID(0x1F, DTI_NODE_RR)] = decode_dti_rr_0x1F;
        bus_handlers[DTI_CAN_ID(0x20, DTI_NODE_RR)] = decode_dti_rr_0x20;
        bus_handlers[DTI_CAN_ID(0x21, DTI_NODE_RR)] = decode_dti_rr_0x21;
        bus_handlers[DTI_CAN_ID(0x22, DTI_NODE_RR)] = decode_dti_rr_0x22;
        bus_handlers[DTI_CAN_ID(0x23, DTI_NODE_RR)] = decode_dti_rr_0x23;
        bus_handlers[DTI_CAN_ID(0x24, DTI_NODE_RR)] = decode_dti_rr_0x24;
        bus_handlers[DTI_CAN_ID(0x25, DTI_NODE_RR)] = decode_dti_rr_0x25;
        bus_handlers[DTI_CAN_ID(0x26, DTI_NODE_RR)] = decode_dti_rr_0x26;

        LOG_INF("Registered 32 DTI decoder handlers on CAN1");
    }
    else if (dev_ == DEVICE_DT_GET(DT_NODELABEL(fdcan2)))
    {
        // CAN2 handlers go here (BMS, dashboard, etc.)
        LOG_INF("CAN2 handler registration — no handlers yet");
    }
    else
    {
        LOG_ERR("Unknown CAN device during handler registration");
        return -1;
    }

    return 0;
}

// ============================================================================
// Init
// ============================================================================

int CanBus::init(const struct device *dev, uint32_t bitrate, uint32_t sample_point)
{
    if (!dev)
    {
        LOG_ERR("CAN device is NULL");
        return -1;
    }

    if (!device_is_ready(dev))
    {
        LOG_ERR("CAN device not ready");
        return -2;
    }

    dev_ = dev;
    bitrate_ = bitrate;
    sample_point_ = sample_point;

    LOG_INF("CAN device ready - calculating timing...");

    // Calculate timing parameters
    struct can_timing timing;
    int ret = can_calc_timing(dev_, &timing, bitrate_, sample_point_);
    if (ret != 0)
    {
        LOG_ERR("can_calc_timing() failed: %d", ret);
        return ret;
    }

    // Stop CAN if already running
    enum can_state state;
    can_get_state(dev_, &state, nullptr);
    if (state != CAN_STATE_STOPPED)
    {
        LOG_INF("Stopping CAN to configure timing");
        ret = can_stop(dev_);
        if (ret != 0)
        {
            LOG_ERR("can_stop() failed: %d", ret);
            return ret;
        }
        started_ = false;
    }

    // Apply timing configuration
    LOG_INF("Applying timing configuration...");
    ret = can_set_timing(dev_, &timing);
    if (ret != 0)
    {
        LOG_ERR("can_set_timing() failed: %d", ret);
        return ret;
    }

    // Select ISR callback based on which CAN peripheral this is
    can_rx_callback_t callback;

    if (dev_ == DEVICE_DT_GET(DT_NODELABEL(fdcan1)))
    {
        callback = can1_rx_isr;
    }
    else if (dev_ == DEVICE_DT_GET(DT_NODELABEL(fdcan2)))
    {
        callback = can2_rx_isr;
    }
    else
    {
        LOG_ERR("Unknown CAN device during callback assignment");
        return -3;
    }

    constexpr struct can_filter accept_all_filter = {
        .id = 0x000,
        .mask = ~CAN_STD_ID_MASK,
        .flags = 0U,
    };

    int filter_id = can_add_rx_filter(dev_, callback, this, &accept_all_filter);
    LOG_INF("Callback attached with code %d", filter_id);

    // Register decode handlers based on which bus this is
    ret = register_handlers();
    if (ret != 0)
    {
        LOG_ERR("register_handlers() failed: %d", ret);
        return ret;
    }

    initialized_ = true;
    LOG_INF("CAN initialized (%d bps, sample point %d)", bitrate_, sample_point_);
    return 0;
}

// ============================================================================
// Start / Stop / Send
// ============================================================================

int CanBus::start()
{
    if (!initialized_)
    {
        LOG_ERR("CAN not initialized");
        return -1;
    }

    if (started_)
    {
        LOG_WRN("CAN already started");
        return 0;
    }

    LOG_INF("Starting CAN...");
    int ret = can_start(dev_);
    if (ret != 0)
    {
        LOG_ERR("can_start() failed: %d", ret);
        return ret;
    }

    started_ = true;
    LOG_INF("CAN started successfully");
    return 0;
}

int CanBus::stop()
{
    if (!initialized_)
    {
        return -1;
    }

    if (!started_)
    {
        return 0;
    }

    int ret = can_stop(dev_);
    if (ret == 0)
    {
        started_ = false;
        LOG_INF("CAN stopped");
    }
    return ret;
}

int CanBus::send(const struct can_frame *frame, k_timeout_t timeout, can_tx_callback_t callback, void *user_data)
{
    if (!initialized_ || !started_)
    {
        return -1;
    }

    return can_send(dev_, frame, timeout, callback, user_data);
}

// ============================================================================
// Filter / State / Mode
// ============================================================================

int CanBus::add_rx_filter_msgq(struct k_msgq *msgq, const struct can_filter *filter)
{
    if (!initialized_)
    {
        return -1;
    }

    return can_add_rx_filter_msgq(dev_, msgq, filter);
}

void CanBus::remove_rx_filter(int filter_id)
{
    if (!initialized_)
    {
        return;
    }

    can_remove_rx_filter(dev_, filter_id);
}

int CanBus::get_state(enum can_state *state) const
{
    if (!initialized_ || !state)
    {
        return -1;
    }

    return can_get_state(dev_, state, nullptr);
}

int CanBus::set_mode(can_mode_t mode)
{
    if (!initialized_)
    {
        return -1;
    }

    bool was_started = started_;
    if (started_)
    {
        int ret = stop();
        if (ret != 0)
        {
            LOG_ERR("Failed to stop CAN before mode change");
            return ret;
        }
    }

    int ret = can_set_mode(dev_, mode);
    if (ret != 0)
    {
        LOG_ERR("can_set_mode failed: %d", ret);
        return ret;
    }

    if (was_started)
    {
        ret = start();
        if (ret != 0)
        {
            LOG_ERR("Failed to restart CAN after mode change");
            return ret;
        }
    }

    LOG_INF("CAN mode set to 0x%x", mode);
    return 0;
}

can_mode_t CanBus::get_mode() const
{
    if (!initialized_)
    {
        return (can_mode_t)0;
    }

    return can_get_mode(dev_);
}
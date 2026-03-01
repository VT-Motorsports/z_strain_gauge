#include "system.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(system, LOG_LEVEL_INF);

// Define heap storage at compile time
K_HEAP_DEFINE(system_heap, 2048);

// Define diagnostics thread stack
K_THREAD_STACK_DEFINE(diag_stack, 1024);

System::System() : hardware_(nullptr), initialized_(false), diagnostics_running_(false)
{
    diag_stack_ = diag_stack;
}

int System::init()
{
    LOG_INF("Initializing system resources...");

    // Test heap allocation
    void *test_ptr = k_heap_alloc(&system_heap, 64, K_NO_WAIT);
    if (!test_ptr)
    {
        LOG_ERR("Heap initialization failed");
        return -1;
    }

    LOG_INF("Heap allocation test succeeded");
    k_heap_free(&system_heap, test_ptr);

    // Store reference to k_heap
    heap_ = system_heap;

    initialized_ = true;
    LOG_INF("System initialized successfully");
    return 0;
}

int System::start_diagnostics(Hardware *hw)
{
    if (!initialized_)
    {
        LOG_ERR("System not initialized");
        return -1;
    }

    if (diagnostics_running_)
    {
        LOG_WRN("Diagnostics already running");
        return 0;
    }

    if (!hw)
    {
        LOG_ERR("Hardware pointer is NULL");
        return -2;
    }

    hardware_ = hw;

    k_thread_create(&diag_thread_, diag_stack_, DIAG_THREAD_STACK_SIZE, diagnostics_thread_entry, this, NULL, NULL,
                    K_PRIO_PREEMPT(DIAG_THREAD_PRIORITY), K_FP_REGS, K_NO_WAIT);

    diagnostics_running_ = true;
    LOG_INF("Diagnostics thread started");
    return 0;
}

void System::stop_diagnostics()
{
    if (diagnostics_running_)
    {
        k_thread_abort(&diag_thread_);
        diagnostics_running_ = false;
        LOG_INF("Diagnostics thread stopped");
    }
}

void System::diagnostics_thread_entry(void *p1, void *p2, void *p3)
{
    System *sys = static_cast<System *>(p1);
    if (sys)
    {
        sys->diagnostics_loop();
    }
}

void System::diagnostics_loop()
{
    while (1)
    {
        uint64_t uptime_ms = k_uptime_get();

        // Get heap statistics
        struct sys_memory_stats mem_stats = {0, 0, 0};
        sys_heap_runtime_stats_get(&heap_.heap, &mem_stats);

        // Get CPU load
        uint8_t cpu_load = cpu_load_get(1);

        LOG_INF("Uptime: %llu ms | Heap: %zu/%zu bytes | CPU: %d.%d%%", uptime_ms, mem_stats.allocated_bytes,
                mem_stats.allocated_bytes + mem_stats.free_bytes, cpu_load / 10, cpu_load % 10);

        // Check CAN1 state if hardware is available
        if (hardware_ && hardware_->can1.is_initialized())
        {
            enum can_state state;
            if (hardware_->can1.get_state(&state) == 0)
            {
                switch (state)
                {
                case CAN_STATE_ERROR_ACTIVE:
                    LOG_INF("CAN1: Active");
                    break;
                case CAN_STATE_ERROR_WARNING:
                    LOG_WRN("CAN1: Warning");
                    break;
                case CAN_STATE_ERROR_PASSIVE:
                    LOG_WRN("CAN1: Error Passive");
                    break;
                case CAN_STATE_BUS_OFF:
                    LOG_ERR("CAN1: Bus Off");
                    break;
                case CAN_STATE_STOPPED:
                    LOG_INF("CAN1: Stopped");
                    break;
                default:
                    LOG_ERR("CAN1: Unknown state");
                    break;
                }
            }
        }

        // Check CAN2 state if hardware is available
        if (hardware_ && hardware_->can2.is_initialized())
        {
            enum can_state state;
            if (hardware_->can2.get_state(&state) == 0)
            {
                switch (state)
                {
                case CAN_STATE_ERROR_ACTIVE:
                    LOG_INF("CAN2: Active");
                    break;
                case CAN_STATE_ERROR_WARNING:
                    LOG_WRN("CAN2: Warning");
                    break;
                case CAN_STATE_ERROR_PASSIVE:
                    LOG_WRN("CAN2: Error Passive");
                    break;
                case CAN_STATE_BUS_OFF:
                    LOG_ERR("CAN2: Bus Off");
                    break;
                case CAN_STATE_STOPPED:
                    LOG_INF("CAN2: Stopped");
                    break;
                default:
                    LOG_ERR("CAN2: Unknown state");
                    break;
                }
            }
        }

        k_msleep(DIAG_PERIOD_MS);
    }
}

void *System::heap_alloc(size_t size, k_timeout_t timeout)
{
    if (!initialized_)
    {
        return nullptr;
    }
    return k_heap_alloc(&heap_, size, timeout);
}

void System::heap_free(void *ptr)
{
    if (!initialized_ || !ptr)
    {
        return;
    }
    k_heap_free(&heap_, ptr);
}

int System::get_heap_stats(struct sys_memory_stats *stats)
{
    if (!initialized_ || !stats)
    {
        return -1;
    }
    return sys_heap_runtime_stats_get(&heap_.heap, stats);
}

uint64_t System::get_uptime_ms() const
{
    return k_uptime_get();
}

uint8_t System::get_cpu_load() const
{
    return cpu_load_get(1);
}
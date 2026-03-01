#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <syscalls/can.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>
#include <zephyr/debug/cpu_load.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/mem_stats.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/toolchain.h>
#include <zephyr/types.h>

#include "adc.h"
#include "hardware.h"
#include "system.h"
#include "vehicle_state.h"

#ifndef __cplusplus
#error "__cplusplus not defined! Build system is compiling as C!"
#endif

LOG_MODULE_REGISTER(main);

CAN_MSGQ_DEFINE(main_can_rx_msgq, 1000);

// /* 1000 msec = 1 sec */
// #define SLEEP_TIME_MS 50
// #define DEBUGGING 0

// const struct k_heap* sys_heap;

// // Yellow LED on PE2 (led0 alias in VCU board)
// #define LED0_NODE DT_ALIAS(led0)
// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// // Button removed from VCU board - keeping definition commented for reference
// // #define SW0_NODE DT_ALIAS(sw0)
// // static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

// int init_user_button(void) {
//     // Button not present on VCU board - function kept for compatibility
//     // Return success to not break existing logic
//     printk("Button not present on VCU board - skipping initialization\n");
//     return 1;
// }

// int init_led(void) {
//     int ret;

//     if (!gpio_is_ready_dt(&led)) {
//         LOG_ERR("Error: LED device %s is not ready", led.port->name);
//         return 0;
//     }

//     ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
//     if (ret != 0) {
//         LOG_ERR("Error %d: failed to configure LED %s pin %d", ret, led.port->name, led.pin);
//         return 0;
//     }

//     LOG_INF("LED initialized successfully on %s pin %d", led.port->name, led.pin);
//     return 1;
// }

// int main(void) {
//     LOG_INF("ENTERED MAIN");

//     can_init();
//     sys_heap = heap_init();
//     diagnostics_init();

//     if (init_adc() != 0) {
//         LOG_ERR("Failed to initialize ADC");
//         return -1;
//     }

//     // Initialize LED
//     if (init_led() != 1) {
//         LOG_ERR("Failed to initialize LED");
//         return -1;
//     }

//     const struct can_filter match_all_rx = {
//         .id = 0x000,
//         .mask = ~CAN_STD_ID_MASK,
//         .flags = 0
//     };

//     int rx_filter_id = can_add_rx_filter_msgq(can1, &main_can_rx_msgq, &match_all_rx);
//     if (rx_filter_id < 0) {
//         LOG_ERR("Failed to add RX MSGQ in main: %d", rx_filter_id);
//     } else {
//         LOG_INF("RX MSGQ installed successfully in main (id=%d)", rx_filter_id);
//     }

//     init_user_button();

//     struct can_frame drive_enable_frame;
//     drive_enable_frame.flags = 0;
//     drive_enable_frame.id = 0x196;
//     drive_enable_frame.dlc = 8;
//     drive_enable_frame.data[0] = 0x00;
//     drive_enable_frame.data[1] = 0xFF;
//     drive_enable_frame.data[2] = 0xFF;
//     drive_enable_frame.data[3] = 0xFF;
//     drive_enable_frame.data[4] = 0xFF;
//     drive_enable_frame.data[5] = 0xFF;
//     drive_enable_frame.data[6] = 0xFF;
//     drive_enable_frame.data[7] = 0xFF;

//     struct can_frame ac_current_frame;
//     int16_t target_current_amps = 2;
//     int16_t scaled_current = target_current_amps * 10;

//     ac_current_frame.flags = 0;
//     ac_current_frame.id = 0x36;
//     ac_current_frame.dlc = 8;
//     ac_current_frame.data[0] = (scaled_current >> 8) & 0xFF;
//     ac_current_frame.data[1] = scaled_current & 0xFF;
//     ac_current_frame.data[2] = 0xFF;
//     ac_current_frame.data[3] = 0xFF;
//     ac_current_frame.data[4] = 0xFF;
//     ac_current_frame.data[5] = 0xFF;
//     ac_current_frame.data[6] = 0xFF;
//     ac_current_frame.data[7] = 0xFF;

//     while (1) {
//         // Toggle Yellow LED - BLINKY!
//         gpio_pin_toggle_dt(&led);

//         struct can_frame rx_frame;
//         int msg_count = 0;

//         // Button not present on VCU board - simulate button always pressed for now
//         // TODO: Replace with actual pedal sensor logic or safety interlock
//         bool button_state = true;
//         // If you had button hardware: bool button_state = gpio_pin_get_dt(&button);

//         LOG_INF("Button state: %d", button_state);

//         if(button_state) {
//             drive_enable_frame.data[0] = 0x01;
//             ac_current_frame.data[0] = (scaled_current >> 8) & 0xFF;
//             ac_current_frame.data[1] = scaled_current & 0xFF;
//             scaled_current = get_pedal_current();
//         } else {
//             ac_current_frame.data[0] = 0;
//             ac_current_frame.data[1] = 0;
//             drive_enable_frame.data[0] = 0x00;
//             target_current_amps = 0;
//         }

//         int ret = can_send(can1, &drive_enable_frame, K_MSEC(100), NULL, NULL);
//         if (ret != 0) {
//             LOG_ERR("Failed to send drive enable: %d", ret);
//         } else {
//             LOG_INF("Drive enable sent successfully");
//         }

//         ret = can_send(can1, &ac_current_frame, K_MSEC(100), NULL, NULL);
//         if (ret != 0) {
//             LOG_ERR("Failed to send AC current command: %d", ret);
//         } else {
//             LOG_INF("Set AC Current to %d A (CAN ID: 0x%03X)", target_current_amps, ac_current_frame.id);
//         }

//         double voltage = static_cast<double>(get_voltage());
//         if (voltage >= 0) {
//             LOG_INF("A0 Pin Voltage: %.3f V", voltage);
//         }

//         k_msgq_purge(&main_can_rx_msgq);

//         // Sleep for 100ms
//         k_msleep(SLEEP_TIME_MS);
//     }

//     can_remove_rx_filter(can1, rx_filter_id);
//     k_msgq_purge(&main_can_rx_msgq);
//     return 0;
// }

// #include <zephyr/kernel.h>
// #include <zephyr/drivers/gpio.h>

void send_heartbeat(Hardware &hw)
{
    static uint32_t heartbeat_counter = 0;

    struct can_frame heartbeat_frame = {.id = 0x100, // Heartbeat message ID
                                        .dlc = 8,
                                        .flags = 0,
                                        .data = {(uint8_t)(heartbeat_counter >> 24), (uint8_t)(heartbeat_counter >> 16),
                                                 (uint8_t)(heartbeat_counter >> 8), (uint8_t)(heartbeat_counter), 0xEF,
                                                 0xFF, 0xFF, 0xFF}};

    int ret = hw.can1.send(&heartbeat_frame, K_NO_WAIT);
    if (ret == 0)
    {
    }
    else
    {
        LOG_ERR("Heartbeat send failed: %d", ret);
    }

    heartbeat_counter++;
}

int main(void)
{
    LOG_INF("***VCU ENTERED MAIN***\n");

    static VehicleState vehicle;
    static Hardware hardware(&vehicle);
    static System system;

    LOG_INF("=== VCU Starting ===");

    // Initialize system resources
    if (system.init() != 0)
    {
        LOG_ERR("System init failed!");
        return -1;
    }

    // Initialize hardware
    if (hardware.init() != 0)
    {
        LOG_ERR("Hardware init failed!");
        return -2;
    }

    // Start diagnostics monitoring
    if (system.start_diagnostics(&hardware) != 0)
    {
        LOG_ERR("Failed to start diagnostics!");
        return -3;
    }

    LOG_INF("=== VCU Ready ===");

    while (1)
    {
        hardware.led_green.toggle();
        send_heartbeat(hardware);

        k_msleep(500);
    }
}
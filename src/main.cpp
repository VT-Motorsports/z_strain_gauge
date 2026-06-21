#include <math.h>
#include <stdlib.h>
#include <sys/errno.h>
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
#include "can.h"
#include <zephyr/drivers/adc.h>

#ifndef __cplusplus
#error "__cplusplus not defined! Build system is compiling as C!"
#endif

LOG_MODULE_REGISTER(main);

CAN_MSGQ_DEFINE(main_can_rx_msgq, 1000);

void send_heartbeat(CanBus can)
{
    static uint32_t heartbeat_counter = 0;

    struct can_frame heartbeat_frame = {.id = 0x100, // Heartbeat message ID
                                        .dlc = 8,
                                        .flags = 0,
                                        .data = {(uint8_t)(heartbeat_counter >> 24), (uint8_t)(heartbeat_counter >> 16),
                                                 (uint8_t)(heartbeat_counter >> 8), (uint8_t)(heartbeat_counter), 0xEF,
                                                 0xFF, 0xFF, 0xFF}};

    int ret = can.send(&heartbeat_frame, K_NO_WAIT);
    if (ret == 0)
    {
    }
    else
    {
        LOG_ERR("Heartbeat send failed: %d", ret);
    }

    heartbeat_counter++;
}

constexpr uint32_t BASE_SG_CAN_ID = 0x600; //TODO: get right ID
constexpr uint8_t NUM_CH = 9;
constexpr uint8_t SMP_AVG_WINDOW = 3;

uint16_t samples[NUM_CH][SMP_AVG_WINDOW];
uint16_t canPayload[NUM_CH];
uint16_t pakGrpCounter = 0; 


void addSample(uint8_t chNum, uint16_t sample){

    if (chNum > NUM_CH){
        LOG_ERR("Channel Number must be valid");
        return;
    }


    for (int i =SMP_AVG_WINDOW; i>0;i--){
        samples[chNum][i] = samples[chNum][i-1];
    }
    samples[chNum][0] = sample;
}

uint16_t getAvgSample(uint8_t chNum){

    if (chNum > NUM_CH){
        LOG_ERR("Channel Number must be valid");
        return 0;
    }

    double sum=0;

    for (int i =0; i< SMP_AVG_WINDOW;i++){
        sum += samples[chNum][i];
    }

    sum/= SMP_AVG_WINDOW;

    return static_cast<uint16_t>(sum);

}

can_frame genChCanFrame(uint8_t packetNum){

    if (packetNum > 3){
        LOG_ERR("Packet Number must be valid");
        struct can_frame retFrame = {.id = 0,
                                     .dlc = 0,
                                     .flags = 0,
                                     .data = {0,0,0,0,0,0,0,0}};
        return retFrame;
    }


    /*
    Packet 1 -> SG1-SG3
    Packet 2 -> SG4-SG6
    Packet 3 -> SG7-SG9
    */

    uint16_t pay1 = canPayload[packetNum*3-3];
    uint16_t pay2 = canPayload[packetNum*3-2];
    uint16_t pay3 = canPayload[packetNum*3-1];


    if (packetNum == 3) pakGrpCounter ++;

    if (pakGrpCounter == 65535) LOG_WRN("Packet Group Counter Overflow");

    struct can_frame retFrame = {.id = (BASE_SG_CAN_ID +packetNum-1),
                                 .dlc = 8,
                                 .flags = 0,
                                 .data = {(uint8_t)(pay1 & 0xFF),(uint8_t)(pay1 << 8),
                                          (uint8_t)(pay2 & 0xFF),(uint8_t)(pay2 << 8),
                                          (uint8_t)(pay3 & 0xFF),(uint8_t)(pay3 << 8),
                                          (uint8_t)(pakGrpCounter & 0xFF),(uint8_t)(pakGrpCounter << 8)}};

    return retFrame;
}

constexpr int msgSendRate = 200; // Hz


int main(void){

    LOG_INF("Main Innit");

    const struct device *can_dev = DEVICE_DT_GET(DT_NODELABEL(fdcan1));

    CanBus can;
    can.init(can_dev);

    
    static const struct adc_dt_spec strainChannels[NUM_CH] = {
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg1),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg2),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg3),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg4),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg5),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg6),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg7),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg8),
        ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), sg9),
    };

    for (adc_dt_spec ch : strainChannels){
        int ret =adc_channel_setup_dt(&ch);

        if (ret == EINVAL){
            LOG_ERR("ADC channel %i failed to setup",ch.channel_id);
        }
    }

    int16_t buf;
    struct adc_sequence seq = {
        .buffer = &buf,
        .buffer_size = sizeof(buf),
    };

    for (int chNum = 0; chNum < NUM_CH; chNum++) {
        
        adc_sequence_init_dt(&strainChannels[chNum], &seq);
        int ret = adc_read_dt(&strainChannels[chNum], &seq);
        if (ret == 0) {
            addSample( chNum, (uint16_t)buf);
        }
        canPayload[chNum] = getAvgSample(chNum);

    }

    for (auto pakNum = 1; pakNum <=3; pakNum++){
        can_frame frame = genChCanFrame(pakNum);
        can.send(&frame,  K_SECONDS(1/msgSendRate));
    }






    k_sleep(K_FOREVER);
};

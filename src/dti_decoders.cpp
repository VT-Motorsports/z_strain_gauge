// dti_decoders.cpp
//
// DTI HV-500/550/850 CAN2 Decoders
// Reference: DTI CAN Manual V2.5
// Byte order: Big Endian (Motorola), 8-byte fixed length, unused = 0xFF

#include "dti_decoders.h"

// ============================================================================
// Internal helpers — one per packet type, parameterized by corner
// ============================================================================

// convert 16 bits starting at d to a 16 bit value
// packet order coming in from CAN is MSB, and STM would read as LSB.
static inline int16_t be16(const uint8_t *d)
{
    return (int16_t)(((uint16_t)d[0] << 8) | d[1]);
}

// convert 16 bits starting at d to a 32 bit value
// packet order coming in from CAN is MSB, and STM would read as LSB if we pointer casted.
static inline int32_t be32(const uint8_t *d)
{
    return (int32_t)(((uint32_t)d[0] << 24) | ((uint32_t)d[1] << 16) | ((uint32_t)d[2] << 8) | d[3]);
}

// --- 0x1F: Control mode, Target Iq, Motor position, isMotorStill ---
static void decode_0x1F(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.control_mode = d[0];
    inv.target_iq = be16(&d[1]);
    inv.motor_position = (uint16_t)be16(&d[3]);
    inv.is_motor_still = d[5];

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x20: ERPM, Duty cycle, Input Voltage ---
static void decode_0x20(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.erpm = be32(&d[0]);
    inv.duty_cycle = be16(&d[4]);
    inv.input_voltage = be16(&d[6]);

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x21: AC Current, DC Current ---
static void decode_0x21(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.ac_current = be16(&d[0]);
    inv.dc_current = be16(&d[2]);

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x22: Controller Temp, Motor Temp, Fault Code ---
static void decode_0x22(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.controller_temp = be16(&d[0]);
    inv.motor_temp = be16(&d[2]);
    inv.fault_code = d[4];

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x23: Id, Iq (FOC components) ---
static void decode_0x23(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.id = be32(&d[0]);
    inv.iq = be32(&d[4]);

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x24: Throttle, Brake, Digital I/O, Drive Enable, Limits, CAN map ---
static void decode_0x24(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.throttle_signal = (int8_t)d[0];
    inv.brake_signal = (int8_t)d[1];

    // Byte 2: digital I/O  bits[3:0]=inputs, bits[7:4]=outputs
    uint8_t dio = d[2];
    inv.digital_in1 = (dio >> 0) & 1;
    inv.digital_in2 = (dio >> 1) & 1;
    inv.digital_in3 = (dio >> 2) & 1;
    inv.digital_in4 = (dio >> 3) & 1;
    inv.digital_out1 = (dio >> 4) & 1;
    inv.digital_out2 = (dio >> 5) & 1;
    inv.digital_out3 = (dio >> 6) & 1;
    inv.digital_out4 = (dio >> 7) & 1;

    // Byte 3: drive enable
    inv.drive_enable = d[3] & 1;

    // Byte 4: limit flags 0
    uint8_t lim0 = d[4];
    inv.limit_cap_temp = (lim0 >> 0) & 1;
    inv.limit_dc_current = (lim0 >> 1) & 1;
    inv.limit_drive_enable = (lim0 >> 2) & 1;
    inv.limit_igbt_accel_temp = (lim0 >> 3) & 1;
    inv.limit_igbt_temp = (lim0 >> 4) & 1;
    inv.limit_input_voltage = (lim0 >> 5) & 1;
    inv.limit_motor_accel_temp = (lim0 >> 6) & 1;
    inv.limit_motor_temp = (lim0 >> 7) & 1;

    // Byte 5: limit flags 1
    uint8_t lim1 = d[5];
    inv.limit_rpm_min = (lim1 >> 0) & 1;
    inv.limit_rpm_max = (lim1 >> 1) & 1;
    inv.limit_power = (lim1 >> 2) & 1;

    // Byte 7: CAN map version
    inv.can_map_version = d[7];

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x25: Configured and Available AC Currents ---
static void decode_0x25(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.max_ac_current = be16(&d[0]);
    inv.avail_max_ac_current = be16(&d[2]);
    inv.min_ac_current = be16(&d[4]);
    inv.avail_min_ac_current = be16(&d[6]);

    inv.last_rx_time_ms = k_uptime_get();
}

// --- 0x26: Configured and Available DC Currents ---
static void decode_0x26(const struct can_frame *frame, volatile VehicleState *vd, Corner c)
{
    volatile auto &inv = vd->INVERTERS[c];
    const uint8_t *d = frame->data;

    inv.max_dc_current = be16(&d[0]);
    inv.avail_max_dc_current = be16(&d[2]);
    inv.min_dc_current = be16(&d[4]);
    inv.avail_min_dc_current = be16(&d[6]);

    inv.last_rx_time_ms = k_uptime_get();
}

// ============================================================================
// Public wrappers — one per (corner × packet), plugs into handler table
// ============================================================================

// Packet 0x1F
void decode_dti_fl_0x1F(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x1F(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x1F(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x1F(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x1F(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x1F(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x1F(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x1F(f, vd, REAR_RIGHT);
}

// Packet 0x20
void decode_dti_fl_0x20(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x20(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x20(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x20(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x20(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x20(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x20(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x20(f, vd, REAR_RIGHT);
}

// Packet 0x21
void decode_dti_fl_0x21(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x21(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x21(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x21(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x21(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x21(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x21(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x21(f, vd, REAR_RIGHT);
}

// Packet 0x22
void decode_dti_fl_0x22(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x22(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x22(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x22(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x22(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x22(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x22(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x22(f, vd, REAR_RIGHT);
}

// Packet 0x23
void decode_dti_fl_0x23(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x23(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x23(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x23(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x23(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x23(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x23(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x23(f, vd, REAR_RIGHT);
}

// Packet 0x24
void decode_dti_fl_0x24(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x24(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x24(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x24(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x24(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x24(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x24(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x24(f, vd, REAR_RIGHT);
}

// Packet 0x25
void decode_dti_fl_0x25(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x25(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x25(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x25(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x25(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x25(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x25(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x25(f, vd, REAR_RIGHT);
}

// Packet 0x26
void decode_dti_fl_0x26(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x26(f, vd, FRONT_LEFT);
}
void decode_dti_fr_0x26(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x26(f, vd, FRONT_RIGHT);
}
void decode_dti_rl_0x26(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x26(f, vd, REAR_LEFT);
}
void decode_dti_rr_0x26(const struct can_frame *f, volatile VehicleState *vd)
{
    decode_0x26(f, vd, REAR_RIGHT);
}
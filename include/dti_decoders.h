// dti_decoders.h
#pragma once
#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include "vehicle_state.h"

// ============================================================================
// Manual: DTI CAN Manual V2.5 — Standard ID (11-bit) format
// CAN ID = (packet_id << 5) | node_id
// Node ID range: 1-30, broadcast = 31 (0x1F)
//
// Node IDs:  FL=22  FR=23  RL=24  RR=25
//
// Registration example:
//   can2.register_handler((0x1F << 5) | 22, decode_dti_fl_0x1F);  // 0x3F6
//   can2.register_handler((0x20 << 5) | 22, decode_dti_fl_0x20);  // 0x416
//   can2.register_handler((0x20 << 5) | 23, decode_dti_fr_0x20);  // 0x417
//   ...
// Max ID = (0x26 << 5) | 25 = 0x4D9 (1241) — fits in 2048 table
// ============================================================================

// --- Packet 0x1F: Control mode, Target Iq, Motor position, isMotorStill ---
void decode_dti_fl_0x1F(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x1F(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x1F(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x1F(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x20: ERPM, Duty, Input Voltage ---
void decode_dti_fl_0x20(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x20(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x20(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x20(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x21: AC Current, DC Current ---
void decode_dti_fl_0x21(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x21(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x21(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x21(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x22: Controller Temp, Motor Temp, Fault Code ---
void decode_dti_fl_0x22(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x22(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x22(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x22(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x23: Id, Iq ---
void decode_dti_fl_0x23(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x23(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x23(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x23(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x24: Throttle, Brake, Digital I/O, Drive Enable, Limits ---
void decode_dti_fl_0x24(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x24(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x24(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x24(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x25: Configured/Available AC Currents ---
void decode_dti_fl_0x25(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x25(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x25(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x25(const struct can_frame *frame, volatile VehicleState *vd);

// --- Packet 0x26: Configured/Available DC Currents ---
void decode_dti_fl_0x26(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_fr_0x26(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rl_0x26(const struct can_frame *frame, volatile VehicleState *vd);
void decode_dti_rr_0x26(const struct can_frame *frame, volatile VehicleState *vd);
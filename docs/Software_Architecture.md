Classes

Hardware
Members:

ZephyrCanBus can1, can2
ZephyrAdcChannel pedal1_adc, pedal2_adc, brake_adc, temp_sensor, ... (8 total)
ZephyrGpioPin status_led, error_led, can_led, rtd_button, brake_relay (5 LEDs, 3 GPIO)

I/O: Defined by member classes


VehicleState
Members:

All signals: throttle_percent, motor_rpm, motor_temp, battery_voltage, battery_current, ...

Functions:
    decode_motor_status(can_frame*)
    decode_battery_status(can_frame*)
    decode_bms_status(can_frame*)
    ....... all important signal decode's 

pedal namespace
    I/O: ErrorCode update(Hardware&, VehicleState&) - reads ADCs, updates throttle_percent, returns error state
    motor_control namespace
    I/O: ErrorCode update(Hardware&, VehicleState&) - reads state, calculates commands
    I/O: void send_commands(Hardware&, VehicleState&) - sends CAN messages
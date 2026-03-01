# VCU STM32H753VIT6 Board Definition - Migration Guide

## Overview
This board definition is for a custom STM32H753VIT6 (LQFP100) based VCU board, migrated from Nucleo H753ZI development.

## Directory Structure
```
your_vcu_repo/
├── boards/
│   └── st/
│       └── vcu_stm32/
│           ├── board.yml
│           ├── board.cmake
│           ├── Kconfig.vcu_stm32
│           ├── Kconfig.defconfig
│           ├── vcu_stm32.dts
│           └── vcu_stm32_defconfig
├── src/
│   └── main.cpp
├── CMakeLists.txt
└── prj.conf
```

## Step 1: Copy Board Files to Your Repository

Copy the entire `boards/st/vcu_stm32/` directory into your VCU repository:

```bash
# From this template directory
cp -r boards/ /path/to/your/vcu_repo/
```

## Step 2: Update Your Project CMakeLists.txt

Modify your top-level `CMakeLists.txt` to include the BOARD_ROOT before finding Zephyr:

```cmake
cmake_minimum_required(VERSION 3.20.0)

# Point Zephyr to our custom board
set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(vcu)

target_sources(app PRIVATE
    src/main.cpp
    # Add your other source files here
)
```

## Step 3: Build with Your Custom Board

```bash
cd /path/to/your/vcu_repo
west build -b vcu_stm32 --pristine
```

Or if not using west:

```bash
cmake -B build -DBOARD=vcu_stm32 -GNinja
ninja -C build
```

## Step 4: Flash to Hardware

```bash
west flash
# or
ninja -C build flash
```

## What's Configured Out of the Box

### Clocks
- HSI: 64 MHz internal oscillator (no external crystal needed)
- PLL configured for 480 MHz system clock
- All domain clocks configured (D1, D2, D3)

### Error Indicator LEDs (All Port E)
- **Yellow LED**: PE2 (pin 1) - alias: led0
- **Orange LED**: PE3 (pin 2) - alias: led1
- **Red LED**: PE4 (pin 3) - alias: led2
- **Blue LED**: PE5 (pin 4) - alias: led3
- **Green LED**: PE6 (pin 6) - alias: led4

### Control Signals
- **HORN_SIG**: PC8 (pin 65) - alias: horn
- **DRIVE_ENABLE**: PC9 (pin 66) - alias: drive-enable
- **AIR_CTRL**: PA8 (pin 67) - alias: air-ctrl

### CAN Buses
- **FDCAN1**: PB9=TX (pin 96), PB8=RX (pin 95) - Primary CAN bus
- **FDCAN2**: PB6=TX (pin 92), PB5=RX (pin 91) - Secondary CAN bus

### Analog Inputs (ADC1 - 8 channels)
- **ANALOG_CH0**: PA0 (pin 22) - ADC1_INP16
- **ANALOG_CH1**: PA1 (pin 23) - ADC1_INP17
- **ANALOG_CH2**: PA2 (pin 24) - ADC1_INP14
- **ANALOG_CH3**: PA3 (pin 25) - ADC1_INP15
- **ANALOG_CH4**: PA4 (pin 28) - ADC1_INP18
- **ANALOG_CH5**: PA5 (pin 29) - ADC1_INP19
- **ANALOG_CH6**: PA6 (pin 30) - ADC1_INP3
- **ANALOG_CH7**: PA7 (pin 31) - ADC1_INP7

### UART Console
- **USART3**: PD8=TX (pin 55), PD9=RX (pin 56) @ 115200 baud

### Debug Interface
- **SWD**: PA13=SWDIO (pin 72), PA14=SWCLK (pin 76)
- **SWO**: PB0 (pin 34) - optional trace output

### Memory Layout
- Flash: 2MB (partitioned for bootloader + dual image slots)
- SRAM: 1MB
- DTCM: 128KB
- ITCM: 64KB

## Customization Guide

### Change Console UART
Edit `vcu_stm32.dts`:
```dts
chosen {
    zephyr,console = &usart1;  // Change to your UART
    zephyr,shell-uart = &usart1;
};

&usart1 {
    pinctrl-0 = <&usart1_tx_pa9 &usart1_rx_pa10>;  // Your pins
    pinctrl-names = "default";
    current-speed = <115200>;
    status = "okay";
};
```

### Add SPI Peripheral
Edit `vcu_stm32.dts`:
```dts
&spi1 {
    pinctrl-0 = <&spi1_sck_pa5 &spi1_miso_pa6 &spi1_mosi_pa7>;
    pinctrl-names = "default";
    cs-gpios = <&gpioa 4 GPIO_ACTIVE_LOW>;
    status = "okay";
    
    your_sensor: sensor@0 {
        compatible = "vendor,sensor";
        reg = <0>;
        spi-max-frequency = <1000000>;
    };
};
```

### Add I2C Peripheral
Edit `vcu_stm32.dts`:
```dts
&i2c1 {
    pinctrl-0 = <&i2c1_scl_pb6 &i2c1_sda_pb7>;
    pinctrl-names = "default";
    clock-frequency = <I2C_BITRATE_FAST>;
    status = "okay";
    
    your_device: device@48 {
        compatible = "vendor,device";
        reg = <0x48>;
    };
};
```

### Enable ADC
Edit `vcu_stm32.dts`:
```dts
&adc1 {
    pinctrl-0 = <&adc1_inp0_pa0>;
    pinctrl-names = "default";
    st,adc-clock-source = <SYNC>;
    st,adc-prescaler = <4>;
    status = "okay";
    
    #address-cells = <1>;
    #size-cells = <0>;
    
    channel@0 {
        reg = <0>;
        zephyr,gain = "ADC_GAIN_1";
        zephyr,reference = "ADC_REF_INTERNAL";
        zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>;
        zephyr,resolution = <12>;
    };
};
```

## Pin Mapping Reference (LQFP100)

Key differences from Nucleo (LQFP144):
- Fewer GPIO pins available
- Some alternate functions may not be accessible
- Verify pinout in STM32H753VI datasheet before assigning

### Common Peripherals (verify against your schematic):
- USART1: PA9/PA10, PB6/PB7, etc.
- USART2: PA2/PA3, PD5/PD6, etc.
- USART3: PB10/PB11, PC10/PC11, PD8/PD9
- FDCAN1: PA11/PA12, PD0/PD1, PH13/PH14
- FDCAN2: PB5/PB6, PB12/PB13
- SPI1: PA5/PA6/PA7, PB3/PB4/PB5
- I2C1: PB6/PB7, PB8/PB9
- I2C2: PB10/PB11, PF0/PF1

## Bring-Up Checklist

### Phase 1: Basic Boot
- [ ] Board powers up
- [ ] SWD connection established
- [ ] Can flash firmware
- [ ] Serial console outputs (verify baud rate and pins)

### Phase 2: Clock Validation
- [ ] HSE oscillating (25 MHz crystal present?)
- [ ] PLL locked
- [ ] System clock at expected frequency
- [ ] UART baud rate correct (if wrong, check HSE config)

### Phase 3: GPIO
- [ ] LED toggles
- [ ] Button reads correctly

### Phase 4: CAN
- [ ] CAN transceiver powered
- [ ] CAN loopback works
- [ ] CAN communication with other nodes

### Phase 5: Additional Peripherals
- [ ] SPI communication
- [ ] I2C communication
- [ ] ADC readings
- [ ] Timers/PWM

## Troubleshooting

### Build fails with "board not found"
- Verify `BOARD_ROOT` is set in CMakeLists.txt
- Check directory structure matches exactly

### UART outputs garbage
- Very unlikely with HSI (internal oscillator is stable)
- Check UART pins match your hardware
- Verify baud rate setting (115200)

### CAN not working
- Check CAN transceiver power
- Verify termination resistors (120Ω at each bus end)
- Check pin assignments (FDCAN1: PB9/PB8, FDCAN2: PB6/PB5)
- Ensure CAN bus speed configured correctly

### GPIO doesn't respond
- Pin might not exist on LQFP100 package
- Check STM32H753VI datasheet pinout
- Verify pinctrl settings

## Next Steps

1. **Verify Hardware**: Use oscilloscope/logic analyzer to confirm:
   - HSE oscillating at 25 MHz
   - UART TX pin activity
   - GPIO toggling

2. **Customize Pinout**: Update `vcu_stm32.dts` with your actual schematic

3. **Enable VCU Peripherals**:
   - Additional CAN buses
   - SPI sensors (IMU, pressure, etc.)
   - ADC for pedal position
   - Timers for PWM outputs

4. **Port Application Code**: Begin migrating your VCU application logic

## Support Files

The pinctrl definitions are in Zephyr's STM32 HAL:
```
$ZEPHYR_BASE/dts/st/h7/stm32h753zitx-pinctrl.dtsi
```

Note: This file is for LQFP144 (ZI package). Some pins referenced may not exist on LQFP100 (VI package). Always cross-reference with the STM32H753VI datasheet.

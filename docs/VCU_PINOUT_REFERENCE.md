# VCU STM32H753VIT6 (LQFP100) Pinout Reference

## Complete Pin Assignment

### Error Indicator LEDs (Port E)
| Signal         | GPIO  | Pin # | Direction | Notes                    |
|----------------|-------|-------|-----------|--------------------------|
| ERR_LED_YELLOW | PE2   | 1     | Output    | Active HIGH              |
| ERR_LED_ORANGE | PE3   | 2     | Output    | Active HIGH              |
| ERR_LED_RED    | PE4   | 3     | Output    | Active HIGH              |
| ERR_LED_BLUE   | PE5   | 4     | Output    | Active HIGH              |
| ERR_LED_GREEN  | PE6   | 6     | Output    | Active HIGH              |

### Analog Input Channels (Port A)
| Signal          | GPIO  | Pin # | ADC Channel | Notes                    |
|-----------------|-------|-------|-------------|--------------------------|
| ANALOG_CHANNEL0 | PA0   | 22    | ADC1_INP16  | 12-bit resolution        |
| ANALOG_CHANNEL1 | PA1   | 23    | ADC1_INP17  | 12-bit resolution        |
| ANALOG_CHANNEL2 | PA2   | 24    | ADC1_INP14  | 12-bit resolution        |
| ANALOG_CHANNEL3 | PA3   | 25    | ADC1_INP15  | 12-bit resolution        |
| ANALOG_CHANNEL4 | PA4   | 28    | ADC1_INP18  | 12-bit resolution        |
| ANALOG_CHANNEL5 | PA5   | 29    | ADC1_INP19  | 12-bit resolution        |
| ANALOG_CHANNEL6 | PA6   | 30    | ADC1_INP3   | 12-bit resolution        |
| ANALOG_CHANNEL7 | PA7   | 31    | ADC1_INP7   | 12-bit resolution        |

### Debug & Programming
| Signal | GPIO  | Pin # | Function | Notes                        |
|--------|-------|-------|----------|------------------------------|
| SWO    | PB0   | 34    | Trace    | Serial Wire Output (optional)|
| SWDIO  | PA13  | 72    | Debug    | Serial Wire Debug I/O        |
| SWCLK  | PA14  | 76    | Debug    | Serial Wire Clock            |

### UART Console (USART3)
| Signal     | GPIO  | Pin # | Function    | Config            |
|------------|-------|-------|-------------|-------------------|
| USART3_TX  | PD8   | 55    | UART TX     | 115200 baud       |
| USART3_RX  | PD9   | 56    | UART RX     | 115200 baud       |

### Control Signals
| Signal       | GPIO  | Pin # | Direction | Notes                    |
|--------------|-------|-------|-----------|--------------------------|
| HORN_SIG     | PC8   | 65    | Output    | Horn/buzzer control      |
| DRIVE_ENABLE | PC9   | 66    | Output    | Motor drive enable       |
| AIR_CTRL     | PA8   | 67    | Output    | AIR (Accumulator Isolation Relay) control |

### CAN Buses
| Signal   | GPIO  | Pin # | Function | Notes                    |
|----------|-------|-------|----------|--------------------------|
| CAN1_TX  | PB9   | 96    | FDCAN1   | Primary CAN bus          |
| CAN1_RX  | PB8   | 95    | FDCAN1   | Primary CAN bus          |
| CAN2_TX  | PB6   | 92    | FDCAN2   | Secondary CAN bus        |
| CAN2_RX  | PB5   | 91    | FDCAN2   | Secondary CAN bus        |

## Memory Configuration

### Flash Memory Layout (2MB Total)
| Partition    | Start Address | Size    | Purpose                          |
|--------------|---------------|---------|----------------------------------|
| MCUboot      | 0x08000000    | 128 KB  | Bootloader                       |
| Image Slot 0 | 0x08020000    | 448 KB  | Primary firmware image           |
| Image Slot 1 | 0x08090000    | 448 KB  | Secondary firmware image (OTA)   |
| Storage      | 0x08100000    | 1024 KB | Persistent storage/config        |

### RAM Configuration
- **SRAM**: 1024 KB (main system RAM)
- **DTCM**: 128 KB (Data Tightly Coupled Memory - fast access)
- **ITCM**: 64 KB (Instruction TCM - zero-wait instruction fetch)

## Clock Configuration

### Internal Oscillators (No External Crystal)
- **HSI**: 64 MHz internal RC oscillator (used as PLL source)
- **LSE**: 32.768 kHz RTC crystal (enabled for RTC/low-power)

### System Clocks
- **System Clock**: 480 MHz (from PLL)
- **AHB Clock**: 240 MHz (HPRE = /2)
- **APB1 Clock**: 120 MHz (D2PPRE1 = /2)
- **APB2 Clock**: 120 MHz (D2PPRE2 = /2)
- **APB3 Clock**: 120 MHz (D1PPRE = /2)
- **APB4 Clock**: 120 MHz (D3PPRE = /2)

### PLL Configuration (Using Internal HSI)
```
Input: 64 MHz (HSI internal oscillator)
PLL_M = 8   → VCO input = 8 MHz (64/8)
PLL_N = 240 → VCO output = 1920 MHz (8*240)
PLL_P = 4   → System clock = 480 MHz (1920/4)
PLL_Q = 8   → PLL_Q = 240 MHz (for peripherals)
PLL_R = 4   → PLL_R = 480 MHz
```

**Note**: Using internal HSI means no external crystal required. HSI has ~1% accuracy which is sufficient for CAN, UART, and most applications. If you need higher precision (e.g., USB), you would need an external HSE crystal.

## Peripheral Assignments

### ADC1 Configuration
- **Clock Source**: Synchronous (from APB2)
- **Prescaler**: /4
- **Resolution**: 12-bit (0-4095)
- **Reference**: Internal (typically ~2.5V on STM32H7)
- **Channels**: 8 channels configured (PA0-PA7)

### CAN Configuration
- **FDCAN1**: Classical CAN or CAN-FD capable
- **FDCAN2**: Classical CAN or CAN-FD capable
- **Default**: Standard 11-bit IDs, 500 kbps typical
- **Transceivers**: External (verify power supply!)

### UART3 Configuration
- **Mode**: 8N1 (8 data bits, no parity, 1 stop bit)
- **Baud Rate**: 115200
- **Flow Control**: None
- **Purpose**: Console logging and shell

## GPIO Port Usage Summary

| Port  | Pins Used | Purpose                              |
|-------|-----------|--------------------------------------|
| PA    | 11 pins   | ADC (8), AIR_CTRL, SWD (2)          |
| PB    | 5 pins    | CAN1 (2), CAN2 (2), SWO             |
| PC    | 2 pins    | HORN, DRIVE_ENABLE                   |
| PD    | 2 pins    | UART3 (TX, RX)                       |
| PE    | 5 pins    | Error LEDs (5)                       |

**Total GPIO Used**: 25 out of 100 pins

## Important Hardware Notes

### Power Requirements
- **VDD**: 3.3V ±5%
- **VDDA**: 3.3V (analog supply, should be clean/filtered)
- **VCAP**: Two 4.7µF capacitors on VCAP1/VCAP2 (required!)
- **VREF+**: Tied to VDDA or separate analog reference

### Boot Configuration
- **BOOT0**: Should be pulled LOW for normal boot from flash
- **NRST**: External reset pin (pull-up recommended)

### Clock Configuration
- **No external crystal required** - uses internal 64 MHz HSI oscillator
- HSI provides ~1% accuracy, sufficient for CAN and UART
- No need to populate HSE crystal components

### CAN Transceivers
- Verify transceivers are powered (typically 5V or 3.3V)
- Check termination resistors (120Ω at each end of bus)
- Ensure CANH/CANL properly connected

### ADC Input Protection
- **Maximum voltage**: Do not exceed VREF+ (typically 3.3V)
- **Input impedance**: Consider series resistor for protection
- **ESD protection**: Recommended for external analog inputs

### LED Current Limiting
- STM32 GPIO typical sink/source: 8-25 mA per pin
- Add series resistors to LEDs (typical: 220Ω - 1kΩ depending on LED)

## Unused/Available Resources

### Available for Future Use
- **USART1, USART2**: Additional serial ports
- **SPI1-6**: All SPI peripherals available
- **I2C1-4**: All I2C peripherals available  
- **Timers**: TIM1-17 available (PWM, input capture, etc.)
- **ADC2, ADC3**: Two additional 12-bit ADCs
- **DAC1, DAC2**: Two 12-bit DACs available
- **USB OTG FS/HS**: USB device/host capability
- **Ethernet**: 10/100 Ethernet MAC available
- **SDMMC**: SD card interface available

## Devicetree Aliases for Application Code

Use these aliases in your application code for clean abstraction:

```c
// LEDs
const struct gpio_dt_spec led_yellow = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
const struct gpio_dt_spec led_orange = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);
const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led4), gpios);

// Control signals
const struct gpio_dt_spec horn = GPIO_DT_SPEC_GET(DT_ALIAS(horn), gpios);
const struct gpio_dt_spec drive_enable = GPIO_DT_SPEC_GET(DT_ALIAS(drive_enable), gpios);
const struct gpio_dt_spec air_ctrl = GPIO_DT_SPEC_GET(DT_ALIAS(air_ctrl), gpios);

// CAN buses
const struct device *can1 = DEVICE_DT_GET(DT_NODELABEL(fdcan1));
const struct device *can2 = DEVICE_DT_GET(DT_NODELABEL(fdcan2));

// ADC
const struct device *adc = DEVICE_DT_GET(DT_NODELABEL(adc1));

// UART console (automatically used by logging/shell)
// No explicit code needed - chosen { zephyr,console = &usart3; }
```

## Testing Checklist

### Power-On Tests
- [ ] Board powers up with 3.3V on all VDD pins
- [ ] HSE crystal oscillating at 25 MHz (check with scope if possible)
- [ ] SWD connection works (can flash firmware)

### Peripheral Tests  
- [ ] UART console outputs at 115200 baud
- [ ] All 5 LEDs blink individually
- [ ] CAN1 transceiver powered and responds
- [ ] CAN2 transceiver powered and responds
- [ ] ADC reads reasonable voltages (0-3.3V range)
- [ ] Control signals (HORN, DRIVE_ENABLE, AIR_CTRL) toggle

### System Tests
- [ ] System clock running at 480 MHz (check via logging)
- [ ] No hard faults on boot
- [ ] Logging timestamps increment correctly
- [ ] Firmware runs stable for extended period

## Pin Conflicts to Avoid

These pins have alternate functions that conflict:

- **PA0-PA7**: Cannot use for GPIO if ADC is active on those channels
- **PB8/PB9**: Cannot use for anything else if CAN1 is needed
- **PB5/PB6**: Cannot use for anything else if CAN2 is needed  
- **PA13/PA14**: Must remain as SWDIO/SWCLK for programming

## References

- STM32H753xI Datasheet: DS12110
- STM32H7x3 Reference Manual: RM0433
- LQFP100 Package Drawing: Verify pin 1 location (typically marked with dot)
- Zephyr STM32H7 Documentation: https://docs.zephyrproject.org/latest/boards/st/

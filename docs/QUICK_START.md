# VCU STM32H753VIT6 Board Template - Quick Start

## What You Got

This is a **complete, ready-to-use Zephyr board definition** for your STM32H753VIT6 custom VCU board.

## File Structure

```
vcu_board_template/
├── MIGRATION_CHECKLIST.md          ← Step-by-step guide (READ THIS FIRST!)
├── CMakeLists.txt                  ← Example project CMake
├── prj.conf                        ← Example Kconfig options
├── boards/st/vcu_stm32/            ← Your custom board definition
│   ├── board.yml                   ← Board metadata
│   ├── board.cmake                 ← Flash/debug runners
│   ├── Kconfig.vcu_stm32          ← Board Kconfig
│   ├── Kconfig.defconfig          ← Default configs
│   ├── vcu_stm32.dts              ← DEVICETREE (pin mappings!)
│   ├── vcu_stm32_defconfig        ← Board defaults
│   └── README.md                   ← Detailed customization guide
└── src/
    └── main.cpp                    ← Test application (blinky + CAN test)
```

## 60-Second Quick Start

```bash
# 1. Copy to your VCU repository
cp -r boards/ /path/to/your/vcu_repo/

# 2. Edit your VCU project's CMakeLists.txt
# Add BEFORE find_package(Zephyr ...):
#   set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# 3. Build
cd /path/to/your/vcu_repo
west build -b vcu_stm32 --pristine

# 4. Flash
west flash

# 5. Open serial console (115200 baud)
screen /dev/ttyUSB0 115200
```

## What Works Out of the Box

✅ **STM32H753VIT6** (LQFP100 package)
✅ **480 MHz** system clock (with 25 MHz HSE crystal)
✅ **USART3** console on PD8/PD9 @ 115200 baud
✅ **FDCAN1** on PD0/PD1
✅ **LED** on PB0 (blinks in test app)
✅ **Button** on PC13
✅ **Flash partitions** for bootloader + dual images

## What You Need to Customize

🔧 **HSE Crystal Frequency** (if not 25 MHz)
   - Edit: `boards/st/vcu_stm32/vcu_stm32.dts`
   - Find: `clock-frequency = <DT_FREQ_M(25)>;`
   - Change to your crystal frequency

🔧 **UART Pins** (if not PD8/PD9)
   - Edit: `boards/st/vcu_stm32/vcu_stm32.dts`
   - Update: `&usart3` pinctrl-0 to your pins

🔧 **LED/Button Pins** (match your schematic)
   - Edit: `boards/st/vcu_stm32/vcu_stm32.dts`
   - Update: `leds` and `gpio_keys` sections

🔧 **Additional Peripherals** (SPI, I2C, ADC, more CANs)
   - See: `boards/st/vcu_stm32/README.md`
   - Examples included for common peripherals

## Critical Files Explained

### vcu_stm32.dts (MOST IMPORTANT!)
This is where ALL pin mappings live. This file controls:
- Which UART pins you use for console
- Which GPIO pins connect to LEDs, buttons, etc.
- CAN, SPI, I2C, ADC pin assignments
- Clock configuration (HSE frequency, PLL settings)
- Memory partitions

**This file MUST match your hardware schematic!**

### vcu_stm32_defconfig
Base Kconfig options. Enables:
- STM32H7 support
- UART, GPIO, Pinctrl
- Logging basics

Add more peripheral CONFIGs in `prj.conf` instead.

### board.cmake
Flash/debug tool configuration. Works with:
- ST-Link (OpenOCD, STM32CubeProgrammer)
- J-Link
- pyOCD

## Common First-Boot Issues

### ❌ No UART Output
**Cause:** Wrong pins or wrong clock
**Fix:** 
1. Verify USART3 pins in DTS match your schematic
2. Check HSE clock frequency in DTS
3. Try different baud rates

### ❌ Garbage on UART
**Cause:** Wrong HSE clock frequency
**Fix:** Update `&clk_hse { clock-frequency = ... }` in DTS

### ❌ LED Doesn't Blink
**Cause:** Wrong GPIO pin
**Fix:** Check LED pin in schematic vs. `leds { ... }` in DTS

### ❌ Build Error: "Board not found"
**Cause:** BOARD_ROOT not set
**Fix:** Add `set(BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})` to CMakeLists.txt

## Pin Reference (STM32H753VI LQFP100)

**Important:** Not all pins from LQFP144 (Nucleo) exist on LQFP100!

Always cross-reference with **STM32H753VI datasheet** when assigning pins.

Common peripherals available:
- USART1-3, UART4-8
- FDCAN1-2
- SPI1-6
- I2C1-4
- ADC1-3
- TIM1-17
- Many GPIOs (but ~44 fewer than LQFP144!)

## Next Steps

1. **Read:** `MIGRATION_CHECKLIST.md` for detailed step-by-step
2. **Customize:** `vcu_stm32.dts` to match your hardware
3. **Build & Flash:** Test basic bring-up
4. **Add Peripherals:** Enable SPI, I2C, ADC, etc. as needed
5. **Port Code:** Migrate your VCU application logic

## Support Resources

- **Board README:** `boards/st/vcu_stm32/README.md`
- **Migration Guide:** `MIGRATION_CHECKLIST.md`
- **Zephyr Docs:** https://docs.zephyrproject.org
- **STM32H753VI Datasheet:** Download from STMicroelectronics
- **Reference Manual RM0433:** For peripheral details

## Pro Tips

💡 **Keep Nucleo as Reference:** Don't throw away your Nucleo board! Use it to test code if custom board has issues.

💡 **One Peripheral at a Time:** Enable UART first, then CAN, then SPI, etc. Don't enable everything at once.

💡 **Use Oscilloscope:** Verify HSE crystal is oscillating, UART TX has activity, CAN signals look good.

💡 **Version Control:** Commit working configurations before making changes.

💡 **Pin Mapping Spreadsheet:** Create a master pin assignment doc matching your schematic.

## Questions?

Check `boards/st/vcu_stm32/README.md` for detailed customization examples.
Check `MIGRATION_CHECKLIST.md` for troubleshooting common issues.

Good luck with your VCU! 🏎️⚡

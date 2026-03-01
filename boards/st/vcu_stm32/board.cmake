# # Copyright (c) 2025 VCU Team
# # SPDX-License-Identifier: Apache-2.0

# # Flash and debug runner configuration
# board_runner_args(jlink "--device=STM32H753VI" "--speed=4000")
# board_runner_args(openocd --target-handle=_CHIPNAME.cpu0)
# board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")

# include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
# include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
# include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)


# SPDX-License-Identifier: Apache-2.0

# keep first
board_runner_args(stm32cubeprogrammer "--port=swd" "--reset-mode=hw")
board_runner_args(jlink "--device=STM32H753ZI" "--speed=4000")
board_runner_args(openocd --target-handle=_CHIPNAME.cpu0)
board_runner_args(pyocd "--target=stm32h753zitx")

# keep first
include(${ZEPHYR_BASE}/boards/common/stm32cubeprogrammer.board.cmake)
include(${ZEPHYR_BASE}/boards/common/openocd-stm32.board.cmake)
include(${ZEPHYR_BASE}/boards/common/pyocd.board.cmake)
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)

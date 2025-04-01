#
# Copyright (c) 2022 The ZMK Contributors
# SPDX-License-Identifier: MIT
#

board_runner_args(nrfjprog "--nrf-family=NRF52" "--softreset")

include(${ZEPHYR_BASE}/boards/common/uf2.board.cmake)
include(${ZEPHYR_BASE}/boards/common/nrfjprog.board.cmake)
# Inside config/boards/arm/adv360/board.cmake

# Find a suitable place, often near other zephyr_library_sources() calls

# Add this block:
if(CONFIG_ZMK_BEHAVIOR_ALT_TAB)
  zephyr_library_sources(alt-tab.c) # If alt-tab.c is directly in this folder
  # If you put it in a subdirectory like 'src':
  # zephyr_library_sources(src/alt-tab.c)
endif()

# SPDX-License-Identifier: MIT

# Declare this directory as a Zephyr library
zephyr_library()

# Add specific source files to the library compilation
# only if the corresponding Kconfig option is enabled.
zephyr_library_sources_ifdef(CONFIG_ZMK_BEHAVIOR_ALT_TAB alt_tab.c)

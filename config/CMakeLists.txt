# SPDX-License-Identifier: MIT

# Add the behaviors subdirectory to the build process
add_subdirectory(behaviors)

# Explicitly add the behaviors Kconfig file to the list of files
# Kconfig should parse for project-specific options.
list(APPEND EXTRA_KCONFIG_PROJECT_DEFCONFIG "${CMAKE_CURRENT_SOURCE_DIR}/behaviors/Kconfig")

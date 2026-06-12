#
# Copyright (c) 2024 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

find_package(Sysbuild REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sysbuild LANGUAGES)

# Add DTS overlays for child images that need custom partition layout
# when PM is disabled and b0n secure boot is used on the net core.
if(SB_CONFIG_SECURE_BOOT_NETCORE)
  add_overlay_dts(b0n ${CMAKE_CURRENT_LIST_DIR}/b0n.overlay)
endif()

#!/bin/bash
# Example: Build & flash for nRF54L15 with dual-slot (app DFU) and 200ms blink
cd "$(dirname "$0")"
./prepare.sh ../l9_e1_sol -b nrf54l15dk/nrf54l15/cpuapp --sleep-ms 200 --no-single-app

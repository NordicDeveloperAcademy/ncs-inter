#!/bin/bash
# Example: Update via serial recovery on nRF54L15, change blink to 500ms
cd "$(dirname "$0")"/.. 
python3 update.py ../../l9_e1_sol -p /dev/ttyACM1 -b nrf54l15dk/nrf54l15/cpuapp --sleep-ms 500

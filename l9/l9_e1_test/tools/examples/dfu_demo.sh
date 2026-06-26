#!/bin/bash
# End-to-end MCUboot DFU demo script.
#
# 1. Patches project to VERSION 0.0.0.0 / SLEEP_TIME_MS=1000, pristine builds, flashes
# 2. Patches project to VERSION 1.0.0.0 / SLEEP_TIME_MS=100, pristine builds
# 3. Performs DFU update via SMP (upload, test swap, confirm)
# 4. Prints slot info before and after update
#
# Usage:
#   ./dfu_demo.sh --port /dev/ttyACM2 --board nrf54l15dk/nrf54l15/cpuapp
#   ./dfu_demo.sh --port /dev/ttyACM2 --board nrf54l15dk/nrf54l15/cpuapp --wait 8

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../../../l9_e1_sol" && pwd)"
MCUBOOT_MGR="$TOOLS_DIR/mcuboot_mgr.py"
DFU_UPDATE="$TOOLS_DIR/dfu_update.sh"

MAIN_C="$PROJECT_DIR/src/main.c"
VERSION_FILE="$PROJECT_DIR/VERSION"

# Defaults
PORT=""
BOARD=""
WAIT=5
NCS_VERSION="v3.4.0-rc2"

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  --port PORT       Serial port for SMP (required)
  --board TARGET    Board target, e.g. nrf54l15dk/nrf54l15/cpuapp (required)
  --wait SECONDS    Seconds to wait after reset (default: $WAIT)
  --ncs-version VER NCS version (default: $NCS_VERSION)
  -h, --help        Show this help
EOF
    exit 1
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)   PORT="$2"; shift 2 ;;
        --board)  BOARD="$2"; shift 2 ;;
        --wait)   WAIT="$2"; shift 2 ;;
        --ncs-version) NCS_VERSION="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "ERROR: Unknown option: $1"; usage ;;
    esac
done

if [[ -z "$PORT" ]]; then
    echo "ERROR: --port is required"; usage
fi
if [[ -z "$BOARD" ]]; then
    echo "ERROR: --board is required"; usage
fi

# Find NCS root
NCS_ROOT="$HOME/ncs/$NCS_VERSION"
if [[ ! -d "$NCS_ROOT" ]]; then
    echo "ERROR: NCS root not found at $NCS_ROOT"
    exit 1
fi

# Set up toolchain environment
TC_BUNDLE=$(python3 -c "
import json, sys
data = json.load(open('$HOME/ncs/toolchains/toolchains.json'))
for entry in data:
    for tc in entry.get('toolchains', []):
        if '$NCS_VERSION' in tc.get('ncs_versions', []):
            print(tc['identifier']['bundle_id']); sys.exit(0)
print(''); sys.exit(1)
") || { echo "ERROR: Cannot find toolchain for $NCS_VERSION"; exit 1; }

TC="$HOME/ncs/toolchains/$TC_BUNDLE"
export PATH="$TC/bin:$TC/usr/bin:$TC/usr/local/bin:$TC/opt/bin:$TC/opt/nanopb/generator-bin:$TC/nrfutil/bin:$TC/opt/zephyr-sdk/gnu/arm-zephyr-eabi/bin:$TC/opt/zephyr-sdk/gnu/riscv64-zephyr-elf/bin:$PATH"
export LD_LIBRARY_PATH="$TC/lib:$TC/lib/x86_64-linux-gnu:$TC/usr/local/lib"
export GIT_EXEC_PATH="$TC/usr/local/libexec/git-core"
export GIT_TEMPLATE_DIR="$TC/usr/local/share/git-core/templates"
export PYTHONHOME="$TC/usr/local"
export PYTHONPATH="$TC/usr/local/lib/python3.12:$TC/usr/local/lib/python3.12/site-packages"
export NRFUTIL_HOME="$TC/nrfutil/home"
export ZEPHYR_TOOLCHAIN_VARIANT="zephyr/gnu"
export ZEPHYR_SDK_INSTALL_DIR="$TC/opt/zephyr-sdk"
export ZEPHYR_BASE="$NCS_ROOT/zephyr"

# Helper: patch SLEEP_TIME_MS in main.c
patch_sleep() {
    local value="$1"
    sed -i "s/#define SLEEP_TIME_MS.*/#define SLEEP_TIME_MS   $value/" "$MAIN_C"
}

# Helper: set VERSION file
set_version() {
    local major="$1" minor="$2" patch="$3" tweak="$4"
    sed -i "s/^VERSION_MAJOR = .*/VERSION_MAJOR = $major/" "$VERSION_FILE"
    sed -i "s/^VERSION_MINOR = .*/VERSION_MINOR = $minor/" "$VERSION_FILE"
    sed -i "s/^PATCHLEVEL = .*/PATCHLEVEL = $patch/" "$VERSION_FILE"
    sed -i "s/^VERSION_TWEAK = .*/VERSION_TWEAK = $tweak/" "$VERSION_FILE"
}

BUILD_DIR="$PROJECT_DIR/build"
SIGNED_BIN="$BUILD_DIR/l9_e1_sol/zephyr/zephyr.signed.bin"

echo "============================================================"
echo "  MCUboot DFU Demo"
echo "============================================================"
echo "  Board:   $BOARD"
echo "  Port:    $PORT"
echo "  Project: $PROJECT_DIR"
echo "  NCS:     $NCS_ROOT (toolchain: $TC_BUNDLE)"
echo "============================================================"
echo

########################################
# PHASE 1: Build and flash initial image (v0.0.0, SLEEP_TIME_MS=1000)
########################################
echo ">>> PHASE 1: Build & flash initial image (v0.0.0, SLEEP=1000ms)"
echo

patch_sleep 1000
set_version 0 0 0 0
echo "  Set VERSION=0.0.0.0, SLEEP_TIME_MS=1000"

echo "  Building..."
cd "$NCS_ROOT"
west build -p -b "$BOARD" --sysbuild "$PROJECT_DIR" -d "$BUILD_DIR" 2>&1 | tail -5
echo
echo "  Flashing (with erase)..."
west flash -d "$BUILD_DIR" --erase 2>&1 | tail -5
echo
echo "  Initial image flashed. Waiting ${WAIT}s for boot..."
sleep "$WAIT"
echo

########################################
# PHASE 2: Check slot info before update
########################################
echo ">>> PHASE 2: Slot info BEFORE update"
echo
python3 "$MCUBOOT_MGR" -p "$PORT" info
echo

########################################
# PHASE 3: Build DFU image (v1.0.0, SLEEP_TIME_MS=100)
########################################
echo ">>> PHASE 3: Build DFU image (v1.0.0, SLEEP=100ms)"
echo

patch_sleep 100
set_version 1 0 0 0
echo "  Set VERSION=1.0.0.0, SLEEP_TIME_MS=100"

echo "  Building..."
cd "$NCS_ROOT"
west build -p -b "$BOARD" --sysbuild "$PROJECT_DIR" -d "$BUILD_DIR" 2>&1 | tail -5
echo

########################################
# PHASE 4: DFU update
########################################
echo ">>> PHASE 4: DFU update (upload, swap, confirm)"
echo
"$DFU_UPDATE" --port "$PORT" --image "$SIGNED_BIN" --wait "$WAIT"
echo

########################################
# PHASE 5: Slot info after update
########################################
echo ">>> PHASE 5: Slot info AFTER update"
echo
python3 "$MCUBOOT_MGR" -p "$PORT" info
echo

echo "============================================================"
echo "  DFU Demo Complete"
echo "  Initial: v0.0.0 (SLEEP=1000ms) → Updated: v1.0.0 (SLEEP=100ms)"
echo "============================================================"

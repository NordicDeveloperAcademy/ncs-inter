#!/bin/bash
# MCUboot DFU update script.
#
# Performs a full update cycle:
#   1. Print current image info
#   2. Upload new image (marked for test swap)
#   3. Reset device
#   4. Confirm the running image
#   5. Print image info again
#
# Usage:
#   ./dfu_update.sh --port /dev/ttyACM2 --image path/to/zephyr.signed.bin
#   ./dfu_update.sh --port /dev/ttyACM2 --image path/to/zephyr.signed.bin --wait 8

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MCUBOOT_MGR="$SCRIPT_DIR/mcuboot_mgr.py"

# Defaults
PORT=""
IMAGE=""
WAIT=5

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  --port PORT       Serial port for SMP (required)
  --image PATH      Path to signed firmware image (required)
  --wait SECONDS    Seconds to wait after reset for device to boot (default: $WAIT)
  -h, --help        Show this help
EOF
    exit 1
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)   PORT="$2"; shift 2 ;;
        --image)  IMAGE="$2"; shift 2 ;;
        --wait)   WAIT="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "ERROR: Unknown option: $1"; usage ;;
    esac
done

# Validate
if [[ -z "$PORT" ]]; then
    echo "ERROR: --port is required"
    usage
fi
if [[ -z "$IMAGE" ]]; then
    echo "ERROR: --image is required"
    usage
fi
if [[ ! -f "$IMAGE" ]]; then
    echo "ERROR: Image file not found: $IMAGE"
    exit 1
fi

echo "============================================================"
echo "  MCUboot DFU Update"
echo "============================================================"
echo "  Port:  $PORT"
echo "  Image: $IMAGE"
echo "  Wait:  ${WAIT}s"
echo "============================================================"
echo

# Step 1: Print current image info
echo ">>> Step 1: Current image info"
python3 "$MCUBOOT_MGR" -p "$PORT" info
echo

# Step 2: Upload image (mark for test swap)
echo ">>> Step 2: Upload image"
python3 "$MCUBOOT_MGR" -p "$PORT" upload "$IMAGE" --test
echo

# Step 3: Reset device
echo ">>> Step 3: Reset device"
python3 "$MCUBOOT_MGR" -p "$PORT" reset
echo "Waiting ${WAIT}s for device to boot..."
sleep "$WAIT"
echo

# Step 4: Confirm running image
echo ">>> Step 4: Confirm running image"
python3 "$MCUBOOT_MGR" -p "$PORT" confirm --slot 0
echo

# Step 5: Print image info after update
echo ">>> Step 5: Image info after update"
python3 "$MCUBOOT_MGR" -p "$PORT" info
echo

echo "============================================================"
echo "  DFU update complete"
echo "============================================================"

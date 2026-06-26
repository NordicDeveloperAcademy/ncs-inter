#!/bin/bash
# MCUboot DFU over UART test script.
#
# Two modes:
#   Build mode:  provide --v1, --v2, --project, --board → builds both images
#   Binary mode: provide --v1-bin, --v2-bin → uses pre-built images
#
# Usage examples:
#   # Build mode
#   ./test_dfu.sh --v1 1.0.0 --v2 2.0.0 \
#     --project ../../l9_e1_sol --board nrf54l15dk/nrf54l15/cpuapp \
#     --port /dev/ttyACM2 --snr 1057770812
#
#   # Binary mode
#   ./test_dfu.sh --v1-bin ./build/merged.bin --v2-bin ./build/l9_e1_sol/zephyr/zephyr.signed.bin \
#     --v1 1.0.0 --v2 2.0.0 --port /dev/ttyACM2 --snr 1057770812
#
#   # Binary mode (no version verification)
#   ./test_dfu.sh --v1-bin ./merged.bin --v2-bin ./zephyr.signed.bin \
#     --port /dev/ttyACM2 --snr 1057770812

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MCUBOOT_MGR="$SCRIPT_DIR/mcuboot_mgr.py"

# Defaults
PORT=""
SNR=""
V1=""
V2=""
V1_BIN=""
V2_BIN=""
PROJECT=""
BOARD=""
NCS_VERSION="v3.4.0-rc2"

usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Common options:
  --port PORT       Serial port for SMP (required)
  --snr SNR         SEGGER serial number for nrfutil flash (optional)
  --v1 X.Y.Z       Version 1 (initial) - required in build mode, optional in binary mode
  --v2 X.Y.Z       Version 2 (update) - required in build mode, optional in binary mode

Build mode (builds images from project):
  --project PATH    Path to project directory
  --board BOARD     Board target (e.g. nrf54l15dk/nrf54l15/cpuapp)
  --ncs-version VER NCS version for toolchain-manager (default: v3.4.0-rc2)

Binary mode (uses pre-built images):
  --v1-bin PATH     Pre-built binary for initial flash (merged MCUboot + app)
  --v2-bin PATH     Pre-built signed binary for DFU upload
EOF
    exit 1
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)   PORT="$2"; shift 2 ;;
        --snr)    SNR="$2"; shift 2 ;;
        --v1)     V1="$2"; shift 2 ;;
        --v2)     V2="$2"; shift 2 ;;
        --v1-bin) V1_BIN="$2"; shift 2 ;;
        --v2-bin) V2_BIN="$2"; shift 2 ;;
        --project) PROJECT="$2"; shift 2 ;;
        --board)  BOARD="$2"; shift 2 ;;
        --ncs-version) NCS_VERSION="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) echo "ERROR: Unknown option: $1"; usage ;;
    esac
done

# Validate common args
if [[ -z "$PORT" ]]; then
    echo "ERROR: --port is required"
    usage
fi

# Determine mode
MODE=""
if [[ -n "$V1_BIN" && -n "$V2_BIN" ]]; then
    MODE="binary"
elif [[ -n "$PROJECT" ]]; then
    MODE="build"
else
    echo "ERROR: Provide either --v1-bin + --v2-bin (binary mode) or --project (build mode)"
    usage
fi

# Validate mode-specific args
if [[ "$MODE" == "build" ]]; then
    if [[ -z "$V1" || -z "$V2" ]]; then
        echo "ERROR: --v1 and --v2 are required in build mode"
        usage
    fi
    if [[ -z "$BOARD" ]]; then
        echo "ERROR: --board is required in build mode"
        usage
    fi
    # Resolve to absolute path
    PROJECT="$(cd "$PROJECT" 2>/dev/null && pwd)" || true
    if [[ ! -d "$PROJECT" ]]; then
        echo "ERROR: Project directory not found: $PROJECT"
        exit 1
    fi
elif [[ "$MODE" == "binary" ]]; then
    if [[ ! -f "$V1_BIN" ]]; then
        echo "ERROR: V1 binary not found: $V1_BIN"
        exit 1
    fi
    if [[ ! -f "$V2_BIN" ]]; then
        echo "ERROR: V2 binary not found: $V2_BIN"
        exit 1
    fi
fi

# nrfutil serial number argument
SNR_ARG=""
if [[ -n "$SNR" ]]; then
    SNR_ARG="--serial-number $SNR"
fi

# Toolchain prefix for west commands
NCS_ROOT="$HOME/ncs/$NCS_VERSION"
if [[ ! -d "$NCS_ROOT" ]]; then
    NCS_ROOT="$HOME/nrf-connect-sdk/$NCS_VERSION"
fi
if [[ ! -d "$NCS_ROOT" ]]; then
    echo "ERROR: Cannot find NCS at ~/ncs/$NCS_VERSION or ~/nrf-connect-sdk/$NCS_VERSION"
    exit 1
fi
export ZEPHYR_BASE="$NCS_ROOT/zephyr"
TC_PREFIX=(nrfutil toolchain-manager launch --ncs-version "$NCS_VERSION" --)

# Build helper: patch VERSION file
patch_version() {
    local version_file="$1"
    local version="$2"
    local major minor patch
    IFS='.' read -r major minor patch <<< "$version"
    sed -i "s/^VERSION_MAJOR = .*/VERSION_MAJOR = ${major}/" "$version_file"
    sed -i "s/^VERSION_MINOR = .*/VERSION_MINOR = ${minor}/" "$version_file"
    sed -i "s/^PATCHLEVEL = .*/PATCHLEVEL = ${patch}/" "$version_file"
}

# Cleanup trap for build mode
ORIGINAL_VERSION=""
cleanup() {
    if [[ -n "$ORIGINAL_VERSION" && -f "$PROJECT/VERSION" ]]; then
        echo "Restoring original VERSION file..."
        echo "$ORIGINAL_VERSION" > "$PROJECT/VERSION"
    fi
}
trap cleanup EXIT

# Verification helper
verify_slot() {
    local output="$1"
    local slot="$2"
    local expected_version="$3"

    if [[ -z "$expected_version" ]]; then
        echo "  [SKIP] No version to verify for slot $slot"
        return 0
    fi

    # Look for the version in the info output
    if echo "$output" | grep -q "$expected_version"; then
        echo "  [PASS] Version $expected_version found in output"
        return 0
    else
        echo "  [FAIL] Expected version $expected_version not found in slot $slot"
        return 1
    fi
}

PASS=0
FAIL=0

step_pass() {
    echo "  [PASS] $1"
    PASS=$((PASS + 1))
}

step_fail() {
    echo "  [FAIL] $1"
    FAIL=$((FAIL + 1))
}

echo "============================================================"
echo "  MCUboot DFU Test"
echo "============================================================"
echo "  Mode:    $MODE"
echo "  Port:    $PORT"
[[ -n "$SNR" ]] && echo "  SNR:     $SNR"
[[ -n "$V1" ]] && echo "  V1:      $V1"
[[ -n "$V2" ]] && echo "  V2:      $V2"
if [[ "$MODE" == "build" ]]; then
    echo "  Project: $PROJECT"
    echo "  Board:   $BOARD"
else
    echo "  V1 bin:  $V1_BIN"
    echo "  V2 bin:  $V2_BIN"
fi
echo "============================================================"
echo

# ===== PHASE 1: Prepare V1 image =====
echo ">>> Phase 1: Prepare V1 image"

if [[ "$MODE" == "build" ]]; then
    ORIGINAL_VERSION="$(cat "$PROJECT/VERSION")"
    BUILD_DIR="$PROJECT/build"
    echo "  Patching VERSION to $V1..."
    patch_version "$PROJECT/VERSION" "$V1"

    echo "  Building V1..."
    "${TC_PREFIX[@]}" west build --sysbuild -p -d "$BUILD_DIR" -b "$BOARD" "$PROJECT" -- -DSB_CONFIG_MCUBOOT_MODE_SINGLE_APP=n
    if [[ $? -ne 0 ]]; then
        step_fail "V1 build failed"
        exit 1
    fi
    step_pass "V1 build succeeded"

    # In build mode, we use west flash for initial programming (handles multi-image)
    V1_BUILD_DIR="$BUILD_DIR"
    echo "  V1 build dir: $V1_BUILD_DIR"
fi

echo

# ===== PHASE 2: Flash V1 to device =====
echo ">>> Phase 2: Flash V1 to device (full erase)"

if [[ "$MODE" == "build" ]]; then
    echo "  Flashing via west flash --erase (multi-image sysbuild)..."
    "${TC_PREFIX[@]}" west flash --erase --build-dir "$V1_BUILD_DIR"
    if [[ $? -ne 0 ]]; then
        step_fail "Flash failed"
        exit 1
    fi
else
    echo "  Flashing: $V1_BIN"
    # shellcheck disable=SC2086
    nrfutil device program --firmware "$V1_BIN" --options chip_erase_mode=ERASE_ALL $SNR_ARG
    if [[ $? -ne 0 ]]; then
        step_fail "Flash failed"
        exit 1
    fi
fi
step_pass "Flash succeeded"

echo "  Waiting for device to boot..."
sleep 3
echo

# ===== PHASE 3: Verify V1 in slot 0 =====
echo ">>> Phase 3: Verify V1 in slot 0"

INFO_OUTPUT=$("$MCUBOOT_MGR" -p "$PORT" info 2>&1) || true
echo "$INFO_OUTPUT"

if [[ -n "$V1" ]]; then
    if echo "$INFO_OUTPUT" | grep -q "$V1"; then
        step_pass "V1 ($V1) found in slot 0"
    else
        step_fail "V1 ($V1) NOT found in slot 0"
    fi
else
    echo "  [SKIP] No V1 version to verify"
fi
echo

# ===== PHASE 4: Prepare V2 image (build mode only) =====
if [[ "$MODE" == "build" ]]; then
    echo ">>> Phase 4: Prepare V2 image"
    echo "  Patching VERSION to $V2..."
    patch_version "$PROJECT/VERSION" "$V2"

    echo "  Building V2..."
    "${TC_PREFIX[@]}" west build --sysbuild -p -d "$BUILD_DIR" -b "$BOARD" "$PROJECT" -- -DSB_CONFIG_MCUBOOT_MODE_SINGLE_APP=n
    if [[ $? -ne 0 ]]; then
        step_fail "V2 build failed"
        exit 1
    fi
    step_pass "V2 build succeeded"

    # Find signed binary for DFU upload
    V2_BIN=$(find "$BUILD_DIR" -name "zephyr.signed.bin" | head -1)
    if [[ -z "$V2_BIN" || ! -f "$V2_BIN" ]]; then
        step_fail "Could not find V2 signed binary"
        exit 1
    fi
    echo "  V2 signed bin: $V2_BIN"
    echo
fi

# ===== PHASE 5: DFU Upload V2 =====
echo ">>> Phase 5: DFU Upload V2"

echo "  Uploading: $V2_BIN"
"$MCUBOOT_MGR" -p "$PORT" upload "$V2_BIN"
if [[ $? -ne 0 ]]; then
    step_fail "DFU upload failed"
    exit 1
fi
step_pass "DFU upload succeeded"
echo

# ===== PHASE 6: Verify V2 in slot 1 =====
echo ">>> Phase 6: Verify V2 in slot 1"

INFO_OUTPUT=$("$MCUBOOT_MGR" -p "$PORT" info 2>&1) || true
echo "$INFO_OUTPUT"

if [[ -n "$V2" ]]; then
    if echo "$INFO_OUTPUT" | grep -q "$V2"; then
        step_pass "V2 ($V2) found in slot 1"
    else
        step_fail "V2 ($V2) NOT found in slot 1"
    fi
else
    echo "  [SKIP] No V2 version to verify"
fi
echo

# ===== PHASE 7: Confirm slot 1 =====
echo ">>> Phase 7: Confirm image in slot 1"

"$MCUBOOT_MGR" -p "$PORT" confirm --slot 1
if [[ $? -ne 0 ]]; then
    step_fail "Confirm slot 1 failed"
    exit 1
fi
step_pass "Confirm slot 1 succeeded"
echo

# ===== PHASE 8: Final info =====
echo ">>> Phase 8: Final image state"

"$MCUBOOT_MGR" -p "$PORT" info
echo

# ===== Summary =====
echo "============================================================"
echo "  TEST SUMMARY"
echo "============================================================"
echo "  PASSED: $PASS"
echo "  FAILED: $FAIL"
if [[ $FAIL -eq 0 ]]; then
    echo "  RESULT: PASS"
    echo "============================================================"
    exit 0
else
    echo "  RESULT: FAIL"
    echo "============================================================"
    exit 1
fi

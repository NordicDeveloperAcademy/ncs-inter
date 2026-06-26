#!/bin/bash
# MCUboot DFU security test (self-contained).
#
# Validates that MCUboot correctly accepts/rejects firmware based on signing key:
#   Phase 1: Build & flash initial image (valid key, SLEEP=1000ms)
#   Phase 2: DFU with valid key   (SLEEP=200ms)  — must succeed
#   Phase 3: DFU with invalid key (SLEEP=1000ms) — must be rejected
#
# Depends only on mcuboot_mgr.py 
# Key type auto-detected from board name: nrf54* → ed25519, else → ecdsa.
#
# Usage:
#   ./dfu_test.sh --port /dev/ttyACM1 --board nrf54lm20dk/nrf54lm20a/cpuapp
#   ./dfu_test.sh --port /dev/ttyACM8 --board nrf52840dk/nrf52840 --snr 683165209
#   ./dfu_test.sh --port /dev/ttyACM4 --board nrf9151dk/nrf9151/ns --wait 20 --from-phase 3
#   ./dfu_test.sh --port /dev/ttyACM1 --board nrf54l15dk/nrf54l15/cpuapp --mgr /path/to/mcuboot_mgr.py

set -euo pipefail

# ── Paths ──────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
MAIN_C="$APP_DIR/src/main.c"

# ── Defaults ───────────────────────────────────────────────────
PORT=""
BOARD=""
KEY_TYPE=""
SNR=""
WAIT=5
NCS_VERSION="v3.4.0-rc2"
FROM_PHASE=1
MCUBOOT_MGR=""

# ── Usage ──────────────────────────────────────────────────────
usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  --port PORT        Serial port for SMP (required)
  --board TARGET     Board target (required)
  --key-type TYPE    ed25519 or ecdsa (auto-detected from board name)
  --snr SERIAL       J-Link serial number (for multi-board setups)
  --wait SECONDS     Wait after reset for boot (default: $WAIT)
  --ncs-version VER  NCS version (default: $NCS_VERSION)
  --from-phase N     Resume from phase 1, 2, or 3 (default: 1)
  --mgr PATH         Path to mcuboot_mgr.py (auto-detected if omitted)
  -h, --help         Show this help
EOF
    exit 1
}

# ── Parse args ─────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --port)        PORT="$2";        shift 2 ;;
        --board)       BOARD="$2";       shift 2 ;;
        --key-type)    KEY_TYPE="$2";    shift 2 ;;
        --snr)         SNR="$2";         shift 2 ;;
        --wait)        WAIT="$2";        shift 2 ;;
        --ncs-version) NCS_VERSION="$2"; shift 2 ;;
        --from-phase)  FROM_PHASE="$2";  shift 2 ;;
        --mgr)         MCUBOOT_MGR="$2"; shift 2 ;;
        -h|--help)     usage ;;
        *)             echo "ERROR: Unknown option: $1"; usage ;;
    esac
done

[[ -z "$PORT"  ]] && { echo "ERROR: --port is required";  usage; }
[[ -z "$BOARD" ]] && { echo "ERROR: --board is required"; usage; }
[[ "$FROM_PHASE" =~ ^[123]$ ]] || { echo "ERROR: --from-phase must be 1, 2, or 3"; usage; }

# Auto-detect mcuboot_mgr.py if not specified
if [[ -z "$MCUBOOT_MGR" ]]; then
    MCUBOOT_MGR="$(cd "$APP_DIR/../l9_e1_test/tools" 2>/dev/null && pwd)/mcuboot_mgr.py"
fi
[[ -f "$MCUBOOT_MGR" ]] || { echo "ERROR: mcuboot_mgr.py not found: $MCUBOOT_MGR"; exit 1; }

# ── Key type / files ──────────────────────────────────────────
if [[ -z "$KEY_TYPE" ]]; then
    [[ "$BOARD" == *nrf54* ]] && KEY_TYPE="ed25519" || KEY_TYPE="ecdsa"
fi

case "$KEY_TYPE" in
    ed25519)
        VALID_KEY="$APP_DIR/ed_ci_key.pem"
        INVALID_KEY="$APP_DIR/ed_ci_key_invalid.pem"
        SIG_ED25519="y"; SIG_ECDSA="n"
        EXTRA_BUILD_ARGS=("-DSB_CONFIG_MCUBOOT_GENERATE_DEFAULT_KEY_FILE=y")
        ;;
    ecdsa)
        VALID_KEY="$APP_DIR/ecdsa_ci_key.pem"
        INVALID_KEY="$APP_DIR/ecdsa_ci_key_invalid.pem"
        SIG_ED25519="n"; SIG_ECDSA="y"
        EXTRA_BUILD_ARGS=()
        ;;
    *) echo "ERROR: Invalid key type '$KEY_TYPE'"; exit 1 ;;
esac

for f in "$VALID_KEY" "$INVALID_KEY"; do
    [[ -f "$f" ]] || { echo "ERROR: Key not found: $f"; exit 1; }
done

# ── NCS / toolchain setup ─────────────────────────────────────
NCS_ROOT="$HOME/ncs/$NCS_VERSION"
[[ -d "$NCS_ROOT" ]] || { echo "ERROR: NCS not found at $NCS_ROOT"; exit 1; }

TC_BUNDLE=$(python3 -c "
import json, sys
for e in json.load(open('$HOME/ncs/toolchains/toolchains.json')):
    for t in e.get('toolchains', []):
        if '$NCS_VERSION' in t.get('ncs_versions', []):
            print(t['identifier']['bundle_id']); sys.exit(0)
sys.exit(1)
") || { echo "ERROR: Toolchain not found for $NCS_VERSION"; exit 1; }

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

BUILD_DIR="$APP_DIR/build"
SIGNED_BIN="$BUILD_DIR/l9_e2_sol/zephyr/zephyr.signed.bin"
BUILD_STAMP="$BUILD_DIR/.build_stamp"
FLASH_ARGS=(); [[ -n "$SNR" ]] && FLASH_ARGS+=(--snr "$SNR")

# ── Helpers ────────────────────────────────────────────────────

# Run with system Python (smpmgr needs it, toolchain Python breaks it)
mgr() { env -u PYTHONHOME -u PYTHONPATH python3 "$MCUBOOT_MGR" -p "$PORT" "$@"; }

# Get slot 0 hash from device
get_hash() { mgr info 2>/dev/null | grep "Slot 0 hash" | awk '{print $4}'; }

# Patch SLEEP_TIME_MS in source
patch_sleep() {
    sed -i "s/#define SLEEP_TIME_MS.*/#define SLEEP_TIME_MS   $1/" "$MAIN_C"
    echo "  SLEEP_TIME_MS=$1"
}

# Build (skip if stamp matches)
do_build() {
    local key_file="$1" label="$2" sleep_ms="$3"
    local stamp="board=$BOARD key=$(basename "$key_file") key_type=$KEY_TYPE sleep=$sleep_ms"

    echo "  Key: $label ($(basename "$key_file"))"
    if [[ -f "$BUILD_STAMP" && -f "$SIGNED_BIN" ]] && [[ "$(cat "$BUILD_STAMP")" == "$stamp" ]]; then
        echo "  Build cache hit — skipping rebuild"
        return 0
    fi

    echo "  Building..."
    local build_log="/tmp/dfu_build_$$.log"
    cd "$NCS_ROOT"
    west build -b "$BOARD" "$APP_DIR" -d "$BUILD_DIR" --sysbuild --pristine \
        -- \
        "-DSB_CONFIG_BOOT_SIGNATURE_TYPE_ED25519=$SIG_ED25519" \
        "-DSB_CONFIG_BOOT_SIGNATURE_TYPE_ECDSA_P256=$SIG_ECDSA" \
        "-DSB_CONFIG_BOOT_SIGNATURE_KEY_FILE=\"$key_file\"" \
        "${EXTRA_BUILD_ARGS[@]}" \
        2>&1 | tee "$build_log" | tail -5
    if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
        echo "  BUILD FAILED — last 30 lines:"
        tail -30 "$build_log"
        exit 1
    fi
    echo "$stamp" > "$BUILD_STAMP"
}

# DFU: upload → mark test → reset → wait → confirm
do_dfu() {
    echo "  Uploading image..."
    mgr upload "$SIGNED_BIN" --test
    echo
    echo "  Resetting device..."
    mgr reset
    echo "  Waiting ${WAIT}s for boot..."
    sleep "$WAIT"
    echo
    echo "  Confirming running image..."
    mgr confirm --slot 0
}

# ── Print header ───────────────────────────────────────────────
PASS=0; FAIL=0

echo "============================================================"
echo "  MCUboot DFU Security Test"
echo "============================================================"
echo "  Board:    $BOARD"
echo "  Port:     $PORT"
echo "  Key type: $KEY_TYPE"
echo "  NCS:      $NCS_ROOT (toolchain: $TC_BUNDLE)"
[[ $FROM_PHASE -gt 1 ]] && echo "  Resuming from phase $FROM_PHASE"
echo "============================================================"
echo

# ── PHASE 1: Flash initial image (valid key, 1000ms) ──────────
if [[ $FROM_PHASE -le 1 ]]; then
    echo ">>> PHASE 1: Build & flash (valid key, SLEEP=1000ms)"
    echo
    patch_sleep 1000
    do_build "$VALID_KEY" "VALID" 1000
    echo
    echo "  Flashing..."
    west flash -d "$BUILD_DIR" --erase --recover "${FLASH_ARGS[@]}" 2>&1 | tail -5
    echo "  Waiting ${WAIT}s for boot..."
    sleep "$WAIT"
    echo
    echo "  Slot info:"
    mgr info
    echo
else
    echo ">>> PHASE 1: SKIPPED (--from-phase $FROM_PHASE)"
    echo
fi

INITIAL_HASH=$(get_hash)

# ── PHASE 2: DFU with valid key (200ms) → should succeed ──────
if [[ $FROM_PHASE -le 2 ]]; then
    echo ">>> PHASE 2: DFU with VALID key (SLEEP=200ms)"
    echo
    patch_sleep 200
    do_build "$VALID_KEY" "VALID" 200
    echo
    do_dfu
    echo

    PHASE2_HASH=$(get_hash)
    if [[ -n "$PHASE2_HASH" && "$PHASE2_HASH" != "$INITIAL_HASH" ]]; then
        echo "  RESULT: PASS — hash changed (valid-key DFU accepted)"
        PASS=$((PASS + 1))
    else
        echo "  RESULT: FAIL — hash unchanged"
        FAIL=$((FAIL + 1))
    fi
    echo
else
    echo ">>> PHASE 2: SKIPPED (--from-phase $FROM_PHASE)"
    echo
fi

# ── PHASE 3: DFU with invalid key (1000ms) → must be rejected ─
if [[ $FROM_PHASE -le 3 ]]; then
    echo ">>> PHASE 3: DFU with INVALID key (SLEEP=1000ms)"
    echo
    [[ $FROM_PHASE -eq 3 ]] && PHASE2_HASH="$INITIAL_HASH"

    patch_sleep 1000
    do_build "$INVALID_KEY" "INVALID" 1000
    echo

    echo "  Uploading image..."
    mgr upload "$SIGNED_BIN" --test
    echo
    echo "  Resetting device (expecting MCUboot to reject)..."
    mgr reset
    echo "  Waiting ${WAIT}s for boot + swap-back..."
    sleep "$WAIT"
    echo

    # Confirm will likely fail (image reverted) — that's expected
    mgr confirm --slot 0 || true
    echo "  Waiting extra ${WAIT}s for recovery..."
    sleep "$WAIT"
    echo

    PHASE3_HASH=$(get_hash)
    if [[ -n "$PHASE3_HASH" && "$PHASE3_HASH" == "$PHASE2_HASH" ]]; then
        echo "  RESULT: PASS — hash unchanged (invalid-key DFU rejected)"
        PASS=$((PASS + 1))
    else
        echo "  RESULT: FAIL — hash changed (invalid-key DFU accepted!)"
        FAIL=$((FAIL + 1))
    fi
    echo
fi

# ── Cleanup & summary ─────────────────────────────────────────
patch_sleep 1000

echo "============================================================"
echo "  Results: $PASS / $((PASS + FAIL)) passed"
if [[ $FAIL -gt 0 ]]; then
    echo "  Status:  FAILED"
    echo "============================================================"
    exit 1
else
    echo "  Status:  ALL PASSED"
    echo "============================================================"
    exit 0
fi

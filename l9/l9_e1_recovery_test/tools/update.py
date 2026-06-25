#!/usr/bin/env python3
"""Update firmware via MCUboot serial recovery.

Modifies SLEEP_TIME_MS in the project's main.c, rebuilds with
SB_CONFIG_MCUBOOT_MODE_SINGLE_APP=y, then uses mcuboot_mgr.py to:
1. Read current image/slot info
2. Upload the new image via SMP
3. Read image/slot info after upload
4. Reset the device

Usage:
    ./update.py ../l9_e1_sol -p /dev/ttyACM1 --sleep-ms 500
    ./update.py ../l9_e1_sol -p /dev/ttyACM1 --sleep-ms 200 --build-only
"""

import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
import time


def find_ncs_root(version: str) -> str:
    candidates = [
        os.path.expanduser(f"~/ncs/{version}"),
        os.path.expanduser(f"~/nrf-connect-sdk/{version}"),
    ]
    for path in candidates:
        if os.path.isdir(path):
            return path
    return ""


def run(cmd: list[str], cwd: str) -> int:
    print(f"  > {' '.join(cmd)}")
    return subprocess.run(cmd, cwd=cwd).returncode


def run_mcuboot_mgr(script_dir: str, port: str, args: list[str]) -> int:
    mgr = os.path.join(script_dir, "mcuboot_mgr.py")
    cmd = [sys.executable, mgr, "-p", port] + args
    print(f"  > {' '.join(cmd)}")
    return subprocess.run(cmd).returncode


def patch_sleep_time(main_c: str, new_value: int) -> tuple[int | None, bool]:
    """Patch SLEEP_TIME_MS in main.c. Returns (old_value, changed)."""
    with open(main_c, "r") as f:
        content = f.read()

    match = re.search(r"(#define\s+SLEEP_TIME_MS\s+)(\d+)", content)
    if not match:
        return None, False

    old_value = int(match.group(2))
    if old_value == new_value:
        return old_value, False

    new_content = content[:match.start()] + match.group(1) + str(new_value) + content[match.end():]
    with open(main_c, "w") as f:
        f.write(new_content)
    return old_value, True


def find_signed_bin(ncs_root: str) -> str | None:
    """Find the signed binary from the build output."""
    candidates = [
        "zephyr/zephyr.signed.bin",
        "zephyr.signed.bin",
    ]
    for build_name in os.listdir(os.path.join(ncs_root, "build")):
        if build_name.startswith("_") or build_name == "mcuboot":
            continue
        for candidate in candidates:
            path = os.path.join(ncs_root, "build", build_name, candidate)
            if os.path.isfile(path):
                return path
    return None


def main():
    parser = argparse.ArgumentParser(
        description="Patch SLEEP_TIME_MS, rebuild, and upload via MCUboot serial recovery",
    )
    parser.add_argument("target", help="Path to the project directory")
    parser.add_argument("-p", "--port", required=True,
                        help="Serial port (e.g. /dev/ttyACM1)")
    parser.add_argument("--sleep-ms", type=int, required=True,
                        help="New SLEEP_TIME_MS value in milliseconds")
    parser.add_argument("--board", "-b", default="nrf54lm20dk/nrf54lm20a/cpuapp",
                        help="Target board (default: nrf54lm20dk/nrf54lm20a/cpuapp)")
    parser.add_argument("--ncs-version", default="v3.4.0-rc2",
                        help="NCS version (default: v3.4.0-rc2)")
    parser.add_argument("--ncs-root", default="",
                        help="NCS installation root (auto-detected if not set)")
    parser.add_argument("--build-only", action="store_true",
                        help="Only patch and build, do not upload")
    args = parser.parse_args()

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.abspath(args.target)
    main_c = os.path.join(project_dir, "src", "main.c")

    if not os.path.isfile(main_c):
        print(f"ERROR: Cannot find {main_c}")
        return 1

    ncs_root = args.ncs_root or find_ncs_root(args.ncs_version)
    if not ncs_root:
        print(f"ERROR: Cannot find NCS {args.ncs_version}. Use --ncs-root.")
        return 1

    # Step 0: Remove all build directories
    print(f"\n{'='*60}")
    print(f"  Step 0: Clean build directories")
    print(f"{'='*60}")
    for build_dir in glob.glob(os.path.join(project_dir, "build*")):
        if os.path.isdir(build_dir):
            print(f"  Removing {build_dir}")
            shutil.rmtree(build_dir)
    ncs_build = os.path.join(ncs_root, "build")
    if os.path.isdir(ncs_build):
        print(f"  Removing {ncs_build}")
        shutil.rmtree(ncs_build)

    # Step 1: Patch SLEEP_TIME_MS (temporary, reverted after build)
    print(f"\n{'='*60}")
    print(f"  Step 1: Patch SLEEP_TIME_MS → {args.sleep_ms}")
    print(f"{'='*60}")
    old_value, changed = patch_sleep_time(main_c, args.sleep_ms)
    if old_value is None:
        print(f"ERROR: SLEEP_TIME_MS not found in {main_c}")
        return 1
    if changed:
        print(f"  Changed SLEEP_TIME_MS: {old_value} → {args.sleep_ms}")
    else:
        print(f"  SLEEP_TIME_MS already set to {args.sleep_ms}")

    # Step 2: Build
    print(f"\n{'='*60}")
    print(f"  Step 2: Build with SB_CONFIG_MCUBOOT_MODE_SINGLE_APP=y")
    print(f"{'='*60}")
    toolchain_prefix = ["nrfutil", "toolchain-manager", "launch",
                        "--ncs-version", args.ncs_version, "--"]
    build_cmd = toolchain_prefix + [
        "west", "build", "-p", "-b", args.board, project_dir,
        "--sysbuild",
        "-DSB_CONFIG_MCUBOOT_MODE_SINGLE_APP=y",
    ]
    rc = run(build_cmd, cwd=ncs_root)

    # Revert SLEEP_TIME_MS regardless of build result
    if changed:
        patch_sleep_time(main_c, old_value)
        print(f"  Reverted SLEEP_TIME_MS: {args.sleep_ms} → {old_value}")

    if rc != 0:
        print(f"\nERROR: Build failed (exit code {rc})")
        return rc

    signed_bin = find_signed_bin(ncs_root)
    if not signed_bin:
        print("ERROR: Cannot find signed binary in build output")
        return 1
    print(f"\n  Signed image: {signed_bin}")

    if args.build_only:
        print("\n--build-only: skipping upload.")
        return 0

    # Step 3: Read current image info
    print(f"\n{'='*60}")
    print(f"  Step 3: Current image state")
    print(f"{'='*60}")
    run_mcuboot_mgr(script_dir, args.port, ["info"])

    # Step 4: Upload new image
    print(f"\n{'='*60}")
    print(f"  Step 4: Upload new image via SMP")
    print(f"{'='*60}")
    rc = run_mcuboot_mgr(script_dir, args.port, ["upload", signed_bin, "--slot", "0"])
    if rc != 0:
        print(f"\nERROR: Upload failed (exit code {rc})")
        return rc

    # Step 5: Read image info after upload
    print(f"\n{'='*60}")
    print(f"  Step 5: Image state after upload")
    print(f"{'='*60}")
    run_mcuboot_mgr(script_dir, args.port, ["info"])

    # Step 6: Reset
    print(f"\n{'='*60}")
    print(f"  Step 6: Reset device")
    print(f"{'='*60}")
    run_mcuboot_mgr(script_dir, args.port, ["reset"])

    print(f"\n  Waiting for device to boot...")
    time.sleep(3)

    # Final: show state after reset
    print(f"\n{'='*60}")
    print(f"  Final: Image state after reset")
    print(f"{'='*60}")
    run_mcuboot_mgr(script_dir, args.port, ["info"])

    print(f"\nDone. SLEEP_TIME_MS is now {args.sleep_ms}ms.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

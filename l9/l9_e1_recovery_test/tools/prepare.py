#!/usr/bin/env python3
"""Prepare the board for serial recovery testing.

Builds l9_e1_sol with MCUboot single-app mode and flashes it to the board.

Usage:
    ./prepare.py ../l9_e1_sol              # build and flash (erase)
    ./prepare.py ../l9_e1_sol --build-only # build without flashing
    ./prepare.py ../l9_e1_sol --flash-only # flash existing build
    ./prepare.py ../l9_e1_sol --sleep-ms 500
    ./prepare.py ../l9_e1_sol -b nrf52840dk/nrf52840
"""

import argparse
import glob
import os
import re
import shutil
import subprocess
import sys


def find_ncs_root(version: str) -> str:
    """Find the NCS installation root."""
    candidates = [
        os.path.expanduser(f"~/ncs/{version}"),
        os.path.expanduser(f"~/nrf-connect-sdk/{version}"),
    ]
    for path in candidates:
        if os.path.isdir(path):
            return path
    return ""


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


def run(cmd: list[str], cwd: str) -> int:
    """Run a command, streaming output to terminal."""
    print(f"\n  > {' '.join(cmd)}\n")
    result = subprocess.run(cmd, cwd=cwd)
    return result.returncode


def main():
    parser = argparse.ArgumentParser(
        description="Build and flash l9_e1_sol with MCUboot single-app (serial recovery) mode",
    )
    parser.add_argument("target", help="Path to the project directory to build")
    parser.add_argument("--board", "-b", default="nrf54lm20dk/nrf54lm20a/cpuapp",
                        help="Target board (default: nrf54lm20dk/nrf54lm20a/cpuapp)")
    parser.add_argument("--ncs-version", default="v3.4.0-rc2",
                        help="NCS version (default: v3.4.0-rc2)")
    parser.add_argument("--ncs-root", default="",
                        help="NCS installation root (auto-detected if not set)")
    parser.add_argument("--build-only", action="store_true",
                        help="Only build, do not flash")
    parser.add_argument("--flash-only", action="store_true",
                        help="Only flash existing build, do not rebuild")
    parser.add_argument("--no-erase", action="store_true",
                        help="Flash without erasing first")
    parser.add_argument("--sleep-ms", type=int, default=None,
                        help="Temporarily patch SLEEP_TIME_MS before build (reverted after)")
    args = parser.parse_args()

    # Find project source
    project_dir = os.path.abspath(args.target)
    if not os.path.isfile(os.path.join(project_dir, "CMakeLists.txt")):
        print(f"ERROR: Cannot find project at {project_dir} (no CMakeLists.txt)")
        return 1

    # Find NCS root
    ncs_root = args.ncs_root or find_ncs_root(args.ncs_version)
    if not ncs_root:
        print(f"ERROR: Cannot find NCS {args.ncs_version} installation. Use --ncs-root.")
        return 1
    print(f"NCS root:    {ncs_root}")
    print(f"Project:     {project_dir}")
    print(f"Board:       {args.board}")

    toolchain_prefix = ["nrfutil", "toolchain-manager", "launch",
                        "--ncs-version", args.ncs_version, "--"]

    # Remove all build directories in the project
    if not args.flash_only:
        for build_dir in glob.glob(os.path.join(project_dir, "build*")):
            if os.path.isdir(build_dir):
                print(f"Removing {build_dir}")
                shutil.rmtree(build_dir)
        ncs_build = os.path.join(ncs_root, "build")
        if os.path.isdir(ncs_build):
            print(f"Removing {ncs_build}")
            shutil.rmtree(ncs_build)

        # Patch SLEEP_TIME_MS if requested
        patched = False
        old_sleep = None
        if args.sleep_ms is not None:
            main_c = os.path.join(project_dir, "src", "main.c")
            if not os.path.isfile(main_c):
                print(f"ERROR: Cannot find {main_c}")
                return 1
            old_sleep, patched = patch_sleep_time(main_c, args.sleep_ms)
            if old_sleep is None:
                print(f"ERROR: SLEEP_TIME_MS not found in {main_c}")
                return 1
            if patched:
                print(f"Patched SLEEP_TIME_MS: {old_sleep} → {args.sleep_ms}")

        print("\n=== Building ===")
        build_cmd = toolchain_prefix + [
            "west", "build", "-p", "-b", args.board, project_dir,
            "--sysbuild",
            "-DSB_CONFIG_MCUBOOT_MODE_SINGLE_APP=y",
        ]
        rc = run(build_cmd, cwd=ncs_root)

        # Revert SLEEP_TIME_MS regardless of build result
        if patched:
            patch_sleep_time(main_c, old_sleep)
            print(f"Reverted SLEEP_TIME_MS: {args.sleep_ms} → {old_sleep}")

        if rc != 0:
            print(f"\nERROR: Build failed (exit code {rc})")
            return rc
        print("\nBuild succeeded.")

    # Flash
    if not args.build_only:
        print("\n=== Flashing ===")
        flash_cmd = toolchain_prefix + ["west", "flash"]
        if not args.no_erase:
            flash_cmd.append("--erase")
        rc = run(flash_cmd, cwd=ncs_root)
        if rc != 0:
            print(f"\nERROR: Flash failed (exit code {rc})")
            return rc
        print("\nFlash succeeded. Board is running with MCUboot serial recovery mode.")

    return 0


if __name__ == "__main__":
    sys.exit(main())

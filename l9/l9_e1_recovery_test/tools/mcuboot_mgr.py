#!/usr/bin/env python3
"""MCUboot image management tool using smpmgr.

Usage examples:
    # Read image state (slots info)
    ./mcuboot_mgr.py -p /dev/ttyACM1 info

    # Upload image to slot 1 (secondary)
    ./mcuboot_mgr.py -p /dev/ttyACM1 upload firmware.bin

    # Upload and mark for test (swap on next reset)
    ./mcuboot_mgr.py -p /dev/ttyACM1 upload firmware.bin --test

    # Upload, mark for test, and reset device
    ./mcuboot_mgr.py -p /dev/ttyACM1 upload firmware.bin --test --reset

    # Confirm currently running image
    ./mcuboot_mgr.py -p /dev/ttyACM1 confirm

    # Reset device
    ./mcuboot_mgr.py -p /dev/ttyACM1 reset
"""

import argparse
import json
import re
import subprocess
import sys
import time


def run_smpmgr(port: str, args: list[str], timeout: float = 10.0) -> subprocess.CompletedProcess:
    """Run an smpmgr command and return the result."""
    cmd = ["smpmgr", "--port", port, "--timeout", str(timeout)] + args
    print(f"  > {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    return result


def cmd_info(port: str, timeout: float) -> int:
    """Read and display image state (slot info)."""
    result = run_smpmgr(port, ["image", "state-read"], timeout)
    if result.returncode != 0:
        print(f"ERROR: {result.stderr.strip() or result.stdout.strip()}")
        return 1
    print(result.stdout)
    return 0


def parse_image_hash(state_output: str, slot: int = 1) -> str | None:
    """Parse the image hash from state-read output for a given slot."""
    # smpmgr output contains hash info per image slot
    # Look for hash in the output - format varies but typically shows hex hash
    lines = state_output.splitlines()
    current_slot = None
    for line in lines:
        # Look for slot indicators
        if "slot" in line.lower():
            slot_match = re.search(r"slot[:\s]*(\d+)", line, re.IGNORECASE)
            if slot_match:
                current_slot = int(slot_match.group(1))
        # Look for hash
        if current_slot == slot:
            hash_match = re.search(r"hash[:\s]*([0-9a-fA-F]{64})", line, re.IGNORECASE)
            if hash_match:
                return hash_match.group(1)
    return None


def cmd_upload(port: str, image: str, slot: int, test: bool, confirm: bool,
               reset: bool, timeout: float) -> int:
    """Upload an image and optionally test/confirm/reset."""
    print(f"Uploading {image} to slot {slot}...")
    result = run_smpmgr(port, ["image", "upload", image, "--slot", str(slot)], timeout=30.0)
    if result.returncode != 0:
        print(f"ERROR uploading: {result.stderr.strip() or result.stdout.strip()}")
        return 1
    print(result.stdout)
    print("Upload complete.")

    if test:
        print("\nMarking image for test (swap on next reset)...")
        # Read state to get the hash of the uploaded image
        state_result = run_smpmgr(port, ["image", "state-read"], timeout)
        if state_result.returncode != 0:
            print(f"ERROR reading state: {state_result.stderr.strip()}")
            return 1
        print(state_result.stdout)

        img_hash = parse_image_hash(state_result.stdout, slot)
        if img_hash:
            test_result = run_smpmgr(port, ["image", "state-write", img_hash], timeout)
            if test_result.returncode != 0:
                print(f"ERROR marking for test: {test_result.stderr.strip() or test_result.stdout.strip()}")
                return 1
            print("Image marked for test swap on next reset.")
        else:
            # Try without hash - some versions mark the secondary slot by default
            test_result = run_smpmgr(port, ["image", "state-write"], timeout)
            if test_result.returncode != 0:
                print(f"ERROR: Could not mark image for test. Try manually.")
                return 1
            print("Image marked for test.")

    if confirm:
        print("\nConfirming image...")
        confirm_result = run_smpmgr(port, ["image", "state-write", "--confirm"], timeout)
        if confirm_result.returncode != 0:
            print(f"ERROR confirming: {confirm_result.stderr.strip() or confirm_result.stdout.strip()}")
            return 1
        print("Image confirmed.")

    if reset:
        print("\nResetting device...")
        time.sleep(0.5)
        reset_result = run_smpmgr(port, ["os", "reset"], timeout)
        if reset_result.returncode != 0:
            print(f"WARNING: Reset command returned error (device may have reset before responding)")
        else:
            print("Device reset triggered.")
        # Wait for device to come back
        print("Waiting for device to boot...")
        time.sleep(3)

        # Read state after reset to show result
        print("\nImage state after reset:")
        cmd_info(port, timeout)

    return 0


def cmd_confirm(port: str, timeout: float) -> int:
    """Confirm the currently running image."""
    print("Confirming currently running image...")
    result = run_smpmgr(port, ["image", "state-write", "--confirm"], timeout)
    if result.returncode != 0:
        print(f"ERROR: {result.stderr.strip() or result.stdout.strip()}")
        return 1
    print(result.stdout if result.stdout.strip() else "Image confirmed.")
    return 0


def cmd_reset(port: str, timeout: float) -> int:
    """Reset the device."""
    print("Resetting device...")
    result = run_smpmgr(port, ["os", "reset"], timeout)
    if result.returncode != 0:
        # Device often resets before it can send a response
        print("Reset sent (device may have disconnected before acknowledging).")
    else:
        print("Device reset triggered.")
    return 0


def main():
    parser = argparse.ArgumentParser(
        description="MCUboot image management tool using smpmgr",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("-p", "--port", required=True, help="Serial port (e.g. /dev/ttyACM1)")
    parser.add_argument("-t", "--timeout", type=float, default=5.0,
                        help="SMP transport timeout in seconds (default: 5)")

    subparsers = parser.add_subparsers(dest="command", required=True)

    # info command
    subparsers.add_parser("info", help="Read image state and slot info")

    # upload command
    upload_parser = subparsers.add_parser("upload", help="Upload a firmware image")
    upload_parser.add_argument("image", help="Path to firmware image (.bin)")
    upload_parser.add_argument("--slot", type=int, default=1,
                               help="Target slot (default: 1 = secondary)")
    upload_parser.add_argument("--test", action="store_true",
                               help="Mark image for test swap on next reset")
    upload_parser.add_argument("--confirm", action="store_true",
                               help="Confirm the image (make permanent)")
    upload_parser.add_argument("--reset", action="store_true",
                               help="Reset device after upload")

    # confirm command
    subparsers.add_parser("confirm", help="Confirm the currently running image")

    # reset command
    subparsers.add_parser("reset", help="Reset the device")

    args = parser.parse_args()

    if args.command == "info":
        return cmd_info(args.port, args.timeout)
    elif args.command == "upload":
        return cmd_upload(args.port, args.image, args.slot, args.test,
                          args.confirm, args.reset, args.timeout)
    elif args.command == "confirm":
        return cmd_confirm(args.port, args.timeout)
    elif args.command == "reset":
        return cmd_reset(args.port, args.timeout)


if __name__ == "__main__":
    sys.exit(main() or 0)

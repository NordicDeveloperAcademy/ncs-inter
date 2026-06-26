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

    # Confirm image in a specific slot
    ./mcuboot_mgr.py -p /dev/ttyACM1 confirm --slot 0

    # Erase image from slot 1 (secondary)
    ./mcuboot_mgr.py -p /dev/ttyACM1 erase

    # Erase image from a specific slot
    ./mcuboot_mgr.py -p /dev/ttyACM1 erase --slot 1

    # Reset device
    ./mcuboot_mgr.py -p /dev/ttyACM1 reset
"""

import argparse
import json
import re
import selectors
import subprocess
import sys
import time


def run_smpmgr(port: str, args: list[str], timeout: float = 10.0,
               quiet: bool = False) -> subprocess.CompletedProcess:
    """Run an smpmgr command, streaming output in real-time and capturing it."""
    cmd = ["smpmgr", "--port", port, "--timeout", str(timeout)] + args
    if not quiet:
        print(f"  > {' '.join(cmd)}")
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    stdout_lines = []
    stderr_lines = []

    sel = selectors.DefaultSelector()
    sel.register(proc.stdout, selectors.EVENT_READ)
    sel.register(proc.stderr, selectors.EVENT_READ)

    while True:
        for key, _ in sel.select(timeout=0.1):
            line = key.fileobj.readline()
            if not line:
                continue
            if key.fileobj is proc.stdout:
                stdout_lines.append(line)
                if not quiet:
                    print(line, end="", flush=True)
            else:
                stderr_lines.append(line)
                if not quiet:
                    print(line, end="", file=sys.stderr, flush=True)
        if proc.poll() is not None:
            # Read remaining
            for line in proc.stdout:
                stdout_lines.append(line)
                if not quiet:
                    print(line, end="", flush=True)
            for line in proc.stderr:
                stderr_lines.append(line)
                if not quiet:
                    print(line, end="", file=sys.stderr, flush=True)
            break

    sel.close()
    return subprocess.CompletedProcess(
        cmd, proc.returncode,
        stdout="".join(stdout_lines),
        stderr="".join(stderr_lines),
    )


def cmd_info(port: str, timeout: float, verbose: bool = False) -> int:
    """Read and display image state (slot info)."""
    result = run_smpmgr(port, ["image", "state-read"], timeout, quiet=not verbose)
    if result.returncode != 0:
        print(f"ERROR: {result.stderr.strip() or result.stdout.strip()}")
        return 1

    # Parse and display summary table
    slots = parse_image_states(result.stdout)
    if slots:
        print(f"\n  {'Slot':<6} {'Version':<12} {'Active':<8} {'Confirmed':<11} {'Pending':<9} {'Bootable':<9}")
        print(f"  {'-'*5:<6} {'-'*11:<12} {'-'*6:<8} {'-'*9:<11} {'-'*7:<9} {'-'*8:<9}")
        for s in slots:
            print(f"  {s['slot']:<6} {s['version'] or 'N/A':<12} "
                  f"{'yes' if s['active'] else 'no':<8} "
                  f"{'yes' if s['confirmed'] else 'no':<11} "
                  f"{'yes' if s['pending'] else 'no':<9} "
                  f"{'yes' if s['bootable'] else 'no':<9}")
        print()
        for s in slots:
            if s.get("hash"):
                print(f"  Slot {s['slot']} hash: {s['hash'][:64]}...")
    else:
        print(result.stdout)

    return 0


def cmd_detailed_info(port: str, timeout: float, echo: bool = False) -> int:
    """Get detailed info: supported groups, image state, echo test, and summary."""

    # 1. Supported groups
    print(f"\n{'='*60}")
    print(f"  SMP Supported Groups")
    print(f"{'='*60}")
    result = run_smpmgr(port, ["enum", "get-supported-groups"], timeout)
    if result.returncode == 0:
        print(result.stdout)
    else:
        print(f"  (not supported)")

    # 2. Image state (raw)
    print(f"\n{'='*60}")
    print(f"  Image State (Raw)")
    print(f"{'='*60}")
    result = run_smpmgr(port, ["image", "state-read"], timeout)
    if result.returncode != 0:
        print(f"  ERROR: {result.stderr.strip() or result.stdout.strip()}")
        return 1
    print(result.stdout)

    # 3. Echo test
    if echo:
        print(f"\n{'='*60}")
        print(f"  SMP Echo Test")
        print(f"{'='*60}")
        echo_result = run_smpmgr(port, ["os", "echo", "hello-from-mcuboot-mgr"], timeout)
        if echo_result.returncode == 0:
            print(f"  {echo_result.stdout.strip()}")
        else:
            print(f"  ERROR: {echo_result.stderr.strip() or echo_result.stdout.strip()}")

    # 4. Summary table
    print(f"\n{'='*60}")
    print(f"  Summary")
    print(f"{'='*60}")
    print(f"  Port:    {port}")
    print(f"  Timeout: {timeout}s")

    slots = parse_image_states(result.stdout)
    if slots:
        print()
        print(f"  {'Slot':<6} {'Version':<12} {'Active':<8} {'Confirmed':<11} {'Pending':<9} {'Bootable':<9}")
        print(f"  {'-'*5:<6} {'-'*11:<12} {'-'*6:<8} {'-'*9:<11} {'-'*7:<9} {'-'*8:<9}")
        for s in slots:
            print(f"  {s['slot']:<6} {s['version'] or 'N/A':<12} "
                  f"{'yes' if s['active'] else 'no':<8} "
                  f"{'yes' if s['confirmed'] else 'no':<11} "
                  f"{'yes' if s['pending'] else 'no':<9} "
                  f"{'yes' if s['bootable'] else 'no':<9}")
        print()
        for s in slots:
            if s.get("hash"):
                print(f"  Slot {s['slot']} hash: {s['hash'][:64]}...")
    print()
    return 0


def parse_image_states(text: str) -> list[dict]:
    """Parse ImageState blocks from smpmgr output."""
    pattern = re.compile(
        r"ImageState\(\s*"
        r"slot=(\d+),.*?"
        r"version='([^']*)'.*?"
        r"bootable=(True|False).*?"
        r"pending=(True|False).*?"
        r"confirmed=(True|False).*?"
        r"active=(True|False)",
        re.DOTALL,
    )
    hash_pattern = re.compile(r"hash=HashBytes\(\s*'([0-9A-Fa-f\s\n]+)'", re.DOTALL)

    slots = []
    for m in pattern.finditer(text):
        slots.append({
            "slot": m.group(1),
            "version": m.group(2),
            "bootable": m.group(3) == "True",
            "pending": m.group(4) == "True",
            "confirmed": m.group(5) == "True",
            "active": m.group(6) == "True",
            "hash": None,
        })

    for i, hm in enumerate(hash_pattern.finditer(text)):
        if i < len(slots):
            slots[i]["hash"] = hm.group(1).replace("\n", "").replace(" ", "")

    return slots


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

        # Use parse_image_states which handles ImageState(...) blocks
        slots = parse_image_states(state_result.stdout)
        img_hash = None
        for s in slots:
            if int(s["slot"]) == slot and s.get("hash"):
                img_hash = s["hash"]
                break

        if img_hash:
            test_result = run_smpmgr(port, ["image", "state-write", img_hash], timeout)
            if test_result.returncode != 0:
                print(f"ERROR marking for test: {test_result.stderr.strip() or test_result.stdout.strip()}")
                return 1
            print("Image marked for test swap on next reset.")
        else:
            print(f"ERROR: Could not find hash for image in slot {slot}. Try manually.")
            return 1

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


def cmd_confirm(port: str, timeout: float, image_hash: str | None = None,
                slot: int | None = None) -> int:
    """Confirm an image. If slot is given, read its hash first. If hash is given, use it directly."""
    if image_hash is None and slot is not None:
        print(f"Reading image state to find hash for slot {slot}...")
        state_result = run_smpmgr(port, ["image", "state-read"], timeout, quiet=True)
        if state_result.returncode != 0:
            print(f"ERROR reading state: {state_result.stderr.strip() or state_result.stdout.strip()}")
            return 1
        slots = parse_image_states(state_result.stdout)
        for s in slots:
            if int(s["slot"]) == slot and s.get("hash"):
                image_hash = s["hash"]
                break
        if image_hash is None:
            print(f"ERROR: No image hash found in slot {slot}")
            return 1

    if image_hash:
        print(f"Confirming image with hash {image_hash[:32]}...")
        result = run_smpmgr(port, ["image", "state-write", image_hash, "--confirm"], timeout)
    else:
        print("Confirming currently running image...")
        result = run_smpmgr(port, ["image", "state-write", "--confirm"], timeout)
    if result.returncode != 0:
        print(f"ERROR: {result.stderr.strip() or result.stdout.strip()}")
        return 1
    print(result.stdout if result.stdout.strip() else "Image confirmed.")
    return 0


def cmd_erase(port: str, timeout: float, slot: int = 1) -> int:
    """Erase image from a slot."""
    print(f"Erasing image in slot {slot}...")
    result = run_smpmgr(port, ["image", "erase", str(slot)], timeout)
    if result.returncode != 0:
        print(f"ERROR: {result.stderr.strip() or result.stdout.strip()}")
        return 1
    print(result.stdout if result.stdout.strip() else f"Image in slot {slot} erased.")
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
    info_parser = subparsers.add_parser("info", help="Read image state and slot info")
    info_parser.add_argument("-v", "--verbose", action="store_true",
                             help="Show raw smpmgr output in addition to summary")

    # detailed-info command
    detailed_parser = subparsers.add_parser("detailed-info",
                                            help="Detailed info: groups, slots, echo")
    detailed_parser.add_argument("--echo", action="store_true",
                                 help="Also run SMP echo test")

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
    confirm_parser = subparsers.add_parser("confirm", help="Confirm the currently running image")
    confirm_parser.add_argument("--hash", default=None,
                                help="Hash of image to confirm (default: currently running)")
    confirm_parser.add_argument("--slot", type=int, default=None,
                                help="Confirm image in this slot (reads hash automatically)")

    # erase command
    erase_parser = subparsers.add_parser("erase", help="Erase image from a slot")
    erase_parser.add_argument("--slot", type=int, default=1,
                              help="Slot to erase (default: 1 = secondary)")

    # reset command
    subparsers.add_parser("reset", help="Reset the device")

    args = parser.parse_args()

    if args.command == "info":
        return cmd_info(args.port, args.timeout, verbose=args.verbose)
    elif args.command == "detailed-info":
        return cmd_detailed_info(args.port, args.timeout, echo=args.echo)
    elif args.command == "upload":
        return cmd_upload(args.port, args.image, args.slot, args.test,
                          args.confirm, args.reset, args.timeout)
    elif args.command == "confirm":
        return cmd_confirm(args.port, args.timeout, image_hash=args.hash, slot=args.slot)
    elif args.command == "erase":
        return cmd_erase(args.port, args.timeout, slot=args.slot)
    elif args.command == "reset":
        return cmd_reset(args.port, args.timeout)


if __name__ == "__main__":
    sys.exit(main() or 0)

#!/usr/bin/env python3
"""Get detailed info about MCUboot slots, images, and device state.

Wrapper around mcuboot_mgr.py detailed-info command.

Usage:
    ./slot_info.py -p /dev/ttyACM2
    ./slot_info.py -p /dev/ttyACM2 --echo
"""

import os
import subprocess
import sys


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    mgr = os.path.join(script_dir, "mcuboot_mgr.py")

    args = sys.argv[1:]

    echo = False
    filtered = []
    for a in args:
        if a == "--echo":
            echo = True
        else:
            filtered.append(a)

    cmd = [sys.executable, mgr] + filtered + ["detailed-info"]
    if echo:
        cmd.append("--echo")

    return subprocess.run(cmd).returncode


if __name__ == "__main__":
    sys.exit(main())

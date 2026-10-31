#!/usr/bin/env python3
# Written by Claude (Anthropic)
"""
Usage:
  flash.py board TARGET flash SIZE port PORT image FILE|NAME=FILE ... [--baud N]

  FILE       -> next free OTA partition (in order of offsets)
  NAME=FILE  -> partition with the given name (e.g. app3=retro.bin)

Example:
  flash.py board esp32s3 flash 32mb port /dev/cu.usbmodem1 image app1.bin app2.bin
"""
import argparse
import json
import struct
import subprocess
import sys
from pathlib import Path

BUILD = Path("build")
MB = 1024 * 1024


def load_base():
    """Return everything idf.py built: bootloader, table, otadata, launcher."""
    cfg = json.loads((BUILD / "flasher_args.json").read_text(encoding="utf-8"))
    files = [(int(off, 0), (BUILD / f).resolve()) for off, f in cfg["flash_files"].items()]
    return files, cfg.get("flash_settings", {})


def load_partitions():
    """Parse the real table from partition-table.bin (the one that goes to the chip)."""
    data = (BUILD / "partition_table" / "partition-table.bin").read_bytes()
    ota, end = [], 0
    for i in range(0, len(data), 32):
        magic, ptype, sub, off, size, name, _ = struct.unpack("<2sBBII16sI", data[i:i + 32])
        if magic != b"\xaa\x50":  # end of entries (MD5 record or empty flash)
            break
        end = max(end, off + size)
        if ptype == 0 and 0x10 <= sub <= 0x1F:  # app, ota_0..ota_15
            ota.append((name.rstrip(b"\0").decode(), off, size))
    return sorted(ota, key=lambda p: p[1]), end


def resolve(value):
    """Find an image as given or relative to the build directory."""
    path = Path(value)
    if not path.is_file():
        path = BUILD / value
    if not path.is_file():
        raise ValueError(f"File not found: {value}")
    return path.resolve()


def assign(ota, items):
    """Match images to partitions: by name (NAME=FILE) or by order."""
    by_name = {p[0]: p for p in ota}
    named, rest = {}, []
    for item in items:
        name, sep, file = item.partition("=")
        if sep:
            if name not in by_name:
                raise ValueError(f"Unknown partition: {name}")
            named[name] = file
        else:
            rest.append(item)

    free = [p for p in ota if p[0] not in named]
    if len(rest) > len(free):
        raise ValueError("Not enough OTA partitions for the supplied images.")

    return [(by_name[n], f) for n, f in named.items()] + list(zip(free, rest))


def main():
    p = argparse.ArgumentParser(description="ESP flasher")
    for word in ("board", "board_value", "flash", "flash_value", "port", "port_value", "image"):
        p.add_argument(word)
    p.add_argument("images", nargs="+")
    p.add_argument("--baud", type=int, default=460800)
    a = p.parse_args()

    if (a.board, a.flash, a.port, a.image) != ("board", "flash", "port", "image"):
        p.error("Expected: flash.py board TARGET flash SIZE port PORT image FILE...")

    size_name = a.flash_value.lower()
    if not size_name.endswith("mb") or not size_name[:-2].isdigit():
        p.error("Flash size must look like 16mb, 32mb, 64mb.")
    capacity = int(size_name[:-2]) * MB

    base, settings = load_base()
    ota, table_end = load_partitions()
    if not ota:
        raise ValueError("No OTA application partitions found.")
    if table_end > capacity:
        raise ValueError(f"Partition table ends at 0x{table_end:X}, flash is {size_name.upper()}.")

    # Validate every application image before touching the chip.
    apps = []
    for part, file in assign(ota, a.images):
        name, off, size = part
        path = resolve(file)
        if path.stat().st_size > size:
            raise ValueError(f"{path.name} exceeds partition '{name}'.")
        apps.append((off, path, name))

    for off, path in base:
        if off + path.stat().st_size > capacity:
            raise ValueError(f"{path.name} exceeds the selected flash size.")

    print(f"Target: {a.board_value}  Flash: {size_name.upper()}  Port: {a.port_value}")
    print("\nBase (bootloader, table, otadata, launcher):")
    for off, path in base:
        print(f"  {path.name} -> 0x{off:X}")
    print("\nApplications:")
    for off, path, name in apps:
        print(f"  {path.name} -> {name} @ 0x{off:X}")

    if input("\nStart flashing? [y/N]: ").strip().lower() != "y":
        print("Flashing cancelled.")
        return

    # A single esptool call means a single chip reset.
    cmd = [sys.executable, "-m", "esptool", "--chip", a.board_value,
           "--port", a.port_value, "--baud", str(a.baud), "--after", "hard_reset",
           "write_flash", "--flash_size", size_name.upper()]
    for key in ("flash_mode", "flash_freq"):
        if key in settings:
            cmd += [f"--{key}", settings[key]]
    for off, path in base + [(o, p) for o, p, _ in apps]:
        cmd += [hex(off), str(path)]

    subprocess.run(cmd, check=True)
    print("\nFlashing completed successfully.")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, subprocess.CalledProcessError, KeyError) as e:
        sys.exit(f"Error: {e}")
    except KeyboardInterrupt:
        sys.exit("\nFlashing interrupted.")

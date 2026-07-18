#!/usr/bin/env python3

import argparse
import hashlib
import json
import re
import subprocess
from pathlib import Path
import sys
import tempfile

import capstone
import pefile

from static_recompile_pilot import (
    ARTIFACT_KIND,
    GENERATED_SOURCE,
    SUPPORTED_EXE_SHA256,
    TARGET_ADDRESS,
    TARGET_BODY_SHA256,
    TARGET_END,
    TARGET_NAME,
    TARGET_SIZE,
    generate,
    sha256_file,
    validate_oracle_manifest,
)


CORE_SYMBOL = "_opensmacx_static_recompiled_core_004C5460"
WRAPPER_SYMBOL = "_opensmacx_static_recompiled_result_004C5460"
IMAGE_START = 0x00400000
IMAGE_END = 0x00A10000


def run(command):
    return subprocess.run(
        command, check=True, text=True, capture_output=True).stdout


def main():
    parser = argparse.ArgumentParser(
        description="Verify the local static recompilation micro-pilot")
    parser.add_argument("--nm", required=True)
    parser.add_argument("--objdump", required=True)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--object", required=True)
    parser.add_argument("--original-object", required=True)
    parser.add_argument("--exe", type=Path, required=True)
    parser.add_argument("--oracle-manifest", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, required=True)
    args = parser.parse_args()

    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    if (manifest.get("artifact_kind")
            != ARTIFACT_KIND
            or manifest.get("distribution") != "local_only"
            or manifest.get("open_source_ready") is not False
            or manifest.get("backend") != "cxx11_static_basic_blocks_v1"
            or manifest.get("provenance")
                != "mechanically_derived_from_original_executable"):
        raise RuntimeError("invalid static recompilation provenance manifest")
    expected_function = {
        "address": f"0x{TARGET_ADDRESS:08X}",
        "canonical_name": TARGET_NAME,
        "end_address": f"0x{TARGET_END:08X}",
        "sha256": TARGET_BODY_SHA256,
        "size": TARGET_SIZE,
    }
    if manifest.get("function") != expected_function:
        raise RuntimeError("static recompilation manifest has unexpected function identity")
    source = manifest.get("source", {})
    if (source.get("file_name") != "terranx_original.exe"
            or source.get("sha256") != SUPPORTED_EXE_SHA256
            or source.get("size") != 3084288):
        raise RuntimeError("static recompilation manifest has unexpected source identity")
    if sha256_file(args.exe) != SUPPORTED_EXE_SHA256:
        raise RuntimeError("static recompilation input executable hash does not match")
    validate_oracle_manifest(args.oracle_manifest)
    if (manifest.get("original_oracle_manifest_sha256")
            != sha256_file(args.oracle_manifest)):
        raise RuntimeError("static recompilation manifest has stale oracle provenance")
    if manifest.get("tool_versions") != {
            "capstone": capstone.__version__, "pefile": pefile.__version__}:
        raise RuntimeError("static recompilation manifest has unexpected tool versions")
    generated = manifest.get("generated_source", {})
    generated_path = args.manifest.parent / generated.get("path", "")
    if (generated.get("path") != GENERATED_SOURCE
            or not generated_path.is_file()
            or generated.get("sha256") != sha256_file(generated_path)):
        raise RuntimeError("generated source hash does not match its manifest")

    verification_parent = args.manifest.parent.parent
    with tempfile.TemporaryDirectory(
            prefix="static-recompile-verification-",
            dir=verification_parent) as directory:
        verification_root = Path(directory)
        fresh_original = verification_root / "original"
        run([
            sys.executable,
            str(Path(__file__).resolve().with_name("extract_legacy_leaves.py")),
            "--exe", str(args.exe),
            "--output", str(fresh_original),
            "--max-size", "64",
            "--address", f"0x{TARGET_ADDRESS:08X}",
        ])
        for name in ("legacy-leaves.S", "manifest.json", "symbols.csv"):
            if ((fresh_original / name).read_bytes()
                    != (args.oracle_manifest.parent / name).read_bytes()):
                raise RuntimeError(f"original oracle is not deterministic: {name}")
        fresh_original_object = verification_root / "original.obj"
        run([
            args.compiler,
            "-x", "assembler-with-cpp",
            "-c", str(fresh_original / "legacy-leaves.S"),
            "-o", str(fresh_original_object),
        ])
        if fresh_original_object.read_bytes() != Path(args.original_object).read_bytes():
            raise RuntimeError("original oracle COFF object is stale or modified")

        fresh_output = verification_root / "generated"
        generate(args.exe, args.oracle_manifest, fresh_output)
        for name in (GENERATED_SOURCE, "manifest.json"):
            if (fresh_output / name).read_bytes() != (args.manifest.parent / name).read_bytes():
                raise RuntimeError(f"static recompilation output is not deterministic: {name}")

    headers = run([args.objdump, "-f", args.object])
    if "file format pe-i386" not in headers:
        raise RuntimeError("generated pilot object is not pe-i386")
    symbols = run([args.nm, args.object])
    for symbol in (CORE_SYMBOL, WRAPPER_SYMBOL):
        if symbol not in symbols:
            raise RuntimeError(f"missing generated pilot symbol: {symbol}")
    if "opensmacx_legacy_004C5460" in symbols:
        raise RuntimeError("generated pilot object depends on the original implementation")
    if run([args.nm, "-u", args.object]).strip():
        raise RuntimeError("generated pilot object has undefined symbols")

    original_contents = run([
        args.objdump, "-s", "-j", ".text$legacy$004C5460",
        args.original_object,
    ])
    original_bytes = bytearray()
    for line in original_contents.splitlines():
        fields = line.split()
        if not fields or not re.fullmatch(r"[0-9a-fA-F]{4,8}", fields[0]):
            continue
        for field in fields[1:]:
            if not re.fullmatch(r"[0-9a-fA-F]{8}", field):
                break
            original_bytes.extend(bytes.fromhex(field))
    if (len(original_bytes) < TARGET_SIZE
            or hashlib.sha256(original_bytes[:TARGET_SIZE]).hexdigest()
                != TARGET_BODY_SHA256):
        raise RuntimeError("original pilot object body hash does not match its manifest")

    disassembly = run([args.objdump, "-d", "-C", args.object])
    wrapper = re.search(
        r"<opensmacx_static_recompiled_result_004C5460>:(?P<body>.*?)"
        r"(?=\n[0-9a-f]+ <|\Z)", disassembly, re.DOTALL)
    if not wrapper:
        raise RuntimeError("could not locate generated thiscall wrapper")
    returns = re.findall(r"\bret(?:\s+\$0x([0-9a-f]+))?\b", wrapper.group("body"))
    if not returns or any(value != "4" for value in returns):
        raise RuntimeError("generated wrapper does not use thiscall stack cleanup")
    for line in disassembly.splitlines():
        if "\t" not in line or not re.match(r"^\s*[0-9a-f]+:", line):
            continue
        assembly = line.split("\t")[-1]
        values = [int(value, 16) for value in re.findall(r"0x([0-9a-f]+)", assembly)]
        direct = re.search(
            r"\b(?:call|jmp)\s+(?:\*)?([0-9a-f]{6,8})\b", assembly)
        if direct:
            values.append(int(direct.group(1), 16))
        if any(IMAGE_START <= value < IMAGE_END for value in values):
            raise RuntimeError("generated pilot object contains an original-image address")

    section_headers = run([args.objdump, "-h", args.object])
    sections = {}
    pending_section = None
    for line in section_headers.splitlines():
        header = re.match(
            r"^\s*\d+\s+(\S+)\s+([0-9a-fA-F]+)\s+", line)
        if header:
            pending_section = header.group(1)
            sections[pending_section] = {
                "size": int(header.group(2), 16),
                "flags": set(),
            }
            continue
        if pending_section and line.strip():
            sections[pending_section]["flags"] = {
                flag.strip() for flag in line.split(",")
            }
            pending_section = None

    for name, section in sections.items():
        flags = section["flags"]
        if (section["size"] == 0 or "ALLOC" not in flags or "DATA" not in flags
                or "CONTENTS" not in flags or "DEBUGGING" in flags):
            continue
        contents = run([args.objdump, "-s", "-j", name, args.object])
        data = bytearray()
        for line in contents.splitlines():
            fields = line.split()
            if not fields or not re.fullmatch(r"[0-9a-fA-F]{4,8}", fields[0]):
                continue
            for field in fields[1:]:
                if not re.fullmatch(r"[0-9a-fA-F]{8}", field):
                    break
                data.extend(bytes.fromhex(field))
        data = data[:section["size"]]
        for offset in range(max(0, len(data) - 3)):
            value = int.from_bytes(data[offset:offset + 4], "little")
            if IMAGE_START <= value < IMAGE_END:
                raise RuntimeError(
                    f"generated pilot data section {name} contains an original-image address")

    relocations = run([args.objdump, "-r", args.object])
    executable_relocations = []
    active = False
    for line in relocations.splitlines():
        section = re.match(r"RELOCATION RECORDS FOR \[(.+)\]:", line)
        if section:
            section_name = section.group(1)
            active = (
                section_name in sections
                and "ALLOC" in sections[section_name]["flags"])
            continue
        if active and re.match(r"^[0-9a-fA-F]+\s+", line):
            executable_relocations.append(line)
    defined = {}
    for fields in (line.split() for line in symbols.splitlines()):
        if len(fields) < 3 or fields[-2].upper() == "U":
            continue
        try:
            value = int(fields[0], 16)
        except ValueError:
            continue
        defined[fields[-1]] = (fields[-2], value)
    for relocation in executable_relocations:
        target = relocation.split()[-1]
        if target not in defined:
            raise RuntimeError(
                f"generated pilot has an unexpected allocated-section relocation: {target}")
        symbol_type, value = defined[target]
        if symbol_type.upper() == "A" and IMAGE_START <= value < IMAGE_END:
            raise RuntimeError(
                f"generated pilot relocation targets original-image absolute symbol: {target}")

    print("Static recompilation pilot ABI and provenance checks passed")


if __name__ == "__main__":
    main()

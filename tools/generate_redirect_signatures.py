#!/usr/bin/env python3
"""Generate the DllMain redirect signature header from the canonical binary.

Every runtime redirect validates a 16-byte signature at its original address
before patching, and near-identical prologues carry a 6-byte discriminating
extension. Hand-transcribing those bytes risks silent whole-batch install
failures at runtime, so this tool reads them from the independently analyzed
canonical executable, cross-checks byte equality against the hash-pinned PRACX
runtime executable actually patched at run time, derives collision extensions
at instruction boundaries with Capstone, and emits a deterministic committed
header. The emission is hard-capped at 16 signature bytes plus 6 extension
bytes per address - the same data class already committed in dllmain.cpp.
"""

import argparse
import csv
import hashlib
from pathlib import Path
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
import pefile


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_REDIRECTS = REPO_ROOT / "docs" / "recovery-redirects.csv"
DEFAULT_CANONICAL_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
DEFAULT_RUNTIME_EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx.exe"
DEFAULT_OUTPUT = REPO_ROOT / "src" / "redirect_signatures.h"

SIGNATURE_SIZE = 16
EXTENSION_SIZE = 6
EXTENSION_SEARCH_LIMIT = 112
VALID_KINDS = ("jump", "call", "dependency")


def read_virtual_bytes(pe, address, size):
    rva = address - pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        section_start = section.VirtualAddress
        section_end = section_start + section.SizeOfRawData
        if section_start <= rva and rva + size <= section_end:
            offset = section.PointerToRawData + rva - section_start
            return pe.__data__[offset:offset + size]
    raise RuntimeError(f"address 0x{address:08X} is not file-backed")


def load_redirects(path):
    rows = []
    seen = set()
    with path.open(newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        if reader.fieldnames != ["address", "kind"]:
            raise RuntimeError("recovery-redirects.csv must have address,kind columns")
        for row in reader:
            address = int(row["address"], 16)
            kind = row["kind"]
            if kind not in VALID_KINDS:
                raise RuntimeError(f"unknown redirect kind: {kind}")
            if address in seen:
                raise RuntimeError(f"duplicate redirect address 0x{address:08X}")
            seen.add(address)
            rows.append((address, kind))
    if not rows:
        raise RuntimeError("recovery-redirects.csv lists no redirects")
    return rows


def derive_extensions(disassembler, canonical, collisions):
    """Derive instruction-aligned 6-byte extensions for colliding addresses.

    Each colliding address gets the earliest instruction-aligned window that
    distinguishes it from every other member of its collision group. Members
    whose bodies stay byte-identical across the whole search window get no
    extension: the signature check is then equivalent for either body, which
    matches the existing runtime behavior for byte-identical functions.
    """
    extensions = []
    identical = []
    for signature, addresses in sorted(collisions.items()):
        window_size = SIGNATURE_SIZE + EXTENSION_SEARCH_LIMIT
        bodies = {
            address: read_virtual_bytes(canonical, address, window_size)
            for address in addresses
        }
        for address in sorted(addresses):
            body = bodies[address]
            # Bodies identical across the whole window are indistinguishable;
            # the signature check is then equivalent for either body.
            remaining = [
                bodies[other] for other in addresses
                if other != address and bodies[other] != body
            ]
            if not remaining:
                identical.append(address)
                continue
            anchors = [
                instruction.address
                for instruction in disassembler.disasm(body, 0)
                if instruction.address + EXTENSION_SIZE <= window_size
            ]
            # Greedily emit windows until every distinguishable collider is
            # excluded; an address may need more than one window when its
            # differences from separate colliders are far apart.
            for anchor in anchors:
                if not remaining:
                    break
                window = bytes(body[anchor:anchor + EXTENSION_SIZE])
                distinguished = [
                    other for other in remaining
                    if bytes(other[anchor:anchor + EXTENSION_SIZE]) != window
                ]
                if distinguished:
                    extensions.append((address, anchor, window))
                    remaining = [
                        other for other in remaining
                        if other not in distinguished
                    ]
            if remaining:
                raise RuntimeError(
                    f"no instruction-aligned windows distinguish "
                    f"0x{address:08X} from its collision group")
    if identical:
        print(
            "generate-redirect-signatures: byte-identical collision bodies "
            "left without extensions: "
            + ", ".join(f"0x{address:08X}" for address in sorted(identical)))
    return extensions


def format_bytes(data):
    return ", ".join(f"0x{byte:02X}" for byte in data)


def generate(redirects, canonical, runtime, canonical_sha):
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    lines = [
        "// Generated by tools/generate_redirect_signatures.py from the",
        f"// canonical executable (sha256 {canonical_sha}),",
        "// cross-checked byte-for-byte against the hash-pinned PRACX runtime",
        "// executable. Regenerate and compare with the",
        "// verify-redirect-signatures target; do not edit by hand.",
        "#pragma once",
        "",
    ]
    signatures = {}
    for address, _kind in redirects:
        canonical_bytes = read_virtual_bytes(canonical, address, SIGNATURE_SIZE)
        runtime_bytes = read_virtual_bytes(runtime, address, SIGNATURE_SIZE)
        if canonical_bytes != runtime_bytes:
            raise RuntimeError(
                f"0x{address:08X} diverges between the canonical and PRACX "
                "executables; a signature-checked redirect there would fail "
                "at run time")
        signatures[address] = canonical_bytes
        lines.append(f"#define OPENSMACX_SIGNATURE_{address:08X} \\")
        lines.append(f"    {{{format_bytes(canonical_bytes[:8])}, \\")
        lines.append(f"     {format_bytes(canonical_bytes[8:])}}}")
        lines.append("")

    collisions = {}
    jump_addresses = [a for a, kind in redirects if kind == "jump"]
    by_signature = {}
    for address in jump_addresses:
        by_signature.setdefault(signatures[address], []).append(address)
    collisions = {
        signature: addresses
        for signature, addresses in by_signature.items()
        if len(addresses) > 1
    }
    extensions = derive_extensions(disassembler, canonical, collisions)
    for address, offset, window in extensions:
        runtime_window = read_virtual_bytes(
            runtime, address + offset, EXTENSION_SIZE)
        if runtime_window != window:
            raise RuntimeError(
                f"extension window at 0x{address:08X}+{offset} diverges "
                "between the canonical and PRACX executables")
    if extensions:
        entry_lines = [
            f"    {{0x{address:08X}, {offset}, {{{format_bytes(window)}}}}}"
            for address, offset, window in sorted(extensions)
        ]
        lines.append("#define OPENSMACX_SIGNATURE_EXTENSIONS { \\")
        lines.append(", \\\n".join(entry_lines) + "}")
        lines.append("")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(
        description="Generate DllMain redirect signatures from the canonical binary")
    parser.add_argument("--redirects", type=Path, default=DEFAULT_REDIRECTS)
    parser.add_argument("--exe", type=Path, default=DEFAULT_CANONICAL_EXE)
    parser.add_argument("--runtime-exe", type=Path, default=DEFAULT_RUNTIME_EXE)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    for path in (args.redirects, args.exe, args.runtime_exe):
        if not path.is_file():
            raise SystemExit(f"required input not found: {path}")
    redirects = load_redirects(args.redirects)
    canonical = pefile.PE(str(args.exe), fast_load=True)
    runtime = pefile.PE(str(args.runtime_exe), fast_load=True)
    canonical_sha = hashlib.sha256(args.exe.read_bytes()).hexdigest()
    header = generate(redirects, canonical, runtime, canonical_sha)
    args.output.write_text(header, encoding="ascii")
    print(
        f"generate-redirect-signatures: wrote {len(redirects)} signatures "
        f"to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

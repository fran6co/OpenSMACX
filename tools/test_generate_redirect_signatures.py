#!/usr/bin/env python3

from pathlib import Path
import struct
import tempfile
import unittest
from unittest import mock

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
import pefile

import generate_redirect_signatures


IMAGE_BASE = 0x00400000
SECTION_RVA = 0x1000


def build_test_pe(path, text_bytes):
    """Assemble a minimal PE32 with one .text section holding text_bytes."""
    aligned = text_bytes + b"\x00" * ((0x200 - len(text_bytes) % 0x200) % 0x200)
    dos = b"MZ" + b"\x00" * 58 + struct.pack("<I", 0x80)
    dos += b"\x00" * (0x80 - len(dos))
    coff = struct.pack("<IHHIIIHH", 0x4550, 0x14C, 1, 0, 0, 0, 0xE0, 0x102)
    optional = struct.pack(
        "<HBBIIIIII", 0x10B, 1, 0, len(aligned), 0, 0, SECTION_RVA, SECTION_RVA, 0)
    optional += struct.pack("<IIIHHHHHHIIIIHHIIIIII",
                            IMAGE_BASE, 0x1000, 0x200, 4, 0, 0, 0, 4, 0, 0,
                            SECTION_RVA + len(aligned), 0x200, 0, 2, 0,
                            0x100000, 0x1000, 0x100000, 0x1000, 0, 0)
    optional += b"\x00" * (0xE0 - len(optional))
    section = (b".text\x00\x00\x00"
               + struct.pack("<IIIIIIHHI", len(aligned), SECTION_RVA,
                             len(aligned), 0x200, 0, 0, 0, 0, 0x60000020))
    header = dos + coff + optional + section
    header += b"\x00" * (0x200 - len(header))
    path.write_bytes(header + aligned)


def function_bytes(displacement, tail=b""):
    """Sprite-setter shaped body: 14-byte prologue + push edi + mov [ecx+disp]."""
    prologue = bytes.fromhex("8b4424048b54240853568b742414")
    push_edi = b"\x57"
    mov = b"\x89\x81" + struct.pack("<I", displacement)
    ret = b"\xc3"
    body = prologue + push_edi + mov + tail + ret
    return body + b"\x90" * (0x50 - len(body))


class GenerateRedirectSignaturesTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.disassembler = Cs(CS_ARCH_X86, CS_MODE_32)

    def build_pair(self, text_bytes, runtime_bytes=None):
        canonical_path = self.root / "canonical.exe"
        runtime_path = self.root / "runtime.exe"
        build_test_pe(canonical_path, text_bytes)
        build_test_pe(runtime_path, runtime_bytes or text_bytes)
        return (pefile.PE(str(canonical_path), fast_load=True),
                pefile.PE(str(runtime_path), fast_load=True))

    def test_reads_virtual_bytes_through_section_mapping(self):
        text = function_bytes(0xA7C)
        canonical, _ = self.build_pair(text)
        data = generate_redirect_signatures.read_virtual_bytes(
            canonical, IMAGE_BASE + SECTION_RVA, 16)
        self.assertEqual(data, text[:16])

    def test_rejects_unmapped_address(self):
        canonical, _ = self.build_pair(function_bytes(0xA7C))
        with self.assertRaises(RuntimeError):
            generate_redirect_signatures.read_virtual_bytes(
                canonical, IMAGE_BASE + 0x800000, 16)

    def test_detects_runtime_divergence(self):
        text = function_bytes(0xA7C)
        diverged = bytearray(text)
        diverged[4] ^= 0xFF
        canonical, runtime = self.build_pair(text, bytes(diverged))
        redirects = [(IMAGE_BASE + SECTION_RVA, "jump")]
        with self.assertRaisesRegex(RuntimeError, "diverges"):
            generate_redirect_signatures.generate(
                redirects, canonical, runtime, "test-sha")

    def test_collision_extensions_reproduce_sprite_setter_shape(self):
        # Two sprite-setter-shaped functions sharing a 16-byte prologue and
        # differing only in the mov displacement, like 0x00605BE0/0x00605C30.
        text = function_bytes(0xA7C) + function_bytes(0xA94)
        canonical, runtime = self.build_pair(text)
        first = IMAGE_BASE + SECTION_RVA
        second = first + 0x50
        header = generate_redirect_signatures.generate(
            [(first, "jump"), (second, "jump")], canonical, runtime, "sha")
        self.assertIn("OPENSMACX_SIGNATURE_EXTENSIONS", header)
        self.assertIn(f"0x{first:08X}, 14", header)
        self.assertIn(f"0x{second:08X}, 14", header)
        self.assertIn("0x57, 0x89, 0x81, 0x7C, 0x0A, 0x00", header)
        self.assertIn("0x57, 0x89, 0x81, 0x94, 0x0A, 0x00", header)

    def test_identical_bodies_and_context_get_no_extension(self):
        # Identical bodies with identical trailing context stay
        # indistinguishable across the whole search window; identical bodies
        # with different neighbors are distinguished by context instead.
        unit = function_bytes(0xA7C) + b"\xCC" * 0x50
        text = unit + unit
        canonical, runtime = self.build_pair(text)
        first = IMAGE_BASE + SECTION_RVA
        second = first + len(unit)
        header = generate_redirect_signatures.generate(
            [(first, "jump"), (second, "jump")], canonical, runtime, "sha")
        self.assertNotIn("OPENSMACX_SIGNATURE_EXTENSIONS", header)

    def test_distant_differences_emit_multiple_windows(self):
        # Three functions: identical up to byte 20 versus one, and identical
        # up to byte 40 versus another, forcing two windows for the first.
        base = bytearray(function_bytes(0xA7C, tail=b"\x90" * 0x20))
        near = bytearray(base)
        near[21] ^= 0x0F
        far = bytearray(base)
        far[45] ^= 0x0F
        text = bytes(base) + bytes(near) + bytes(far)
        canonical, runtime = self.build_pair(text)
        first = IMAGE_BASE + SECTION_RVA
        addresses = [(first, "jump"), (first + 0x50, "jump"),
                     (first + 0xA0, "jump")]
        header = generate_redirect_signatures.generate(
            addresses, canonical, runtime, "sha")
        first_entries = [line for line in header.splitlines()
                         if f"0x{first:08X}," in line and "SIGNATURE_EXT" not in line
                         and line.strip().startswith("{")]
        self.assertGreaterEqual(len(first_entries), 2)

    def test_load_redirects_validates_schema(self):
        csv_path = self.root / "redirects.csv"
        csv_path.write_text("address,kind\n0x00401560,jump\n", encoding="utf-8")
        rows = generate_redirect_signatures.load_redirects(csv_path)
        self.assertEqual(rows, [(0x00401560, "jump")])
        csv_path.write_text(
            "address,kind\n0x00401560,jump\n0x00401560,call\n", encoding="utf-8")
        with self.assertRaisesRegex(RuntimeError, "duplicate"):
            generate_redirect_signatures.load_redirects(csv_path)
        csv_path.write_text("address,kind\n0x00401560,mystery\n", encoding="utf-8")
        with self.assertRaisesRegex(RuntimeError, "unknown redirect kind"):
            generate_redirect_signatures.load_redirects(csv_path)


if __name__ == "__main__":
    unittest.main()

"""Bounded source and pinned controls for DesignWin's signed stack index."""
from __future__ import annotations

import hashlib
import importlib.util
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
OWNER = ROOT / "src/designwin.cpp"
HEADER = ROOT / "src/designwin.h"
HARNESS = ROOT / "recovery/tests/designwin_button_harness.cpp"
BUTTON = (0x00438460, "void DesignWin::on_button_clicked(")
KEY = (0x00439A20, "int DesignWin::on_key_click(")
CENTER = (0x0043D490, "void DesignWin::center_stack(")
INIT = (0x0043C270, "void DesignWin::init(")
EXEC = (0x0043E9A0, "int DesignWin::exec(")
DRAW = (0x0043D510, "void DesignWin::draw_stack(")


_spec = importlib.util.spec_from_file_location(
    "designwin_signed_index_definition_helper",
    ROOT / "recovery/tests/test_game_logic_seed_batch_behavior.py")
_helper = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_helper)


def _body(address: int, signature: str) -> str:
    row, = (record for record in read_file(OWNER) if record.address == address)
    assert row.image_spans
    source = OWNER.read_text()
    body = _helper._definition(source, signature)
    assert "BODY GOES HERE." not in body
    return body


def _actual_bodies() -> tuple[str, str, str]:
    header = HEADER.read_text()
    declaration, = re.findall(
        r"^\s*int\s+prototype_stack_start_;\s*//\s*0xCE4\s*$", header, re.M)
    assert declaration
    lines = header.splitlines()
    member_line = next(index for index, line in enumerate(lines)
                       if "int prototype_stack_start_;" in line)
    assert "uint32_t field_CE0_;" in lines[member_line - 1]
    assert "uint32_t field_CE8_;" in lines[member_line + 1]
    assert "field_CE4_" not in header
    records = {record.address: record for record in read_file(OWNER)}
    assert records[BUTTON[0]].mode == Mode.BODY
    assert records[KEY[0]].mode == Mode.FILE
    assert records[CENTER[0]].mode == Mode.FILE
    expected_spans = {
        BUTTON[0]: ((0x00438460, 0x004395E1), (0x00654010, 0x006541FC)),
        KEY[0]: ((0x00439A20, 0x00439B3B),),
        CENTER[0]: ((0x0043D490, 0x0043D50B),),
        INIT[0]: ((0x0043C270, 0x0043D0B7), (0x006546C5, 0x006546DA)),
        EXEC[0]: ((0x0043E9A0, 0x0043ECAD), (0x00654709, 0x0065471E)),
        DRAW[0]: ((0x0043D510, 0x0043DD43), (0x006546DA, 0x006546EC)),
    }
    for address, spans in expected_spans.items():
        assert tuple(tuple(span) for span in records[address].image_spans) == spans
    owner_bodies = tuple(
        _body(*subject) for subject in (BUTTON, KEY, CENTER, INIT, EXEC, DRAW))
    assert all("prototype_stack_start_" in body for body in owner_bodies)
    assert all("field_CE4_" not in body for body in owner_bodies)
    assert all("static_cast<int>(prototype_stack_start_)" not in body
               for body in owner_bodies)
    bodies = owner_bodies[:3]
    assert "if (ProtoSortCount > 0)" not in bodies[1]
    return bodies


def _source(harness: str, bodies: tuple[str, str, str]) -> str:
    markers = (
        "/* EXTRACTED_DESIGNWIN_BUTTON */",
        "/* EXTRACTED_DESIGNWIN_KEY_CLICK */",
        "/* EXTRACTED_DESIGNWIN_CENTER_STACK */",
    )
    for marker, body in zip(markers, bodies):
        assert harness.count(marker) == 1
        harness = harness.replace(marker, body)
    return harness


def _compile_run(
    tmp_path: Path, compiler: str, source: str, scenario: str
) -> subprocess.CompletedProcess[str]:
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "actual_designwin_signed_index.cpp"
    exe = tmp_path / "actual_designwin_signed_index"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
        text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(exe), scenario], text=True, capture_output=True,
        timeout=10, env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})


def test_complete_actual_owner_paths_preserve_signed_index_behavior_and_reject_unsigned(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither GCC nor Clang is installed for host controls"
    bodies = _actual_bodies()
    fixture = _source(HARNESS.read_text(), bodies)
    for compiler in compilers:
        unsigned = fixture.replace(
            "int prototype_stack_start_;", "uint32_t prototype_stack_start_;")
        assert unsigned != fixture
        for scenario in ("signed-index", "signed-key-page-up"):
            passed = _compile_run(
                tmp_path / (Path(compiler).name + "-" + scenario),
                compiler, fixture, scenario)
            assert passed.returncode == 0, passed.stdout + passed.stderr
            damaged = _compile_run(
                tmp_path / (Path(compiler).name + "-unsigned-" + scenario),
                compiler, unsigned, scenario)
            assert damaged.returncode != 0, (
                "unsigned member mutant passed " + scenario + " controls")


def test_pinned_click_and_key_spans_retain_signed_branch_and_remainder_rules():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    assert image_pin(ROOT) == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    click = list(decoder.disasm(image.read(Span(0x004384BF, 0x004384D9)), 0x004384BF))
    assert [(item.address, item.mnemonic, item.op_str) for item in click] == [
        (0x004384BF, "mov", "eax, dword ptr [esi + 0xce4]"),
        (0x004384C5, "dec", "eax"),
        (0x004384C6, "mov", "dword ptr [esi + 0xce4], eax"),
        (0x004384CC, "jns", "0x4384d9"),
        (0x004384CE, "mov", "eax, dword ptr [0x945b40]"),
        (0x004384D3, "mov", "dword ptr [esi + 0xce4], eax"),
    ]

    span = Span(0x00439A20, 0x00439B3B)
    key = list(decoder.disasm(image.read(span), span.start))
    assert sum(item.size for item in key) == span.end - span.start
    at = {item.address: item for item in key}
    assert (at[0x00439A8A].mnemonic, at[0x00439A8A].op_str) == ("cdq", "")
    assert (at[0x00439A8B].mnemonic, at[0x00439A8B].op_str) == ("idiv", "ecx")
    assert (at[0x00439A91].mnemonic, at[0x00439A91].op_str) == ("test", "edx, edx")
    assert (at[0x00439A93].mnemonic, at[0x00439A93].op_str) == ("setl", "al")
    assert (at[0x00439A99].mnemonic, at[0x00439A99].op_str) == (
        "mov", "dword ptr [esi + 0xce4], eax")
    assert (at[0x00439AC1].mnemonic, at[0x00439AC1].op_str) == (
        "lea", "eax, [ecx - 0xa]")
    assert (at[0x00439AC8].mnemonic, at[0x00439AC8].op_str) == ("setl", "dl")

    signed_divides = (
        (0x0043D4BF, 0x0043D4C2, 0x0043D4C0, "ecx"),
        (0x0043D4E9, 0x0043D4EC, 0x0043D4EA, "ecx"),
        (0x0043D699, 0x0043D69E, 0x0043D69C, "ecx"),
        (0x0043EAC0, 0x0043EAC3, 0x0043EAC1, "edi"),
        (0x0043EADA, 0x0043EADD, 0x0043EADB, "edi"),
    )
    for start, end, idiv_address, divisor in signed_divides:
        sequence = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sequence[-1].address == idiv_address
        assert sequence[-1].mnemonic == "idiv"
        assert sequence[-1].op_str == divisor

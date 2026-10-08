"""Actual-owner host controls for DiploWin::on_button_clicked.

Tests require the tracked, mapped 0x4415C0 BODY record; no draft fallback.
Dependency doubles cover bounded behavior, not ABI or full-game correctness.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
ARTIFACTS = Path(__file__).resolve().parent
ADDRESS = 0x004415C0
DECORATED = "?on_button_clicked@DiploWin@@QAEXH@Z"
SIGNATURE = r"void\s+(?:__thiscall\s+)?DiploWin::on_button_clicked\s*\(\s*int\s+button_id\s*\)"
METHOD_MARKER = "// @@DIPLO_BUTTON_METHOD@@"
MUTATIONS = [
    ("dispatch_offset", r"switch\s*\(\s*button_id\s*\)", "switch (button_id + 1)"),
    ("header_literal", r'"DIPLOTECH"', '"DIPLOTECH_MUTATED"'),
    ("symmetric_receiver",
     r"(?<=&& \()PlayersData\[faction_id_with_\]\.diplo_treaties\[faction\](?= & 8\) == 0\))",
     "PlayersData[faction].diplo_treaties[faction_id_with_]"),
    ("automatic_faction_receiver",
     r"PlayersData\[other\]\.diplo_treaties\[faction\]",
     "PlayersData[faction].diplo_treaties[other]"),
    ("offer_value", r"add_offer\(0,\s*0,\s*tech\)", "add_offer(0, 0, 0)"),
    ("return_state",
     r'if\s*\(X_pop\("REALLYENDPACT",\s*pop_wait\)\s*==\s*0\)\s*return;',
     'if (X_pop("REALLYENDPACT", pop_wait) != 0) return;'),
    ("treaty_fallthrough",
     r"case 1:\s*add_offer\(1,\s*3,\s*0\);\s*// fall through",
     "case 1: add_offer(1, 3, 0); break;"),
    ("known_tech_marker",
     r"if\s*\(!counterpart_has_tech\)\s*strcat\(StringTemp,\s*\"\*\"\);",
     'if (counterpart_has_tech) strcat(StringTemp, "*");'),
    ("vendetta_visibility_or",
     r"if\s*\(\(GameDataStateStorage\.fields_\.lock_masks_\.enable_mask_\s*&\s*\(1u\s*<<\s*faction\)\)\s*==\s*0\s*&&\s*\(PlayersData\[faction_id_\]\.diplo_treaties\[faction\]\s*&\s*8\)\s*==\s*0\s*&&\s*\(PlayersData\[faction_id_with_\]\.diplo_treaties\[faction\]\s*&\s*8\)\s*==\s*0\)",
     "if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << faction)) == 0 || ((PlayersData[faction_id_].diplo_treaties[faction] & 8) == 0 && (PlayersData[faction_id_with_].diplo_treaties[faction] & 8) == 0))"),
]


def actual_method() -> str:
    records = [row for row in read_file(ROOT / "src/diplowin.cpp") if row.address == ADDRESS]
    assert len(records) == 1, f"expected one mapped source record at {ADDRESS:#x}"
    row = records[0]
    assert row.name == DECORATED
    assert row.mode == Mode.BODY, "0x4415C0 must be a source-owned BODY, not a seed or placeholder"
    assert row.region, f"source extraction failed: {row.extract_error}"
    return _definition(row.region, "void DiploWin::on_button_clicked(")


def _compose(method: str) -> str:
    fixture = (ARTIFACTS / "diplo_button_owner_harness.cpp").read_text()
    assert fixture.count(METHOD_MARKER) == 1
    return fixture.replace(METHOD_MARKER, method, 1)


def _build(tmp_path: Path, method: str, label: str) -> Path:
    compiler = os.environ.get("CXX") or shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("GCC or Clang host compiler unavailable")
    cpp = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    cpp.write_text(_compose(method))
    result = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    return binary


def _run(binary: Path, should_pass: bool) -> None:
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    if should_pass:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "host control did not detect the source mutation"


def test_mapped_diplo_button_method_host_controls(tmp_path):
    """Require actual source and exercise branch, lifetime and field controls."""
    _run(_build(tmp_path, actual_method(), "diplo_button_actual"), True)


def _assert_mutations_detected(tmp_path, method):
    for name, pattern, replacement in MUTATIONS:
        damaged, count = re.subn(pattern, replacement, method, count=1)
        assert count == 1, f"mutation anchor changed for {name}"
        _run(_build(tmp_path, damaged, "mutant_" + name), False)


def test_mutations_are_caught(tmp_path):
    _assert_mutations_detected(tmp_path, actual_method())


def test_native_diplomacy_button_dispatch_layout_and_popup_lifetime():
    import struct
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.source_inventory import image_pin
    from match_recovery.model import Span
    from match_recovery.codeview import class_layout
    from test_application_link_recovery import references
    from recovery.environment.wine import VC6_ROOT

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("pinned image, VC6 and Wine32 required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    assert struct.unpack("<I", image.read(Span(0x66B634 + 44*4, 0x66B634 + 45*4))) == (ADDRESS,)
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    symbol = "?on_button_clicked@DiploWin@@UAEXH@Z"
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / "src/diplowin.cpp", (), flags + settings.layout_flags))
        layout = class_layout(obj, "DiploWin")
        for name, offset in (("field_A24_",0xA24), ("field_A28_",0xA28),
                             ("offer_type_",0xA2C), ("offer_value_",0xA6C),
                             ("faction_id_",0xAB4), ("faction_id_with_",0xAB8)):
            assert layout["members"][name] == offset
        assert [(b["class"], b["offset"]) for b in layout["bases"]] == [("GraphicWin",0), ("SubInterface",0xA14)]
        table, = (s for s in obj.symbols.values() if s.name == "??_7DiploWin@@6BWin@@@" and s.section > 0)
        section = obj.sections[table.section - 1]
        relocated = {offset: obj.symbols[target].name for offset, target, kind in section.relocations if kind == 6}
        assert relocated[table.value + 44*4] == symbol
        calls = references(obj, symbol)
        assert calls.count("??0Popup@@QAE@XZ") == 1
        assert "??1Popup@@UAE@XZ" in calls

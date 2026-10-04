"""Complete source-owned global view callbacks and their native operand controls."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.source import read_file
from match_recovery.pe_coff import PeImage
from match_recovery.model import Span
from match_recovery.source_inventory import image_pin
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
UNIT = ROOT / "src/worldwin.cpp"
FIXTURE = Path(__file__).with_name("world_map_view_callbacks_harness.cpp")
CALLBACKS = ((0x4C38E0, "show_detail_map_view", "// ACTUAL_DETAIL_CALLBACK"),
             (0x4C3910, "show_world_map_view", "// ACTUAL_WORLD_CALLBACK"))


def actual_source():
    projected = FIXTURE.read_text()
    for address, name, marker in CALLBACKS:
        record, = (r for r in read_file(UNIT) if r.address == address)
        assert record.byte_exact and record.name == f"sub_{address:x}"
        assert record.symbol == f"?{name}@@YAXXZ"
        body = _definition(record.region, f"void __cdecl {name}()")
        assert projected.count(marker) == 1
        projected = projected.replace(marker, body)
    return projected


def run_control(tmp_path, compiler, source):
    cpp = tmp_path / "view.cpp"
    binary = tmp_path / "view"
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, capture_output=True)
    return subprocess.run([str(binary)], capture_output=True)


@pytest.mark.parametrize("name", ("g++", "clang++"))
def test_actual_global_view_callbacks(tmp_path, name):
    compiler = shutil.which(name)
    if not compiler:
        pytest.skip(name + " unavailable")
    assert run_control(tmp_path, compiler, actual_source()).returncode == 0


@pytest.mark.parametrize("before,after", [
    ("MainInterfaceGlobal.button_group(1).set(1007, 0);", "MainInterfaceGlobal.button_group(1).set(1006, 0);"),
    ("MapWinTable[1]->show(0);", "MapWinTable[0]->show(0);"),
    ("WorldClimateWorldWin.Win::hide();", "WorldClimateWorldWin.hide();"),
    ("WorldClimateWorldWin.Win::show(0);", "WorldClimateWorldWin.show(0);"),
    ("MapWinTable[1]->hide();", "MapWinTable[1]->show(0);"),
])
def test_callback_damage_is_detected(tmp_path, before, after):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("g++ unavailable")
    source = actual_source()
    assert source.count(before) == 1
    assert run_control(tmp_path, compiler, source.replace(before, after, 1)).returncode != 0


def test_original_operands_and_dispatch_are_distinct_from_selection_setters():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for start, end, selected_id, slot, tail in (
        (0x4C38E0, 0x4C3910, "0x3ef", "4", 0x5EDCD0),
        (0x4C3910, 0x4C3941, "0x3ee", "8", 0x5ED9D0),
    ):
        ops = [(i.mnemonic, i.op_str) for i in decoder.disasm(image.read(Span(start, end)), start)]
        assert ops[:4] == [("push", "0"), ("push", selected_id),
                           ("mov", "ecx, 0x7cd12c"), ("call", "0x62b870")]
        assert ("mov", "eax, dword ptr [0x7d3c40]") in ops
        assert ("mov", "edx, dword ptr [ecx + 4]") in ops  # real virtual base
        assert ("call", f"dword ptr [eax + {slot}]") in ops
        assert ("mov", "ecx, 0x8e9f60") in ops
        assert ("jmp" if slot == "4" else "call", hex(tail)) in ops
        assert not any(op == "mov" and "+ 0x239cc]" in arg for op, arg in ops)


def test_compiled_callbacks_bind_the_actual_objects_and_direct_win_operations():
    """Strict body-reference check; not linked selection or runtime acceptance."""
    from match_recovery.bound_objects import FunctionBinding, check_objects
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.model import Origin
    from match_recovery.pe_coff import Coff
    from recovery.environment.wine import VC6_ROOT

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("pinned original image, VC6 and Wine32 required")
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    records = [r for r in read_file(UNIT) if r.address in (0x4C38E0, 0x4C3910)]
    assert len(records) == 2
    bindings = [FunctionBinding(Origin(pin, "pe:va", r.address),
                               tuple(Span(a, b) for a, b in r.image_spans),
                               r.symbol, "src/worldwin.cpp") for r in records]
    # Identities are source-bound independently of the operands under test.
    targets = {
        "?MainInterfaceGlobal@@3VMainInterface@@A": 0x7AE820,
        "?MapWinTable@@3PAPAVMapWin@@A": 0x7D3C3C,
        "?WorldClimateWorldWin@@3VWorldWin@@A": 0x8E9F60,
        "?set@ButtonGroup@@QAEHHH@Z": 0x62B870,
        "?hide@Win@@UAEXXZ": 0x5EDCD0,
        "?show@Win@@UAEXH@Z": 0x5ED9D0,
    }
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    matched = set()
    for flags in settings.flag_sets:
        obj = Coff(compiler(UNIT, (), flags + settings.layout_flags))
        rows = check_objects(bindings, {"src/worldwin.cpp": obj}, image,
                             external_targets=targets)
        for row in rows:
            if row["boundary"] == row["bytes"] == "pass":
                assert row["references"] == "pass", row
                matched.add(row["symbol"])
    assert matched == {b.symbol for b in bindings}

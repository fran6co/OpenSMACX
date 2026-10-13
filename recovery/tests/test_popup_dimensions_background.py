"""Actual-source behavioral and virtual-dispatch controls for Popup dimensions."""
from pathlib import Path
import re
import shutil
import subprocess
import sys
import os
import struct

import pytest

from match_recovery.source import Mode, read_file


HERE = Path(__file__).resolve()
ROOT = HERE.parents[2]
TESTS = str(ROOT / "recovery/tests")
if TESTS not in sys.path:
    sys.path.insert(0, TESTS)
from test_game_logic_seed_batch_behavior import _definition
SOURCE = ROOT / "src/popup.cpp"
FIXTURE = HERE.with_name("popup_dimensions_background_harness.cpp")
ADDRESS = 0x00404B10
ORIGINAL_NAME = "?calculate_dimensions@Popup@@QAEHXZ"
EMITTED_NAME = "?calculate_dimensions@Popup@@UAEHXZ"
SIGNATURE = "int Popup::calculate_dimensions()"


def actual_body():
    records = [row for row in read_file(SOURCE) if row.address == ADDRESS]
    assert len(records) == 1, "missing or duplicate Popup dimensions source record"
    record = records[0]
    assert record.mode == Mode.BODY
    assert record.name == ORIGINAL_NAME
    assert record.region and "BODY GOES HERE." not in record.region
    assert "// symbol    " + EMITTED_NAME in record.region
    return _definition(record.region, SIGNATURE)


def projected_control(body):
    fixture = FIXTURE.read_text()
    assert fixture.count("/*__POPUP_MEMBER__*/") == 1
    return fixture.replace("/*__POPUP_MEMBER__*/", body)


def compile_control(tmp_path, compiler, source):
    cpp = tmp_path / "popup_dimensions.cpp"
    binary = tmp_path / "popup_dimensions"
    cpp.write_text(source)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(binary)],
        check=True, capture_output=True,
    )
    return subprocess.run([str(binary)], capture_output=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_popup_dimensions_body_cpp98(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = compile_control(tmp_path, compiler, projected_control(actual_body()))
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_popup_dimensions_uses_existing_base_virtual_slot():
    base_header = (ROOT / "src/basepop.h").read_text()
    popup_header = (ROOT / "src/popup.h").read_text()
    declaration = r"(?m)^\s*virtual\s+int\s+calculate_dimensions\s*\(\s*\)\s*;"
    assert re.search(declaration, base_header), "BasePop slot 90 must be declared virtual"
    assert re.search(declaration, popup_header), "Popup must declare its existing-slot override"
    record, = [row for row in read_file(SOURCE) if row.address == ADDRESS]
    assert record.name == ORIGINAL_NAME
    assert "// symbol    " + EMITTED_NAME in record.region


def test_popup_parent_fallback_selection_uses_actual_exec_expression(tmp_path):
    """Exercise the exact parent-selection declaration extracted from exec."""
    source = (ROOT / "src/basepop.cpp").read_text(encoding="utf-8")
    exec_body = _definition(source, "int BasePop::exec(int flags, int (__cdecl *callback)())")
    statements = re.findall(
        r"(?m)^\s*Win \*const parent = external_window_ != 0\s*"
        r"\? external_window_\s*:\s*fallback_parent_window_;\s*$",
        exec_body,
    )
    assert len(statements) == 1, "exec must select attached parent or fallback member once"
    selection = statements[0].strip()
    fixture = r'''#include <cassert>
struct Win { int tag; };
struct ParentSelection {
    Win *external_window_;
    Win *fallback_parent_window_;
    Win *selected() const {
        PARENT_SELECTION
        return parent;
    }
};
int main() {
    Win fallback = {1}, attached = {2};
    ParentSelection p = {0, &fallback};
    assert(p.selected() == &fallback);
    p.external_window_ = &attached;
    assert(p.selected() == &attached);
    p.external_window_ = 0;
    p.fallback_parent_window_ = 0;
    assert(p.selected() == 0);
}
'''.replace("PARENT_SELECTION", selection)
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if compiler is None:
            continue
        compiler_dir = tmp_path / compiler_name.replace("+", "p")
        compiler_dir.mkdir()
        result = compile_control(compiler_dir, compiler, fixture)
        assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_native_and_compiled_basepop_popup_callback_order():
    """Physical original words and fresh compiler relocations, not mock slots."""
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.model import Span
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.source_inventory import image_pin
    from match_recovery.codeview import class_layout
    from recovery.environment.wine import VC6_ROOT

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("pinned image, VC6 and Wine32 required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    originals = {
        "BasePop": (0x6698D4, (0x603800, 0x404280, 0x404280, 0x404280,
                              0x406B20, 0x406840, 0x406B30)),
        "Popup": (0x6695C8, (0x404B10, 0x404280, 0x404EF0, 0x405020,
                            0x405050, 0x405910, 0x405C80)),
    }
    methods = ("calculate_dimensions", "on_pre_window_init", "on_post_window_init",
               "on_adjust_button_width", "on_adjust_button_placement", "on_exec",
               "on_draw_background")
    suffixes = ("UAEHXZ", "UAEXXZ", "UAEXXZ", "UAEXXZ", "UAEXPAH00@Z",
                "UAEHXZ", "UAEHXZ")
    for table, targets in originals.values():
        assert struct.unpack("<7I", image.read(Span(table + 90 * 4, table + 97 * 4))) == targets
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    for flags in settings.flag_sets:
        obj = Coff(compiler(SOURCE, (), flags + settings.layout_flags))
        assert class_layout(obj, "BasePop")["size"] == 0x3230
        assert class_layout(obj, "Popup")["size"] == 0x537C
        for owner in originals:
            table, = (s for s in obj.symbols.values()
                      if s.name == f"??_7{owner}@@6BWin@@@" and s.section > 0)
            section = obj.sections[table.section - 1]
            assert len(section.data) == 97 * 4
            relocated = {offset: obj.symbols[target].name
                         for offset, target, kind in section.relocations if kind == 6}
            expected = []
            for index, (method, suffix) in enumerate(zip(methods, suffixes)):
                method_owner = "BasePop" if owner == "BasePop" or index == 1 else "Popup"
                expected.append(f"?{method}@{method_owner}@@{suffix}")
            assert [relocated[4 * slot] for slot in range(90, 97)] == expected


@pytest.mark.parametrize("name,old,new", [
    ("800-wide cap", "if (width > 0x1ED) width = 0x1EE;",
     "if (width > 0x1ED) width = 0x1ED;"),
    ("Diplomacy item boundary", "dialogs_.get_num_items() > 5",
     "dialogs_.get_num_items() >= 5"),
    ("mode tag scale", "field_30AC_ += mode * 10000;",
     "field_30AC_ += mode * 1000;"),
    ("Diplomacy colors", "TextColourTable[182]", "TextColourTable[70]"),
    ("sprite scale numerator",
     "int width = sprite_width * static_cast<int>(field_3104_)",
     "int width = sprite_width * static_cast<int>(field_3108_)"),
    ("active mode selection", "active_slot != 0 && active_slot != 10",
     "active_slot != 0 && active_slot != 3"),
    ("turn redraw flag", "TurnRedrawPending != 0 || mode == 6",
     "TurnRedrawPending != 0 && mode == 6"),
    ("default redraw callback", "redraw_callback_ = sprite_draw_func",
     "redraw_callback_ = 0"),
    ("BasePop result propagation", "if (result != 0) return result;",
     "if (result != 0) return 0;"),
    ("minimum height", "field_30C0_ = 300;", "field_30C0_ = 299;"),
    ("signed minimum-height test", "current_height < 300", "field_30C0_ < 300"),
])
def test_popup_dimensions_damage_is_caught(tmp_path, name, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    body = actual_body()
    assert old in body, "mutation anchor missing for " + name
    damaged = body.replace(old, new, 1)
    result = compile_control(tmp_path, compiler, projected_control(damaged))
    assert result.returncode != 0, "damaged body escaped controls: " + name

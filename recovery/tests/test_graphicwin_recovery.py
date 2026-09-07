"""Live project regression controls; no executable-derived fixtures are stored."""

from pathlib import Path
import shutil

import pytest

from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.pe_coff import Coff
from match_recovery.source.reader import read_file


ROOT = Path(__file__).resolve().parents[2]


@pytest.fixture(scope="module")
def compiled():
    from recovery.environment.wine import VC6_ROOT
    if (not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None
            or not (ROOT / "build/compile_commands.json").is_file()):
        pytest.skip("project compiler and build database are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / "src/graphicwin.cpp", (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    return compiler, settings, objects


def symbol_at(path, address):
    record, = (record for record in read_file(ROOT / path) if record.address == address)
    return record.symbol or record.name


def calls(obj, symbol):
    _, relocations, _ = obj.subject(obj.function(symbol))
    return [obj.symbols[int(row.target)].name for row in relocations]


def test_window_initializer_reaches_its_base_and_uses_native_allocation(compiled):
    _, _, objects = compiled
    derived = symbol_at("src/graphicwin.cpp", 0x005D4EF0)
    base = symbol_at("src/win.cpp", 0x005EBD80)
    for obj in objects:
        targets = calls(obj, derived)
        assert base in targets
        assert derived not in targets  # The previous unqualified call recursed.
        assert "??2@YAPAXI@Z" in targets
        assert not any("WaveOperatorNew" in target for target in targets)


def test_owned_buffer_is_pointer_storage_not_an_embedded_buffer_or_integer(compiled):
    _, _, objects = compiled
    for obj in objects:
        layout = class_layout(obj, "GraphicWin")
        assert layout["size"] == 0xA14
        assert layout["members"]["owned_buffer_"] == 0xA08
        assert layout["member_type_kinds"]["owned_buffer_"] == "pointer"
        assert "owned_buffer_" not in layout["class_value_members"]


def test_unqualified_initializer_damage_really_changes_the_compiled_call_target(compiled):
    compiler, settings, _ = compiled
    path = ROOT / "src/graphicwin.cpp"
    source = path.read_bytes()
    anchor = b"const int base_result = Win::init("
    assert source.count(anchor) == 1
    damaged = source.replace(anchor, b"const int base_result = init(")
    obj = Coff(compiler.overlay(path, {"src/graphicwin.cpp": damaged}, (), settings.flag_sets[0]))
    derived = symbol_at("src/graphicwin.cpp", 0x005D4EF0)
    targets = calls(obj, derived)
    assert derived in targets
    assert symbol_at("src/win.cpp", 0x005EBD80) not in targets
    assert path.read_bytes() == source

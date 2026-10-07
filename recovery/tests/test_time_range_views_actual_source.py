"""Post-publication controls that read the installed source, not the emitter."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]


def actual_view_source() -> tuple[str, Path]:
    path = ROOT / "src/time.cpp"
    text = path.read_text()
    begin_marker = "// These are behavior views over observed fields"
    end_marker = "// Pump redraws before non-input work"
    records = {item.address: item for item in read_file(path)
               if item.address in (0x004C8110, 0x004C8350)}
    assert set(records) == {0x004C8110, 0x004C8350}
    assert records[0x004C8110].mode is Mode.BODY
    assert records[0x004C8110].byte_exact
    assert records[0x004C8110].source_role.value == "scaffold"
    assert "constructor-to-instance lifetime flow remain unproved" in records[0x004C8110].source_role_reason
    assert records[0x004C8110].image_spans == ((0x004C8110, 0x004C813B),)
    assert records[0x004C8350].mode is Mode.FILE
    assert not records[0x004C8350].byte_exact
    assert records[0x004C8350].image_spans == ((0x004C8350, 0x004C8396),)
    assert begin_marker in text and end_marker in text
    begin = text.index(begin_marker)
    end = text.index(end_marker, begin)
    source = text[begin:end]
    assert "// ORIGINAL: 0x004C8110 sub_4c8110" in text
    assert "// ORIGINAL: 0x004C8350 sub_4c8350" in text
    assert "unknown_dword_0_" in source
    assert "zeroed_dword_4_" in source
    assert "unknown_dword_8_" in source
    assert "unknown_dword_C_" in source
    assert "0x004C836D" in source and "0x004C8384" in source
    assert "0x006465F0" in source
    assert "git:aee589a5dd0fca997773070668af4e1ff9026684:src/recovered/004c8110.cpp" in source
    assert "historical-only seed hint" in source
    assert "deleted seed-file spelling is not emitted as the current symbol" in source
    assert not (ROOT / "src/recovered/004c8110.cpp").exists()
    assert not (ROOT / "src/recovered/units/004c8350.cpp").exists()
    return source, path


def host_harness(source: str) -> str:
    return r'''#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
typedef uint32_t DWORD;
class TimeRangeReceiverView;
static TimeRangeReceiverView *clock_receiver;
static int mutate_upper;
static int new_upper;
static DWORD tick_value;
static int calls;
DWORD timeGetTime();
#define private public
''' + source + r'''
#undef private
DWORD timeGetTime() {
    ++calls;
    if (mutate_upper) clock_receiver->upper_bound_C_ = new_upper;
    return tick_value;
}
int main() {
    RangePrefixInitializationView initialized;
    assert(initialized.unknown_dword_0_ == 0);
    assert(initialized.zeroed_dword_4_ == 0);
    assert(initialized.unknown_dword_8_ == 0);
    assert(initialized.unknown_dword_C_ == 0);

    TimeRangeReceiverView state;
    clock_receiver = &state;
    state.enabled_flags_4_ = 0;
    state.lower_bound_8_ = 1;
    state.upper_bound_C_ = 3;
    assert(state.sample() == 0 && calls == 0);

    state.enabled_flags_4_ = 1;
    state.lower_bound_8_ = 2;
    state.upper_bound_C_ = 4;
    tick_value = 5;
    assert(state.sample() == 6 && calls == 1);

    state.lower_bound_8_ = 5;
    state.upper_bound_C_ = 3;
    tick_value = 4;
    assert(state.sample() == 4 && calls == 2);

    state.lower_bound_8_ = -2;
    state.upper_bound_C_ = 2;
    tick_value = 7;
    assert(state.sample() == 4 && calls == 3);

    state.lower_bound_8_ = 2;
    state.upper_bound_C_ = -2;
    tick_value = 7;
    assert(state.sample() == 0 && calls == 4);

    state.lower_bound_8_ = 1;
    state.upper_bound_C_ = 2;
    tick_value = 1;
    mutate_upper = 1;
    new_upper = 20;
    assert(state.sample() == 21 && calls == 5);
    mutate_upper = 0;

    state.lower_bound_8_ = INT_MAX - 1;
    state.upper_bound_C_ = INT_MAX;
    tick_value = 1;
    assert(state.sample() == INT_MIN && calls == 6);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_installed_time_owner_bodies_and_historical_mappings(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source, _ = actual_view_source()
    damaged_reload = source.replace(
        "range_view_wrapped_upper_add(upper_bound_C_, remainder)",
        "range_view_wrapped_upper_add(upper, remainder)")
    damaged_initialization = source.replace(
        "unknown_dword_C_ = 0;", "unknown_dword_C_ = 1;")
    assert damaged_reload != source and damaged_initialization != source
    for label, block in (("actual", source), ("cached-upper", damaged_reload),
                         ("wrong-initialization", damaged_initialization)):
        cpp = tmp_path / (label + ".cpp")
        exe = tmp_path / label
        cpp.write_text(host_harness(block))
        built = subprocess.run(
            [compiler, "-std=c++98", "-O2", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        if label == "actual":
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "damaged source escaped the behavior oracle"


def test_native_view_offsets_unsigned_division_and_clock_import():
    """Current VC6 views and instructions, not original allocation/layout proof."""
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.compiler import Compiler
    from match_recovery.codeview import class_layout
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("VC6 and Wine32 required")
    actual_view_source()
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / "src/time.cpp", (), flags + settings.layout_flags))
        prefix = class_layout(obj, "RangePrefixInitializationView")
        assert prefix["bases"] == []
        assert prefix["members"] == {"unknown_dword_0_": 0, "zeroed_dword_4_": 4,
                                     "unknown_dword_8_": 8, "unknown_dword_C_": 12}
        receiver = class_layout(obj, "TimeRangeReceiverView")
        assert receiver["bases"] == []
        assert receiver["members"] == {"unobserved_prefix_0_": 0, "enabled_flags_4_": 4,
                                       "unobserved_bytes_5_7_": 5,
                                       "lower_bound_8_": 8, "upper_bound_C_": 12}
        obj.function("??0RangePrefixInitializationView@@QAE@XZ")
        code, refs, _ = obj.subject(obj.function("?sample@TimeRangeReceiverView@@QAEHXZ"))
        assert any(op.mnemonic == "div" for op in Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0))
        targets = {obj.symbols[int(ref.target)].name for ref in refs}
        assert "__imp__timeGetTime@0" in targets

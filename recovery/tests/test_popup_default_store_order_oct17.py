"""Bounded actual-source and pinned-instruction controls for popup stores."""
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from capstone import CS_ARCH_X86, CS_MODE_32, CS_OP_IMM, CS_OP_MEM, CS_OP_REG, Cs
from capstone.x86_const import X86_REG_EAX, X86_REG_INVALID

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from recovery.tests.test_pact_ascent import definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/popup.cpp").is_file())

FIXTURE = r'''#include <assert.h>

#ifndef __cdecl
#define __cdecl
#endif

enum {
    PopupSmallFont = 11, PopupSmallBoldFont = 12, PopupSmallItalicFont = 13,
    InterludeNormalFont = 21, InterludeSecondFont = 22, InterludeThirdFont = 23,
    PopupMediumFont = 31, PopupMediumBoldFont = 32, PopupMediumItalicFont = 33,
    MessageWinTextColor = 44, PopupThemeSurfaceColor = 55
};

struct Event { int kind, a, b, c, d; };
static Event trace[32];
static int trace_size;
static void record(int kind, int a = 0, int b = 0, int c = 0, int d = 0) {
    assert(trace_size < 32);
    trace[trace_size].kind = kind;
    trace[trace_size].a = a;
    trace[trace_size].b = b;
    trace[trace_size].c = c;
    trace[trace_size].d = d;
    ++trace_size;
}

struct TrackedArray {
    int kind;
    struct Cell {
        TrackedArray *owner;
        int index;
        int value;
        Cell(TrackedArray *a, int i) : owner(a), index(i), value(0) {}
        Cell &operator=(int v) {
            value = v;
            record(owner->kind, index, v);
            return *this;
        }
        Cell &operator=(const Cell &other) {
            return operator=(other.value);
        }
    };
    explicit TrackedArray(int event_kind) : kind(event_kind) {}
    Cell operator[](int index) { return Cell(this, index); }
};

TrackedArray BasePopDefaultField21C0(4);
TrackedArray BasePopDefaultField3104(5);
TrackedArray DialogDefaults(9);
int PopupModalState;

struct BasePop {
    static void set_def_string_font(int a, int b, int c, int d) {
        record(1, a, b, c, d);
    }
    static void set_def_button_font(int a, int b, int c) {
        record(3, a, b, c);
    }
    static void set_def_string_color(int a, int b, int c, int d) {
        record(6, a, b, c, d);
    }
    static void set_def_string_color2(int a, int b, int c, int d) {
        record(7, a, b, c, d);
    }
    static void set_def_string_color3(int a, int b, int c, int d) {
        record(8, a, b, c, d);
    }
};
struct Dialog {
    static void set_def_dialog_font(int a, int b, int c) {
        record(2, a, b, c);
    }
};
struct BaseButton {
    static void set_def_font(int a, int b, int c) { record(10, a, b, c); }
    static void set_def_text_color(int a, int b, int c, int d) {
        record(11, a, b, c, d);
    }
};

// @@POPUPS_SMALL@@
// @@POPUPS_TUTORIAL@@
// @@POPUPS_MEDIUM@@
// @@POPUPS_INTERLUDE_MEDIUM@@

static void expect_event(int index, int kind, int a = 0, int b = 0,
                         int c = 0, int d = 0) {
    assert(index < trace_size);
    assert(trace[index].kind == kind);
    assert(trace[index].a == a && trace[index].b == b);
    assert(trace[index].c == c && trace[index].d == d);
}

static void expect_small_or_tutorial(int string_font, int string_bold,
                                     int string_italic, int button_font,
                                     int button_bold, int button_italic,
                                     int mode) {
    assert(PopupModalState == mode);
    assert(trace_size == 13);
    expect_event(0, 1, string_font, string_bold, string_italic, 0);
    expect_event(1, 2, string_font, string_bold, string_italic);
    expect_event(2, 3, button_font, button_bold, button_italic, 0);
    expect_event(3, 4, 0, 3);
    expect_event(4, 4, 1, 3);
    expect_event(5, 5, 2, 0);
    expect_event(6, 6, MessageWinTextColor, -1, 1, 1);
    expect_event(7, 7, MessageWinTextColor, -1, 1, 1);
    expect_event(8, 8, MessageWinTextColor, -1, 1, 1);
    expect_event(9, 9, 3, 1);
    expect_event(10, 10, PopupSmallFont, 0, 0);
    expect_event(11, 11, 0, -1, 1, 1);
    expect_event(12, 9, 0, PopupThemeSurfaceColor);
}

static void check_small() {
    trace_size = 0;
    popups_small();
    expect_small_or_tutorial(PopupSmallFont, PopupSmallBoldFont,
                             PopupSmallItalicFont, PopupSmallFont,
                             PopupSmallBoldFont, PopupSmallItalicFont, 1);
}
static void check_tutorial() {
    trace_size = 0;
    popups_tutorial();
    expect_small_or_tutorial(InterludeNormalFont, InterludeSecondFont,
                             InterludeThirdFont, PopupMediumFont,
                             PopupMediumBoldFont, PopupMediumItalicFont, 2);
}
static void check_medium() {
    trace_size = 0;
    popups_medium();
    assert(PopupModalState == 3 && trace_size == 5);
    expect_event(0, 1, PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont, 0);
    expect_event(1, 2, PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont);
    expect_event(2, 3, PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont, 0);
    expect_event(3, 4, 0, 2);
    expect_event(4, 4, 1, 2);
}
static void check_interlude_medium() {
    trace_size = 0;
    popups_interlude_medium();
    assert(PopupModalState == 4 && trace_size == 5);
    expect_event(0, 1, InterludeNormalFont, InterludeSecondFont, InterludeThirdFont, 0);
    expect_event(1, 2, InterludeNormalFont, InterludeSecondFont, InterludeThirdFont);
    expect_event(2, 3, PopupMediumFont, PopupMediumBoldFont, PopupMediumItalicFont, 0);
    expect_event(3, 4, 0, 2);
    expect_event(4, 4, 1, 2);
}

int main() {
    check_small();
    check_tutorial();
    check_medium();
    check_interlude_medium();
    return 0;
}
'''


FUNCTIONS = (
    ("small", r"void\s+__cdecl\s+popups_small\s*\("),
    ("tutorial", r"void\s+__cdecl\s+popups_tutorial\s*\("),
    ("medium", r"void\s+__cdecl\s+popups_medium\s*\("),
    ("interlude_medium", r"void\s+__cdecl\s+popups_interlude_medium\s*\("),
)


def _actual_bodies():
    source = (ROOT / "src/popup.cpp").read_text()
    return {name: definition(source, signature) for name, signature in FUNCTIONS}


def _program(bodies):
    result = FIXTURE
    for name in ("small", "tutorial", "medium", "interlude_medium"):
        token = "// @@POPUPS_" + name.upper() + "@@"
        assert result.count(token) == 1
        result = result.replace(token, bodies[name], 1)
    assert "@@POPUPS_" not in result
    return result


def _store_lines(value):
    return ("BasePopDefaultField21C0[0] = " + str(value) + ";\n"
            "    BasePopDefaultField21C0[1] = " + str(value) + ";")


def _mutations(bodies):
    result = []
    values = {"small": 3, "tutorial": 3, "medium": 2, "interlude_medium": 2}
    for name, value in values.items():
        before = _store_lines(value)
        after = ("BasePopDefaultField21C0[1] = " + str(value) + ";\n"
                 "    BasePopDefaultField21C0[0] = " + str(value) + ";")
        assert bodies[name].count(before) == 1
        swapped = dict(bodies)
        swapped[name] = bodies[name].replace(before, after, 1)
        result.append((name + "_swapped", swapped))

        chained = dict(bodies)
        chained[name] = bodies[name].replace(
            before,
            "BasePopDefaultField21C0[0] = BasePopDefaultField21C0[1] = "
            + str(value) + ";", 1)
        result.append((name + "_chained", chained))
    return result


def _run_candidate(compiler, folder, label, bodies, expect_success):
    source_path = folder / (label + ".cpp")
    binary_path = folder / label
    source_path.write_text(_program(bodies))
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=45,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    outcome = subprocess.run([str(binary_path)], capture_output=True, text=True, timeout=15)
    if expect_success:
        assert outcome.returncode == 0, outcome.stdout + outcome.stderr
    else:
        assert outcome.returncode != 0, "store-order mutation escaped: " + label


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_popup_bodies_preserve_order_and_all_other_effects(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    bodies = _actual_bodies()
    with tempfile.TemporaryDirectory(prefix="popup-default-store-order-") as temp:
        folder = Path(temp)
        _run_candidate(compiler, folder, "actual", bodies, True)
        for label, damaged in _mutations(bodies):
            _run_candidate(compiler, folder, "damage_" + label, damaged, False)


def test_pinned_original_writes_the_two_defaults_in_index_order():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for start, end, value in (
        (0x588800, 0x5888DE, 3),
        (0x5888E0, 0x5889BE, 3),
        (0x5889C0, 0x588A2A, 2),
        (0x588A30, 0x588A9A, 2),
    ):
        code = image.read(Span(start, end))
        instructions = list(decoder.disasm(code, start))
        assert sum(instruction.size for instruction in instructions) == end - start
        stores = []
        for index, instruction in enumerate(instructions):
            if instruction.mnemonic != "mov" or len(instruction.operands) != 2:
                continue
            destination, source = instruction.operands
            if destination.type != CS_OP_MEM or source.type != CS_OP_REG:
                continue
            memory = destination.mem
            if (memory.base == X86_REG_INVALID and memory.index == X86_REG_INVALID
                    and memory.disp in (0x696ED0, 0x696ED4)):
                assert source.reg == X86_REG_EAX
                stores.append((memory.disp, index))
        assert [address for address, _ in stores] == [0x696ED0, 0x696ED4], hex(start)
        for _, store_index in stores:
            eax_writer = None
            writer_index = None
            for prior_index in range(store_index - 1, -1, -1):
                prior = instructions[prior_index]
                _, written = prior.regs_access()
                if X86_REG_EAX in written:
                    eax_writer = prior
                    writer_index = prior_index
                    break
            assert eax_writer is not None
            assert writer_index is not None and store_index - writer_index <= 4
            assert all(not instruction.mnemonic.startswith(("call", "j", "ret", "loop"))
                       for instruction in instructions[writer_index + 1:store_index])
            assert eax_writer.mnemonic == "mov"
            assert len(eax_writer.operands) == 2
            assert eax_writer.operands[0].type == CS_OP_REG
            assert eax_writer.operands[0].reg == X86_REG_EAX
            assert eax_writer.operands[1].type == CS_OP_IMM
            assert eax_writer.operands[1].imm == value

"""Host controls for BasePop's post-redraw Dialogs attachment handoff.

The C++ fixture splices the live bounded BasePop::exec source slice from
on_redraw() through on_adjust_button_placement(). It checks ordinary host C++
behavior and ordering; the pinned disassembly separately establishes the
original call and subobject evidence. This is not a VC6 ABI or game proof.
"""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _function_body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line"
            index += 1
        elif char == "/" and nxt == "*":
            state = "block"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
        index += 1
    raise AssertionError("unterminated BasePop::exec(int, callback) body")


def live_slice():
    source = SOURCE.read_text(encoding="utf-8")
    body = _function_body(source, "int BasePop::exec(int flags,")
    start = body.index("    on_redraw();")
    end_call = "    on_adjust_button_placement(&origin, &row, &row_style);"
    end = body.index(end_call, start) + len(end_call)
    return body[start:end]


HARNESS = r'''#include <climits>
#include <cstdio>
#include <cstring>

static int failures;
static char events[8][16];
static int event_count;
static void check(bool ok, const char *what) {
    if (!ok) {
        ++failures;
        if (failures <= 10) std::fprintf(stderr, "FAIL: %s\n", what);
    }
}
static void event(const char *name) {
    if (event_count < 8) {
        std::strncpy(events[event_count], name, sizeof(events[event_count]) - 1);
        events[event_count][sizeof(events[event_count]) - 1] = 0;
    }
    ++event_count;
}
static bool same_event(int index, const char *name) {
    return index >= 0 && index < event_count && std::strcmp(events[index], name) == 0;
}

struct Win {};
struct Dialog { unsigned int field_20_; };
struct SpriteBox { int field_4_; };
struct SharedGraphicWin {
    virtual void show(int visible) {
        check(visible == 0, "show argument");
        event("base-show");
    }
};
struct Dialogs : Dialog, SpriteBox, SharedGraphicWin {
    int kind_;
    int attach_result;
    int attached_kind;
    Win *attached_owner;
    int attached_x, attached_y, attached_style, attach_count;
    Dialogs() : kind_(0), attach_result(0), attached_kind(0),
                attached_owner(0), attached_x(0), attached_y(0),
                attached_style(0), attach_count(0) {}
    virtual void show(int visible) {
        check(visible == 0, "Dialogs shared GraphicWin show argument");
        event("shared-show");
    }
    int attach(int kind, Win *owner, int x, int y, int style) {
        event("attach");
        ++attach_count;
        attached_kind = kind;
        attached_owner = owner;
        attached_x = x;
        attached_y = y;
        attached_style = style;
        return attach_result;
    }
};

static int BasePopDrawnBottom;
static int BasePopDrawnRight;
struct RedrawPlan {
    int bottom, right, kind, text_height, line_height, button_x;
    unsigned int flags;
};
struct BasePop : Win {
    Dialogs dialogs_;
    Win *owner_window_;
    int field_30D4_, field_30E4_, field_21CC_;
    int field_30F4_, field_30C0_, field_30EC_, field_30E0_, field_30D0_;
    unsigned int field_30A8_, field_30A4_;
    RedrawPlan plan;
    int seen_origin, seen_row, seen_style;
    int adjusted_origin, adjusted_row, adjusted_style;
    BasePop() : owner_window_(0), field_30D4_(0), field_30E4_(0),
                field_21CC_(0), field_30F4_(17), field_30C0_(100),
                field_30EC_(10), field_30E0_(10), field_30D0_(7),
                field_30A8_(0), field_30A4_(0), seen_origin(0), seen_row(0),
                seen_style(0), adjusted_origin(0), adjusted_row(0),
                adjusted_style(0) {
        plan.bottom = plan.right = plan.kind = plan.text_height = 0;
        plan.line_height = plan.button_x = 0;
        plan.flags = 0;
        dialogs_.Dialog::field_20_ = 0;
        dialogs_.SpriteBox::field_4_ = 0;
    }
    void on_redraw() {
        event("redraw");
        BasePopDrawnBottom = plan.bottom;
        BasePopDrawnRight = plan.right;
        dialogs_.kind_ = plan.kind;
        field_30D4_ = plan.text_height;
        field_30E4_ = plan.line_height;
        field_21CC_ = plan.button_x;
        field_30A8_ = plan.flags;
    }
    void on_adjust_button_placement(int *origin, int *row, int *style) {
        event("adjust");
        seen_origin = *origin;
        seen_row = *row;
        seen_style = *style;
        *origin = 111;
        *row = 222;
        *style = 333;
        adjusted_origin = *origin;
        adjusted_row = *row;
        adjusted_style = *style;
    }
    void run_slice() {
__LIVE_SLICE__
    }
};

static void check_case(int kind, int button_x, int text_height,
                       unsigned int flags, unsigned int dialog_bits,
                       int previous_sprite, int attach_result,
                       int own_owner, int bottom, int right, int line_height,
                       int button_style) {
    BasePop pop;
    Win owner;
    pop.owner_window_ = own_owner ? static_cast<Win *>(&pop) : &owner;
    pop.dialogs_.kind_ = 0x1234;  // redraw must publish the current kind
    pop.dialogs_.Dialog::field_20_ = dialog_bits;
    pop.dialogs_.SpriteBox::field_4_ = previous_sprite;
    pop.dialogs_.attach_result = attach_result;
    pop.plan.bottom = bottom;
    pop.plan.right = right;
    pop.plan.kind = kind;
    pop.plan.text_height = text_height;
    pop.plan.line_height = line_height;
    pop.plan.button_x = button_x;
    pop.plan.flags = flags;
    pop.field_30A4_ = static_cast<unsigned int>(button_style);
    BasePopDrawnBottom = -7001;
    BasePopDrawnRight = -8001;
    event_count = 0;
    pop.run_slice();

    check(same_event(0, "redraw"), "redraw is first");
    if (kind == -1) {
        check(pop.dialogs_.attach_count == 0, "kind -1 skips attach");
        check(pop.dialogs_.Dialog::field_20_ == dialog_bits,
              "kind -1 preserves Dialog flags");
        check(pop.dialogs_.SpriteBox::field_4_ == previous_sprite,
              "kind -1 preserves SpriteBox field");
        check(event_count == 2 && same_event(1, "adjust"),
              "kind -1 has no show and proceeds to row hook");
    } else {
        int expected_x = button_x >= 0 ? button_x : right;
        int expected_y = bottom + (text_height != 0 ? line_height : 0);
        int expected_style = (button_style & 0x40) | 0x80000;
        check(pop.dialogs_.attach_count == 1, "kind other than -1 attaches once");
        check(pop.dialogs_.attached_kind == kind, "attach reads post-redraw kind");
        check(pop.dialogs_.attached_owner == pop.owner_window_, "attach owner identity");
        check(pop.dialogs_.attached_x == expected_x, "attach x branch and fresh right global");
        check(pop.dialogs_.attached_y == expected_y, "attach y and text-height branch");
        check(pop.dialogs_.attached_style == expected_style, "attach style mask");
        check(pop.dialogs_.Dialog::field_20_ ==
                  (dialog_bits | ((flags & 0x800000) ? 0x1000U : 0U)),
              "Dialog flag OR preserves existing bits");
        check(pop.dialogs_.SpriteBox::field_4_ == static_cast<int>(flags & 0x800),
              "SpriteBox field is overwritten from popup flags");
        check(event_count == 4 && same_event(1, "attach") &&
                  same_event(2, "shared-show") && same_event(3, "adjust"),
              "attach precedes shared virtual show and row hook");
        check(attach_result != -999 || same_event(2, "shared-show"),
              "ignored attach result still reaches show");
    }
    check(pop.seen_origin == -17, "row origin uses live field_30F4");
    check(pop.seen_row == 73, "row expression uses live height and spacing fields");
    check(pop.seen_style == 0, "row style starts at zero");
    check(pop.adjusted_origin == 111 && pop.adjusted_row == 222 &&
              pop.adjusted_style == 333,
          "row hook mutates all three outputs");
}

int main() {
    const int kinds[] = {-2, -1, 0, 1, 2, 4, 8, 16, INT_MAX};
    const int xs[] = {-2, INT_MIN, 0, 19};
    const int heights[] = {0, 9, -1};
    const unsigned int flag_sets[] = {
        0U, 0x800U, 0x800000U, 0x800800U,
        0x40U, 0x21040U, 0xffffffffU, 0x10000001U
    };
    for (int k = 0; k != 9; ++k) {
      for (int x = 0; x != 4; ++x) {
       for (int h = 0; h != 3; ++h) {
        for (int f = 0; f != 8; ++f) {
         for (int a = 0; a != 2; ++a) {
          for (int o = 0; o != 2; ++o) {
            int seed = k * 1000 + x * 100 + h * 10 + f;
            check_case(kinds[k], xs[x], heights[h], flag_sets[f],
                       0x202U + static_cast<unsigned int>(seed), 0x345 + seed,
                       a ? -999 : 0, o, 100 + seed, 300 + seed,
                       7 + seed, f % 2 ? 0x40 : 0xA0);
          }
         }
        }
       }
      }
    }
    if (failures != 0)
        std::fprintf(stderr, "%d behavior checks failed (first 10 shown)\n", failures);
    return failures == 0 ? 0 : 1;
}
'''


def _compile_run(tmp_path, compiler_name, snippet, label):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source = HARNESS.replace("__LIVE_SLICE__", snippet)
    cpp = tmp_path / (label + ".cpp")
    binary = tmp_path / label
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_live_exec_slice_obeys_dialog_attachment_matrix(
        compiler_name, tmp_path):
    result = _compile_run(tmp_path, compiler_name, live_slice(), "actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_source_behavior_mutants_compile_and_fail_controls(
        compiler_name, tmp_path):
    actual = live_slice()
    positive = _compile_run(tmp_path, compiler_name, actual, "positive")
    assert positive.returncode == 0, positive.stdout + positive.stderr

    attach_call = (
        "        dialogs_.Dialogs::attach(\n"
        "            dialogs_.kind_, owner_window_, dialog_x, dialog_y,\n"
        "            static_cast<int>((field_30A4_ & 0x40) | 0x80000));")
    markers = "    // THE ROW ORIGIN."
    before_row = "    on_redraw();"
    origin = actual.index(markers)
    redraw = actual.index(before_row)
    assert origin > redraw and actual.count(attach_call) == 1
    mutants = {
        "drop-attachment-region": actual[:redraw + len(before_row)] + "\n\n" + actual[origin:],
        "omit-attach-call": actual.replace(attach_call,
                                            "        (void)dialog_x; (void)dialog_y;", 1),
        "omit-show": actual.replace("        dialogs_.show(0);", "        (void)0;", 1),
        "wrong-invalid-kind-gate": actual.replace(
            "dialogs_.kind_ != -1", "dialogs_.kind_ >= 0", 1),
        "negative-x-only-minus-one": actual.replace(
            "if (dialog_x < 0)", "if (dialog_x == -1)", 1),
        "lost-dialog-flag-or": actual.replace(
            "dialogs_.Dialog::field_20_ |= 0x1000;",
            "dialogs_.Dialog::field_20_ = 0x1000;", 1),
        "wrong-sprite-mask": actual.replace(
            "dialogs_.SpriteBox::field_4_ = field_30A8_ & 0x800;",
            "dialogs_.SpriteBox::field_4_ = field_30A8_ & 0x400;", 1),
        "wrong-attach-style": actual.replace(
            "static_cast<int>((field_30A4_ & 0x40) | 0x80000)",
            "static_cast<int>((field_30A4_ & 0x40) | 0x40000)", 1),
        "show-bypasses-virtual-dispatch": actual.replace(
            "dialogs_.show(0);", "dialogs_.SharedGraphicWin::show(0);", 1),
        "cache-kind-before-redraw": actual.replace(
            "    on_redraw();", "    int cached_kind = dialogs_.kind_;\n    on_redraw();", 1
        ).replace("dialogs_.kind_ != -1", "cached_kind != -1", 1).replace(
            "dialogs_.kind_, owner_window_, dialog_x",
            "cached_kind, owner_window_, dialog_x", 1),
        "show-before-attach": actual.replace(
            attach_call + "\n        dialogs_.show(0);",
            "dialogs_.show(0);\n" + attach_call, 1),
        "failure-skips-show": actual.replace(
            attach_call,
            "if (dialogs_.Dialogs::attach(\n"
            "                dialogs_.kind_, owner_window_, dialog_x, dialog_y,\n"
            "                static_cast<int>((field_30A4_ & 0x40) | 0x80000)) != 0) return;", 1),
    }
    for name, mutant in mutants.items():
        assert mutant != actual, name
        result = _compile_run(tmp_path, compiler_name, mutant, name)
        assert result.returncode != 0, (name, result.stdout, result.stderr)
        assert "FAIL:" in result.stderr, (name, result.stdout, result.stderr)


def test_pinned_original_disassembly_proves_attach_path_and_shared_show_slot():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    import struct

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    span = Span(0x00602600, 0x00602C6A)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(span), span.start))
    cursor = span.start
    decoded = {}
    for insn in instructions:
        assert insn.address == cursor
        cursor += insn.size
        decoded[insn.address] = insn
    assert cursor == span.end

    expected = {
        0x00602B4D: ("call", "dword ptr [eax + 0x30]"),
        0x00602B50: ("mov", "eax, dword ptr [esi + 0x30d4]"),
        0x00602B56: ("mov", "ecx, dword ptr [0x9b8ce8]"),
        0x00602B5C: ("test", "eax, eax"),
        0x00602B62: ("je", "0x602b6e"),
        0x00602B64: ("add", "ecx, dword ptr [esi + 0x30e4]"),
        0x00602B6E: ("mov", "eax, dword ptr [esi + 0x21cc]"),
        0x00602B76: ("jge", "0x602b84"),
        0x00602B78: ("mov", "edx, dword ptr [0x9b8cec]"),
        0x00602B88: ("cmp", "dword ptr [esi + 0x2350], -1"),
        0x00602B8F: ("je", "0x602c1b"),
        0x00602B95: ("test", "dword ptr [esi + 0x30a8], 0x800000"),
        0x00602BA7: ("mov", "ecx, dword ptr [eax + 8]"),
        0x00602BB8: ("or", "ch, 0x10"),
        0x00602BBB: ("mov", "dword ptr [eax], ecx"),
        0x00602BCD: ("and", "eax, 0x800"),
        0x00602BD2: ("mov", "dword ptr [esi + 0x2244], eax"),
        0x00602BE2: ("and", "edx, 0x40"),
        0x00602BE5: ("or", "edx, 0x80000"),
        0x00602BEB: ("push", "edx"),
        0x00602BF2: ("push", "ecx"),
        0x00602BF9: ("push", "eax"),
        0x00602BFA: ("push", "ecx"),
        0x00602BFB: ("push", "edx"),
        0x00602BFC: ("mov", "ecx, edi"),
        0x00602BFE: ("call", "0x612970"),
        0x00602C05: ("push", "0"),
        0x00602C07: ("mov", "ecx, dword ptr [eax + 4]"),
        0x00602C11: ("lea", "ecx, [ecx + esi + 0x21d0]"),
        0x00602C18: ("call", "dword ptr [edx + 4]"),
        0x00602C1B: ("mov", "eax, dword ptr [esi + 0x30c0]"),
        0x00602C64: ("call", "dword ptr [edx + 0x178]"),
    }
    for address, instruction in expected.items():
        actual = decoded[address]
        assert (actual.mnemonic, actual.op_str) == instruction
    # The +8 view above is Dialog; the later +4 view is the shared
    # GraphicWin branch. This prevents treating the two vbtable entries as a
    # selector into one common Dialog implementation.
    assert decoded[0x00602BA7].op_str.endswith("[eax + 8]")
    assert decoded[0x00602C07].op_str.endswith("[eax + 4]")
    # __thiscall pushes style, y, x, owner and kind in reverse argument order.
    assert [decoded[address].op_str for address in (
        0x00602BEB, 0x00602BF2, 0x00602BF9, 0x00602BFA, 0x00602BFB
    )] == ["edx", "ecx", "eax", "ecx", "edx"]
    assert struct.unpack("<III", image.read(Span(0x00670A78, 0x00670A84))) == (
        0, 0x188, 0xBA0)

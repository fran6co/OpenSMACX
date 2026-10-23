"""Actual-source controls for two typed Win flag predicates.

The pointer-overload fixture executes the complete installed body with narrow
Win stubs. The redraw fixture executes only the extracted early guard; it is
explicitly not a whole-body behavior proof. Pinned PE checks anchor each
historical byte test and branch independently.
"""

from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BASELINE = "7270943e6988505e93cff9d32d126c129f70e8ab"
REDRAW = (0x005F2CF0, 0x005F37BA)
NONCLIENT = (0x005ED3F0, 0x005ED48D)
REDRAW_SHA256 = "55cb6b9821816982dd162c7c8cc3ee9bceac0c582e5dc6b40c80950f26277582"
NONCLIENT_SHA256 = "d32c469f29148f5a04f15cfd09cc5b3016f427e1945fbaffc7fd16c29e834a9b"
REDRAW_SIGNATURE = "void Win::on_redraw_nc(RECT * area, int flags)"
NONCLIENT_SIGNATURE = "void Win::nonclient_to_screen(int * x, int * y)"
OLD_REDRAW = "if ((*(unsigned char *)&this->iSomeFlag_ & 2) == 0) return;"
NEW_REDRAW = "if ((this->iSomeFlag_ & 2U) == 0) return;"
OLD_NONCLIENT = (
    "if ((*(unsigned char *)&iFlags_ & 0x20) != 0 && win_parent_ != 0) {")
NEW_NONCLIENT = (
    "if ((iFlags_ & WinFlagClipToParent) != 0 && win_parent_ != 0) {")


def _definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
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
                return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _source():
    cpp = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    point_cpp = (ROOT / "src/win_point_coordinates.cpp").read_text(encoding="utf-8")
    return (cpp, _definition(cpp, REDRAW_SIGNATURE),
            _definition(point_cpp, NONCLIENT_SIGNATURE))


def test_only_two_win_predicates_change_and_existing_mappings_remain():
    cpp, redraw, nonclient = _source()
    baseline = subprocess.check_output(
        ["git", "show", f"{BASELINE}:src/win.cpp"],
        cwd=ROOT, text=True, timeout=30)
    assert baseline.count(OLD_REDRAW) == 1
    assert baseline.count(OLD_NONCLIENT) == 1
    expected = baseline.replace(OLD_REDRAW, NEW_REDRAW, 1)
    expected = expected.replace(OLD_NONCLIENT, NEW_NONCLIENT, 1)
    assert cpp == expected
    assert NEW_REDRAW in redraw and OLD_REDRAW not in redraw
    assert NEW_NONCLIENT in nonclient and OLD_NONCLIENT not in nonclient

    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert re.search(
        r"uint32_t\s+iFlags_;\s*//\s*0x98\b[^\n]*\n\s*uint32_t\s+iSomeFlag_\s*;",
        header)
    assert re.search(r"WinFlagClipToParent\s*=\s*0x20\s*;\s*//\s*iFlags_", header)
    assert "static_assert(sizeof(Win) == 0x444" in header
    assert "virtual void on_redraw_nc(RECT *area, int flags);  // slot 67  0x005F2CF0" in header
    assert "void nonclient_to_screen(int * x, int * y);" in header
    assert "void nonclient_to_screen(RECT * rect);" in header

    records = read_text(cpp, ROOT / "src/win.cpp")
    point_records = read_text(
        (ROOT / "src/win_point_coordinates.cpp").read_text(encoding="utf-8"),
        ROOT / "src/win_point_coordinates.cpp")
    redraw_row, = [row for row in records if row.address == REDRAW[0]]
    nonclient_row, = [row for row in point_records if row.address == NONCLIENT[0]]
    assert redraw_row.name == "?on_redraw_nc@Win@@QAEXPAURECT@@H@Z"
    assert redraw_row.image_spans == (REDRAW,)
    assert redraw_row.byte_exact is False and redraw_row.semantic is False
    assert redraw_row.symbol == "?on_redraw_nc@Win@@UAEXPAUtagRECT@@H@Z"
    assert nonclient_row.name == "?nonclient_to_screen@Win@@QAEXPAH0@Z"
    assert nonclient_row.image_spans == (NONCLIENT,)
    assert nonclient_row.byte_exact is True and nonclient_row.semantic is False
    assert nonclient_row.symbol == "" and nonclient_row.context == ""


def test_pinned_image_spans_and_mask_control_flow():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86_const import X86_REG_AL

    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    assert image_pin(ROOT) == PIN
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PIN
    image = PeImage(image_bytes, image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    redraw_raw = image.read(Span(*REDRAW))
    assert len(redraw_raw) == 2762
    assert hashlib.sha256(redraw_raw).hexdigest() == REDRAW_SHA256
    redraw_instructions = list(decoder.disasm(redraw_raw, REDRAW[0]))
    redraw_cursor = REDRAW[0]
    for instruction in redraw_instructions:
        assert instruction.address == redraw_cursor
        redraw_cursor += instruction.size
    assert redraw_cursor == REDRAW[1]
    redraw = {item.address: item for item in redraw_instructions}
    guard = redraw[0x005F2CF9]
    assert guard.mnemonic == "test" and guard.operands[0].size == 1
    assert guard.operands[0].mem.disp == 0x9C and guard.operands[1].imm == 2
    assert redraw[0x005F2D00].mnemonic == "je"
    assert redraw[0x005F2D00].operands[0].imm == 0x005F37B0

    nonclient_raw = image.read(Span(*NONCLIENT))
    assert len(nonclient_raw) == 157
    assert hashlib.sha256(nonclient_raw).hexdigest() == NONCLIENT_SHA256
    nonclient_instructions = list(decoder.disasm(nonclient_raw, NONCLIENT[0]))
    by_address = {}
    cursor = NONCLIENT[0]
    for instruction in nonclient_instructions:
        assert instruction.address == cursor, f"undecoded original byte at {cursor:#x}"
        by_address[instruction.address] = instruction
        cursor += instruction.size
    assert cursor == NONCLIENT[1] and len(nonclient_instructions) == 52
    load = by_address[0x005ED425]
    assert load.mnemonic == "mov" and load.operands[0].reg == X86_REG_AL
    assert load.operands[1].size == 1 and load.operands[1].mem.disp == 0x98
    mask = by_address[0x005ED42B]
    assert mask.mnemonic == "test" and mask.operands[0].reg == X86_REG_AL
    assert mask.operands[1].imm == 0x20
    assert by_address[0x005ED42D].operands[0].imm == 0x005ED46F
    call = by_address[0x005ED43B]
    assert call.mnemonic == "call" and call.operands[0].imm == 0x005ED240
    assert by_address[0x005ED48A].mnemonic == "ret"
    assert by_address[0x005ED48A].operands[0].imm == 8


def _run_nonclient_host(tmp_path, compiler, body, name):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    program = textwrap.dedent(r"""\
        #include <cassert>
        #include <cstdint>
        #define __thiscall
        enum { WinFlagClipToParent = 0x20u };
        struct RECT { int left, top, right, bottom; };
        struct Win {
            uint32_t iFlags_;
            RECT outer_rect_;
            RECT client_rect_;
            Win *win_parent_;
            int delta_x_, delta_y_;
            void client_to_screen(int *x, int *y);
            void nonclient_to_screen(int *x, int *y);
        };
        static int callback_count;
        static int callback_x, callback_y;
        static Win *callback_owner;
        static Win *callback_child;
        static Win *callback_redirect_parent;
        static int mutate_during_callback;
        static uint32_t child_flags_after_callback;
        static int child_left_after_callback, child_top_after_callback;
        static int owner_left_after_callback, owner_top_after_callback;
        void Win::client_to_screen(int *x, int *y) {
            ++callback_count;
            callback_owner = this;
            callback_x = *x;
            callback_y = *y;
            *x += delta_x_;
            *y += delta_y_;
            if (mutate_during_callback) {
                callback_child->iFlags_ = child_flags_after_callback;
                callback_child->outer_rect_.left = child_left_after_callback;
                callback_child->outer_rect_.top = child_top_after_callback;
                outer_rect_.left = owner_left_after_callback;
                outer_rect_.top = owner_top_after_callback;
                callback_child->win_parent_ = callback_redirect_parent;
            }
        }
        BODY

        static void setup(Win &child, Win &parent, uint32_t flags) {
            child.iFlags_ = flags;
            child.outer_rect_ = RECT{2, 3, 0, 0};
            child.client_rect_ = RECT{5, 7, 0, 0};
            child.win_parent_ = &parent;
            parent.outer_rect_ = RECT{11, 13, 0, 0};
            parent.client_rect_ = RECT{0, 0, 0, 0};
            parent.win_parent_ = 0;
            parent.delta_x_ = 17;
            parent.delta_y_ = 19;
            child.delta_x_ = child.delta_y_ = 0;
            callback_count = 0;
            callback_owner = 0;
            callback_child = &child;
            callback_redirect_parent = &parent;
            mutate_during_callback = 0;
        }
        static void check_distinct(Win &child, Win &parent, uint32_t flags,
                                   bool parent_present) {
            setup(child, parent, flags);
            if (!parent_present) child.win_parent_ = 0;
            int x = 23, y = -31;
            const int before_parent_x = x + child.outer_rect_.left + child.client_rect_.left;
            const int before_parent_y = y + child.outer_rect_.top + child.client_rect_.top;
            const bool call_parent = parent_present && (flags & 0x20u) != 0;
            int expected_x = before_parent_x;
            int expected_y = before_parent_y;
            if (call_parent) {
                expected_x += parent.delta_x_;
                expected_y += parent.delta_y_;
                if ((flags & 0x8000u) != 0) {
                    expected_x -= parent.outer_rect_.left;
                    expected_y -= parent.outer_rect_.top;
                }
            }
            expected_x -= child.outer_rect_.left;
            expected_y -= child.outer_rect_.top;

            child.nonclient_to_screen(&x, &y);
            assert(x == expected_x && y == expected_y);
            assert(callback_count == (call_parent ? 1 : 0));
            if (call_parent) {
                assert(callback_owner == &parent);
                assert(callback_x == before_parent_x && callback_y == before_parent_y);
            }
        }
        static void check_callback_mutations(Win &child, Win &first_parent,
                                             Win &second_parent, bool set_parent_offset) {
            setup(child, first_parent, set_parent_offset ? 0x20u : 0x8020u);
            second_parent = Win{};
            second_parent.outer_rect_ = RECT{7, 8, 0, 0};
            callback_child = &child;
            callback_redirect_parent = set_parent_offset ? &second_parent : &first_parent;
            mutate_during_callback = 1;
            child_flags_after_callback = set_parent_offset ? 0x8020u : 0x20u;
            child_left_after_callback = 4;
            child_top_after_callback = 6;
            owner_left_after_callback = 101;
            owner_top_after_callback = 103;

            int x = 23, y = -31;
            child.nonclient_to_screen(&x, &y);
            // Initial own/client additions use old extents; the call then
            // mutates the flags, parent relation, and rects. Later branches
            // must observe those updated values in the pinned source order.
            assert(callback_count == 1 && callback_owner == &first_parent);
            assert(callback_x == 30 && callback_y == -21);
            if (set_parent_offset) {
                assert(x == 36 && y == -16); // corrected through new parent (7,8), then new own rect (4,6)
                assert(child.win_parent_ == &second_parent);
            } else {
                assert(x == 43 && y == -8); // callback cleared 0x8000, so no parent-origin subtraction
                assert(child.win_parent_ == &first_parent);
            }
            assert(child.outer_rect_.left == 4 && child.outer_rect_.top == 6);
            assert(first_parent.outer_rect_.left == 101 && first_parent.outer_rect_.top == 103);
        }
        int main() {
            Win child = {};
            Win parent = {};
            Win second_parent = {};
            // Exercise every individual flag bit with a live parent. Only
            // 0x20 calls the parent; 0x8000 matters only inside that branch.
            for (unsigned int bit = 0; bit < 32; ++bit)
                check_distinct(child, parent, (uint32_t(1) << bit), true);
            check_distinct(child, parent, 0x8020u, true);
            check_distinct(child, parent, 0x8020u, false);
            check_callback_mutations(child, parent, second_parent, true);
            check_callback_mutations(child, parent, second_parent, false);

            // Aliasing x and y observes the body's ordered pointer reloads:
            // each write and the mutating parent callback acts on one int.
            setup(child, parent, 0x8020u);
            int coordinate = 29;
            const int expected = coordinate + child.client_rect_.left
                + child.client_rect_.top + parent.delta_x_ + parent.delta_y_
                - parent.outer_rect_.left - parent.outer_rect_.top;
            child.nonclient_to_screen(&coordinate, &coordinate);
            assert(coordinate == expected);
            assert(callback_count == 1);
            assert(callback_x == 29 + child.outer_rect_.left + child.client_rect_.left
                                 + child.outer_rect_.top + child.client_rect_.top);
            assert(callback_y == callback_x);
            return 0;
        }
    """).replace("BODY", body)
    cpp_path = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp_path.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp_path), "-o", str(binary)],
        capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_complete_nonclient_pointer_body_runs_on_host(tmp_path, compiler):
    _, _, body = _source()
    result = _run_nonclient_host(tmp_path, compiler, body, "nonclient_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", (
    "mask", "polarity", "stale_flags", "stale_parent", "stale_rect",
    "callback_copy", "callback_order"))
def test_nonclient_behavior_mutations_fail_at_runtime(tmp_path, compiler, mutation):
    _, _, body = _source()
    if mutation == "mask":
        assert "WinFlagClipToParent" in body
        damaged = body.replace("WinFlagClipToParent", "0x40u", 1)
    elif mutation == "polarity":
        old = "(iFlags_ & WinFlagClipToParent) != 0"
        assert old in body
        damaged = body.replace(old, "(iFlags_ & WinFlagClipToParent) == 0", 1)
    elif mutation == "stale_flags":
        opening = "void Win::nonclient_to_screen(int * x, int * y) {"
        assert opening in body
        damaged = body.replace(opening, opening + "\n    uint32_t stale_flags = iFlags_;", 1)
        old = "if ((iFlags_ & 0x8000) != 0) {"
        assert old in damaged
        damaged = damaged.replace(old, "if ((stale_flags & 0x8000) != 0) {", 1)
    elif mutation == "stale_parent":
        opening = "void Win::nonclient_to_screen(int * x, int * y) {"
        assert opening in body
        damaged = body.replace(opening, opening + "\n    Win *stale_parent = win_parent_;", 1)
        for old, new in (
            ("*x = *x - win_parent_->outer_rect_.left;",
             "*x = *x - stale_parent->outer_rect_.left;"),
            ("*y = *y - win_parent_->outer_rect_.top;",
             "*y = *y - stale_parent->outer_rect_.top;"),
        ):
            assert old in damaged
            damaged = damaged.replace(old, new, 1)
    elif mutation == "stale_rect":
        opening = "void Win::nonclient_to_screen(int * x, int * y) {"
        assert opening in body
        damaged = body.replace(
            opening, opening + "\n    const int stale_left = outer_rect_.left;\n"
            "    const int stale_top = outer_rect_.top;", 1)
        for old, new in (
            ("*x = *x - outer_rect_.left;", "*x = *x - stale_left;"),
            ("*y = *y - outer_rect_.top;", "*y = *y - stale_top;"),
        ):
            assert old in damaged
            damaged = damaged.replace(old, new, 1)
    elif mutation == "callback_copy":
        old = "win_parent_->client_to_screen(x, y);"
        new = ("int copy_x = *x, copy_y = *y;\n"
               "        win_parent_->client_to_screen(&copy_x, &copy_y);\n"
               "        *x = copy_x;\n"
               "        *y = copy_y;")
        assert old in body
        damaged = body.replace(old, new, 1)
    else:
        old = ("win_parent_->client_to_screen(x, y);\n"
               "        if ((iFlags_ & 0x8000) != 0) {\n"
               "            *x = *x - win_parent_->outer_rect_.left;\n"
               "            *y = *y - win_parent_->outer_rect_.top;\n"
               "        }")
        new = ("if ((iFlags_ & 0x8000) != 0) {\n"
               "            *x = *x - win_parent_->outer_rect_.left;\n"
               "            *y = *y - win_parent_->outer_rect_.top;\n"
               "        }\n"
               "        win_parent_->client_to_screen(x, y);")
        assert old in body
        damaged = body.replace(old, new, 1)
    assert damaged != body
    result = _run_nonclient_host(tmp_path, compiler, damaged, f"nonclient_{mutation}")
    assert result.returncode != 0, f"{mutation} mutant unexpectedly passed"


def _redraw_guard(body):
    line = next(line.strip() for line in body.splitlines()
                if "this->iSomeFlag_" in line and line.strip().startswith("if ("))
    match = re.fullmatch(r"if \((.*)\) return;", line)
    assert match is not None
    return match.group(1)


def _run_redraw_guard(tmp_path, compiler, expression, name):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    expression = expression.replace("this->", "window->")
    program = textwrap.dedent(r"""\
        #include <cassert>
        #include <cstdint>
        struct Win { uint32_t iSomeFlag_; };
        static bool redraw_guard(Win *window) { return (EXPRESSION); }
        int main() {
            Win window = {};
            for (unsigned int bit = 0; bit < 32; ++bit) {
                window.iSomeFlag_ = (uint32_t(1) << bit);
                assert(redraw_guard(&window) == ((window.iSomeFlag_ & 2u) == 0));
            }
            window.iSomeFlag_ = 0xffffffffu;
            assert(!redraw_guard(&window));
            window.iSomeFlag_ = 0xfffffffdu;
            assert(redraw_guard(&window));
            return 0;
        }
    """).replace("EXPRESSION", expression)
    cpp_path = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp_path.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp_path), "-o", str(binary)],
        capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_redraw_early_guard_only_probe_and_mutations(tmp_path, compiler):
    _, body, _ = _source()
    expression = _redraw_guard(body)
    result = _run_redraw_guard(tmp_path, compiler, expression, "redraw_guard")
    assert result.returncode == 0, result.stdout + result.stderr
    for old, new in (("2U", "4U"), ("== 0", "!= 0")):
        assert old in expression
        damaged = expression.replace(old, new, 1)
        result = _run_redraw_guard(tmp_path, compiler, damaged, "redraw_guard_mutant")
        assert result.returncode != 0, "redraw guard mutant unexpectedly passed"

"""Source and pinned-image controls for Dialogs' sparse kind dispatchers."""

from pathlib import Path
import os
import shutil
import struct
import subprocess

from capstone import CS_ARCH_X86, CS_MODE_32, CS_OP_IMM, Cs
import pytest
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated function {signature}")


SOURCE = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")
FOCUS = body(SOURCE, "void Dialogs::on_dialog_focus(int focus_id)")
ATTACH = body(SOURCE, "int Dialogs::attach(GraphicWin *parent, int x, int y, int flags)")


HARNESS = r'''#include <cassert>
#include <climits>
#include <vector>

struct GraphicWin {};
static int focus_owner = 0, focus_value = 0;
static int attach_owner = 0, attach_x = 0, attach_y = 0, attach_flags = 0;
static GraphicWin *attach_parent = 0;

#define OWNER_METHODS(Type, Owner, Result) \
    void on_dialog_focus(int value) { focus_owner = Owner; focus_value = value; } \
    int attach(GraphicWin *parent, int x, int y, int flags) { \
        attach_owner = Owner; attach_parent = parent; attach_x = x; \
        attach_y = y; attach_flags = flags; return Result; \
    }
struct RadioButton { OWNER_METHODS(RadioButton, 5, 105) };
struct ListBox { OWNER_METHODS(ListBox, 2, 102) };
struct CheckBox { OWNER_METHODS(CheckBox, 1, 101) };
struct SpriteBox { OWNER_METHODS(SpriteBox, 4, 104) };
struct EditGroup { OWNER_METHODS(EditGroup, 3, 103) };

struct DialogsMock : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {
    int kind_;
    DialogsMock(): kind_(0) {}
    void on_dialog_focus(int focus_id);
    int attach(GraphicWin *parent, int x, int y, int flags);
};
FOCUS_BODY
ATTACH_BODY

static int expected_owner(int kind) {
    switch (kind) {
    case 1: return 1;
    case 2: return 2;
    case 4: return 3;
    case 8: return 4;
    case 16: return 5;
    default: return 0;
    }
}
static int expected_attach_result(int kind) {
    int owner = expected_owner(kind);
    return owner == 0 ? 7 : 100 + owner;
}
int main() {
    DialogsMock d;
    GraphicWin parent;
    const int extras[] = {INT_MIN, -1, INT_MAX};
    for (int kind = 0; kind <= 31; ++kind) {
        d.kind_ = kind;
        focus_owner = 0; focus_value = -999;
        d.on_dialog_focus(0x123456);
        assert(focus_owner == expected_owner(kind));
        assert(focus_value == (expected_owner(kind) ? 0x123456 : -999));

        attach_owner = attach_x = attach_y = attach_flags = 0; attach_parent = 0;
        int result = d.attach(&parent, -31, 47, 0x1234);
        assert(result == expected_attach_result(kind));
        assert(attach_owner == expected_owner(kind));
        if (expected_owner(kind)) {
            assert(attach_parent == &parent && attach_x == -31 && attach_y == 47 && attach_flags == 0x1234);
        } else {
            assert(attach_parent == 0 && attach_x == 0 && attach_y == 0 && attach_flags == 0);
        }
    }
    for (int kind = 1; kind <= 16; ++kind) {
        if (expected_owner(kind) == 0) continue;
        d.kind_ = kind;
        attach_owner = attach_x = attach_y = attach_flags = 0; attach_parent = &parent;
        const int result = d.attach(0, 71, -92, 0x3456);
        assert(result == expected_attach_result(kind));
        assert(attach_owner == expected_owner(kind) && attach_parent == 0);
        assert(attach_x == 71 && attach_y == -92 && attach_flags == 0x3456);
    }
    for (unsigned i = 0; i < sizeof(extras) / sizeof(extras[0]); ++i) {
        d.kind_ = extras[i];
        focus_owner = 0; focus_value = -999;
        d.on_dialog_focus(9);
        assert(focus_owner == 0 && focus_value == -999);
        attach_owner = attach_x = attach_y = attach_flags = 0; attach_parent = 0;
        assert(d.attach(&parent, 1, 2, 3) == 7);
        assert(attach_owner == 0 && attach_parent == 0 && attach_x == 0 && attach_y == 0 && attach_flags == 0);
    }
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_sparse_dispatch_source_actual_routing_and_arguments(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    source = HARNESS.replace("FOCUS_BODY", FOCUS.replace("Dialogs::", "DialogsMock::"))
    source = source.replace("ATTACH_BODY", ATTACH.replace("Dialogs::", "DialogsMock::"))
    cpp, binary = tmp_path / "dispatch.cpp", tmp_path / "dispatch"
    cpp.write_text(source)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)

    wrong_route = SOURCE.replace(
        "case 1:\n        CheckBox::on_dialog_focus(focus_id);",
        "case 1:\n        RadioButton::on_dialog_focus(focus_id);",
        1,
    )
    assert wrong_route != SOURCE
    wrong_route_focus = body(wrong_route, "void Dialogs::on_dialog_focus(int focus_id)")
    wrong_route_harness = HARNESS.replace(
        "FOCUS_BODY", wrong_route_focus.replace("Dialogs::", "DialogsMock::")
    ).replace("ATTACH_BODY", ATTACH.replace("Dialogs::", "DialogsMock::"))
    wrong_cpp, wrong_binary = tmp_path / "wrong_route.cpp", tmp_path / "wrong_route"
    wrong_cpp.write_text(wrong_route_harness)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(wrong_cpp), "-o", str(wrong_binary)], check=True, timeout=30)
    assert subprocess.run([str(wrong_binary)], timeout=30).returncode != 0

    wrong_attach_route = SOURCE.replace(
        "case 1:\n        return CheckBox::attach(parent, x, y, flags);",
        "case 1:\n        return RadioButton::attach(parent, x, y, flags);",
        1,
    )
    assert wrong_attach_route != SOURCE
    wrong_attach_body = body(
        wrong_attach_route,
        "int Dialogs::attach(GraphicWin *parent, int x, int y, int flags)",
    )
    wrong_attach_harness = HARNESS.replace(
        "FOCUS_BODY", FOCUS.replace("Dialogs::", "DialogsMock::")
    ).replace("ATTACH_BODY", wrong_attach_body.replace("Dialogs::", "DialogsMock::"))
    wrong_attach_cpp = tmp_path / "wrong_attach_route.cpp"
    wrong_attach_binary = tmp_path / "wrong_attach_route"
    wrong_attach_cpp.write_text(wrong_attach_harness)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(wrong_attach_cpp), "-o", str(wrong_attach_binary)],
                   check=True, timeout=30)
    assert subprocess.run([str(wrong_attach_binary)], timeout=30).returncode != 0

    wrong_default = SOURCE.replace("default:\n        return 7;", "default:\n        return 3;", 1)
    assert wrong_default != SOURCE
    wrong_default_attach = body(
        wrong_default,
        "int Dialogs::attach(GraphicWin *parent, int x, int y, int flags)",
    )
    wrong_default_harness = HARNESS.replace(
        "FOCUS_BODY", FOCUS.replace("Dialogs::", "DialogsMock::")
    ).replace("ATTACH_BODY", wrong_default_attach.replace("Dialogs::", "DialogsMock::"))
    default_cpp, default_binary = tmp_path / "wrong_default.cpp", tmp_path / "wrong_default"
    default_cpp.write_text(wrong_default_harness)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(default_cpp), "-o", str(default_binary)], check=True, timeout=30)
    assert subprocess.run([str(default_binary)], timeout=30).returncode != 0


def pinned_image() -> PeImage:
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    return PeImage(path.read_bytes(), image_pin(ROOT))


def test_original_sparse_tables_and_targets():
    image = pinned_image()
    attach_selectors = image.read(Span(0x6130C8, 0x6130D8))
    focus_selectors = image.read(Span(0x61316C, 0x61317C))
    attach_targets = struct.unpack("<6I", image.read(Span(0x6130B0, 0x6130C8)))
    focus_targets = struct.unpack("<6I", image.read(Span(0x613154, 0x61316C)))
    assert attach_targets == (0x613043, 0x613021, 0x613087, 0x613065, 0x612FFF, 0x6130A6)
    assert focus_targets == (0x61311E, 0x61310B, 0x613144, 0x613131, 0x6130F8, 0x613151)

    expected_slots = {1: 0, 2: 1, 4: 2, 8: 3, 16: 4}
    for kind in range(1, 17):
        slot = expected_slots.get(kind, 5)
        assert attach_targets[attach_selectors[kind - 1]] == attach_targets[slot]
        assert focus_targets[focus_selectors[kind - 1]] == focus_targets[slot]

    # Every selected path ends in the independently identified base method;
    # the default slots are inline returns/no-ops rather than widget calls.
    attach_calls = (0x60E800, 0x60A670, 0x611AF0, 0x610540, 0x60D1F0)
    focus_calls = (0x60FB90, 0x60C7C0, 0x612670, 0x6115E0, 0x60E300)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for block, target in zip(attach_targets[:5], attach_calls):
        code = image.read(Span(block, block + 0x20))
        assert any(ins.mnemonic == "call" and ins.operands[0].type == CS_OP_IMM and
                   ins.operands[0].imm == target
                   for ins in decoder.disasm(code, block))
    for block, target in zip(focus_targets[:5], focus_calls):
        code = image.read(Span(block, block + 0x18))
        assert any(ins.mnemonic == "call" and ins.operands[0].type == CS_OP_IMM and
                   ins.operands[0].imm == target
                   for ins in decoder.disasm(code, block))

    attach_default = list(decoder.disasm(image.read(Span(0x6130A6, 0x6130AE)), 0x6130A6))
    assert [(ins.mnemonic, ins.op_str) for ins in attach_default[:2]] == [
        ("mov", "eax, 7"), ("ret", "0x10")]
    focus_default = list(decoder.disasm(image.read(Span(0x613151, 0x613154)), 0x613151))
    assert [(ins.mnemonic, ins.op_str) for ins in focus_default] == [("ret", "4")]

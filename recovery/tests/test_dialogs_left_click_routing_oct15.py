"""Controls for Dialogs' five qualified left-click dispatch cases."""

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
SOURCE = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")
SIGNATURE = "void Dialogs::on_left_click(int a, int b)"


def extract_body(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start + len(SIGNATURE))
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated Dialogs::on_left_click")


BODY = extract_body(SOURCE)
WIN_SOURCE = (ROOT / "src/win.h").read_text(encoding="utf-8")


def class_body(source: str, name: str) -> str:
    start = source.index("class " + name)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated class: {name}")


def test_non_sprite_click_inherits_the_actual_empty_win_operation():
    assert "virtual void on_left_click(int = 0, int = 0) {}" in WIN_SOURCE
    for owner, path in (
        ("CheckBox", ROOT / "src/jackal/checkbox.h"),
        ("ListBox", ROOT / "src/listbox.h"),
        ("EditGroup", ROOT / "src/editgroup.h"),
        ("RadioButton", ROOT / "src/jackal/radiobutton.h"),
    ):
        assert "on_left_click" not in class_body(path.read_text(encoding="utf-8"), owner)
    assert "on_left_click" in class_body((ROOT / "src/spritebox.h").read_text(encoding="utf-8"), "SpriteBox")


HARNESS = r'''#include <cassert>
#include <climits>

static int calls = 0;
static int received_x = 0, received_y = 0;
struct Win { virtual void on_left_click(int = 0, int = 0) {} };
struct GraphicWin : virtual Win {};
struct Dialog : virtual Win {};
struct ListBox : virtual GraphicWin, virtual Dialog {};
struct RadioButton : virtual GraphicWin, virtual Dialog {};
struct CheckBox : virtual GraphicWin, virtual Dialog {};
struct SpriteBox : virtual GraphicWin, virtual Dialog {
    void on_left_click(int x, int y) { ++calls; received_x = x; received_y = y; }
};
struct EditGroup : virtual GraphicWin, virtual Dialog {};
struct DialogsMock : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {
    int kind_;
    DialogsMock(): kind_(0) {}
    void on_left_click(int a, int b);
};
BODY_HERE

int main() {
    DialogsMock dialog;
    const int outside[] = {INT_MIN, INT_MIN + 1, -1, 17, 31, INT_MAX - 1, INT_MAX};
    for (int kind = 0; kind <= 31; ++kind) {
        dialog.kind_ = kind;
        calls = 0; received_x = received_y = 0;
        dialog.on_left_click(-12345, 67890);
        assert(calls == (kind == 8 ? 1 : 0));
        if (kind == 8) assert(received_x == -12345 && received_y == 67890);
    }
    for (unsigned i = 0; i < sizeof(outside) / sizeof(outside[0]); ++i) {
        dialog.kind_ = outside[i];
        calls = 0; received_x = received_y = 0;
        dialog.on_left_click(INT_MIN, INT_MAX);
        assert(calls == 0 && received_x == 0 && received_y == 0);
    }
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_left_click_actual_source_kind_domain_and_coordinates(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    source = HARNESS.replace("BODY_HERE", BODY.replace("Dialogs::", "DialogsMock::"))
    cpp, binary = tmp_path / "left_click.cpp", tmp_path / "left_click"
    cpp.write_text(source)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)

    wrong_body = BODY.replace("case 8:", "case 7:", 1)
    assert wrong_body != BODY
    wrong_body = wrong_body.replace("Dialogs::", "DialogsMock::")
    wrong_cpp, wrong_binary = tmp_path / "wrong_map.cpp", tmp_path / "wrong_map"
    wrong_cpp.write_text(HARNESS.replace("BODY_HERE", wrong_body))
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(wrong_cpp), "-o", str(wrong_binary)], check=True, timeout=30)
    assert subprocess.run([str(wrong_binary)], timeout=30).returncode != 0


def test_pinned_selector_routes_only_kind_eight_to_spritebox():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    selector_table = image.read(Span(0x612EBC, 0x612ECC))
    target_table = struct.unpack("<3I", image.read(Span(0x612EB0, 0x612EBC)))
    assert target_table[1] == 0x612E98
    assert target_table[0] == target_table[2] == 0x612EAD

    for kind in range(1, 17):
        selected_block = target_table[selector_table[kind - 1]]
        assert selected_block == (0x612E98 if kind == 8 else 0x612EAD)

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    click_block = list(decoder.disasm(image.read(Span(0x612E98, 0x612EAD)), 0x612E98))
    assert any(ins.mnemonic == "call" and ins.operands[0].type == CS_OP_IMM and
               ins.operands[0].imm == 0x611060 for ins in click_block)
    fallback = list(decoder.disasm(image.read(Span(0x612EAD, 0x612EB0)), 0x612EAD))
    assert [(ins.mnemonic, ins.op_str) for ins in fallback] == [("ret", "8")]

    entry = list(decoder.disasm(image.read(Span(0x612E80, 0x612E98)), 0x612E80))
    assert any(ins.mnemonic == "dec" and ins.op_str == "eax" for ins in entry)
    assert any(ins.mnemonic == "cmp" and ins.op_str == "eax, 0xf" for ins in entry)
    assert any(ins.mnemonic == "ja" and ins.operands[0].type == CS_OP_IMM and
               ins.operands[0].imm == 0x612EAD for ins in entry)

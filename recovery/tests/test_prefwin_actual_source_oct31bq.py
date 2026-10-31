"""PrefWin callback snapshot and signed-toggle controls over actual C++.

The existing fixtures provide service doubles; these witnesses independently pin
observable ordering and distinguish a late geometry read from original behavior.
They establish neither the whole PrefWin ABI nor application runtime acceptance.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
import test_prefwin_behavior as behavior
import test_prefwin_volume_contract_oct13h as volume


def run_host(tmp_path, compiler, text):
    cxx = shutil.which(compiler)
    if not cxx:
        pytest.skip(f"{compiler} unavailable")
    source, executable = tmp_path / "actual.cpp", tmp_path / "actual"
    source.write_text(text)
    subprocess.run([cxx, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


def display_witness():
    text = behavior.display_source()
    text = text[:text.rindex("int main() {")]
    text = text.replace("struct Buffer {", "void after_box();\nstruct Buffer {", 1)
    text = text.replace('event("box", style->index);',
                        'event("box", style->index); after_box();', 1)
    text += r'''
void after_box() {
    if (drawn_rects.size() == 1) {
        PrefWinGeometryValue_857888 = 1000;
        PrefWinGeometryValue_857884 = 2000;
        PrefWinGeometryValue_8578C0 = 3000;
    }
}
int main() {
    ConsolePrefWin.display(-1);
    assert(drawn_rects[2].left == 10 && drawn_rects[2].top == 20);
    assert(drawn_rects[2].right == 230 && drawn_rects[2].bottom == 120);
    assert(events[8].name == "show_restore");
    assert(events[9].name == "show_ok");
    assert(events[10].name == "show_cancel");
}
'''
    return text


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None, "late_geometry", "show_order"])
def test_display_original_snapshot_and_show_order(tmp_path, compiler, mutation):
    text = display_witness()
    if mutation == "late_geometry":
        start = text.index("  RECT buttons =")
        end = text.index(";", start) + 1
        declaration = text[start:end]
        text = text[:start] + text[end:]
        anchor = "  Buffer::box_sprite(&outer, &InterfaceBoxStyles[9]);"
        assert text.count(anchor) == 1
        text = text.replace(anchor, anchor + "\n" + declaration, 1)
    elif mutation == "show_order":
        original = "  okButton_.show(0);\n  cancelButton_.show(0);"
        assert text.count(original) == 1
        text = text.replace(original, "  cancelButton_.show(0);\n  okButton_.show(0);", 1)
    result = run_host(tmp_path, compiler, text)
    assert (result.returncode == 0) == (mutation is None), result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_any_nonzero_selected_word_and_null_result_are_preserved(tmp_path, compiler):
    text = volume.TEXT + volume.label_functions(volume.actual_source()) + r'''
int main() {
    PrefWin w;
    const int selected[] = {-2147483647 - 1, -1, 1, 0x100};
    for (int value : selected) {
        events.clear();
        w.volumeMainToggle_.selected = value;
        null_index = 1081;
        w.change_volume_button_text(0x55);
        assert((events == std::vector<std::string>{"label:1081", "main.name:<null>"}));
    }
}
'''
    assert run_host(tmp_path, compiler, text).returncode == 0


def test_original_pin_establishes_snapshot_boundary_and_button_order():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    root = Path(__file__).resolve().parents[2]
    image = Path(os.environ.get("OPENSMACX_IMAGE", root / ".opensmacx/game/terranx_original.exe"))
    if not image.is_file():
        pytest.skip("pinned original image unavailable")
    pe = PeImage(image.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    ins = {i.address: (i.mnemonic, i.op_str) for i in
           Cs(CS_ARCH_X86, CS_MODE_32).disasm(pe.read(Span(0x48fa00, 0x48fcc5)), 0x48fa00)}
    assert ins[0x48fae0] == ("mov", "eax, dword ptr [0x857884]")
    assert ins[0x48fae5] == ("mov", "dword ptr [ebp - 0x30], ecx")
    assert ins[0x48fb16] == ("mov", "dword ptr [ebp - 0x2c], eax")
    assert ins[0x48fb19] == ("call", "0x5e2dd3")
    # The names are pinned separately from the show-call receiver sequence.
    header = (root / "src/prefwin.h").read_text()
    assert "FlatButton restoreDefaultButton_;  // +0x18684" in header
    assert "FlatButton cancelButton_;          // +0x191D0" in header
    assert "FlatButton okButton_;              // +0x19D1C" in header
    assert ins[0x48fc0c] == ("lea", "ecx, [esi + 0x18684]")
    assert ins[0x48fc1c] == ("lea", "ecx, [esi + 0x19d1c]")
    assert ins[0x48fc2c] == ("lea", "ecx, [esi + 0x191d0]")

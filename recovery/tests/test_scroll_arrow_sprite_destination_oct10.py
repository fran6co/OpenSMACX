"""Original arrow-sprite producer destination, not full Scroll acceptance."""
from pathlib import Path
import json
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_scroll_default_and_icon_owner_closure_oct9 import _definition
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
BASE = "c013ef7f6e2828e1770caeadca9f29921af12538"
THUMB_MUTATOR_BASE = "106e28f429562f9aa025ea5f3162e76b99823c9b"


@pytest.mark.parametrize("wrong_destination", [False, True])
def test_actual_setter_writes_redraw_array_not_sound_words(tmp_path, wrong_destination):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    header = (ROOT / "src/flatbutton.h").read_text()
    setter = _definition(header, "void set_scroll_icons(")
    if wrong_destination:
        setter = setter.replace("field_AD4_", "sound_words_")
    source = """
#include <cassert>
struct Sprite { int tag; };
struct FlatButton {
  Sprite *sound_words_[3];
  Sprite *field_AD4_[3];
""" + setter + """
};
int main() {
  Sprite prior[3] = {{1},{2},{3}}, icons[3] = {{4},{5},{6}};
  FlatButton b = {{&prior[0],&prior[1],&prior[2]}, {0,0,0}};
  b.set_scroll_icons(&icons[0], &icons[1], &icons[2]);
  for (int i=0; i<3; ++i) {
    assert(b.field_AD4_[i] == &icons[i]);
    assert(b.sound_words_[i] == &prior[i]);
  }
}
"""
    path, binary = tmp_path / "setter.cpp", tmp_path / "setter"
    path.write_text(source)
    result = subprocess.run([compiler, str(path), "-o", str(binary)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=30)
    assert (result.returncode == 0) == (not wrong_destination)


def test_pinned_both_button_receivers_write_the_redraw_triplet():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    # Both sequences store EDX/EAX/ECX at edi+AD8/AD4/ADC, respectively.
    expected = bytes.fromhex("8997d80a00008987d40a0000898fdc0a0000")
    for address in (0x60569A, 0x6057A1):
        assert image.read(Span(address, address + len(expected))) == expected
    assert image.read(Span(0x605673, 0x605679)) == bytes.fromhex("8dbeac0a0000")


def test_pinned_geometry_mutator_spans_fields_branch_and_void_epilogues():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    spans = {
        0x605B80: (0x605BE0, {0xA1C, 0xA4C, 0xA50, 0xA54, 0xA58, 0xA60}, "4"),
        0x605B10: (0x605B74, {0xA1C, 0xA4C, 0xA50, 0xA54, 0xA58, 0xA60}, "4"),
        0x606EA0: (0x606EF8, {0xA1C, 0xA4C, 0xA50, 0xA54, 0xA58, 0xA60}, ""),
    }
    for start, (end, expected_offsets, ret_operand) in spans.items():
        span = Span(start, end)
        instructions = list(decoder.disasm(image.read(span), start))
        assert instructions and instructions[0].address == start
        assert instructions[-1].address + instructions[-1].size == end
        assert instructions[-1].mnemonic == "ret"
        assert instructions[-1].op_str == ret_operand
        displacements = {
            operand.mem.disp & 0xFFFFFFFF
            for instruction in instructions
            for operand in instruction.operands
                if operand.type == 3  # X86_OP_MEM
        }
        assert expected_offsets <= displacements
        # The border sentinel gates all four inset stores; unconditional
        # initialization precedes the compare in each original body.
        assert any(instruction.mnemonic == "cmp" and "-1" in instruction.op_str
                   for instruction in instructions)
        assert any(instruction.mnemonic == "je" for instruction in instructions)


def test_original_identities_claims_and_seed_history_survive():
    for filename in ("scroll.cpp", "win.cpp"):
        path = ROOT / "src" / filename
        prior = subprocess.check_output(["git", "show", BASE + ":src/" + filename],
                                        cwd=ROOT, text=True)
        before = {r.address: r for r in read_text(prior, path)}
        after = {r.address: r for r in read_text(path.read_text(), path)}
        # The separately reviewed menu-entry migration subsequently bound this
        # independent Win helper; its native writer/claim controls live in
        # test_win_menu_entry_ownership_oct11.py. No arbitrary additions allowed.
        additions = {0x5F0520} if filename == "win.cpp" else set()
        assert set(before) | additions == set(after)
        for address, row in before.items():
            assert _continuity_fields(row) == _continuity_fields(after[address])
            # Keep the earlier close/set_pos contracts and original historical
            # baseline; this task adds only the two authorized thumb mutators.
            migrations = {
                0x605370: ("?close@Scroll@@QAEIXZ", "?close@Scroll@@QAEXXZ"),
                0x605D20: ("?set_pos@Scroll@@QAEIH@Z", "?set_pos@Scroll@@QAEXH@Z"),
                0x605B80: ("?set_bar_thickness@Scroll@@QAEIH@Z", "?set_bar_thickness@Scroll@@QAEXH@Z"),
                0x606EA0: ("?set_thumb_rect@Scroll@@QAEIXZ", "?set_thumb_rect@Scroll@@QAEXXZ"),
            }
            if filename == "scroll.cpp" and address in migrations:
                assert (row.symbol, after[address].symbol) == migrations[address]
            else:
                assert row.symbol == after[address].symbol


def test_thumb_symbol_migrations_match_immutable_task_authorization():
    task = json.loads((ROOT / "recovery/tasks/scroll-thumb-mutator-contract-oct17.json").read_text())
    assert task["base_revision"] == THUMB_MUTATOR_BASE
    path = ROOT / "src/scroll.cpp"
    prior = subprocess.check_output(
        ["git", "show", THUMB_MUTATOR_BASE + ":src/scroll.cpp"], cwd=ROOT, text=True)
    before = {r.address: r for r in read_text(prior, path)}
    after = {r.address: r for r in read_text(path.read_text(), path)}
    assert before.keys() == after.keys()
    changes = {}
    for address, row in before.items():
        assert _continuity_fields(row) == _continuity_fields(after[address])
        if row.symbol != after[address].symbol:
            changes[address] = (row.symbol, after[address].symbol)
    assert changes == {
        0x605B80: ("?set_bar_thickness@Scroll@@QAEIH@Z", "?set_bar_thickness@Scroll@@QAEXH@Z"),
        0x606EA0: ("?set_thumb_rect@Scroll@@QAEIXZ", "?set_thumb_rect@Scroll@@QAEXXZ"),
    }
    subjects = {row["address"] for row in task["subjects"]}
    assert changes.keys() <= subjects


def test_producer_and_redraw_share_the_real_sprite_run():
    scroll = (ROOT / "src/scroll.cpp").read_text()
    init = _definition(scroll, "int Scroll::init(")
    for button in ("flat_button_left_", "flat_button_right_"):
        assert init.count(button + ".set_scroll_icons(") == 2
    redraw = _definition((ROOT / "src/flatbutton.cpp").read_text(),
                          "void FlatButton::on_redraw()")
    assert "Sprite *const fill_sprite = field_AD4_[state];" in redraw

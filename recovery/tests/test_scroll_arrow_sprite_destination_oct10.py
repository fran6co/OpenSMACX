"""Original arrow-sprite producer destination, not full Scroll acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_scroll_default_and_icon_owner_closure_oct9 import _definition
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
BASE = "c013ef7f6e2828e1770caeadca9f29921af12538"


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
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    # Both sequences store EDX/EAX/ECX at edi+AD8/AD4/ADC, respectively.
    expected = bytes.fromhex("8997d80a00008987d40a0000898fdc0a0000")
    for address in (0x60569A, 0x6057A1):
        assert image.read(Span(address, address + len(expected))) == expected
    assert image.read(Span(0x605673, 0x605679)) == bytes.fromhex("8dbeac0a0000")


def test_original_identities_claims_and_seed_history_survive():
    for filename in ("scroll.cpp", "win.cpp"):
        path = ROOT / "src" / filename
        prior = subprocess.check_output(["git", "show", BASE + ":src/" + filename],
                                        cwd=ROOT, text=True)
        before = {r.address: r for r in read_text(prior, path)}
        after = {r.address: r for r in read_text(path.read_text(), path)}
        assert set(before) == set(after)
        for address, row in before.items():
            assert _continuity_fields(row) == _continuity_fields(after[address])
            assert row.symbol == after[address].symbol


def test_producer_and_redraw_share_the_real_sprite_run():
    scroll = (ROOT / "src/scroll.cpp").read_text()
    init = _definition(scroll, "int Scroll::init(")
    for button in ("flat_button_left_", "flat_button_right_"):
        assert init.count(button + ".set_scroll_icons(") == 2
    redraw = _definition((ROOT / "src/flatbutton.cpp").read_text(),
                          "void FlatButton::on_redraw()")
    assert "Sprite *const fill_sprite = field_AD4_[state];" in redraw

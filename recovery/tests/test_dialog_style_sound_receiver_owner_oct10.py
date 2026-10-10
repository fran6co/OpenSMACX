"""Actual Dialog setters and bounded BasePop geometry receiver control.

The geometry branch is extracted from the real body, not a full modal/geometry
simulation. Host widths/layout and mock objects do not prove the original ABI,
linked preservation or GUI behavior. Native builds/measurements are separate.
"""
from pathlib import Path
import re

import pytest

from match_recovery.source.reader import read_file
from test_texture_sub61f0d0_actual_source import build_and_run, extract_method

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/basepop.cpp").is_file())


def test_setter_identities_live_only_in_actual_dialog_owner():
    old = read_file(ROOT / "src/basepop.cpp")
    current = read_file(ROOT / "src/dialog.cpp")
    for address, name, span in (
        (0x605180, "?UNK3@BasePop@@QAEXH@Z", (0x605180, 0x60519B)),
        (0x6051A0, "?UNK4@BasePop@@QAEXH@Z", (0x6051A0, 0x6051BB)),
    ):
        assert not any(row.address == address for row in old)
        rows = [row for row in current if row.address == address]
        assert len(rows) == 1 and rows[0].name == name
        assert rows[0].image_spans == (span,) and rows[0].byte_exact


HOST = r"""
#include <cassert>
#include <stdint.h>
struct Sound { int value; };
static uintptr_t AutoSoundDefaults[37];
class AutoSound {
protected:
    // SOUND_DECLARATION
public:
    void copy_word() { COPY_WORD }
    void reset_word() { RESET_WORD }
    Sound *sound() const { return val_8_; }
};
class Dialog {
public:
    unsigned int field_20_;
    int field_2C_,field_30_;
    void set_style_bit_0(int);
    void set_style_bit_1(int);
};
struct Dialogs : public virtual Dialog {};
class BasePop : public AutoSound {
public:
    Dialogs dialogs_;
    void *external_window_;
    int dialog_width_,field_30E0_,field_21B4_,dialog_height_;
    int field_30B0_,field_30D0_,field_30EC_,field_30E8_,field_30A8_;
    int default_loc_a_,default_loc_b_,loc_a_,loc_b_;
    void geometry_branch() {
        const int border_x=7;
        // GEOMETRY
    }
};
static int WinScreenWidth=1200;
// SETTERS
int main() {
    Sound sound={8}; BasePop p;
    AutoSoundDefaults[7]=reinterpret_cast<uintptr_t>(&sound);
    p.copy_word(); assert(p.sound()==&sound);
    p.dialogs_.field_20_=0xABCDEF00u;
    p.dialogs_.set_style_bit_0(1); p.dialogs_.set_style_bit_1(1);
    assert(p.dialogs_.field_20_==0xABCDEF03u && p.sound()==&sound);
    p.dialogs_.set_style_bit_0(0); p.dialogs_.set_style_bit_1(0);
    assert(p.dialogs_.field_20_==0xABCDEF00u && p.sound()==&sound);
    p.external_window_=&sound; p.dialog_width_=500; p.field_30E0_=3;
    p.field_21B4_=4; p.dialog_height_=400; p.field_30B0_=1;
    p.field_30D0_=10; p.field_30EC_=5; p.field_30E8_=6;
    p.field_30A8_=0; p.default_loc_a_=27; p.default_loc_b_=0x2000;
    p.loc_a_=0; p.loc_b_=31;
    p.geometry_branch();
    assert(p.dialogs_.field_2C_==483 && p.dialogs_.field_30_==379);
    assert(p.loc_a_==27 && p.loc_b_==31);
    assert(p.dialogs_.field_20_==0xABCDEF03u && p.sound()==&sound);
    p.external_window_=0; p.dialogs_.field_20_=0xABCDEF00u;
    p.geometry_branch(); assert(p.dialogs_.field_20_==0xABCDEF00u);
    p.reset_word(); assert(p.sound()==0);
    AutoSoundDefaults[7]=0; p.copy_word(); assert(p.sound()==0);
}
"""


def unit(damage=None):
    header = (ROOT / "src/autosound.h").read_text()
    declarations = re.findall(r"^\s*(Sound \*val_8_;)", header, re.M)
    assert len(declarations) == 1
    auto = (ROOT / "src/autosound.cpp").read_text()
    stores = re.findall(r"^\s*(val_8_ = [^;]+;)", auto, re.M)
    assert len(stores) == 4 and stores[:3] == [stores[0]] * 3
    body = extract_method((ROOT / "src/basepop.cpp").read_text(),
                          "int BasePop::calculate_dimensions()")
    prefix = body[:body.index("    int result = dialogs_.calculate_dimensions();")]
    branch = prefix[prefix.rindex("    if (external_window_ != 0) {"):]
    dialog = (ROOT / "src/dialog.cpp").read_text()
    setters = "\n".join(extract_method(dialog, "void Dialog::set_style_bit_" + str(n) + "(int on)")
                        for n in (0, 1))
    source = HOST.replace("// SOUND_DECLARATION", declarations[0]).replace("COPY_WORD", stores[0])
    source = source.replace("RESET_WORD", stores[3]).replace("// GEOMETRY", branch).replace("// SETTERS", setters)
    # The original full body sees this global from win.h; the fixture keeps
    # that declaration visible before its extracted inline branch too.
    source = source.replace("class BasePop :", "extern int WinScreenWidth;\nclass BasePop :")
    source = source.replace("static int WinScreenWidth=1200;", "int WinScreenWidth=1200;")
    if damage:
        old, new = damage
        assert old in source
        source = source.replace(old, new, 1)
    return source


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_disjoint_dialog_style_and_sound_receiver(compiler):
    build_and_run(unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("dialogs_.Dialog::set_style_bit_1(1);", "val_8_ = reinterpret_cast<Sound *>(2);"),
    ("field_20_ &= ~1U;", "field_20_ = 0;"),
    ("field_20_ |= 2U;", "field_20_ |= 1U;"),
    ("val_8_ = 0;", "(void)0;"),
))
def test_rejects_wrong_receiver_or_word_damage(compiler, old, new):
    build_and_run(unit((old, new)), compiler, expect_success=False)

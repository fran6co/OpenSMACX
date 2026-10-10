"""Actual callback bodies and declarations exercised through the Win interface."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_game_logic_seed_batch_behavior import _definition
from test_popup_scroll_member_geometry import projected_control

ROOT = Path(__file__).resolve().parents[2]


def declaration(owner, method):
    text = (ROOT / f"src/{owner.lower()}.h").read_text()
    return re.search(r"^\s*Scroll \*" + method + r"\([^\n]*\) override;", text, re.M).group(0)


def control(damage=None):
    text = projected_control()
    win = (ROOT / "src/win.h").read_text()
    defaults = "\n".join(re.findall(
        r"virtual Scroll \*on_listbox_scroll_(?:create|delete)\([^\n]*?\}\s*", win))
    assert defaults.count("virtual Scroll") == 2
    base = "struct Win { bool visible_; explicit Win(bool v=false):visible_(v){} bool is_visible(){return visible_;}\n" + defaults + "};\n"
    text = text.replace("class Popup:public Buffer {", base + "class Popup:public Buffer, public Win {")
    text = text.replace("field_30AC_(mode),visible_(visible)", "Win(visible),field_30AC_(mode),visible_(visible)")
    text = text.replace("Scroll *on_listbox_scroll_create();", declaration("Popup", "on_listbox_scroll_create") + "\n" + declaration("Popup", "on_listbox_scroll_delete") + "\nScroll *owned_scroll(){return &scroll_;}")
    text = text.replace("// ACTUAL_SOURCE_BODY", "")
    extra = "void scroll_hide(Win*,Scroll*) { ++log_state.show; }\n"
    extra += _definition((ROOT / "src/popup.cpp").read_text(), "Scroll *Popup::on_listbox_scroll_delete(")
    extra += "\nstruct AlphaSave : Win { Scroll scroll_; explicit AlphaSave(bool v):Win(v){} void scroll_normal(){++log_state.normal;} Scroll *owned_scroll(){return &scroll_;}\n"
    extra += declaration("AlphaSave", "on_listbox_scroll_create") + "\n" + declaration("AlphaSave", "on_listbox_scroll_delete") + "\n};\n"
    extra += "void scroll_show(AlphaSave*,Scroll*) { ++log_state.show; }\n"
    for method in ("create", "delete"):
        extra += _definition((ROOT / "src/alphasave.cpp").read_text(), "Scroll *AlphaSave::on_listbox_scroll_" + method + "(") + "\n"
    text = text[:text.index("int main() {")] + extra + r'''
int main() {
    Scroll input; Win base; Win *parent=&base;
    assert(parent->on_listbox_scroll_create()==0);
    assert(parent->on_listbox_scroll_delete(&input)==&input);
    MainInterfaceGlobal.scroll_frame_sprite(0).set_height(13);
    MainInterfaceGlobal.scroll_frame_sprite(1).set_height(23);
    MainInterfaceGlobal.scroll_frame_sprite(2).set_height(17);
    Popup quiet(0,false); parent=&quiet;
    assert(parent->on_listbox_scroll_create()==quiet.owned_scroll());
    assert(log_state.init==1 && log_state.hide==1 && log_state.show==0);
    assert(parent->on_listbox_scroll_delete(&input)==0 && log_state.show==0);
    Popup visible(0,true); parent=&visible;
    assert(parent->on_listbox_scroll_create()==visible.owned_scroll());
    assert(log_state.show==1);
    assert(parent->on_listbox_scroll_delete(&input)==0 && log_state.show==2);
    Popup bypass(10000000,true); parent=&bypass;
    assert(parent->on_listbox_scroll_create()==0);
    assert(parent->on_listbox_scroll_delete(&input)==&input);
    assert(log_state.init==2 && log_state.show==2);
    AlphaSave save(false); parent=&save;
    assert(parent->on_listbox_scroll_create()==save.owned_scroll());
    assert(log_state.left==0x147 && log_state.width==0x18 && log_state.height==0xD8);
    assert(parent->on_listbox_scroll_delete(&input)==0 && log_state.show==2);
    AlphaSave shown(true); parent=&shown;
    assert(parent->on_listbox_scroll_create()==shown.owned_scroll());
    assert(parent->on_listbox_scroll_delete(&input)==0 && log_state.show==4);
    assert(log_state.normal==4 && log_state.init==4 && log_state.hide==4);
}
'''
    if damage:
        old, new = damage
        assert old in text
        text = text.replace(old, new, 1)
    return text


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("return &scroll_;", "return 0;"),
    ("return scroll;", "return 0;"),
    ("scroll_.init(0x147", "scroll_.init(0x148")))
def test_actual_virtual_callbacks(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "callbacks.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "callbacks"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], check=True, capture_output=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)

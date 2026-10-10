"""Bounded actual-source reset controls, not whole-close or native ABI proof."""

from pathlib import Path
import re

import pytest

from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, ("poWinBase_ = this;", "poWinBase_ = 0;")))
def test_actual_close_reset_preserves_receiver_identity(tmp_path, compiler, damage):
    source = (ROOT / "src/win.cpp").read_text()
    close = extract(source, "void Win::close()")
    assignments = re.findall(r"\bpoWinBase_\s*=\s*[^;]+;", close)
    assert assignments == ["poWinBase_ = this;"]
    header = (ROOT / "src/win.h").read_text()
    declaration = re.findall(r"^\s*(Win \*poWinBase_;)", header, re.M)
    assert declaration == ["Win *poWinBase_;"]
    # Extract the actual consecutive reset writes, with unrelated reset state
    # represented by four-byte host fields. No teardown/callback proof follows.
    start = close.index("    win_parent_ = 0;")
    end = close.index("    cursor_sprite_ = 0;", start)
    reset = close[start:end]
    control = r'''
#include <cassert>
struct Win {
    Win *win_parent_, *poWinBase_;
    char *cursor_name_;
    unsigned iVertScaleDenom_, iVertScaleNum_, field_134_, field_138_;
    void reset() { BODY }
};
int main() {
    Win windows[3] = {};
    for (int i=0; i<3; ++i) {
        Win &w=windows[i];
        w.win_parent_=&windows[(i+1)%3];
        w.poWinBase_=&windows[(i+2)%3];
        char caption[]="caption";w.cursor_name_=caption;
        w.iVertScaleDenom_=w.iVertScaleNum_=w.field_134_=w.field_138_=~0u;
        w.reset();
        assert(w.poWinBase_==&w);
        assert(!w.win_parent_ && !w.cursor_name_);
        assert(!w.iVertScaleDenom_ && !w.iVertScaleNum_);
        assert(!w.field_134_ && !w.field_138_);
    }
}
'''.replace("BODY", reset)
    run_control(tmp_path, compiler, control, damage)

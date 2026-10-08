"""Application font policy ownership; not original layout/runtime acceptance."""

from pathlib import Path
import re
import subprocess


ROOT = Path(__file__).resolve().parents[2]
BASE = '3c512264649a20403a84d8dd2821c10a1972e6ae'


def test_popup_font_bindings_and_storage_are_preserved_in_application_owner():
    old = subprocess.check_output(
        ['git', 'show', BASE + ':src/font.cpp'], cwd=ROOT).decode()
    pattern = re.compile(
        r'// RECOVERY_OBJECT: v1 [^\n]+\nFont \*(?:Popup|Interlude)[^\n]+')
    bindings = pattern.findall(old)
    assert len(bindings) == 20
    popup = (ROOT / 'src/popup.cpp').read_text()
    font = (ROOT / 'src/font.cpp').read_text()
    for binding in bindings:
        assert popup.count(binding) == 1
        assert binding not in font


def test_generic_font_no_longer_publishes_application_policy_caches():
    font_header = (ROOT / 'src/font.h').read_text()
    policy = (ROOT / 'src/popup_fonts.h').read_text()
    names = re.findall(r'extern Font \*((?:Popup|Interlude)\w+);', policy)
    assert len(names) == len(set(names)) == 20
    for name in names:
        assert name not in font_header
    for consumer in ('popup', 'alphamenu', 'factionprofilewin', 'interlude', 'newgame'):
        assert '#include "popup_fonts.h"' in (ROOT / f'src/{consumer}.cpp').read_text()

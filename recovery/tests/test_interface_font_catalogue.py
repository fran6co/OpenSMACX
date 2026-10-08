"""Source ownership/continuity controls, not native lifetime acceptance."""

from pathlib import Path
import json
import subprocess

from match_recovery.source import read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
BASE = 'e5d7f3ee9526578c1b078d88ebadbc811e602d17'


def test_lookup_mapping_and_complete_body_move_without_a_new_claim():
    old_text = subprocess.check_output(
        ['git', 'show', BASE + ':src/font.cpp'], cwd=ROOT).decode()
    old, = (r for r in read_text(old_text, ROOT / 'src/font.cpp')
            if r.address == 0x5882F0)
    current, = (r for r in read_file(ROOT / 'src/interface_fonts.cpp')
                if r.address == old.address)
    for attr in ('name', 'image_spans', 'byte_exact', 'semantic', 'levers',
                 'ruled_out', 'unrecoverable', 'deferred'):
        assert getattr(old, attr) == getattr(current, attr)
    start = old_text.index('Font *__cdecl find_font(')
    end = old_text.index('\n}\n', start) + 2
    assert old_text[start:end] in (ROOT / 'src/interface_fonts.cpp').read_text()
    assert not any(r.address == old.address for r in read_file(ROOT / 'src/jackal/font.cpp'))


def test_application_catalogue_is_not_a_generic_font_dependency():
    app = (ROOT / 'src/interface_fonts.cpp').read_text()
    app_header = (ROOT / 'src/interface_fonts.h').read_text()
    generic = (ROOT / 'src/jackal/font.cpp').read_text()
    generic_header = (ROOT / 'src/jackal/font.h').read_text()
    for name in ('FontTable', 'FontSizeTable', 'FontStyleTable', 'find_font',
                 'DefaultFontFace', 'DefaultLargeFontFace', 'g_JACKAL_FONT'):
        assert name in app and name in app_header
        assert name not in generic and name not in generic_header
    assert app.index('Font FontTable[48];') < app.index('Font g_JACKAL_FONT;')
    assert 'src/interface_fonts.cpp' in (ROOT / 'CMakeLists.txt').read_text()
    assert '#include "interface_fonts.h"' not in (ROOT / 'src/jackal/jackal.cpp').read_text()


def test_generic_font_retains_identities_and_explicit_archive_membership():
    old_text = subprocess.check_output(
        ['git', 'show', 'daf92e54129454df352b0cc2eadc5fdd6c0721cf:src/font.cpp'], cwd=ROOT).decode()
    old = read_text(old_text, ROOT / 'src/font.cpp')
    current = read_file(ROOT / 'src/jackal/font.cpp')
    def identities(rows):
        return {r.address: (r.name, r.image_spans, r.byte_exact, r.semantic,
                           r.levers, r.ruled_out, r.unrecoverable, r.deferred)
                for r in rows}
    assert len(old) == 13
    assert identities(old) == identities(current)
    assert not (ROOT / 'src/font.cpp').exists()
    assert not (ROOT / 'src/font.h').exists()
    build = json.loads((ROOT / 'recovery/build.json').read_text())
    assert {'archive': 'src/jackal/jackal.lib',
            'object': 'src/jackal/CMakeFiles/jackal.dir/font.cpp.obj',
            'member': 'src/jackal/CMakeFiles/jackal.dir/font.cpp.obj'} in build['compile_link']['archive_members']
    project = json.loads((ROOT / 'recovery/project.json').read_text())
    assert project['measurement']['unit_flag_sets']['src/jackal/font.cpp'] == [0]

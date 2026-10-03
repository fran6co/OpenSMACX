"""Actual announcement-member/helpers controls, not full-game acceptance."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def definition(text, signature):
    text = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.S)
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError('unterminated source definition')


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_announcement_storage_and_helpers(tmp_path, compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip('host compiler unavailable')
    header = (ROOT / 'src/faction.h').read_text()
    start = header.index('    uint8_t facility_announced[')
    end = header.index('    int base_id_atk_target;', start)
    fields = header[start:end]
    base = (ROOT / 'src/base.cpp').read_text()
    getter = definition(base, 'BOOL __cdecl has_fac_announced(')
    setter = definition(base, 'void __cdecl set_fac_announced(')
    general = (ROOT / 'src/general.h').read_text()
    mask = definition(general, 'MEASURED inline void __cdecl bitmask(')
    faction = (ROOT / 'src/faction.cpp').read_text()
    reset_start = faction.index('        for (int announced = 0;')
    reset_end = faction.index('        for (int satellite = 0;', reset_start)
    reset = faction[reset_start:reset_end]
    program = r'''
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#define __cdecl
#define MEASURED
typedef int BOOL;
const int FacilitySPStart = 70;
struct PlayerData {
FIELDS
};
PlayerData PlayersData[8];
MASK
void bitmask_call(int input, int *offset, int *mask) { bitmask(input, offset, mask); }
GETTER
SETTER
void reset(PlayerData &p) {
RESET
}
int main() {
    assert(sizeof(PlayersData[0].facility_announced) == 9);
    assert(offsetof(PlayerData, announcement_reserved) == 9);
    assert(offsetof(PlayerData, planet_ecology) == 12);
    for (int faction = 0; faction < 8; ++faction) {
        memset(&PlayersData[faction], 0xA5, sizeof(PlayerData));
        reset(PlayersData[faction]);
        for (int i = 0; i < 9; ++i) assert(PlayersData[faction].facility_announced[i] == 0);
        for (int i = 0; i < 3; ++i) assert(PlayersData[faction].announcement_reserved[i] == 0xA5);
        unsigned char ecology[sizeof(int)];
        memcpy(ecology, &PlayersData[faction].planet_ecology, sizeof(int));
        for (unsigned int i = 0; i < sizeof(int); ++i) assert(ecology[i] == 0xA5);
    }
    for (int faction = 0; faction < 8; ++faction) {
        for (int id = 0; id <= 70; ++id) {
            assert(!has_fac_announced(faction, id));
            set_fac_announced(faction, id, 1);
            for (int other = 0; other <= 70; ++other)
                assert(!!has_fac_announced(faction, other) == (other == id));
            for (int other_faction = 0; other_faction < 8; ++other_faction)
                if (other_faction != faction) assert(!has_fac_announced(other_faction, id));
            set_fac_announced(faction, id, 0);
            assert(!has_fac_announced(faction, id));
        }
        assert(has_fac_announced(faction, 71));
        for (int i = 0; i < 3; ++i) assert(PlayersData[faction].announcement_reserved[i] == 0xA5);
    }
}
'''
    for key, value in [('FIELDS', fields), ('MASK', mask), ('GETTER', getter),
                       ('SETTER', setter), ('RESET', reset)]:
        program = program.replace(key, value)
    source = tmp_path / 'announcement.cpp'
    source.write_text(program)
    binary = tmp_path / 'announcement'
    subprocess.run([executable, '-std=c++98', '-Wall', '-Wextra', '-Werror',
                    '-fsanitize=undefined,bounds', '-fno-sanitize-recover=all',
                    str(source), '-o', str(binary)], check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)

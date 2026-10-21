"""Actual-source controls for two pinned communicate adjective producers.

These host tests exercise the mapped source branches, not a reconstructed
copy. They establish bounded producer/slot/order behavior only; they do not
claim native ABI, runtime, or whole-game acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / 'src/faction_dialogue.cpp').exists())


def _braced(source, anchor):
    start = source.index(anchor)
    opening = source.index('{', start)
    masked = re.sub(
        r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
        lambda match: ' ' * len(match.group()), source, flags=re.S)
    depth = 0
    for end in range(opening, len(source)):
        depth += (masked[end] == '{') - (masked[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated source block at ' + anchor)


def _actual_blocks():
    path = ROOT / 'src/faction_dialogue.cpp'
    rows = [row for row in read_file(path) if row.address == 0x54FFD0]
    assert len(rows) == 1
    row = rows[0]
    assert row.name == '?communicate@@YAXHHH@Z'
    assert row.image_spans == ((0x54FFD0, 0x558980), (0x65FF4F, 0x65FF64))
    assert not row.extract_error and 'BODY GOES HERE.' not in row.region
    even_anchor = 'if (communicate_late_contact_region_reported == 0) {'
    territory_anchor = ('if (territory(peer_id, faction_id, 0,\n'
                        '                  &communicate_late_contact_base_id, 0) != 0) {')
    return (_braced(row.region, even_anchor),
            _braced(row.region, territory_anchor))


HARNESS = r'''#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

struct PlayerData { unsigned char region_total_bases[128]; };
struct Base { int x, y; };
static std::vector<std::string> events;
struct MapWin {
    void focus(int x, int y) {
        char text[80]; std::sprintf(text, "focus:%d:%d", x, y); events.push_back(text);
    }
};
static PlayerData PlayersData[8];
static Base Bases[16];
static unsigned int DiplomacyRegionMask[128];
static int FactionDiplomacyPortraits[8];
static int pop_wait = 17;
static int faction_id = 2;
static int peer_id = 4;
static int communicate_late_contact_region = 0;
static int communicate_late_contact_region_reported = 0;
static int communicate_late_contact_region_rival = 3;
static int communicate_late_contact_base_id = -1;
static int g_territory_result = 1;

static void event(const char *text) { events.push_back(text); }
static const char *named(const char *kind, int id) {
    static char value[64]; std::sprintf(value, "%s%d", kind, id); return value;
}
const char *get_adjective(int id) { return named("adj", id); }
const char *get_title(int id) { return named("title", id); }
const char *get_name(int id) { return named("name", id); }
const char *get_noun(int id) { return named("noun", id); }
const char *get_noun_phrase(int id, int form) {
    static char value[64]; std::sprintf(value, "nounphrase%d:%d", id, form); return value;
}
const char *get_pact(int id) { return named("pact", id); }
int bad_reg(int) { return 0; }
void parse_says(int slot, const char *value, int fallback_a, int fallback_b) {
    char text[128];
    std::sprintf(text, "parse:%d:%s:%d:%d", slot, value, fallback_a, fallback_b);
    event(text);
}
void X_pops(const char *key, unsigned int flags, int portrait, int wait) {
    char text[128]; std::sprintf(text, "popup:%s:%u:%d:%d", key, flags, portrait, wait);
    event(text);
}
void spot_base(int base_id, int actor) {
    char text[80]; std::sprintf(text, "spot:%d:%d", base_id, actor); event(text);
}
MapWin *console_map_win() { static MapWin map; return &map; }
void mention_prototypes(int actor, int peer) {
    char text[80]; std::sprintf(text, "mention:%d:%d", actor, peer); event(text);
}
int territory(int peer, int actor, int mode, int *base_id, int flags) {
    char text[80]; std::sprintf(text, "territory:%d:%d:%d:%d", peer, actor, mode, flags);
    event(text);
    if (g_territory_result) *base_id = 9;
    return g_territory_result;
}

static void expect(const char *const *wanted, int count) {
    assert(static_cast<int>(events.size()) == count);
    for (int i = 0; i < count; ++i) assert(events[i] == wanted[i]);
}
static void reset() { events.clear(); }

static void report_active() {
    communicate_late_contact_region = 0;
    communicate_late_contact_region_reported = 0;
    communicate_late_contact_region_rival = 3;
    PlayersData[peer_id].region_total_bases[1] = 1;
    PlayersData[communicate_late_contact_region_rival].region_total_bases[1] = 1;
    /*ACTUAL_REPORT_BLOCK*/
    const char *wanted[] = {
        "parse:1:title3:-1:-1", "parse:2:name3:-1:-1",
        "parse:3:noun3:-1:-1", "parse:4:adj3:-1:-1",
        "parse:5:nounphrase3:8:-1:-1", "popup:REPORTEVEN:1048576:44:17"
    };
    expect(wanted, 6);
    assert(communicate_late_contact_region == 1);
}
static void report_skipped() {
    reset();
    communicate_late_contact_region = 0;
    communicate_late_contact_region_reported = 0;
    communicate_late_contact_region_rival = 3;
    PlayersData[peer_id].region_total_bases[1] = 0;
    PlayersData[communicate_late_contact_region_rival].region_total_bases[1] = 1;
    /*ACTUAL_REPORT_BLOCK*/
    assert(events.empty());
}
static void territory_success() {
    reset(); g_territory_result = 1; communicate_late_contact_base_id = -1;
    Bases[9].x = 11; Bases[9].y = 12;
    FactionDiplomacyPortraits[peer_id] = 44;
    /*ACTUAL_TERRITORY_BLOCK*/
    const char *wanted[] = {
        "territory:4:2:0:0", "spot:9:2", "focus:11:12",
        "parse:1:adj4:-1:-1", "popup:TERRITORY:1048576:44:17", "mention:2:4"
    };
    expect(wanted, 6);
}
static void territory_failure() {
    reset(); g_territory_result = 0; communicate_late_contact_base_id = -1;
    /*ACTUAL_TERRITORY_BLOCK*/
    const char *wanted[] = { "territory:4:2:0:0" };
    expect(wanted, 1);
    assert(communicate_late_contact_base_id == -1);
}
int main() {
    FactionDiplomacyPortraits[peer_id] = 44;
    report_active(); report_skipped(); territory_success(); territory_failure();
    std::puts("communicate adjective producer paths: 4 passed");
    return 0;
}
'''


def _candidate(damage):
    report, territory = _actual_blocks()
    if damage == 'omit-even-adjective':
        exact = re.compile(r'\s*parse_says\(4,\s*get_adjective\(\s*'
                           r'communicate_late_contact_region_rival\), -1, -1\);')
        report, count = exact.subn('', report)
        assert count == 1
    elif damage == 'wrong-even-rival':
        old = 'get_adjective(\n                                  communicate_late_contact_region_rival)'
        assert report.count(old) == 1
        report = report.replace(old, 'get_adjective(faction_id)')
    elif damage == 'wrong-even-slot':
        old = 'parse_says(4, get_adjective('
        assert report.count(old) == 1
        report = report.replace(old, 'parse_says(0, get_adjective(')
    elif damage == 'territory-pact':
        old = 'parse_says(1, get_adjective(peer_id), -1, -1);'
        assert territory.count(old) == 1
        territory = territory.replace(old, 'parse_says(1, get_pact(peer_id), -1, -1);')
    source = HARNESS.replace('/*ACTUAL_REPORT_BLOCK*/', report)
    source = source.replace('/*ACTUAL_TERRITORY_BLOCK*/', territory)
    assert '/*ACTUAL_' not in source
    return source


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['none', 'omit-even-adjective', 'wrong-even-rival',
                                    'wrong-even-slot', 'territory-pact'])
def test_actual_communicate_adjective_producers(compiler, damage, tmp_path):
    command = shutil.which(compiler)
    if command is None:
        pytest.skip('host compiler unavailable')
    cpp, binary = tmp_path / 'actual.cpp', tmp_path / 'actual'
    cpp.write_text(_candidate(damage))
    built = subprocess.run([command, '-std=c++98', '-O0', '-Wall', '-Wextra',
                            str(cpp), '-o', str(binary)], capture_output=True,
                           text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (run.returncode == 0) == (damage == 'none'), run.stdout + run.stderr
    if damage == 'none':
        assert 'communicate adjective producer paths: 4 passed' in run.stdout

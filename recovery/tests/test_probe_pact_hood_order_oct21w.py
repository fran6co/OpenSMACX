"""Bounded actual-source witness for Probe's original pact-hood operand order.

The indexed table is a valid-row test double that records order. It documents
the original unchecked read sequence; it does not execute invalid indices or
claim ABI/runtime equivalence.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / 'src/probe.cpp').exists())


def _braced(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    masked = re.sub(
        r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
        lambda match: ' ' * len(match.group()), source, flags=re.S)
    depth = 0
    for end in range(opening, len(source)):
        depth += (masked[end] == '{') - (masked[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated source body: ' + signature)


def _actual_inputs():
    probe_path = ROOT / 'src/probe.cpp'
    game_path = ROOT / 'src/game.cpp'
    probe_rows = [row for row in read_file(probe_path) if row.address == 0x59F120]
    pact_rows = [row for row in read_file(game_path) if row.address == 0x58FB00]
    assert len(probe_rows) == len(pact_rows) == 1
    probe_row, pact_row = probe_rows[0], pact_rows[0]
    assert probe_row.name == '?probe@@YAHHHHH@Z'
    assert probe_row.image_spans == ((0x59F120, 0x5A5837), (0x6613D0, 0x661B3D))
    assert not probe_row.extract_error and 'BODY GOES HERE.' not in probe_row.region
    assert pact_row.name == '?get_pact_hood@@YAPADHH@Z'
    assert pact_row.image_spans == ((0x58FB00, 0x58FB61),)
    assert not pact_row.extract_error and 'BODY GOES HERE.' not in pact_row.region

    probe_body = _braced(probe_row.region, 'int __cdecl probe(int actor_vehicle_id, int target_base_id,')
    expression_pattern = re.compile(
        r'parse_says\(2,\s*get_pact_hood\(\s*actor_faction_id,\s*'
        r'selected_response_faction\s*\),\s*-1,\s*-1\);')
    expressions = expression_pattern.findall(probe_body)
    assert len(expressions) == 1
    helper = _braced(pact_row.region,
                     'char *__cdecl get_pact_hood(int faction_id, int other_id) {')
    return helper, expressions[0]


HARNESS = r'''#include <cstdio>
#include <vector>

#define __cdecl

typedef char *LPSTR;
struct Player { int is_leader_female; };
struct PlayerTable {
    Player values[8];
    std::vector<int> reads;
    Player &operator[](int index) { reads.push_back(index); return values[index]; }
};
static PlayerTable Players;
static int GenderDefault = 71;
static int PluralityDefault = 82;
struct LabelOwner { char *labels[256]; char **strings_ptr; };
static LabelOwner Labels;
struct StringTableOwner { char *get(char *value) { return value; } };
static StringTableOwner StringTable;
static int parse_count;
static int parsed_slot, parsed_fallback_a, parsed_fallback_b;
static char *parsed_value;

/*ACTUAL_GET_PACT_HOOD*/

static void parse_says(int slot, char *value, int fallback_a, int fallback_b) {
    ++parse_count; parsed_slot = slot; parsed_value = value;
    parsed_fallback_a = fallback_a; parsed_fallback_b = fallback_b;
}
static int invoke_actual_probe_expression(int actor_value,
                                          int selected_response_value) {
    const int actor_faction_id = actor_value;
    const int selected_response_faction = selected_response_value;
    /*ACTUAL_PROBE_EXPRESSION*/
    return 0;
}

static bool check_pair(int actor_female, int selected_female) {
    const int actor = 2;
    const int selected = 5;
    Players.values[actor].is_leader_female = actor_female;
    Players.values[selected].is_leader_female = selected_female;
    Players.reads.clear();
    GenderDefault = 71; PluralityDefault = 82;
    parse_count = 0; parsed_slot = -1; parsed_value = 0;
    parsed_fallback_a = 19; parsed_fallback_b = 23;
    if (invoke_actual_probe_expression(actor, selected) != 0) return false;

    std::vector<int> expected_reads;
    expected_reads.push_back(actor);
    if (actor_female) expected_reads.push_back(selected);
    if (Players.reads != expected_reads) return false;

    const bool both_female = actor_female && selected_female;
    if (parse_count != 1 || parsed_slot != 2 ||
        parsed_value != Labels.labels[both_female ? 0xce : 0xcd] ||
        parsed_fallback_a != -1 || parsed_fallback_b != -1 ||
        GenderDefault != 0 || PluralityDefault != 0) return false;
    return true;
}
int main() {
    static char pact_plural[] = "pact-plural";
    static char pact_singular[] = "pact-singular";
    Labels.labels[0xcd] = pact_plural;
    Labels.labels[0xce] = pact_singular;
    Labels.strings_ptr = Labels.labels;
    if (!check_pair(0, 0) || !check_pair(0, 1) ||
        !check_pair(1, 0) || !check_pair(1, 1)) return 4;
    std::puts("actual pact-hood source: 4 valid gender pairs passed");
    return 0;
}
'''


def _candidate(reverse_arguments):
    helper, expression = _actual_inputs()
    if reverse_arguments:
        replacement, count = re.subn(
            r'get_pact_hood\(\s*actor_faction_id,\s*selected_response_faction\s*\)',
            'get_pact_hood(selected_response_faction, actor_faction_id)',
            expression)
        assert count == 1
        expression = replacement
    source = HARNESS.replace('/*ACTUAL_GET_PACT_HOOD*/', helper)
    source = source.replace('/*ACTUAL_PROBE_EXPRESSION*/', expression)
    assert '/*ACTUAL_' not in source
    return source


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('reverse_arguments', [False, True],
                         ids=['pinned-order', 'compile-success-reversal'])
def test_actual_probe_pact_hood_order(compiler, reverse_arguments, tmp_path):
    command = shutil.which(compiler)
    if command is None:
        pytest.skip('host compiler unavailable')
    cpp, binary = tmp_path / 'actual.cpp', tmp_path / 'actual'
    cpp.write_text(_candidate(reverse_arguments))
    built = subprocess.run([command, '-std=c++98', '-O0', '-Wall', '-Wextra',
                            str(cpp), '-o', str(binary)], capture_output=True,
                           text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (run.returncode == 0) == (not reverse_arguments), run.stdout + run.stderr
    if not reverse_arguments:
        assert 'actual pact-hood source: 4 valid gender pairs passed' in run.stdout

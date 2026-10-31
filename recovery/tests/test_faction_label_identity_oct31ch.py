"""Original object references and actual log arguments; no whole-game proof."""
from pathlib import Path
from hashlib import sha256
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
LABELS = {'S': ('Sending TREATY ON', 'DiploLabelSendingTREATYON', 0x68D444),
          'L': ('Getting DIPLOMATIC LOCK', 'DiploLabelGettingDIPLOMATICLOCK', 0x68D554)}
PATTERNS = {'faction.cpp': 'SSSSLLLLLSLSLSLLSSLSSLS',
            'faction_dialogue.cpp': 'LLLLSSSSLLS', 'faction_network.cpp': 'S'}
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'


def calls(source):
    return list(re.finditer(
        r'log_say\((?:"Sending TREATY ON"|"Getting DIPLOMATIC LOCK"|'
        r'DiploLabelSendingTREATYON|DiploLabelGettingDIPLOMATICLOCK)\s*,[^;]+\);',
        source))


def definition(source, signature):
    start = source.index(signature)
    index = source.index('{', start)
    depth = 1
    end = index + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def test_original_immediates_and_existing_named_storage():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86_const import X86_OP_IMM
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source import read_file
    image_path = Path(os.environ['OPENSMACX_IMAGE'])
    raw = image_path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    addresses = {v[2] for v in LABELS.values()}
    references = []
    for unit in PATTERNS:
        source = (ROOT / 'src' / unit).read_text()
        matches = calls(source)
        assert len(matches) == len(PATTERNS[unit])
        for match, kind in zip(matches, PATTERNS[unit]):
            assert match.group().startswith('log_say(' + LABELS[kind][1] + ',')
        for record in read_file(ROOT / 'src' / unit):
            for lo, hi in record.image_spans:
                for instruction in decoder.disasm(image.read(Span(lo, hi)), lo):
                    for operand in instruction.operands:
                        if operand.type == X86_OP_IMM and operand.imm in addresses:
                            assert instruction.imm_size == 4
                            assert instruction.address + instruction.imm_offset in image.relocations
                            references.append((record.address, instruction.address, operand.imm))
    assert len(references) == 39
    assert {0x5391C0, 0x53CAF0, 0x54FFD0, 0x541100} <= {r[0] for r in references}
    for text, name, address in LABELS.values():
        assert image.read(Span(address, address + len(text) + 1)) == text.encode() + b'\0'
        flags = [flags for (span, _, size), flags in zip(image.sections, image.section_flags)
                 if span.start <= address < span.start + size]
        assert len(flags) == 1 and flags[0] & 0x80000000
        assert re.search(r'^char ' + name + r'\[\] = "' + text + r'";',
                         (ROOT / 'src/faction.cpp').read_text(), re.M)
        assert 'extern char ' + name + '[];' in (ROOT / 'src/faction.h').read_text()


COMMON = r'''
#include <cassert>
#include <cstring>
#include <vector>
#define __cdecl
EXTERNS
extern char OtherSending[];
extern int IsMultiplayerNet;
void expect_label(char);
void log_say(const char*,int,int,int);
int message_data(int,int,int,int,int,int);
void treaty_on(int,int,int);
struct Net { void await_diplo(int); };
extern Net NetDaemonNet;
'''

MAIN = r'''
#include "fixture.h"
DEFINITIONS
// A different writable array with identical bytes is still the wrong object.
char OtherSending[]="Sending TREATY ON";
int IsMultiplayerNet;Net NetDaemonNet;
static char expected_kind;
static int sending_calls,lock_calls;
static bool wrapper_test;
static std::vector<char> events;
void expect_label(char kind) { expected_kind=kind; }
void log_say(const char *p,int a,int b,int c) {
 char *expected=expected_kind=='S'?DiploLabelSendingTREATYON:DiploLabelGettingDIPLOMATICLOCK;
 assert(p==expected);
 int &count=expected_kind=='S'?sending_calls:lock_calls;
 assert(p[0]==(expected_kind=='S'?(count?'s':'S'):(count?'g':'G')));
 expected[0]=expected_kind=='S'?'s':'g';++count;
 if(expected_kind=='L')assert(!a&&!b&&!c);
 if(wrapper_test) {
  assert(a==3&&b==5&&c==0x1234);events.push_back('L');
  // Original branch selection precedes logging. A logger callback may mutate
  // live state, but the selected message/await operations still follow.
  IsMultiplayerNet=0;
 }
}
int message_data(int code,int zero,int a,int b,int c,int tail) {
 assert(wrapper_test&&!IsMultiplayerNet&&code==0x2441&&!zero&&a==3&&b==5&&c==0x1234&&!tail);
 events.push_back('M');return -7;
}
void Net::await_diplo(int kind) {assert(kind==0x441);events.push_back('A');}
void treaty_on(int a,int b,int c) {assert(a==3&&b==5&&c==0x1234);events.push_back('T');}
void probe_faction();void probe_faction_dialogue();void probe_faction_network();
void __cdecl net_treaty_on(int,int,int,int);
int main() {
 probe_faction();probe_faction_dialogue();probe_faction_network();
 assert(sending_calls+lock_calls==35);
 wrapper_test=true;expect_label('S');IsMultiplayerNet=1;
 net_treaty_on(3,5,0x1234,1);assert((events==std::vector<char>{'L','M','A'}));
 events.clear();IsMultiplayerNet=0;net_treaty_on(3,5,0x1234,1);
 assert((events==std::vector<char>{'T'}));
}
'''


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'different_writable_object', 'argument_order',
                                    'message_before_log', 'callback_reread'])
def test_all_actual_arguments_cross_tu_identity_and_order(tmp_path, compiler, damage):
    compiler = shutil.which(compiler)
    assert compiler, 'required host compiler unavailable'
    header = (ROOT / 'src/faction.h').read_text()
    core = (ROOT / 'src/faction.cpp').read_text()
    externs, definitions = [], []
    for _, name, _ in LABELS.values():
        externs.append(re.search(r'extern char ' + name + r'\[\];', header).group())
        definitions.append(re.search(r'^char ' + name + r'\[\] = "[^"]*";', core, re.M).group())
    (tmp_path / 'fixture.h').write_text(COMMON.replace('EXTERNS', '\n'.join(externs)))
    sources = []
    for unit, kinds in PATTERNS.items():
        text = (ROOT / 'src' / unit).read_text()
        snippets = [m.group() for m in calls(text)]
        assert len(snippets) == len(kinds)
        if damage == 'different_writable_object' and unit == 'faction.cpp':
            snippets[0] = snippets[0].replace('DiploLabelSendingTREATYON', 'OtherSending', 1)
        variables = ('faction_id', 'faction_id_with', 'chosen', 'local_faction',
                     'target_faction', 'against_id', 'other_faction_id', 'other',
                     'target', 'actor', 'partner', 'peer_id', 'treaty')
        declarations = '\n'.join('const int %s=%d;' % (v, 11 + n) for n, v in enumerate(variables))
        body = '\n'.join("expect_label('%s');\n%s" % (kind, snippet)
                         for kind, snippet in zip(kinds, snippets))
        generated = '#include "fixture.h"\nvoid probe_' + Path(unit).stem + '(){\n' + declarations + '\n' + body + '\n}\n'
        if unit == 'faction_network.cpp':
            network = definition(text, 'void __cdecl net_treaty_on(')
            log = 'log_say(DiploLabelSendingTREATYON, faction_id, faction_id_with, treaty);'
            message = 'message_data(0x2441, 0, faction_id, faction_id_with, treaty, 0);'
            assert log in network and message in network
            if damage == 'argument_order':
                network = network.replace(log, log.replace('faction_id, faction_id_with', 'faction_id_with, faction_id'), 1)
            elif damage == 'message_before_log':
                network = network.replace(log, '@LOG@', 1).replace(message, log, 1).replace('@LOG@', message, 1)
            elif damage == 'callback_reread':
                network = network.replace(log, log + '\nif (!IsMultiplayerNet) return;', 1)
            generated += network
        path = tmp_path / unit
        path.write_text(generated)
        sources.append(path)
    main = tmp_path / 'main.cpp'
    main.write_text(MAIN.replace('DEFINITIONS', '\n'.join(definitions)))
    exe = tmp_path / 'identity'
    built = subprocess.run([compiler, '-std=c++11', '-O2', str(main),
                            *map(str, sources), '-o', str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage is None), result.stderr

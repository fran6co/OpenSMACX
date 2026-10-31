"""Actual shared-label expressions and independent pointer-identity damage controls.

Representative calls cover all fifteen migrated families; the ignored inverse
ledger and coordinator native comparison cover all eighty-eight source sites.
This is not full dialogue control-flow or application acceptance proof.
"""
from pathlib import Path
from hashlib import sha256
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
# Independent selected source sites, original targets and label identities.
REPRESENTATIVES = [('src/faction.cpp', 482, 'DiploLabelSendingENERGY', 'Sending ENERGY', 6870204), ('src/faction.cpp', 14210, 'DiploLabelSendingTECH', 'Sending TECH', 6870256), ('src/faction.cpp', 6290, 'DiploLabelSendingSETTREATY', 'Sending SET TREATY', 6870124), ('src/faction.cpp', 7244, 'DiploLabelPACTATTACKS', 'PACTATTACKS', 6874580), ('src/faction.cpp', 7569, 'DiploLabelPACTTRUCE', 'PACTTRUCE', 6874656), ('src/faction.cpp', 8067, 'DiploLabelSendingTRADEMAPS', 'Sending TRADE MAPS', 6870236), ('src/faction.cpp', 8930, 'DiploLabelSendingTREATYOFF', 'Sending TREATY OFF', 6870104), ('src/faction.cpp', 9150, 'DiploLabelSendingCEDEBASE', 'Sending CEDE BASE', 6870312), ('src/faction.cpp', 9157, 'DiploLabelReleasingDIPLOMATICLOCK', 'Releasing DIPLOMATIC LOCK', 6870392), ('src/faction.cpp', 9905, 'DiploLabelSendingDOUBLECROSS', 'Sending DOUBLE CROSS', 6870332), ('src/faction.cpp', 11276, 'DiploLabelSendingWITHDRAWAL', 'Sending WITHDRAWAL', 6870292), ('src/faction.cpp', 11633, 'DiploLabelSendingLOAN', 'Sending LOAN', 6870220), ('src/faction.cpp', 14402, 'DiploLabelSendingAGENDAON', 'Sending AGENDA ON', 6870144), ('src/faction.cpp', 12469, 'DiploLabelSendingPACTENDS', 'Sending PACT ENDS', 6870272), ('src/faction_dialogue.cpp', 624, 'DiploLabelSendingAGENDAOFF', 'Sending AGENDA OFF', 6870164)]


def actual_call(unit, line):
    lines = (ROOT / unit).read_text().splitlines(keepends=True)
    tail = ''.join(lines[line - 1:])
    match = re.match(r'\s*((?:log_say|NetMsgPrimary\.pop)\([^;]+;)', tail)
    assert match, (unit, line)
    return match[1]


def test_existing_mutable_objects_match_original_targets():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    raw = Path(os.environ['OPENSMACX_IMAGE']).read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    core = (ROOT / 'src/faction.cpp').read_text()
    header = (ROOT / 'src/faction.h').read_text()
    for unit, line, name, text, target in REPRESENTATIVES:
        assert image.read(Span(target, target + len(text) + 1)) == text.encode() + b'\0'
        assert 'extern char ' + name + '[];' in header
        assert re.search(r'^char ' + name + r'\[\] = "' + re.escape(text) + r'";', core, re.M)
        assert actual_call(unit, line).split('(', 1)[1].split(',', 1)[0] == name


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_calls_share_objects_and_reject_misbinding(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    assert compiler, 'required independent host compiler unavailable'
    core = (ROOT / 'src/faction.cpp').read_text()
    header = (ROOT / 'src/faction.h').read_text()
    externs, definitions = [], []
    for _, _, name, _, _ in REPRESENTATIVES:
        externs.append(re.search(r'extern char ' + name + r'\[\];', header).group())
        definitions.append(re.search(r'^char ' + name + r'\[\] = "[^\n]*?";', core, re.M).group())
    common = '#include <cassert>\n#define __cdecl\n' + '\n'.join(externs)
    common += '\nvoid log_say(const char*,int,int,int);\nstruct Msg {void pop(const char*,int,int,void*);};\nextern Msg NetMsgPrimary;\n'
    common += '\n'.join('extern char Other_%d[];' % i for i in range(len(REPRESENTATIVES)))
    (tmp_path / 'fixture.h').write_text(common)
    units = {}
    for index, (unit, line, name, _, _) in enumerate(REPRESENTATIVES):
        call = actual_call(unit, line)
        # Preserve every actual argument. Supply narrow scalar service inputs.
        identifiers = set(re.findall(r'\b[A-Za-z_]\w*\b', call))
        excluded = {'log_say', 'NetMsgPrimary', 'pop', 'nullptr', name}
        declarations = '\n'.join('const int %s=%d;' % (v, 20 + j)
                                 for j, v in enumerate(sorted(identifiers - excluded)))
        different = call.replace(name, 'Other_%d' % index, 1)
        wrong_name = REPRESENTATIVES[(index + 1) % len(REPRESENTATIVES)][2]
        wrong = call.replace(name, wrong_name, 1)
        body = ('void probe_%d(int mode){\n%s\nif(mode==1){%s}\n'
                'else if(mode==2){%s}\nelse{%s}\n}\n') % (index, declarations, different, wrong, call)
        units[unit] = units.get(unit, '#include "fixture.h"\n') + body
    paths = []
    for unit, body in units.items():
        path = tmp_path / Path(unit).name
        path.write_text(body)
        paths.append(path)
    main = '#include "fixture.h"\n#include <cstdlib>\n' + '\n'.join(definitions)
    for index, (_, _, _, text, _) in enumerate(REPRESENTATIVES):
        main += '\nchar Other_%d[]="%s";' % (index, text)
    main += '\nstatic char *expected;static int calls;\nMsg NetMsgPrimary;\n'
    main += "void observe(const char *p){assert(p==expected);expected[0]='!';++calls;}\n"
    main += 'void log_say(const char *p,int,int,int){observe(p);}\n'
    main += 'void Msg::pop(const char *p,int timeout,int zero,void *picture){assert(timeout==5000&&!zero&&!picture);observe(p);}\n'
    main += '\n'.join('void probe_%d(int);' % i for i in range(len(REPRESENTATIVES)))
    main += '\nint main(int argc,char **argv){int bad=argc>1?std::atoi(argv[1]):-1;int mode=argc>2?std::atoi(argv[2]):0;\n'
    for index, (_, _, name, _, _) in enumerate(REPRESENTATIVES):
        main += "expected=%s;probe_%d(bad==%d?mode:0);assert(expected[0]=='!');\n" % (name, index, index)
    main += 'assert(calls==15);return 0;}\n'
    path = tmp_path / 'main.cpp'
    path.write_text(main)
    exe = tmp_path / 'identity'
    built = subprocess.run([compiler, '-std=c++11', '-O2', str(path),
                            *map(str, paths), '-o', str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    good = subprocess.run([str(exe)], capture_output=True, text=True)
    assert good.returncode == 0, good.stderr
    for index in range(len(REPRESENTATIVES)):
        for mode in (1, 2):
            # Each family independently rejects an equal-text writable copy and
            # another real named object; mutations affect the actual call token.
            bad = subprocess.run([str(exe), str(index), str(mode)], capture_output=True, text=True)
            assert bad.returncode != 0, (index, mode)
            assert 'p==expected' in bad.stderr, bad.stderr

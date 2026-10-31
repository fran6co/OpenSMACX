"""Actual writable script instances; host sizeof is not native COFF extent proof."""
from pathlib import Path
import os
import re
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]
ROLES = (
    ('AlphaNetServiceScript', 0x689238),
    ('AlphaNetConnectScript', 0x68925C),
    ('AlphaNetCreateScript', 0x689298),
    ('AlphaNetMissingSessionScript', 0x6892B8),
    ('AlphaNetMissingPlayerScript', 0x6892D8),
    ('AlphaNetJoinScript', 0x689318),
    ('AlphaNetNoSessionsScript', 0x68934C),
    ('AlphaNetSessionsScript', 0x689368),
)


@pytest.mark.parametrize('compiler', ('g++', 'clang++'))
@pytest.mark.parametrize('alias_damage', (False, True))
def test_actual_independent_writable_instances(compiler, alias_damage, tmp_path):
    compiler = shutil.which(compiler)
    if not compiler:
        pytest.skip('host compiler unavailable')
    source = (ROOT / 'src/alphanet.cpp').read_text()
    declarations = []
    for name, _ in ROLES:
        matches = re.findall(r'^char ' + name + r'\[7\] = "jackal";$', source, re.MULTILINE)
        assert len(matches) == 1
        declarations.append(matches[0])
    code = '\n'.join(declarations)
    if alias_damage:
        anchor = 'char AlphaNetMissingPlayerScript[7] = "jackal";'
        assert code.count(anchor) == 1
        damaged = code.replace(anchor,
            'char *AlphaNetMissingPlayerScript = AlphaNetMissingSessionScript;', 1)
        assert damaged != code
        code = damaged
    program = '#include <cassert>\n#include <cstring>\n' + code
    program += '\nint main() {\n char *objects[] = {' + ','.join(n for n, _ in ROLES) + '};\n'
    program += r'''
 for(int i=0;i<8;++i) {
   for(int j=0;j<8;++j) if(i!=j) assert(objects[i]!=objects[j]);
   assert(std::memcmp(objects[i], "jackal", 7)==0);
   objects[i][0]='X';
   for(int j=0;j<8;++j) if(i!=j) assert(std::memcmp(objects[j], "jackal", 7)==0);
   objects[i][0]='j';
 }
'''
    program += '\n'.join(' assert(sizeof(' + n + ')==7);' for n, _ in ROLES) + '\n}\n'
    cpp = tmp_path / 'scripts.cpp'; exe = tmp_path / 'scripts'
    cpp.write_text(program)
    built = subprocess.run([compiler, '-std=c++98', '-O2', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert (result.returncode != 0) if alias_damage else (result.returncode == 0), result.stderr


def test_pinned_payloads_are_distinct_writable_starts():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    if not path.is_file():
        pytest.skip('pinned original unavailable')
    image = PeImage(path.read_bytes(), '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    assert len({a for _, a in ROLES}) == 8
    for _, address in ROLES:
        assert image.read(Span(address, address+7)) == b'jackal\0'
        flags, = [flags for (span, _, _), flags in zip(image.sections, image.section_flags)
                  if span.start <= address < address+7 <= span.end]
        assert flags & 0x80000000 and flags & 0x40000000 and not flags & 0x20

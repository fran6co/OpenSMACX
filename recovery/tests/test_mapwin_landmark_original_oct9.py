"""Actual-source controls for the native landmark predicates.

The predicate fixture uses a counted safe reader. It verifies the original
negative-index short-circuit and positive-index behavior without reproducing
unknown adjacent-table reads; the pinned control includes Mount's compare and
branch before its first table read.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'


def condition(source, member):
    body = source.index('int __cdecl gen_terrain_poly(')
    position = source.index(
        f'offsetof(TerrainLandmarkStorageWindow, {member})', body)
    start = source.rfind('if (', body, position)
    assert start >= body
    marker = 'if ('
    first = start + len(marker)
    depth = 1
    for end in range(first, len(source)):
        if source[end] == '(':
            depth += 1
        elif source[end] == ')':
            depth -= 1
            if depth == 0:
                assert source[end + 1:end + 2] == ' '
                return source[first:end]
    raise AssertionError(f'unterminated {member} predicate')


def test_original_predicates_run_valid_and_sentinel_rows(tmp_path):
    source = (ROOT / 'src/mapwin.cpp').read_text()
    predicates = {
        name: condition(source, member)
        for name, member in (('MountPlanetTable', 'mount_planet'),
                             ('SunnyMesaTable', 'sunny_mesa'),
                             ('GarlandCraterTable', 'garland_crater'))
    }
    for name, predicate in predicates.items():
        assert 'terrain_window_signed_byte(' in predicate
        assert '!= -1' in predicate
        lower_guard = re.search(r'-1\s*<\s*(?:local_94|iVar5)', predicate)
        assert lower_guard
        assert not re.search(r'(?:local_94|iVar5)\s*<\s*(?:49|static_cast)', predicate)

    compiler = shutil.which('g++') or shutil.which('clang++')
    if compiler is None:
        pytest.skip('no host C++ compiler')
    mount, mesa, crater = (predicates[name] for name in predicates)
    harness = f'''#include <cstddef>
struct Pair {{ signed char texture, rotation; }};
typedef Pair LandmarkTextureChoice;
struct TerrainLandmarkStorageWindow {{
  Pair mount_planet[49]; char gap_mount_mesa[2];
  Pair sunny_mesa[49]; char gap_before_crater[118];
  Pair garland_crater[49]; char tail[242];
}};
int read_count, texture_value;
int terrain_window_signed_byte(size_t offset) {{ (void)offset; ++read_count; return texture_value; }}
int local_70, local_34, local_94, iVar5;
int main() {{
  local_70=0; local_34=2; local_94=-1; texture_value=1; read_count=0;
  if ({mount} || read_count != 0) return 1;
  local_94=0; read_count=0;
  if (!({mount}) || read_count != 1) return 2;
  local_94=48; read_count=0;
  if (!({mount}) || read_count != 1) return 3;
  local_94=49; read_count=0;
  if (!({mount}) || read_count != 1) return 4;
  texture_value=-1; local_94=0; read_count=0;
  if ({mount} || read_count != 1) return 5;
  texture_value=1; local_34=0x100; iVar5=-1; read_count=0;
  if ({mesa} || read_count != 0) return 6;
  iVar5=0; read_count=0;
  if (!({mesa}) || read_count != 1) return 7;
  local_34=1; iVar5=-1; read_count=0;
  if ({crater} || read_count != 0) return 8;
  iVar5=0; read_count=0;
  if (!({crater}) || read_count != 1) return 9;
  local_70=1; local_34=0x103; local_94=0; iVar5=0;
  if ({mount} || {mesa} || {crater}) return 10;
  local_70=0; local_34=0;
  if ({mount} || {mesa} || {crater}) return 11;
  return 0;
}}
'''
    cpp = tmp_path / 'landmark_predicates.cpp'
    exe = tmp_path / 'landmark_predicates'
    cpp.write_text(harness)
    built = subprocess.run([compiler, '-std=c++11', '-O0', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize('mutation', [
    '&& local_94 < 49',
    '&& iVar5 < 49',
])
def test_original_predicates_reject_reintroduced_bounds_guards(mutation):
    source = (ROOT / 'src/mapwin.cpp').read_text()
    assert mutation not in source


def test_original_negative_index_guards_match_each_family():
    source = (ROOT / 'src/mapwin.cpp').read_text()
    assert source.count('-1 < local_94') == 1
    assert source.count('&& -1 < iVar5') == 2


@pytest.mark.parametrize('damage', ['missing_lower', 'new_upper'])
def test_predicate_controls_reject_mount_guard_mutants(tmp_path, damage):
    source = (ROOT / 'src/mapwin.cpp').read_text()
    mount = condition(source, 'mount_planet')
    if damage == 'missing_lower':
        mutant = re.sub(r'-1\s*<\s*local_94\s*&&', '', mount, count=1)
    else:
        mutant = mount.replace(
            '-1 < local_94 &&', '-1 < local_94 && local_94 < 49 &&', 1)
    assert mutant != mount
    compiler = shutil.which('g++') or shutil.which('clang++')
    if compiler is None:
        pytest.skip('no host C++ compiler')
    harness = f'''#include <cstddef>
struct Pair {{ signed char texture, rotation; }};
typedef Pair LandmarkTextureChoice;
struct TerrainLandmarkStorageWindow {{
  Pair mount_planet[49]; char gap_mount_mesa[2];
  Pair sunny_mesa[49]; char gap_before_crater[118];
  Pair garland_crater[49]; char tail[242];
}};
int reads, value;
int terrain_window_signed_byte(size_t offset) {{ (void)offset; ++reads; return value; }}
bool mount_gate(int local_94) {{ const int local_70=0, local_34=2; return ({mutant}); }}
int main() {{ value=1; reads=0; if (mount_gate(-1) || reads != 0) return 1;
  reads=0; if (!mount_gate(49) || reads != 1) return 2; return 0; }}
'''
    cpp = tmp_path / f'mount_{damage}.cpp'
    exe = tmp_path / f'mount_{damage}'
    cpp.write_text(harness)
    built = subprocess.run([compiler, '-std=c++11', '-O0', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert ran.returncode != 0, damage


def test_pinned_landmark_receiver_branches_match_family_predicates():
    image_path = next((parent / '.opensmacx/game/terranx_original.exe'
                       for parent in (ROOT, *ROOT.parents)
                       if (parent / '.opensmacx/game/terranx_original.exe').is_file()), None)
    assert image_path is not None, 'pinned original image is required for branch control'
    image = PeImage(image_path.read_bytes(), PIN)
    code = image.read(Span(0x4632D0, 0x466C92))
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(code, 0x4632D0))
    by_address = {instruction.address: instruction for instruction in instructions}
    assert (by_address[0x464723].mnemonic, by_address[0x464723].op_str) == (
        'xor', 'esi, esi')
    assert (by_address[0x464725].mnemonic, by_address[0x464725].op_str) == (
        'cmp', 'eax, esi')
    assert (by_address[0x464727].mnemonic, by_address[0x464727].op_str) == (
        'je', '0x46477b')
    assert (by_address[0x464729].mnemonic, by_address[0x464729].op_str) == (
        'cmp', 'dword ptr [ebp - 0x1c], esi')
    assert (by_address[0x46472C].mnemonic, by_address[0x46472C].op_str) == (
        'je', '0x46477b')

    def direct_branch_target(instruction):
        if instruction.mnemonic.startswith('j') and re.fullmatch(
                r'0x[0-9a-f]+', instruction.op_str):
            return int(instruction.op_str, 16)
        return None

    assert [instruction.address for instruction in instructions
            if direct_branch_target(instruction) == 0x46477B] == [0x464727, 0x46472C]
    assert not any(direct_branch_target(instruction) in (0x464727, 0x46472C)
                   for instruction in instructions)
    assert (by_address[0x464787].mnemonic, by_address[0x464787].op_str) == (
        'mov', 'edi, dword ptr [ebp - 0x90]')
    assert (by_address[0x464795].mnemonic, by_address[0x464795].op_str) == (
        'cmp', 'edi, esi')
    for instruction in instructions:
        if 0x46477B <= instruction.address < 0x464795:
            _, writes = instruction.regs_access()
            assert all(instruction.reg_name(register) not in ('esi', 'si')
                       for register in writes)
    assert (by_address[0x464797].mnemonic, by_address[0x464797].op_str) == (
        'jl', '0x4648fb')
    assert (by_address[0x46479D].mnemonic, by_address[0x46479D].op_str) == (
        'movsx', 'eax, byte ptr [edi*2 + 0x685a98]')
    assert by_address[0x464797].address < by_address[0x46479D].address
    assert (by_address[0x4647A5].mnemonic, by_address[0x4647A5].op_str) == (
        'cmp', 'eax, -1')
    assert by_address[0x4647A8].mnemonic == 'je'
    assert (by_address[0x464912].mnemonic, by_address[0x464912].op_str) == (
        'jl', '0x464a38')
    assert (by_address[0x464A4F].mnemonic, by_address[0x464A4F].op_str) == (
        'jl', '0x464b6d')

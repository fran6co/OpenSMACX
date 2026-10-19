"""Actual-source controls for the native landmark predicates.

The predicate fixture executes in-range rows and texture sentinels. Mesa and
Crater retain native negative-index skips; Mount does not. Negative Mount
indices and positive indices past the table are never executed because their
original adjacent-storage effects are not established by the bounded arrays.
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
        assert bool(lower_guard) == (name != 'MountPlanetTable')
        assert not re.search(r'(?:local_94|iVar5)\s*<\s*(?:49|static_cast)', predicate)

    compiler = shutil.which('g++') or shutil.which('clang++')
    if compiler is None:
        pytest.skip('no host C++ compiler')
    mount, mesa, crater = (predicates[name] for name in predicates)
    harness = f'''#include <cstddef>
#include <cstdlib>
struct Pair {{ signed char texture, rotation; }};
typedef Pair LandmarkTextureChoice;
struct TerrainLandmarkStorageWindow {{
  Pair mount_planet[49]; char gap_mount_mesa[2];
  Pair sunny_mesa[49]; char gap_before_crater[118];
  Pair garland_crater[49]; char tail[242];
}};
TerrainLandmarkStorageWindow TerrainLandmarkStorageWindowData = {{}};
int terrain_window_signed_byte(size_t offset) {{
  const unsigned char value = reinterpret_cast<const unsigned char *>(
      &TerrainLandmarkStorageWindowData)[offset];
  return value < 0x80 ? value : static_cast<int>(value) - 0x100;
}}
int local_70, local_34, local_94, iVar5;
int main() {{
  local_70 = 0; local_34 = 2; local_94 = 0;
  if (!({mount})) return 1;
  TerrainLandmarkStorageWindowData.mount_planet[48].texture = 1; local_94 = 48;
  if (!({mount})) return 2;
  TerrainLandmarkStorageWindowData.mount_planet[48].texture = -1;
  if ({mount}) return 3;
  local_34 = 0x100; iVar5 = 0;
  if (!({mesa})) return 4;
  TerrainLandmarkStorageWindowData.sunny_mesa[48].texture = 1; iVar5 = 48;
  if (!({mesa})) return 5;
  TerrainLandmarkStorageWindowData.sunny_mesa[48].texture = -1;
  if ({mesa}) return 6;
  local_34 = 1; iVar5 = 0;
  if (!({crater})) return 7;
  TerrainLandmarkStorageWindowData.garland_crater[48].texture = 1; iVar5 = 48;
  if (!({crater})) return 8;
  TerrainLandmarkStorageWindowData.garland_crater[48].texture = -1;
  if ({crater}) return 9;
  local_70 = 1; local_34 = 0x103; local_94 = 0; iVar5 = 0;
  if ({mount} || {mesa} || {crater}) return 10;
  local_70 = 0; local_34 = 0;
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
    assert '&& -1 < local_94' not in source
    assert source.count('&& -1 < iVar5') == 2


def test_pinned_landmark_receiver_branches_match_family_predicates():
    image_path = next((parent / '.opensmacx/game/terranx_original.exe'
                       for parent in (ROOT, *ROOT.parents)
                       if (parent / '.opensmacx/game/terranx_original.exe').is_file()), None)
    assert image_path is not None, 'pinned original image is required for branch control'
    image = PeImage(image_path.read_bytes(), PIN)
    code = image.read(Span(0x46479D, 0x464B62))
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, 0x46479D))
    by_address = {instruction.address: instruction for instruction in instructions}
    assert (by_address[0x46479D].mnemonic, by_address[0x46479D].op_str) == (
        'movsx', 'eax, byte ptr [edi*2 + 0x685a98]')
    assert (by_address[0x4647A5].mnemonic, by_address[0x4647A5].op_str) == (
        'cmp', 'eax, -1')
    assert by_address[0x4647A8].mnemonic == 'je'
    assert not any(instruction.mnemonic == 'jl'
                   for instruction in instructions
                   if 0x46479D <= instruction.address < 0x4648FB)
    assert (by_address[0x464912].mnemonic, by_address[0x464912].op_str) == (
        'jl', '0x464a38')
    assert (by_address[0x464A4F].mnemonic, by_address[0x464A4F].op_str) == (
        'jl', '0x464b6d')

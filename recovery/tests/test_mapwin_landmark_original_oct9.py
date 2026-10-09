"""Actual-source controls for the native unchecked landmark predicates.

The predicate fixture executes in-range rows and texture sentinels. Existing
generation controls also exercise safe negative-index skips. Positive indices
past the table are not executed: their original adjacent-storage effects are
not established by the bounded source arrays.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def condition(source, table):
    body = source.index('int __cdecl gen_terrain_poly(')
    position = source.index(f'{table}[', body)
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
    raise AssertionError(f'unterminated {table} predicate')


def test_original_predicates_run_valid_and_sentinel_rows(tmp_path):
    source = (ROOT / 'src/mapwin.cpp').read_text()
    predicates = {
        name: condition(source, name)
        for name in ('MountPlanetTable', 'SunnyMesaTable', 'GarlandCraterTable')
    }
    for name, predicate in predicates.items():
        assert f'{name}[' in predicate
        assert '.texture != -1' in predicate
        assert re.search(r'-1\s*<\s*(?:local_94|iVar5)', predicate)
        assert not re.search(r'(?:local_94|iVar5)\s*<\s*(?:49|static_cast)|sizeof\(', predicate)

    compiler = shutil.which('g++') or shutil.which('clang++')
    if compiler is None:
        pytest.skip('no host C++ compiler')
    mount, mesa, crater = (predicates[name] for name in predicates)
    harness = f'''#include <cstdlib>
struct Pair {{ signed char texture, rotation; }};
Pair MountPlanetTable[49] = {{}};
Pair SunnyMesaTable[49] = {{}};
Pair GarlandCraterTable[49] = {{}};
int local_70, local_34, local_94, iVar5;
int main() {{
  local_70 = 0; local_34 = 2; local_94 = 0;
  if (!({mount})) return 1;
  MountPlanetTable[48].texture = 1; local_94 = 48;
  if (!({mount})) return 2;
  MountPlanetTable[48].texture = -1;
  if ({mount}) return 3;
  local_34 = 0x100; iVar5 = 0;
  if (!({mesa})) return 4;
  SunnyMesaTable[48].texture = 1; iVar5 = 48;
  if (!({mesa})) return 5;
  SunnyMesaTable[48].texture = -1;
  if ({mesa}) return 6;
  local_34 = 1; iVar5 = 0;
  if (!({crater})) return 7;
  GarlandCraterTable[48].texture = 1; iVar5 = 48;
  if (!({crater})) return 8;
  GarlandCraterTable[48].texture = -1;
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


@pytest.mark.parametrize('predicate', [
    '&& -1 < local_94',
    '&& -1 < iVar5',
])
def test_original_negative_index_guards_are_preserved(predicate):
    source = (ROOT / 'src/mapwin.cpp').read_text()
    assert predicate in source

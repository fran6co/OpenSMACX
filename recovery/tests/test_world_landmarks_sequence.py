"""Actual-source orchestration controls, not generator or native runtime proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
NAMES = ('monsoon', 'crater', 'volcano', 'mesa', 'ridge', 'diamond', 'ruin',
         'unity', 'fossil', 'temple', 'borehole', 'sargasso', 'dune', 'fresh',
         'geothermal', 'canyon')


def body():
    text = (ROOT / 'src/map.cpp').read_text()
    start = text.index('void __cdecl world_landmarks()')
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return text[start:end + 1]
    raise AssertionError('unterminated original owner body')


HARNESS = r'''
#include <cassert>
#include <vector>
#define __cdecl
typedef int BOOL;
int ExpansionEnabled;
bool disable_after_unity;
std::vector<int> seen;
void note(int id,int x,int y) { assert(x==-1 && y==-1); seen.push_back(id); }
GENERATORS
void fixup_landmarks() { seen.push_back(16); }
ACTUAL_BODY
void check(int enabled,bool disable) {
    ExpansionEnabled=enabled; disable_after_unity=disable; seen.clear();
    world_landmarks();
    std::vector<int> expected;
    for(int i=0;i<7;++i) expected.push_back(i);
    if(enabled) {
        expected.push_back(7);
        if(!disable) expected.push_back(8);
    }
    for(int i=9;i<15;++i) expected.push_back(i);
    expected.push_back(16);
    assert(seen==expected);
}
int main() {
    check(0,false); check(1,false); check(-1,false);
    check(1,true); check(-1,true);
}
'''


def execute(compiler_name, tmp_path, mutation=None):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f'{compiler_name} unavailable')
    product = body()
    if mutation:
        old, new = mutation
        assert old in product
        product = product.replace(old, new, 1)
    generators = []
    for index, name in enumerate(NAMES):
        if name == 'volcano':
            generators.append(f'void world_{name}(int x,int y,BOOL extra) {{ assert(extra==0); note({index},x,y); }}')
        elif name == 'unity':
            generators.append(f'void world_{name}(int x,int y) {{ note({index},x,y); if(disable_after_unity) ExpansionEnabled=0; }}')
        else:
            generators.append(f'void world_{name}(int x,int y) {{ note({index},x,y); }}')
    source = tmp_path / 'landmarks.cpp'
    source.write_text(HARNESS.replace('GENERATORS', '\n'.join(generators)).replace('ACTUAL_BODY', product))
    binary = tmp_path / 'landmarks'
    compiled = subprocess.run([compiler, '-std=c++11', '-Wall', '-Wextra', '-Werror',
                               '-fsanitize=undefined', '-fno-sanitize-recover=all',
                               str(source), '-o', str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize('compiler', ('g++', 'clang++'))
def test_actual_original_generation_sequence(compiler, tmp_path):
    result = execute(compiler, tmp_path)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('compiler', ('g++', 'clang++'))
@pytest.mark.parametrize('mutation', (
    ('fixup_landmarks();', 'world_canyon(-1,-1); fixup_landmarks();'),
    ('world_unity(-1, -1);\n        if (ExpansionEnabled)',
     'world_unity(-1, -1);\n        if (true)'),
    ('world_monsoon(-1, -1);', 'world_crater(-1, -1);'),
    ('fixup_landmarks();', '/* missing fixup */'),
    ('world_volcano(-1, -1, false);', 'world_volcano(-1, -1, true);'),
))
def test_sequence_damage_controls(compiler, tmp_path, mutation):
    result = execute(compiler, tmp_path, mutation)
    assert result.returncode != 0, 'damaged sequence passed'

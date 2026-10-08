"""Actual cursor bodies with host nodes; not native layout or game proof."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_cursor_traversal_and_failed_search(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    source = (ROOT / 'src/stringstruct.cpp').read_text()
    header = (ROOT / 'src/stringstruct.h').read_text()
    bodies = '\n'.join((
        definition(source, r'int StringStruct::current_id\(\)'),
        definition(source, r'int StringStruct::seek_id\(int id\)'),
        definition(header, r'MEASURED inline int StringStruct::seek_pos\(int a1\)')))
    prefix = '''#include <cassert>
#include <cstdlib>
#define MEASURED
typedef int (*AbsFnT)(int);
struct StringStructEntry { int id; StringStructEntry *next, *previous; };
struct StringStruct {
  StringStructEntry *head_, *current_;
  int entry_count_, current_position_;
  int current_id(); int seek_id(int); int seek_pos(int);
};
'''
    control = '''
int main() {
  StringStructEntry nodes[3];
  for (int i=0; i!=3; ++i) {
    nodes[i].id=10+i;
    nodes[i].next=&nodes[(i+1)%3];
    nodes[i].previous=&nodes[(i+2)%3];
  }
  StringStruct list={nodes, nodes, 3, 0};
  assert(list.seek_id(12)==1 && list.current_==nodes+2);
  assert(list.current_id()==12 && list.current_position_==2);
  assert(list.seek_id(99)==0 && list.current_==nodes);
  assert(list.current_position_==3);
  assert(list.seek_pos(1)==1 && list.current_==nodes+1);
  assert(list.current_position_==1);
  assert(list.seek_pos(-1)==1 && list.current_==nodes+2);
  assert(list.current_position_==2);
  assert(list.seek_pos(-3)==1 && list.current_==nodes);
  assert(list.current_position_==0);
  list.current_=nodes+1;
  assert(list.seek_pos(3)==0 && list.current_==nodes+1);
  assert(list.seek_pos(-4)==0 && list.current_==nodes);
  list.head_=0;
  assert(list.current_id()==0 && list.seek_id(10)==0);
}
'''
    for label, candidate in (
        ('actual', bodies),
        ('wrong_direction', bodies.replace('current_ = current_->previous;',
                                           'current_ = current_->next;')),
        ('wrong_id', bodies.replace('return current_->id;', 'return 0;'))):
        cpp, binary = tmp_path / (label + '.cpp'), tmp_path / label
        cpp.write_text(prefix + candidate + control)
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=address,undefined',
                                '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        # No heap allocation occurs in this control. LeakSanitizer's process
        # inspection is unavailable under the workspace sandbox's ptrace.
        environment = dict(os.environ)
        environment['ASAN_OPTIONS'] = environment.get('ASAN_OPTIONS', '') + ':detect_leaks=0'
        run = subprocess.run([str(binary)], capture_output=True, text=True,
                             timeout=10, env=environment)
        assert (run.returncode == 0) == (label == 'actual'), label + run.stderr

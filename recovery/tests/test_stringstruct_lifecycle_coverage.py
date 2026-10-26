"""Actual declarations and lifecycle bodies; host control is not native ABI proof."""
from pathlib import Path
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_stringstruct_construction_and_deletion(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    header = (ROOT / 'src/stringstruct.h').read_text()
    declarations = []
    for start, end in (
        ('class StringAllocationBase {', 'class ListPayloadBase :'),
        ('class ListPayloadBase :', 'struct StringStructEntry :'),
        ('struct StringStructEntry :', 'static_assert(sizeof(StringStructEntry)'),
        ('class StringStructPayload :', 'static_assert(sizeof(StringStructPayload)'),
        ('class StringStruct :', 'static_assert(sizeof(StringStruct)')):
        begin = header.index(start)
        declarations.append(header[begin:header.index(end, begin)])
    constructor = definition(header, r'StringStruct::StringStruct\(\)')
    prefix = '''#include <cassert>
#include <cstdlib>
#include <cstddef>
#include <new>
typedef unsigned int uint32_t;
#define nullptr 0
#define MEASURED
class Heap {};
class StringStructPayload;
struct SpriteStruct;
struct StringStructEntry;
static Heap *StringAllocationHeap;
static int releases;
static void controlled_free(void *p) { ++releases; std::free(p); }
#define free controlled_free
'''
    suffix = '''
#undef free
// The shared header contains the actual inline teardown stages; the full
// populated-list walk is separately exercised by the lifetime harness.
@@BODIES@@
int StringStruct::on_insert(void *) { return 0; }
void StringStruct::on_remove(void *) {}
int main() {
  Heap heap;
  // This is an explicit call to the actual static deallocator body, not a
  // delete-null expression or a destructor call on a null receiver.
  StringAllocationHeap = &heap;
  StringAllocationBase::operator delete(0);
  assert(releases == 0 && StringAllocationHeap == &heap);
  StringAllocationHeap = 0;
  StringAllocationBase::operator delete(0);
  assert(releases == 0 && !StringAllocationHeap);
  for (int owned=0; owned!=2; ++owned) {
    StringAllocationHeap = owned ? &heap : 0;
    void *storage=std::malloc(sizeof(StringStruct)); assert(storage);
    StringStruct *p=::new(storage) StringStruct;
    assert(!p->head_ && !p->current_ && !p->entry_count_);
    assert(!p->current_position_ && !p->allocator_);
    assert(p->allocation_owner_ == (owned ? &heap : 0));
    assert(!StringAllocationHeap);
    StringAllocationBase *base=p;
    delete base;
    assert(releases == 1);
    assert(!StringAllocationHeap);
    if (owned) std::free(storage);
  }
}
'''
    mutations = {'actual': constructor}
    for field in ('head_', 'current_', 'entry_count_', 'current_position_', 'allocator_'):
        store = field + ' = 0;'
        assert constructor.count(store) == 1
        value = 'reinterpret_cast<' + ('Heap' if field == 'allocator_' else 'StringStructEntry') + ' *>(1)' if field in ('head_', 'current_', 'allocator_') else '1'
        mutations['wrong_' + field] = constructor.replace(store, field + ' = ' + value + ';')
    for name, body in mutations.items():
        cpp, binary = tmp_path / (name + '.cpp'), tmp_path / name
        cpp.write_text(prefix + '\n'.join(declarations) + suffix.replace('@@BODIES@@', body))
        build = subprocess.run([compiler, '-std=c++98', '-fsanitize=undefined', '-fno-sanitize-recover=all', str(cpp), '-o', str(binary)], capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert (run.returncode == 0) == (name == 'actual'), name + run.stderr

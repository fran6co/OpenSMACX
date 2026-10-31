"""Independent key-admission and selection controls for the actual BasePop body."""
from pathlib import Path
import shutil
import subprocess
import pytest
from test_basepop_item_check_owner_oct9 import definition

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include <cassert>
struct Dialog {
    struct Items { int entry_count_; } item_list_;
    int selected_position_, selected_id_, reads;
    int get_selected_id() { ++reads; return selected_id_; }
};
struct CheckBox : virtual Dialog {
    unsigned int state_word() const { return Dialog::selected_position_; }
};
struct ListBox : virtual Dialog {};
struct Dialogs : ListBox, CheckBox { int kind_; };
struct BasePop {
    int field_30A8_, selected_id_;
    Dialogs dialogs_;
    int on_key_down(int);
};
ACTUAL
int main() {
    BasePop w;
    w.field_30A8_=0x4000; w.selected_id_=77;
    assert(w.on_key_down(-123)==0 && w.selected_id_==77);
    w.field_30A8_=0;
    const int kinds[]={-1,0,1,2,4,8,16};
    const int counts[]={0,1,-3};
    for(unsigned i=0;i<sizeof(kinds)/sizeof(kinds[0]);++i) {
        for(unsigned j=0;j<sizeof(counts)/sizeof(counts[0]);++j) {
            w.dialogs_.kind_=kinds[i];
            w.dialogs_.item_list_.entry_count_=counts[j];
            w.dialogs_.selected_position_=-37; w.dialogs_.selected_id_=45;
            w.dialogs_.reads=0; w.selected_id_=88;
            assert(w.on_key_down(0x1234)==1);
            const bool selected=kinds[i]!=-1 && counts[j]!=0;
            assert(w.selected_id_==(selected?(kinds[i]==1?-37:45):0));
            assert(w.dialogs_.reads==(selected && kinds[i]!=1?1:0));
        }
    }
}
'''

@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_key_selection_and_disabled_admission(tmp_path, compiler):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + ' unavailable')
    body = definition((ROOT / 'src/basepop.cpp').read_text(),
                      'int BasePop::on_key_down(int key)')
    def run(candidate, name):
        cpp = tmp_path / (name + '.cpp')
        binary = tmp_path / name
        cpp.write_text(HARNESS.replace('ACTUAL', candidate))
        subprocess.run([executable, '-std=c++11', '-O2', str(cpp), '-o', str(binary)],
                       check=True, capture_output=True, text=True)
        return subprocess.run([str(binary)], capture_output=True, text=True)
    assert run(body, 'actual').returncode == 0
    assert run(body.replace('if (kind == 1)', 'if (kind == 2)', 1),
               'wrong_kind').returncode != 0
    assert run(body.replace('(field_30A8_ & 0x4000) != 0',
                            '(field_30A8_ & 0x4000) == 0', 1),
               'inverted_admission').returncode != 0

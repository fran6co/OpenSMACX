"""Original stack_put call-result/link-snapshot boundary; not game proof.

The boundary double varies veh_lift's result to expose the consumer's original
EAX dependency. The actual recovered veh_lift returns its input, so this does
not assert an independently reachable gameplay defect in the old spelling.
"""
from pathlib import Path
import re
import shutil
import subprocess
import pytest
from recovery.tests.test_pact_ascent import definition
ROOT=Path(__file__).resolve().parents[2]
HARNESS=r'''
#include <stdint.h>
#include <assert.h>
#include <vector>
#define __cdecl
struct Veh {int16_t prev_veh_id_stack,next_veh_id_stack;} Vehs[8];
std::vector<int> lifted,dropped;
int veh_lift(int id){lifted.push_back(id);Vehs[id].next_veh_id_stack=-1;return id+4;}
void veh_drop(int id,int x,int y){assert(x==-13&&y==22);dropped.push_back(id);}
PRODUCT
int main(){
 for(int i=0;i<8;++i){Vehs[i].prev_veh_id_stack=-1;Vehs[i].next_veh_id_stack=-1;}
 Vehs[0].next_veh_id_stack=1;Vehs[1].prev_veh_id_stack=0;
 Vehs[1].next_veh_id_stack=2;Vehs[2].prev_veh_id_stack=1;
 stack_put(1,-13,22);
 assert((lifted==std::vector<int>{0,1,2}));assert((dropped==std::vector<int>{4,5,6}));
 lifted.clear();dropped.clear();
 stack_put(-1,-13,22);stack_put(-7,-13,22);assert(lifted.empty()&&dropped.empty());
 Vehs[3].prev_veh_id_stack=-17;Vehs[3].next_veh_id_stack=-9;
 stack_put(3,-13,22);assert((lifted==std::vector<int>{3}));assert((dropped==std::vector<int>{7}));
}
'''
@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('damage',['none','discard_return','late_next_snapshot'])
def test_stack_put_actual_boundary_and_damage(tmp_path,compiler_name,damage):
    compiler=shutil.which(compiler_name)
    if not compiler:pytest.skip(compiler_name+' unavailable')
    code=definition((ROOT/'src/veh.cpp').read_text(),r'void\s+__cdecl\s+stack_put\s*\(')
    call='            veh_drop(veh_lift(top_veh_id), x, y);'
    assert call in code
    if damage=='discard_return':
        code=code.replace(call,'            veh_lift(top_veh_id);\n            veh_drop(top_veh_id, x, y);',1)
    elif damage=='late_next_snapshot':
        snapshot='            veh_id_loop = Vehs[top_veh_id].next_veh_id_stack;'
        assert snapshot in code
        code=code.replace(snapshot,'',1).replace(call,call+'\n'+snapshot,1)
    cpp=tmp_path/'stack.cpp';exe=tmp_path/'stack';cpp.write_text(HARNESS.replace('PRODUCT',code))
    built=subprocess.run([compiler,'-std=c++11','-O2','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],capture_output=True,text=True)
    assert built.returncode==0,built.stdout+built.stderr
    run=subprocess.run([str(exe)],capture_output=True,text=True)
    assert (run.returncode==0)==(damage=='none'),run.stdout+run.stderr

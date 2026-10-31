"""Bounded actual moved-body controls; no original TU or live-game claim."""
from pathlib import Path
import importlib.util
import shutil
import subprocess
ROOT = Path(__file__).resolve().parents[2]

def actual_body(signature):
    text=(ROOT/'src/movement_queries.cpp').read_text();start=text.index(signature);opening=text.index('{',start);depth=0
    for end in range(opening,len(text)):
        depth+=(text[end]=='{')-(text[end]=='}')
        if not depth:return text[start:end+1]
    raise AssertionError('unterminated')

def test_moved_coast_with_unchanged_independent_oracles():
    spec=importlib.util.spec_from_file_location('coast_oracle',ROOT/'recovery/tests/test_coast_native_contract_oct12h.py');oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
    actual=actual_body('int __cdecl coast_or_border(');compilers=[x for x in ('g++','clang++') if shutil.which(x)];assert compilers
    for compiler in compilers:
        assert oracle.compile_and_run(actual,compiler).returncode==0
        for label,mutant in oracle.damage_mutants(actual).items():
            assert mutant!=actual,label
            assert oracle.compile_and_run(mutant,compiler).returncode!=0,label

PRELUDE=r'''
#include <cassert>
#include <vector>
#define __cdecl
using uint32_t=unsigned; using BOOL=int;
enum {TRIAD_LAND,TRIAD_SEA,TRIAD_AIR};
struct Vehicle {int x,y;}; Vehicle Vehs[1]={{10,20}};
static unsigned triad,rs,rd; static int bs,bd,wet; static std::vector<int> events;
unsigned get_triad(int v){assert(v==0);events.push_back(1);return triad;}
unsigned region_at(int x,int y){assert((x==10&&y==20)||(x==30&&y==40));events.push_back(x);return x==10?rs:rd;}
int base_at(int x,int y){assert((x==10&&y==20)||(x==30&&y==40));events.push_back(x+100);return x==10?bs:bd;}
bool is_ocean(int x,int y){assert(x==30&&y==40);events.push_back(200);return wet;}
int base_on_sea(int b,int r){events.push_back(300+b*10+r);return 7;}
int port_to_port(int a,int b){events.push_back(400+a*10+b);return 9;}
int sea_coast(int d,int s){events.push_back(500+d*10+s);return 11;}
int port_to_coast(int b,int r){events.push_back(600+b*10+r);return 13;}
'''
SUFFIX=r'''
int main(){
for(triad=0;triad<3;++triad)for(bs=-1;bs<2;++bs)for(bd=-1;bd<2;++bd)
for(rs=1;rs<3;++rs)for(rd=1;rd<3;++rd)for(wet=0;wet<2;++wet){
 std::vector<int> ex;ex.push_back(1);int result;
 if(triad==2)result=1;
 else{ex.push_back(10);ex.push_back(30);
  if(triad==0)result=(rs==rd);
  else{ex.push_back(110);ex.push_back(130);
   if(bd>=0){if(bs<0){ex.push_back(300+bd*10+rs);result=7;}else{ex.push_back(400+bs*10+bd);result=9;}}
   else{ex.push_back(200);
    if(wet){if(bs<0)result=(rs==rd);else{ex.push_back(300+bs*10+rd);result=7;}}
    else{if(bs<0){ex.push_back(500+rd*10+rs);result=11;}else{ex.push_back(600+bs*10+rd);result=13;}}
   }
  }
 }
 events.clear();assert(get_there(0,30,40)==result);assert(events==ex);
}}
'''

def test_get_there_all_branches_raw_results_order_and_damage(tmp_path):
    actual=actual_body('BOOL __cdecl get_there(');mutants={}
    for label,old,new in [
      ('port direction','port_to_port(base_id_src, base_id_dst)','port_to_port(base_id_dst, base_id_src)'),
      ('sea direction','sea_coast(region_dst, region_src)','sea_coast(region_src, region_dst)'),
      ('land comparison','return (region_src == region_dst);','return (region_src != region_dst);')]:
        assert actual.count(old)==1;mutants[label]=actual.replace(old,new,1)
    old='return (base_id_src < 0) ? base_on_sea(base_id_dst, region_src)';tail=': port_to_port(base_id_src, base_id_dst);'
    assert actual.count(old)==actual.count(tail)==1
    mutants['raw result normalization']=actual.replace(old,old.replace('return ','return !!(',1),1).replace(tail,tail[:-1]+');',1)
    compilers=[x for x in ('g++','clang++') if shutil.which(x)];assert compilers
    for compiler in compilers:
        for label,code in [('actual',actual),*mutants.items()]:
            assert label=='actual' or code!=actual
            source=tmp_path/'query.cpp';exe=tmp_path/'query';source.write_text(PRELUDE+code+SUFFIX)
            built=subprocess.run([compiler,'-std=c++11','-Wall','-Wextra','-Werror',str(source),'-o',str(exe)],capture_output=True,text=True)
            assert built.returncode==0,built.stderr
            run=subprocess.run([str(exe)],capture_output=True,text=True)
            assert (run.returncode==0)==(label=='actual'),label

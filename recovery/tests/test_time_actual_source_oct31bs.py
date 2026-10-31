"""Actual TimeRange sample controls; original class/lifetime remain unresolved.

The arithmetic oracle operates on uint32 bit patterns and copies the expected
result to int32 independently. Clock replacement witnesses field reload timing.
"""
from pathlib import Path
import os
import shutil
import subprocess
import pytest
import test_time_range_views_actual_source as previous

ROOT=Path(__file__).resolve().parents[2]


def compile_run(tmp_path,compiler,text):
    cxx=shutil.which(compiler)
    if not cxx:pytest.skip(f"{compiler} unavailable")
    cpp=tmp_path/'range.cpp';exe=tmp_path/'range'
    from match_recovery.includes import COMMENTS_OR_STRINGS
    # Keep the complete actual code while excluding archived nested-comment prose.
    text=COMMENTS_OR_STRINGS.sub(lambda m: ''.join('\n' if c=='\n' else ' ' for c in m[0])
                                if m[0].startswith(('//','/*')) else m[0],text)
    cpp.write_text(text)
    built=subprocess.run([cxx,'-std=c++17','-O2','-Wall','-Wextra','-Werror',
                          '-fsanitize=undefined','-fno-sanitize-recover=all',
                          str(cpp),'-o',str(exe)],capture_output=True,text=True)
    assert built.returncode==0,built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True)


MATRIX=r'''
int main(){
 assert(old_checks()==0);
 const int endpoints[]={INT_MIN,INT_MIN+1,-65536,-2,-1,0,1,2,65536,INT_MAX-1,INT_MAX};
 const uint32_t ticks[]={0,1,0xfffffff,0xffffffffu};
 TimeRangeReceiverView state;clock_receiver=&state;
 for(int lower:endpoints)for(int upper:endpoints)for(uint32_t tick:ticks)for(int change=0;change<2;++change){
  uint32_t divisor=lower>upper?uint32_t(lower)-uint32_t(upper)+1u:uint32_t(upper)-uint32_t(lower)+1u;
  if(!divisor)continue; // The original zero-divisor failure is a separate process control.
  state.enabled_flags_4_=1;state.lower_bound_8_=lower;state.upper_bound_C_=upper;
  tick_value=tick;mutate_upper=change;new_upper=lower;int before=calls;
  uint32_t bits=uint32_t(change?lower:upper)+tick%divisor;
  int expected;memcpy(&expected,&bits,sizeof(expected));
  assert(state.sample()==expected && calls==before+1);
 }
 clock_receiver=0;
}
'''


@pytest.mark.parametrize('compiler',['g++','clang++'])
@pytest.mark.parametrize('mutation',[None,'cached_upper','signed_range','wrong_extent','wrong_flag'])
def test_actual_unsigned_ranges_and_post_clock_reload(tmp_path,compiler,mutation):
    source,_=previous.actual_view_source()
    if mutation=='cached_upper':
        old='range_view_wrapped_upper_add(upper_bound_C_, remainder)'
        assert source.count(old)==1
        source=source.replace(old,'range_view_wrapped_upper_add(upper, remainder)',1)
    elif mutation=='signed_range':
        old='static_cast<DWORD>(upper) - static_cast<DWORD>(lower) + 1u'
        assert source.count(old)==1
        source=source.replace(old,'static_cast<DWORD>(upper - lower) + 1u',1)
    elif mutation=='wrong_extent':
        old='range = static_cast<DWORD>(upper) - static_cast<DWORD>(lower) + 1u;'
        assert source.count(old)==1
        source=source.replace(old,old.replace('+ 1u','+ 2u'),1)
    elif mutation=='wrong_flag':
        old='(enabled_flags_4_ & 1) == 0'
        assert source.count(old)==1
        source=source.replace(old,'(enabled_flags_4_ & 2) == 0',1)
    fixture=previous.host_harness(source).replace('int main() {','int old_checks() {',1)
    result=compile_run(tmp_path,compiler,fixture+MATRIX)
    assert (result.returncode==0)==(mutation is None),result.stderr


@pytest.mark.parametrize('compiler',['g++','clang++'])
def test_original_zero_divisor_path_still_fails(tmp_path,compiler):
    source,_=previous.actual_view_source()
    fixture=previous.host_harness(source).replace('int main() {','int old_checks() {',1)
    main=r'''
int main(){TimeRangeReceiverView state;clock_receiver=&state;state.enabled_flags_4_=1;
state.lower_bound_8_=INT_MAX;state.upper_bound_C_=INT_MIN;tick_value=1;return state.sample();}
'''
    result=compile_run(tmp_path,compiler,fixture+main)
    assert result.returncode!=0
    assert 'division by zero' in result.stderr


def test_original_pin_has_unsigned_division_and_post_clock_upper_load():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not image.is_file():pytest.skip('pinned original unavailable')
    pe=PeImage(image.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    ins={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(pe.read(Span(0x4c8350,0x4c8396)),0x4c8350)}
    assert ins[0x4c8366]==('jle','0x4c837f')
    for address in (0x4c836d,0x4c8384):assert ins[address]==('call','dword ptr [0x669368]')
    for address in (0x4c8375,0x4c838c):assert ins[address]==('div','esi')
    for address in (0x4c8377,0x4c838e):assert ins[address]==('mov','eax, dword ptr [edi + 0xc]')
    for address in (0x4c837c,0x4c8393):assert ins[address]==('add','eax, edx')

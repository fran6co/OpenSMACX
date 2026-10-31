"""Exercise the source setup body; host doubles do not establish target ABI."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.local_variants import _function_body_bytes


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("old_return", (False, True))
def test_setup_picker_exit_matches_original(compiler_name, old_return, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/alphanet.cpp").read_bytes()
    lo, hi = _function_body_bytes(source.decode(), "AlphaNet::setup")
    body = source[lo:hi].decode()
    if old_return:
        assert "if (picked != 0) return 0;" in body
        body = body.replace("if (picked != 0) return 0;", "if (picked != 0) return picked;", 1)
    program = r'''
#include <cassert>
struct GUID { int identity; };
const GUID LobbyAppGuid = {42};
int IsMultiplayerPBEM;
int pick_result, pick_pbem, selection, join_result, create_result;
int closes, inits, picks, popups, joins, creates;
int x_pop_caption_flags(const char *, const char *, int flags, int zero) {
  assert(flags == 0x50 && zero == 0); ++popups; return selection;
}
struct Net {
  void close() { ++closes; }
  void init(GUID *guid, int a, int b) {
    assert(guid == &LobbyAppGuid && a == 3 && b == 9); ++inits;
  }
};
struct AlphaNet : Net {
  struct Slot { int pid_; } process_slots_[8];
  int player_count_, voice_flags_D8_, voice_hotkey_48_, voice_hotkey_4c_;
  int pick_service(int *pbem) { ++picks; *pbem=pick_pbem; return pick_result; }
  int join() { ++joins; return join_result; }
  int create() { ++creates; return create_result; }
  int setup(int);
};
int AlphaNet::setup(int option) BODY
void reset(AlphaNet &n) {
  closes=inits=picks=popups=joins=creates=0;
  pick_result=pick_pbem=selection=0; join_result=create_result=1;
  n.player_count_=5; n.voice_flags_D8_=8;
  n.voice_hotkey_48_=n.voice_hotkey_4c_=-1;
  for(int i=0;i<8;++i) n.process_slots_[i].pid_=99;
}
void initial_state(const AlphaNet &n) {
  assert(n.player_count_==0);
  for(int i=0;i<8;++i) assert(n.process_slots_[i].pid_==0);
}
int main() {
  AlphaNet n; const int errors[]={1,4,7,-9};
  for(int i=0;i<4;++i) {
    reset(n); pick_result=errors[i]; assert(n.setup(0)==0);
    initial_state(n); assert(closes==1 && inits==1 && picks==1);
    assert(popups==0 && joins==0 && creates==0);
    assert(n.voice_hotkey_48_==-1 && n.voice_hotkey_4c_==-1);
  }
  reset(n); pick_pbem=1; assert(n.setup(0)==0);
  assert(closes==2 && popups==0 && n.voice_hotkey_48_==-1);
  reset(n); selection=-1; assert(n.setup(0)==1);
  assert(popups==1 && joins==0 && creates==0 && n.voice_hotkey_48_==-1);
  for(int choice=0;choice<2;++choice) {
    reset(n); selection=choice; assert(n.setup(0)==0);
    assert(joins==choice && creates==1-choice);
    assert(n.voice_hotkey_48_==0xDC && n.voice_hotkey_4c_==0x100DC);
    reset(n); selection=choice; join_result=create_result=0;
    assert(n.setup(0)==1 && n.voice_hotkey_48_==-1);
  }
  reset(n); assert(n.setup(1)==0); initial_state(n);
  assert(n.voice_flags_D8_==9 && closes==1 && inits==0 && picks==0);
  assert(n.voice_hotkey_48_==0xDC && n.voice_hotkey_4c_==0x100DC);
}
'''.replace("BODY", body)
    cpp = tmp_path / "setup.cpp"
    cpp.write_text(program)
    executable = tmp_path / "setup"
    built = subprocess.run([compiler, "-std=c++98", str(cpp), "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
    assert (ran.returncode != 0) if old_return else (ran.returncode == 0), ran.stdout + ran.stderr

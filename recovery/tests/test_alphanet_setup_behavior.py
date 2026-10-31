"""Exercise the source setup body; host doubles do not establish target ABI."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.local_variants import _function_body_bytes


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("zero_normalization", (False, True))
def test_setup_picker_exit_matches_original(compiler_name, zero_normalization, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    root = Path(__file__).resolve().parents[2]
    source = (root / "src/alphanet.cpp").read_bytes()
    lo, hi = _function_body_bytes(source.decode(), "AlphaNet::setup")
    body = source[lo:hi].decode()
    if zero_normalization:
        anchor = "if (picked != 0) return picked;"
        assert body.count(anchor) == 1
        damaged = body.replace(anchor, "if (picked != 0) return 0;", 1)
        assert damaged != body
        body = damaged
    scripts = "\n".join(re.findall(
        r'^char AlphaNet\w+Script\[7\] = "jackal";$', source.decode(), re.MULTILINE))
    needs_script = "AlphaNetConnectScript" in body
    if needs_script:
        assert len(scripts.splitlines()) == 8
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
    reset(n); pick_result=errors[i]; assert(n.setup(0)==errors[i]);
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
    if needs_script:
        program = program.replace("#include <cassert>", "#include <cassert>\n" + scripts, 1)
        anchor = "assert(flags == 0x50 && zero == 0);"
        assert program.count(anchor) == 1
        program = program.replace(anchor,
            "assert(script == AlphaNetConnectScript); " + anchor, 1)
        program = program.replace("int x_pop_caption_flags(const char *, const char *,",
                                  "int x_pop_caption_flags(const char *script, const char *,", 1)
    cpp = tmp_path / "setup.cpp"
    cpp.write_text(program)
    executable = tmp_path / "setup"
    built = subprocess.run([compiler, "-std=c++98", str(cpp), "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
    assert (ran.returncode != 0) if zero_normalization else (ran.returncode == 0), ran.stdout + ran.stderr


def test_pinned_picker_error_skips_zero_return_instruction():
    """The original nonzero picker edge reaches a bare return with EAX intact."""
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    root = Path(__file__).resolve().parents[2]
    path = Path(os.environ.get("OPENSMACX_IMAGE", root / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    picker = [(i.address, i.mnemonic, i.op_str)
              for i in decoder.disasm(image.read(Span(0x4E3238, 0x4E3241)), 0x4E3238)]
    assert picker == [(0x4E3238, "call", "0x4e26e0"),
                      (0x4E323D, "test", "eax, eax"),
                      (0x4E323F, "jne", "0x4e32b5")]
    epilogue = [(i.address, i.mnemonic, i.op_str)
                for i in decoder.disasm(image.read(Span(0x4E32B3, 0x4E32BA)), 0x4E32B3)]
    assert epilogue == [(0x4E32B3, "xor", "eax, eax"),
                        (0x4E32B5, "pop", "esi"),
                        (0x4E32B6, "pop", "ebp"),
                        (0x4E32B7, "ret", "4")]

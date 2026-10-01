"""Actual-source controls for the FameWin entry and record-presence clearing.

Scripted I/O and window doubles check ordering and failure branches, not the
native ABI, actual disk/UI behavior, byte matching or game acceptance.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(signature):
    text = (ROOT / "src/famewin.cpp").read_text()
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source function")


def test_hall_of_fame_entry_preserves_io_modal_and_failure_order(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    harness = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
#include "halloffame.h"
HallOfFameRecord HallOfFameRecords[HallOfFameRecordCount];
int HallOfFameIOMode;
std::vector<std::string> events;
bool read_open, write_open, read_ok, write_ok;
int init_result, adds, closes, selected_faction;
FILE *test_fopen(const char *name, const char *mode) {
  assert(std::strcmp(name,"hallfame.jhh")==0);
  events.push_back(std::string("open:")+mode);
  return ((std::strcmp(mode,"rb")==0 ? read_open : write_open)
          ? reinterpret_cast<FILE *>(1) : nullptr);
}
size_t test_fread(void *data,size_t size,size_t count,FILE *file) {
  assert(data==HallOfFameRecords && size==0x1324 && count==1 && file);
  assert(HallOfFameIOMode==0); events.push_back("read");
  return read_ok ? 1 : 0;
}
size_t test_fwrite(const void *data,size_t size,size_t count,FILE *file) {
  assert(data==HallOfFameRecords && size==0x1324 && count==1 && file);
  assert(HallOfFameIOMode==1); events.push_back("write");
  return write_ok ? 1 : 0;
}
int test_fclose(FILE *file) { assert(file); ++closes; events.push_back("close"); return 0; }
int pop_wait() { return 1; }
struct Console { int faction_id_; } ConsoleGlobal;
struct ButtonGroup { void set(int faction,int value) {
  assert(value==1); selected_faction=faction; events.push_back("select"); } };
struct FameWin {
  ButtonGroup buttonGroup_;
  int init() { events.push_back("init"); return init_result; }
  void try_to_add_me() { ++adds; events.push_back("add"); }
  virtual int set_modal(int mode,int (*poll)(),void *owner) {
    assert(mode==0 && poll==pop_wait && !owner);
    events.push_back("modal"); ConsoleGlobal.faction_id_=6; return 0;
  }
} g_FAMEWIN;
#define fopen test_fopen
#define fread test_fread
#define fwrite test_fwrite
#define fclose test_fclose
'''
    harness += body("void __cdecl clear_hall_of_fame()") + "\n"
    harness += body("void __cdecl hall_of_fame(int add_current_player)")
    harness += r'''
void reset() {
  events.clear(); read_open=write_open=read_ok=write_ok=true;
  init_result=adds=closes=0; selected_faction=-1;
  HallOfFameIOMode=7; ConsoleGlobal.faction_id_=3;
  for (auto &record:HallOfFameRecords) { record={}; record.present=1; record.year=2222; }
}
void check_presence(int value) {
  for (const auto &record:HallOfFameRecords) {
    assert(record.present==value && record.year==2222);
  }
}
int main() {
  reset(); clear_hall_of_fame(); check_presence(0); assert(HallOfFameIOMode==7);
  reset(); hall_of_fame(1); check_presence(1);
  assert(adds==1 && closes==2 && selected_faction==3 && HallOfFameIOMode==1);
  assert((events==std::vector<std::string>{"open:rb","read","close","init",
    "add","select","modal","open:wb","write","close"}));
  reset(); init_result=1; read_ok=false; hall_of_fame(1); check_presence(0);
  assert(adds==0 && closes==1 && HallOfFameIOMode==0 && events.back()=="init");
  reset(); read_open=false; write_open=false; hall_of_fame(0); check_presence(0);
  assert(adds==0 && closes==0 && HallOfFameIOMode==7);
  assert((events==std::vector<std::string>{"open:rb","init","select","modal","open:wb"}));
  reset(); write_ok=false; hall_of_fame(1); check_presence(0);
  assert(closes==2 && HallOfFameIOMode==1);
  reset(); write_open=false; hall_of_fame(0); check_presence(1);
  assert(closes==1 && HallOfFameIOMode==0);
}
'''
    source = tmp_path / "hall_of_fame.cpp"
    source.write_text(harness)
    executable = tmp_path / "hall_of_fame"
    result = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                             "-iquote", str(ROOT / "src"), str(source), "-o", str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr

"""Controls for the actual cooperative wait; not scheduler/game acceptance."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_wait_pumps_once_preserves_order_and_unsigned_wrap(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/time.cpp").read_text()
    start = source.index("void __cdecl clock_wait(int milliseconds)")
    end = source.index("\n}", start) + 2
    body = source[start:end]
    harness = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
#define WINAPI
typedef uint32_t DWORD;
std::vector<DWORD> ticks;
std::vector<int> events;
unsigned index_;
DWORD timeGetTime() {
    events.push_back(0);
    assert(index_ < ticks.size());
    return ticks[index_++];
}
void do_all_draws() { events.push_back(1); }
int do_non_input() { events.push_back(2); return 17; }
BODY
void check(int duration, std::vector<DWORD> values, unsigned rounds) {
    ticks=values; events.clear(); index_=0;
    clock_wait(duration);
    assert(index_==rounds+1);
    std::vector<int> expected={0};
    for(unsigned i=0;i<rounds;++i) {
        expected.push_back(1); expected.push_back(2); expected.push_back(0);
    }
    assert(events==expected);
}
int main() {
    check(0,{50,50},1);
    check(5,{100,101,104,105},3);
    check(4,{0xFFFFFFFEu,0xFFFFFFFFu,0,2},3);
    check(-1,{100,99},1);
}
'''.replace("BODY", body)
    unit = tmp_path / "wait.cpp"
    unit.write_text(harness)
    executable = tmp_path / "wait"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)

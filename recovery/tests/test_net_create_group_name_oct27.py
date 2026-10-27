"""Original DirectPlay group naming and output/error paths from the actual body."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from match_recovery.local_variants import _function_body_bytes

ROOT = Path(__file__).resolve().parents[2]


def _actual_body():
    row, = (r for r in read_file(ROOT / "src/net_class.cpp")
            if r.address == 0x0062ED40)
    lo, hi = _function_body_bytes(row.region, "Net::create_group")
    return "int Net::create_group(char *group_name) " + row.region.encode()[lo:hi].decode()


PRELUDE = r'''
#include <cassert>
#include <cstring>
typedef unsigned int DWORD;
typedef DWORD DPID;
typedef int HRESULT;
struct DPNAME {
    DWORD dwSize, dwFlags;
    char *lpszShortNameA, *lpszLongNameA;
};
struct DirectPlay {
    int calls, result;
    char *expected;
    HRESULT CreateGroup(DPID *id, DPNAME *name, void *data, DWORD size, DWORD flags) {
        ++calls;
        assert(name->dwSize == sizeof(DPNAME));
        assert(name->dwFlags == 0 && name->lpszShortNameA == 0);
        assert(name->lpszLongNameA == expected);
        assert(data == 0 && size == 0 && flags == 0);
        if (result == 0) *id = 123;
        return result;
    }
};
DirectPlay *NetDirectPlay;
struct Net { int create_group(char *); };
'''
MAIN = r'''
int main() {
    Net net;
    NetDirectPlay = 0;
    assert(net.create_group(0) == 0);
    char group[] = "original formal name";
    DirectPlay dp = {};
    dp.expected = group;
    NetDirectPlay = &dp;
    assert(net.create_group(group) == 123 && dp.calls == 1);
    dp.result = -1;
    assert(net.create_group(group) == 0 && dp.calls == 2);
    dp.result = 1;
    assert(net.create_group(group) == 0 && dp.calls == 3);
    dp.result = 0; dp.expected = 0;
    assert(net.create_group(0) == 123 && dp.calls == 4);
}
'''


@pytest.mark.parametrize("mutation", (None, "short-name", "positive-hresult"))
def test_original_group_name_and_error_contract(tmp_path, mutation):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    body = _actual_body()
    if mutation == "short-name":
        assert body.count("name.lpszLongNameA = group_name;") == 1
        body = body.replace("name.lpszLongNameA = group_name;",
                            "name.lpszShortNameA = group_name;")
    elif mutation == "positive-hresult":
        assert body.count("result == 0") == 1
        body = body.replace("result == 0", "result >= 0")
    source = tmp_path / "group.cpp"
    source.write_text(PRELUDE + body + MAIN)
    program = tmp_path / "group"
    subprocess.run([compiler, "-std=c++11", "-O0", str(source), "-o", str(program)],
                   check=True, capture_output=True)
    result = subprocess.run([str(program)], capture_output=True)
    assert (result.returncode == 0) == (mutation is None)

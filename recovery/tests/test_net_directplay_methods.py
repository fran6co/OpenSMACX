"""Focused host behavior controls for the three typed DirectPlay wrappers.

These compile the actual function definitions from src/net_class.cpp against
an SDK-shaped test double. They are not a Win32 ABI or game acceptance test.
"""

from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/net_class.cpp"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated function: {signature}")


def actual_definitions() -> dict[str, str]:
    source = SOURCE.read_text(encoding="utf-8")
    signatures = {
        "delete": "void Net::delete_group(int group)",
        "add": "int Net::add_to_group(long group, unsigned long member)",
        "player": "int __stdcall sub_62ee00(int first, int second)",
    }
    return {name: extract_function(source, signature) for name, signature in signatures.items()}


HARNESS = r"""
#include <assert.h>
#include <stdint.h>
#define __stdcall
typedef uint32_t DWORD;
typedef DWORD DPID;
typedef int32_t HRESULT;

class IDirectPlay2 {
public:
    int destroy_calls;
    int add_calls;
    int delete_player_calls;
    DPID destroy_id;
    DPID add_first;
    DPID add_second;
    DPID delete_first;
    DPID delete_second;
    HRESULT destroy_result;
    HRESULT add_result;
    HRESULT delete_result;
    IDirectPlay2() : destroy_calls(0), add_calls(0), delete_player_calls(0),
        destroy_id(0), add_first(0), add_second(0), delete_first(0),
        delete_second(0), destroy_result(0), add_result(0), delete_result(0) {}
    HRESULT DestroyGroup(DPID id) {
        ++destroy_calls; destroy_id = id; return destroy_result;
    }
    HRESULT AddPlayerToGroup(DPID first, DPID second) {
        ++add_calls; add_first = first; add_second = second; return add_result;
    }
    HRESULT DeletePlayerFromGroup(DPID first, DPID second) {
        ++delete_player_calls; delete_first = first; delete_second = second;
        return delete_result;
    }
};
static IDirectPlay2 *NetDirectPlay = 0;
class Net {
public:
    void delete_group(int group);
    int add_to_group(long group, unsigned long member);
};

@@ACTUAL_FUNCTIONS@@

int main() {
    Net net;
    NetDirectPlay = 0;
    net.delete_group(-1985229329);
    assert(net.add_to_group(-1985229329L, 4275878552UL) == 7);
    assert(sub_62ee00(-1985229329, -19088744) == 0);

    IDirectPlay2 api;
    NetDirectPlay = &api;
    const DPID first = 0x89ABCDEFu;
    const DPID second = 0xFEDCBA98u;

    api.destroy_result = static_cast<HRESULT>(0x80004005u);
    net.delete_group(-1985229329);
    assert(api.destroy_calls == 1 && api.destroy_id == first);

    api.add_result = 0;
    assert(net.add_to_group(-1985229329L, 4275878552UL) == 0);
    assert(api.add_calls == 1 && api.add_first == first && api.add_second == second);
    api.add_result = 1; // positive nonzero HRESULT is still true in the original
    assert(net.add_to_group(-1985229329L, 4275878552UL) == 1);
    assert(api.add_calls == 2 && api.add_first == first && api.add_second == second);
    api.add_result = static_cast<HRESULT>(0x88770003u);
    assert(net.add_to_group(-1985229329L, 4275878552UL) == 1);
    assert(api.add_calls == 3);

    api.delete_result = 1;
    assert(sub_62ee00(-1985229329, -19088744) == 1);
    assert(api.delete_player_calls == 1 && api.delete_first == first && api.delete_second == second);
    api.delete_result = static_cast<HRESULT>(0x88770003u);
    assert(sub_62ee00(-1985229329, -19088744) == api.delete_result);
    assert(api.delete_player_calls == 2 && api.delete_first == first && api.delete_second == second);
    assert(api.destroy_calls == 1 && api.add_calls == 3);
    return 0;
}
"""


def compile_and_run(definitions: dict[str, str], compiler: str) -> subprocess.CompletedProcess[str]:
    cpp = HARNESS.replace("@@ACTUAL_FUNCTIONS@@", "\n\n".join(definitions.values()))
    with tempfile.TemporaryDirectory(prefix="net-directplay-host-") as temp:
        source = Path(temp) / "test.cpp"
        binary = Path(temp) / "test"
        source.write_text(cpp, encoding="utf-8")
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
            text=True,
            capture_output=True,
        )
        if build.returncode:
            pytest.fail(f"{compiler} host build failed:\n{build.stdout}\n{build.stderr}")
        return subprocess.run([str(binary)], text=True, capture_output=True)


COMPILERS = [pytest.param(path, id=Path(path).name) for name in ("g++", "clang++") if (path := shutil.which(name))]


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_typed_directplay_behavior(compiler: str) -> None:
    result = compile_and_run(actual_definitions(), compiler)
    assert result.returncode == 0, result.stderr


def test_damage_mutants_compile_but_fail_runtime() -> None:
    baseline = actual_definitions()
    mutants = {
        "interface pointer returned instead of HRESULT": {
            **baseline,
            "player": baseline["player"].replace(
                "return static_cast<int>(direct_play->DeletePlayerFromGroup(",
                "(void)direct_play->DeletePlayerFromGroup(",
            ).replace(
                "static_cast<DPID>(second)));",
                "static_cast<DPID>(second));\n"
                "  return static_cast<int>(reinterpret_cast<intptr_t>(direct_play));",
            ),
        },
        "wrong null delete-player result": {
            **baseline,
            "player": baseline["player"].replace("return 0;", "return 7;"),
        },
        "destroy group skipped": {
            **baseline,
            "delete": baseline["delete"].replace(
                "direct_play->DestroyGroup(static_cast<DPID>(group));",
                "(void)group;",
            ),
        },
        "wrong add SDK method": {
            **baseline,
            "add": baseline["add"].replace("AddPlayerToGroup", "DeletePlayerFromGroup"),
        },
        "swapped add argument order": {
            **baseline,
            "add": baseline["add"].replace(
                "static_cast<DPID>(group), static_cast<DPID>(member)",
                "static_cast<DPID>(member), static_cast<DPID>(group)",
            ),
        },
        "booleanized delete-player HRESULT": {
            **baseline,
            "player": baseline["player"].replace(
                "return static_cast<int>(direct_play->DeletePlayerFromGroup(",
                "return direct_play->DeletePlayerFromGroup(",
            ).replace(
                "static_cast<DPID>(second)));", "static_cast<DPID>(second)) != 0;"
            ),
        },
        "add tests failure sign instead of nonzero": {
            **baseline,
            "add": baseline["add"].replace("return result != 0;", "return result < 0;"),
        },
        "wrong null add result": {
            **baseline,
            "add": baseline["add"].replace("return 7;", "return 0;"),
        },
    }
    for label, definitions in mutants.items():
        assert definitions != baseline, f"mutation anchor absent: {label}"
        for compiler in (shutil.which("g++"), shutil.which("clang++")):
            if compiler is None:
                continue
            result = compile_and_run(definitions, compiler)
            assert result.returncode != 0, f"{label} unexpectedly survived with {compiler}"

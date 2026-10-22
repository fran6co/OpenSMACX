"""Host controls for the moved NetDaemon wait and notification wrappers.

Every tested body is extracted from its current canonical C++ owner. The small
host doubles verify source-level routing/arguments only, not VC6 ABI or game
runtime behavior.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import read_file
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/netdaemon_interface.cpp").is_file())
NEW_SOURCE = ROOT / "src/netdaemon_interface.cpp"
OLD_SOURCE = ROOT / "src/netdaemon.cpp"
FUNCTIONS = (
    ("wait_2", "void __cdecl wait_2()"),
    ("synch_veh", "void __cdecl synch_veh(int id)"),
    ("synch_base", "void __cdecl synch_base(int id)"),
    ("synch_energy", "void __cdecl synch_energy(int id)"),
    ("synch_researching", "void __cdecl synch_researching(int id)"),
    ("synch_leader", "void __cdecl synch_leader(int id)"),
    ("synch_ai", "void __cdecl synch_ai(int id)"),
    ("synch_research", "void __cdecl synch_research(int id)"),
    ("synch_alloc", "void __cdecl synch_alloc(int id)"),
    ("synch_soc", "void __cdecl synch_soc(int id)"),
    ("synch_proto", "void __cdecl synch_proto(int id)"),
    ("synch_obs", "void __cdecl synch_obs(int id)"),
    ("synch_diplo", "void __cdecl synch_diplo(int a, int b)"),
    ("synch_template", "void __cdecl synch_template(int id)"),
    ("synch_radius", "void __cdecl synch_radius(int id)"),
)
TOKENS = {name: f"/* ACTUAL_{name.upper()} */" for name, _ in FUNCTIONS}


HARNESS = r'''#include <cassert>
#include <cstdio>
#define __cdecl
static int IsMultiplayerNet;
static int wait_calls, net_task_calls;
static void wait_task() { ++wait_calls; }
struct NetDaemon {
    int calls, args[7];
    void net_tasks() { ++net_task_calls; }
    void synch(int op, int a, int b, int c, const char *text, int repeated, int flags) {
        ++calls;
        args[0]=op; args[1]=a; args[2]=b; args[3]=c;
        args[4]=(text == 0 ? -1 : 1); args[5]=repeated; args[6]=flags;
    }
} NetDaemonNet;
/* ACTUAL_WAIT_2 */
/* ACTUAL_SYNCH_VEH */
/* ACTUAL_SYNCH_BASE */
/* ACTUAL_SYNCH_ENERGY */
/* ACTUAL_SYNCH_RESEARCHING */
/* ACTUAL_SYNCH_LEADER */
/* ACTUAL_SYNCH_AI */
/* ACTUAL_SYNCH_RESEARCH */
/* ACTUAL_SYNCH_ALLOC */
/* ACTUAL_SYNCH_SOC */
/* ACTUAL_SYNCH_PROTO */
/* ACTUAL_SYNCH_OBS */
/* ACTUAL_SYNCH_DIPLO */
/* ACTUAL_SYNCH_TEMPLATE */
/* ACTUAL_SYNCH_RADIUS */

static int wait_modes() {
    const int modes[3] = {0, 2, -3};
    for (int i=0; i<3; ++i) {
        wait_calls=net_task_calls=0; IsMultiplayerNet=modes[i]; wait_2();
        if (modes[i] == 0) {
            if (wait_calls != 1 || net_task_calls != 0) return 0;
        } else if (wait_calls != 0 || net_task_calls != 1) return 0;
    }
    return 1;
}
static int wrapper_calls() {
    const int id=731, other=-29;
    const int expected[][3] = {
        {0x11,id,0},{0x13,id,0},{0xB,id,0},{0xA,id,0},{0x6,id,0},
        {0x8,id,0},{0x9,id,0},{0xC,id,0},{0xD,id,0},{0xE,id,0},
        {0x10,id,0},{0x16,id,other},{0x18,id,0},{0x23,id,0}
    };
    void (*calls[])(int) = {synch_veh,synch_base,synch_energy,synch_researching,
        synch_leader,synch_ai,synch_research,synch_alloc,synch_soc,synch_proto,
        synch_obs,synch_template,synch_radius};
    for (int i=0; i<13; ++i) {
        NetDaemonNet.calls=0; calls[i](id);
        if (NetDaemonNet.calls != 1 || NetDaemonNet.args[0] != expected[i < 11 ? i : i+1][0] ||
            NetDaemonNet.args[1] != id || NetDaemonNet.args[2] != 0 ||
            NetDaemonNet.args[3] != 0 || NetDaemonNet.args[4] != -1 ||
            NetDaemonNet.args[5] != 1 || NetDaemonNet.args[6] != 0x2101) return 0;
    }
    NetDaemonNet.calls=0; synch_diplo(id,other);
    return NetDaemonNet.calls == 1 && NetDaemonNet.args[0] == expected[11][0] &&
        NetDaemonNet.args[1] == id && NetDaemonNet.args[2] == other &&
        NetDaemonNet.args[3] == 0 && NetDaemonNet.args[4] == -1 &&
        NetDaemonNet.args[5] == 1 && NetDaemonNet.args[6] == 0x2101;
}
int main() {
    if (!wait_modes()) { std::puts("wait route mismatch"); return 1; }
    if (!wrapper_calls()) { std::puts("synch wrapper mismatch"); return 2; }
    return 0;
}
'''


def actual_text() -> tuple[str, str]:
    return NEW_SOURCE.read_text(encoding="utf-8"), OLD_SOURCE.read_text(encoding="utf-8")


def source_ownership() -> None:
    new, old = actual_text()
    addresses = (0x51F2C0, 0x593220, 0x593250, 0x593280, 0x5932B0,
                 0x5932E0, 0x593310, 0x593340, 0x593390, 0x5933C0,
                 0x5933F0, 0x593420, 0x593450, 0x5934B0, 0x5934E0)
    new_rows = read_file(NEW_SOURCE)
    old_rows = read_file(OLD_SOURCE)
    assert len(new_rows) == len(addresses)
    assert {row.address for row in new_rows} == set(addresses)
    assert not ({row.address for row in old_rows} & set(addresses))
    for name, signature in FUNCTIONS:
        assert new.count(signature) == 1, name
        assert old.count(signature) == 0, name
    # State and service implementation were intentionally not migrated.
    for token in ("IsMultiplayerNet", "wait_task()", "NetDaemonNet.net_tasks()"):
        assert token in new
    for definition in ("void NetDaemon::synch(", "void NetDaemon::net_tasks()"):
        assert definition not in new
        assert old.count(definition) == 1


def translation_unit(source: str) -> str:
    assert all(HARNESS.count(token) == 1 for token in TOKENS.values())
    rendered = HARNESS
    for name, signature in FUNCTIONS:
        body = extract_definition(source, signature)
        rendered = rendered.replace(TOKENS[name], body, 1)
    return rendered


def compile_and_run(source: str, compiler_name: str, tmp_path: Path,
                    stem: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp, exe = tmp_path / f"{stem}.cpp", tmp_path / stem
    cpp.write_text(translation_unit(source), encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


def test_moved_bindings_have_one_real_source_owner() -> None:
    source_ownership()


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_wait_and_all_synch_wrappers(compiler_name: str, tmp_path: Path) -> None:
    source, _ = actual_text()
    result = compile_and_run(source, compiler_name, tmp_path, "netdaemon_interface_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("name", "old", "new"), (
    ("wait_route", "if (IsMultiplayerNet != 0)", "if (IsMultiplayerNet == 0)"),
    ("opcode", "NetDaemonNet.synch(0x11, id", "NetDaemonNet.synch(0x12, id"),
    ("argument", "NetDaemonNet.synch(0x11, id, 0, 0, nullptr",
     "NetDaemonNet.synch(0x11, id + 1, 0, 0, nullptr"),
    ("diplo_argument", "NetDaemonNet.synch(0x16, a, b, 0, nullptr",
     "NetDaemonNet.synch(0x16, a, b + 1, 0, nullptr"),
))
def test_compiling_mutants_are_observed(compiler_name: str, name: str,
                                        old: str, new: str, tmp_path: Path) -> None:
    source, _ = actual_text()
    assert source.count(old) == 1, (name, source.count(old))
    result = compile_and_run(source.replace(old, new, 1), compiler_name, tmp_path,
                             f"netdaemon_interface_{name}_mutant")
    assert result.returncode != 0, f"{name} mutation escaped actual-body controls"

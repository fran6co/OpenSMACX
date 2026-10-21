"""Actual-source host controls for the six relocated network diplomacy wrappers."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
NETWORK_SOURCE = ROOT / "src/faction_network.cpp"

FUNCTIONS = (
    "net_treaty_on",
    "net_treaty_off",
    "net_set_treaty",
    "net_agenda_on",
    "net_agenda_off",
    "net_set_agenda",
)


def body(source: str, name: str) -> str:
    match = re.search(r"(?m)^void __cdecl " + re.escape(name) + r"\s*\(", source)
    assert match, f"missing actual source function {name}"
    opening = source.index("{", match.end())
    cursor, depth = opening + 1, 1
    while depth:
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
        cursor += 1
    return source[match.start():cursor]


HARNESS = r'''#include <cassert>
#include <sstream>
#include <string>
#include <vector>

#ifndef __cdecl
#define __cdecl
#endif

char DiploLabelSendingTREATYON[] = "Sending TREATY ON";
char DiploLabelSendingTREATYOFF[] = "Sending TREATY OFF";
char DiploLabelSendingSETTREATY[] = "Sending SET TREATY";
char DiploLabelSendingAGENDAON[] = "Sending AGENDA ON";
char DiploLabelSendingAGENDAOFF[] = "Sending AGENDA OFF";
char DiploLabelSendingSETAGENDA[] = "Sending SET AGENDA";

int IsMultiplayerNet = 0;
std::vector<std::string> effects;

std::string four(const char *tag, int a, int b, int c) {
    std::ostringstream out; out << tag << '|' << a << '|' << b << '|' << c;
    return out.str();
}
std::string five(const char *tag, int a, int b, int c, int d) {
    std::ostringstream out; out << tag << '|' << a << '|' << b << '|' << c << '|' << d;
    return out.str();
}
std::string logged(const char *label, int a, int b, int c) {
    return std::string("log|") + label + four("", a, b, c);
}
std::string messaged(int op, int zero, int a, int b, int c, int d) {
    std::ostringstream out; out << "message|" << op << '|' << zero << '|'
        << a << '|' << b << '|' << c << '|' << d;
    return out.str();
}
std::string waited(int op) {
    std::ostringstream out; out << "wait|" << op; return out.str();
}
void log_say(char *label, int a, int b, int c) { effects.push_back(logged(label, a, b, c)); }
void message_data(int op, int zero, int a, int b, int c, int d) {
    effects.push_back(messaged(op, zero, a, b, c, d));
}
struct NetDaemon {
    void await_diplo(int op) {
        std::ostringstream out; out << "wait|" << op; effects.push_back(out.str());
    }
} NetDaemonNet;

void treaty_on(int a, int b, int c) { effects.push_back(four("treaty_on", a, b, c)); }
void treaty_off(int a, int b, int c) { effects.push_back(four("treaty_off", a, b, c)); }
void set_treaty(int a, int b, int c, int d) { effects.push_back(five("set_treaty", a, b, c, d)); }
void agenda_on(int a, int b, int c) { effects.push_back(four("agenda_on", a, b, c)); }
void agenda_off(int a, int b, int c) { effects.push_back(four("agenda_off", a, b, c)); }
void set_agenda(int a, int b, int c, int d) { effects.push_back(five("set_agenda", a, b, c, d)); }

void __cdecl net_treaty_on(int, int, int, int);
void __cdecl net_treaty_off(int, int, int, int);
void __cdecl net_set_treaty(int, int, int, int, int);
void __cdecl net_agenda_on(int, int, int, int);
void __cdecl net_agenda_off(int, int, int, int);
void __cdecl net_set_agenda(int, int, int, int, int);

void expect_one(const std::string &a) {
    assert(effects.size() == 1 && effects[0] == a);
}
void expect_two(const std::string &a, const std::string &b) {
    assert(effects.size() == 2 && effects[0] == a && effects[1] == b);
}
void expect_three(const std::string &a, const std::string &b, const std::string &c) {
    assert(effects.size() == 3 && effects[0] == a && effects[1] == b && effects[2] == c);
}

#define CHECK_FOUR(fn, local, label, opcode, waitop) do { \
    effects.clear(); IsMultiplayerNet = 0; fn(11, 22, 33, 0); \
    expect_one(four(#local, 11, 22, 33)); \
    effects.clear(); IsMultiplayerNet = 1; fn(11, 22, 33, 0); \
    expect_two(logged(label, 11, 22, 33), messaged(opcode, 0, 11, 22, 33, 0)); \
    effects.clear(); fn(11, 22, 33, 1); \
    expect_three(logged(label, 11, 22, 33), messaged(opcode, 0, 11, 22, 33, 0), \
                 waited(waitop)); \
} while (0)

#define CHECK_SET(fn, local, label, opcode, waitop) do { \
    effects.clear(); IsMultiplayerNet = 0; fn(11, 22, 33, -7, 0); \
    expect_one(five(#local, 11, 22, 33, -7)); \
    effects.clear(); IsMultiplayerNet = 1; fn(11, 22, 33, -7, 0); \
    expect_two(logged(label, 11, 22, 33), messaged(opcode, 0, 11, 22, 33, -7)); \
    effects.clear(); fn(11, 22, 33, -7, 1); \
    expect_three(logged(label, 11, 22, 33), messaged(opcode, 0, 11, 22, 33, -7), \
                 waited(waitop)); \
} while (0)

int main() {
    CHECK_FOUR(net_treaty_on, treaty_on, DiploLabelSendingTREATYON, 0x2441, 0x441);
    CHECK_FOUR(net_treaty_off, treaty_off, DiploLabelSendingTREATYOFF, 0x2442, 0x442);
    CHECK_SET(net_set_treaty, set_treaty, DiploLabelSendingSETTREATY, 0x2443, 0x443);
    CHECK_FOUR(net_agenda_on, agenda_on, DiploLabelSendingAGENDAON, 0x2444, 0x444);
    CHECK_FOUR(net_agenda_off, agenda_off, DiploLabelSendingAGENDAOFF, 0x2445, 0x445);
    CHECK_SET(net_set_agenda, set_agenda, DiploLabelSendingSETAGENDA, 0x2446, 0x446);
}
'''


def program(source: str) -> str:
    return HARNESS + "\n" + "\n".join(body(source, name) for name in FUNCTIONS)


def compile_and_run(compiler: str, tmp_path: Path, source: str) -> subprocess.CompletedProcess:
    cpp = tmp_path / "faction_network_owner.cpp"
    exe = tmp_path / "faction_network_owner"
    cpp.write_text(program(source), encoding="utf-8")
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True, timeout=45)
    return subprocess.run([str(exe)], timeout=15)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_six_actual_wrappers_preserve_local_network_and_wait_effects(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = NETWORK_SOURCE.read_text(encoding="utf-8")
    assert compile_and_run(compiler, tmp_path, source).returncode == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("old, new", (
    ("message_data(0x2441, 0, faction_id, faction_id_with, treaty, 0);",
     "message_data(0x2442, 0, faction_id, faction_id_with, treaty, 0);"),
    ("NetDaemonNet.await_diplo(0x446);", "NetDaemonNet.await_diplo(0x445);"),
    ("agenda_off(faction_id, faction_id_with, agenda);",
     "agenda_on(faction_id, faction_id_with, agenda);"),
))
def test_compile_success_behavior_mutants_are_caught(tmp_path, compiler_name, old, new):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = NETWORK_SOURCE.read_text(encoding="utf-8")
    assert old in source
    mutant = source.replace(old, new, 1)
    assert mutant != source
    assert compile_and_run(compiler, tmp_path, mutant).returncode != 0

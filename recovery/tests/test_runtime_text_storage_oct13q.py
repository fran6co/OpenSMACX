"""Actual-source controls for writable enemy-log and Zone-launcher text.

The harness compiles the four product arrays and exact consumer call
expressions extracted from their owner functions. It checks the source-owned
array addresses and contents at the call boundary. It does not execute the
complete enemy AI or Net join implementation, nor prove native/VC6 lifetime or
whole-application behavior.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
GENERAL = ROOT / "src" / "general.cpp"
NET = ROOT / "src" / "net_class.cpp"


def extract_definition(source: str, signature: str) -> str:
    if source.count(signature) != 1:
        raise AssertionError(f"expected one {signature!r}")
    start = source.index(signature)
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing body for {signature}")
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block"
                i += 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return source[start : i + 1]
        elif state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 2
                continue
        else:
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed body for {signature}")


def extract_call(body: str, prefix: str) -> str:
    if body.count(prefix) != 1:
        raise AssertionError(f"expected one {prefix!r} call in selected body")
    start = body.index(prefix)
    opening = body.find("(", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(body):
        c = body[i]
        n = body[i + 1] if i + 1 < len(body) else ""
        if state == "code":
            if c == '"':
                state = "string"
            elif c == "'":
                state = "char"
            elif c == "(":
                depth += 1
            elif c == ")":
                depth -= 1
                if depth == 0:
                    semicolon = body.find(";", i)
                    if semicolon < 0:
                        raise AssertionError(f"missing semicolon after {prefix}")
                    return body[start : semicolon + 1]
        else:
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed call {prefix}")


def extract_if_block(body: str, condition: str) -> str:
    if body.count(condition) != 1:
        raise AssertionError(f"expected one branch {condition!r}")
    start = body.index(condition)
    opening = body.find("{", start)
    if opening < 0:
        raise AssertionError("missing if-block body")
    depth = 0
    for i in range(opening, len(body)):
        if body[i] == "{":
            depth += 1
        elif body[i] == "}":
            depth -= 1
            if depth == 0:
                return body[start : i + 1]
    raise AssertionError("unclosed if-block")


def declaration(source: str, name: str, *, is_static: bool = False) -> str:
    prefix = "static " if is_static else ""
    pattern = rf'(?m)^{re.escape(prefix)}char {re.escape(name)}\[\] = "[^"]*";$'
    matches = re.findall(pattern, source)
    if len(matches) != 1:
        raise AssertionError(f"expected one writable char array declaration for {name}")
    return matches[0]


def actual_fragments(general_source: str, net_source: str):
    arrays = "\n".join(
        (
            declaration(general_source, "LogEnemyVehicleDone"),
            declaration(general_source, "LogEnemyVehicleAction"),
            declaration(general_source, "LogEnemyVehicle"),
            declaration(net_source, "NetZoneLauncher", is_static=True),
        )
    )
    enemy = extract_definition(general_source, "int __cdecl enemy_veh(int veh_id)")
    done = extract_call(enemy, "log_say(LogEnemyVehicleDone,")
    action = extract_call(enemy, "log_say(LogEnemyVehicleAction,")
    state = extract_call(enemy, "log_say(LogEnemyVehicle,")
    join = extract_definition(net_source, "int Net::join_service(ServiceStruct *service)")
    launch = extract_if_block(
        join, "if (memcmp(provider, &KeptServiceProviderGuid, 0x10) == 0)"
    )
    return arrays, done, action, state, launch


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

ARRAYS

struct VehForLog {
    int order;
    int waypoint_x[1];
    int waypoint_y[1];
    int x;
    int y;
};
VehForLog Vehs[1];

struct LogRecord {
    const char *label;
    char *text;
    int count;
    int values[3];
};
static LogRecord log_record;
static int log_count;
void log_say(char *label, int first, int second, int third) {
    log_record = LogRecord{label, 0, 3, {first, second, third}};
    ++log_count;
}
void log_say(const char *label, int first, int second, int third) {
    log_record = LogRecord{label, 0, 3, {first, second, third}};
    ++log_count;
}
void log_say(char *label, char *text, int first, int second, int third) {
    log_record = LogRecord{label, text, 3, {first, second, third}};
    ++log_count;
}
void log_say(const char *label, char *text, int first, int second, int third) {
    log_record = LogRecord{label, text, 3, {first, second, third}};
    ++log_count;
}

static void emit_done() {
    const int unit_id = 0;
    DONE_CALL
}
static void emit_action() {
    const int unit_id = 0;
    char message[8] = "17";
    ACTION_CALL
    assert(log_record.text == message);
}
static void emit_state() {
    const int unit_id = 0;
    char state_text[2] = {'S', 0};
    STATE_CALL
    assert(log_record.text == state_text);
}

struct GUID { unsigned char bytes[16]; };
static const GUID KeptServiceProviderGuid = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}};
static const unsigned int SW_SHOW = 5;
static const char *winexec_text;
static unsigned int winexec_flags;
static int winexec_calls;
static unsigned int winexec_result = 33;
static unsigned int WinExec(const char *name, unsigned int flags) {
    winexec_text = name;
    winexec_flags = flags;
    ++winexec_calls;
    return winexec_result;
}
static int launch_if_kept(GUID *provider) {
    LAUNCH_BRANCH
    return -7;
}

int main() {
    assert(sizeof(LogEnemyVehicleDone) == 19);
    assert(sizeof(LogEnemyVehicleAction) == 21);
    assert(sizeof(LogEnemyVehicle) == 14);
    assert(sizeof(NetZoneLauncher) == 13);
    assert(std::strcmp(LogEnemyVehicleDone, "Enemy Vehicle DONE") == 0);
    assert(std::strcmp(LogEnemyVehicleAction, "Enemy Vehicle ACTION") == 0);
    assert(std::strcmp(LogEnemyVehicle, "Enemy Vehicle") == 0);
    assert(std::strcmp(NetZoneLauncher, "mplaynow.exe") == 0);

    Vehs[0].order = 31;
    Vehs[0].waypoint_x[0] = 41;
    Vehs[0].waypoint_y[0] = 42;
    Vehs[0].x = 51;
    Vehs[0].y = 52;

    LogEnemyVehicleDone[0] = '!';
    log_count = 0; emit_done();
    assert(log_count == 1 && log_record.label == LogEnemyVehicleDone);
    assert(std::strcmp(log_record.label, "!nemy Vehicle DONE") == 0);
    assert(log_record.text == 0 && log_record.count == 3);
    assert(log_record.values[0] == 0 && log_record.values[1] == 0 && log_record.values[2] == 0);

    LogEnemyVehicleAction[0] = '!';
    log_count = 0; emit_action();
    assert(log_count == 1 && log_record.label == LogEnemyVehicleAction);
    assert(std::strcmp(log_record.label, "!nemy Vehicle ACTION") == 0);
    assert(log_record.values[0] == 31 && log_record.values[1] == 41 && log_record.values[2] == 42);

    LogEnemyVehicle[0] = '!';
    log_count = 0; emit_state();
    assert(log_count == 1 && log_record.label == LogEnemyVehicle);
    assert(std::strcmp(log_record.label, "!nemy Vehicle") == 0);
    assert(log_record.values[0] == 0 && log_record.values[1] == 51 && log_record.values[2] == 52);

    GUID matching = KeptServiceProviderGuid;
    NetZoneLauncher[0] = 'M';
    assert(launch_if_kept(&matching) == 0x16);
    assert(winexec_calls == 1 && winexec_text == NetZoneLauncher);
    assert(std::strcmp(winexec_text, "Mplaynow.exe") == 0 && winexec_flags == SW_SHOW);
    GUID other = {{0}};
    assert(launch_if_kept(&other) == -7);
    assert(winexec_calls == 1);
    return 0;
}
'''


def build_harness(general_source: str, net_source: str, mutation: str = "") -> str:
    arrays, done, action, state, launch = actual_fragments(general_source, net_source)
    if mutation == "wrong_enemy_window":
        done = done.replace("LogEnemyVehicleDone", "LogEnemyVehicleAction", 1)
    elif mutation == "literal_enemy_label":
        done = done.replace("LogEnemyVehicleDone", '"Enemy Vehicle DONE"', 1)
    elif mutation == "wrong_launcher_window":
        launch = launch.replace("NetZoneLauncher", "LogEnemyVehicle", 1)
    elif mutation == "wrong_show_flag":
        launch = launch.replace("WinExec(NetZoneLauncher, SW_SHOW)",
                                "WinExec(NetZoneLauncher, 0)", 1)
    return (
        HARNESS.replace("ARRAYS", arrays)
        .replace("DONE_CALL", done)
        .replace("ACTION_CALL", action)
        .replace("STATE_CALL", state)
        .replace("LAUNCH_BRANCH", launch)
    )


def compile_run(tmp_path: Path, compiler: str, program: str, should_pass: bool):
    source_path = tmp_path / "runtime_text.cpp"
    executable = tmp_path / "runtime_text"
    source_path.write_text(program, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(executable)],
        text=True, capture_output=True, timeout=45, check=False,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(executable)], text=True, capture_output=True,
                            timeout=15, check=False)
    if should_pass:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "source mutation escaped the runtime oracle"


def compiler_names():
    names = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert names, "a host C++ compiler is required"
    return names


@pytest.mark.parametrize("compiler", compiler_names())
def test_actual_storage_and_consumer_calls_use_writable_text_windows(tmp_path, compiler):
    compile_run(tmp_path, compiler,
                build_harness(GENERAL.read_text(), NET.read_text()), True)


@pytest.mark.parametrize("compiler", compiler_names())
@pytest.mark.parametrize("mutation", ("wrong_enemy_window", "literal_enemy_label", "wrong_launcher_window", "wrong_show_flag"))
def test_text_window_and_argument_mutations_are_detected(tmp_path, compiler, mutation):
    general_source = GENERAL.read_text()
    net_source = NET.read_text()
    compile_run(tmp_path, compiler,
                build_harness(general_source, net_source, mutation), False)

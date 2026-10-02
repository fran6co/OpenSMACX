"""Actual-source behavioral controls; typed mocks do not prove native layout/ABI."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(os.environ.get("OPENSMACX_ROOT", Path(__file__).resolve().parents[2])).resolve()
OWNER_SOURCE = ROOT / "src/netdaemon.cpp"
HARNESS = Path(__file__).with_name("netdaemon_protocol_harness.cpp")


def _end_of_function(text: str, start: int) -> int:
    brace = text.index("{", start)
    depth = 0
    state = "code"
    escaped = False
    i = brace
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif c == "\\":
                escaped = True
            elif (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        elif c == "/" and n == "/":
            state = "line"
            i += 1
        elif c == "/" and n == "*":
            state = "block"
            i += 1
        elif c == '"':
            state = "string"
        elif c == "'":
            state = "char"
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise AssertionError("unterminated NetDaemon method")


def _actual_methods() -> tuple[str, str]:
    if not OWNER_SOURCE.is_file():
        raise AssertionError(f"actual owner source does not exist: {OWNER_SOURCE}")
    source = OWNER_SOURCE.read_text()
    extracted = []
    for signature in ("int NetDaemon::action(int veh_id, int mode)",
                      "int NetDaemon::order_veh(int veh_id, int direction, int mode)"):
        if source.count(signature) != 1:
            raise AssertionError(f"expected exactly one actual owner definition: {signature}")
        start = source.index(signature)
        body = source[start:_end_of_function(source, start)]
        if "BODY GOES HERE." in body or "PLACEHOLDER" in body:
            raise AssertionError(f"{signature} is still a placeholder in {OWNER_SOURCE}")
        extracted.append(body)
    return extracted[0], extracted[1]


def _translation_unit(action_body: str, order_body: str) -> str:
    harness = HARNESS.read_text()
    assert harness.count("/*ACTUAL_METHODS*/") == 1
    # Source spelling `nullptr` is lowered to its C++03 null-pointer constant
    # solely for this legacy-language host compile; method structure is intact.
    action_body = action_body.replace("nullptr", "0")
    order_body = order_body.replace("nullptr", "0")
    return harness.replace("/*ACTUAL_METHODS*/", action_body + "\n\n" + order_body)


def _run_control(mode: str, action_body: str, order_body: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(os.environ.get("CXX", "g++"))
    if not compiler:
        pytest.skip("no host C++ compiler available")
    with tempfile.TemporaryDirectory(prefix="netdaemon-protocol-host-") as temp:
        tmp = Path(temp)
        cpp = tmp / "control.cpp"
        exe = tmp / "control-test"
        cpp.write_text(_translation_unit(action_body, order_body))
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
             str(cpp), "-o", str(exe)], text=True, capture_output=True, timeout=45)
        assert build.returncode == 0, f"{build.stdout}\n{build.stderr}"
        env = dict(os.environ)
        env.setdefault("ASAN_OPTIONS", "detect_leaks=0:halt_on_error=1")
        env.setdefault("UBSAN_OPTIONS", "halt_on_error=1:print_stacktrace=1")
        return subprocess.run([str(exe), mode], text=True, capture_output=True,
                              timeout=15, env=env)


def test_extractor_requires_actual_netdaemon_owner_methods() -> None:
    action_body, order_body = _actual_methods()
    assert "int NetDaemon::action(int veh_id, int mode)" in action_body
    assert "int NetDaemon::order_veh(int veh_id, int direction, int mode)" in order_body
    assert "NetDaemonNet.lock_state_[1].leading_words_[3]" in action_body
    assert "PlayersData[" in order_body and "RadiusOffsetX" in order_body


@pytest.mark.parametrize("mode", ["action", "timing", "order_speed", "radius", "direction",
                                   "globalfailed", "mode0wait", "earlyreject", "syncgate"])
def test_actual_owner_host_controls(mode: str) -> None:
    action_body, order_body = _actual_methods()
    run = _run_control(mode, action_body, order_body)
    assert run.returncode == 0, f"actual owner source: {mode}\n{run.stdout}\n{run.stderr}"


@pytest.mark.parametrize(
    "mode,needle,replacement,description",
    [
        ("timing", "Vehs[veh_id].order, 0)", "action_type, 0)",
         "action log must reload order after automated_lock callback"),
        ("timing", "NetDaemonNet.lock_state_[1].leading_words_[3] = saved_action_mode;",
         "lock_state_[1].leading_words_[3] = saved_action_mode;",
         "0x93E904 timing word belongs to singleton, not receiver"),
        ("radius", "PlayersData[other_faction]\n                             .diplo_treaties[ConsoleGlobal.faction_id_]",
         "PlayersData[ConsoleGlobal.faction_id_]\n                             .diplo_treaties[other_faction]",
         "native radius treaty row is the other faction"),
        ("direction", "PlayersData[faction]\n                 .diplo_treaties[Vehs[fixed_vehicle].faction_id]",
         "PlayersData[Vehs[fixed_vehicle].faction_id]\n                 .diplo_treaties[faction]",
         "planet-buster treaty row is the vehicle faction"),
        ("earlyreject", "difference == 0))\n            goto release_lock_only;",
         "difference == 0))\n            goto release_lock;",
         "native short reject tail must not synchronize vehicle"),
        ("globalfailed", 'NetMsgPopup->pop("GLOBALFAILED",', 'NetMsgPopup->pop("XFAILED",',
         "native GLOBALFAILED edge has its own popup tail"),
    ],
)
def test_protocol_mutants_are_detected(mode: str, needle: str,
                                       replacement: str, description: str) -> None:
    action_body, order_body = _actual_methods()
    target = action_body if mode == "timing" else order_body
    assert needle in target, f"mutation anchor missing: {description}"
    changed = target.replace(needle, replacement, 1)
    if mode == "timing":
        action_body = changed
    else:
        order_body = changed
    run = _run_control(mode, action_body, order_body)
    assert run.returncode != 0, f"protocol mutation escaped: {description}"

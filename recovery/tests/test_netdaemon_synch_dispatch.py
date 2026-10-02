"""Strict actual-source controls for NetDaemon synch copy/commit dispatch."""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve()
_root = os.environ.get("OPENSMACX_ROOT")
ROOT = Path(_root).resolve() if _root else HERE.parents[2]
NET = ROOT / "src/netdaemon.cpp"


def _mask(text):
    out = list(text)
    mode = "code"
    i = 0
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if mode == "code":
            if c == "/" and n == "/":
                out[i] = out[i + 1] = " "
                i += 2
                mode = "line"
                continue
            if c == "/" and n == "*":
                out[i] = out[i + 1] = " "
                i += 2
                mode = "block"
                continue
            if c == '"':
                out[i] = " "
                mode = "string"
            elif c == "'":
                out[i] = " "
                mode = "char"
        elif mode == "line":
            if c == "\n":
                mode = "code"
            else:
                out[i] = " "
        elif mode == "block":
            out[i] = " "
            if c == "*" and n == "/":
                out[i + 1] = " "
                i += 2
                mode = "code"
                continue
        else:
            out[i] = " "
            if c == "\\" and i + 1 < len(text):
                out[i + 1] = " "
                i += 2
                continue
            if (mode == "string" and c == '"') or (mode == "char" and c == "'"):
                mode = "code"
        i += 1
    return "".join(out)


def _definition(text, signature, label):
    code = _mask(text)
    found = []
    for match in re.finditer(signature + r"\s*\(", code):
        depth = 1
        pos = match.end()
        while pos < len(code) and depth:
            depth += (code[pos] == "(") - (code[pos] == ")")
            pos += 1
        if depth:
            continue
        while pos < len(code) and code[pos].isspace():
            pos += 1
        if pos < len(code) and code[pos] == "{":
            found.append((match.start(), pos))
    assert len(found) == 1, "expected one actual source definition: " + label
    start, opening = found[0]
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            assert "BODY GOES HERE" not in body, label + " is unfinished"
            return body
    raise AssertionError("unterminated definition: " + label)


def _extract(net):
    stage = re.search(
        r"^static unsigned char NetDaemonRulesSyncStaging\[0x1920\];$",
        net, re.MULTILINE)
    assert stage, "actual persistent rules staging declaration is required"
    capture = _definition(net, r"uint32_t\s+NetDaemonSyncWire::current_faction",
                          "current_faction")
    out = _definition(net, r"bool\s+NetDaemonSyncWire::copy_out", "copy_out")
    incoming = _definition(net, r"bool\s+NetDaemonSyncWire::copy_in", "copy_in")
    commit = _definition(net, r"void\s+NetDaemonSyncWire::commit_rules", "commit_rules")
    required = ("NetDaemonSyncRules::copy_rule_wire_chunk", "copy_nonrule_chunk",
                "NetDaemonSyncRules::commit_staged_rules")
    all_body = "\n".join((capture, out, incoming, commit))
    for token in required:
        assert token in all_body, "dispatcher source no longer forwards " + token
    return (stage.group(0) + "\n" + capture + "\n" + out + "\n" + incoming +
            "\n" + commit)


def _compiler_names():
    requested = os.environ.get("NETDAEMON_HOST_CXX")
    names = [requested] if requested else ["g++", "clang++"]
    return [n for n in names if n and shutil.which(n)]


def _compile(tmp_path, compiler, injection, expected=True):
    harness = (HERE.with_name("netdaemon_synch_dispatch_harness.cpp").read_text()
               .replace("/* SOURCE EXTRACTS */", injection))
    cpp = tmp_path / "dispatch.cpp"
    exe = tmp_path / "dispatch"
    cpp.write_text(harness)
    cmd = [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror", "-pedantic",
           "-fsanitize=address,undefined", "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)]
    built = subprocess.run(cmd, capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=20,
                         env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
                                  UBSAN_OPTIONS="halt_on_error=1"))
    if expected:
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0, "dispatch mutation survived"


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_dispatch_and_commit_glue(tmp_path, compiler):
    assert NET.is_file(), "set OPENSMACX_ROOT to a checkout containing src/netdaemon.cpp"
    source = NET.read_text()
    injection = _extract(source)
    _compile(tmp_path, compiler, injection)
    # Endpoints and byte direction are tested by the fixture below; these source
    # mutations ensure the extracted product router itself is under test.
    for old, new in [
        ("opcode >= 0x80 && opcode <= 0x8D", "opcode >= 0x80 && opcode < 0x8D"),
        ("bytes, length, true, NetDaemonRulesSyncStaging",
         "bytes, length, false, NetDaemonRulesSyncStaging"),
        ("NetDaemonRulesSyncStaging);", "NetDaemonRulesSyncStaging + 1);"),
    ]:
        assert old in injection
        _compile(tmp_path, compiler, injection.replace(old, new, 1), expected=False)


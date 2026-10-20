"""Actual-source Log formatting and pinned CRT-call controls.

The Log class is extracted from log.h and its four external calls are replaced
with local spies. Formatting and branch behavior are therefore exercised from
the actual class body; the separate pinned instruction check establishes the
original call target and does not infer it from the host harness.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/log.h").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
HEADER = ROOT / "src/log.h"
OWNER = ROOT / "src/log.cpp"


def find_pinned_image() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured)] if configured else [])
    candidates.extend(parent / ".opensmacx/game/terranx_original.exe"
                      for parent in (ROOT, *ROOT.parents))
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise AssertionError("pinned terranx_original.exe is unavailable")


def extract_class() -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    source = HEADER.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)
    signature = "class Log {"
    assert masked.count(signature) == 1
    start = masked.index(signature)
    opening = masked.find("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                end = index + 1
                while end < len(masked) and masked[end].isspace():
                    end += 1
                assert end < len(masked) and masked[end] == ";"
                body = source[start:end + 1]
                assert body.count("fprintf(") == 0
                for declaration in (
                        "MEASURED Log(LPCSTR input);", "MEASURED void reset();",
                        "MEASURED void say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);",
                        "MEASURED void say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3);"):
                    assert declaration in body
                return body.replace("MEASURED ", "").replace(
                    " private:", " public:", 1)
    raise AssertionError("Log class definition is not brace-balanced")


def extract_owner_methods() -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    source = OWNER.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)
    signatures = (
        "MEASURED Log::Log(LPCSTR input)",
        "MEASURED void Log::reset()",
        "MEASURED void Log::say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3)",
        "MEASURED void Log::say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3)",
    )
    bodies = []
    for signature in signatures:
        assert masked.count(signature) == 1
        start = masked.index(signature)
        opening = masked.find("{", start)
        assert opening >= 0
        depth = 0
        for index in range(opening, len(masked)):
            if masked[index] == "{":
                depth += 1
            elif masked[index] == "}":
                depth -= 1
                if depth == 0:
                    bodies.append(source[start:index + 1].replace("MEASURED ", ""))
                    break
        else:
            raise AssertionError(f"unbalanced owner definition: {signature}")
    body = "\n".join(bodies)
    assert body.count("fprintf(") == 4
    return body


def instrument_class(body: str) -> str:
    # These substitutions only redirect external dependencies for the host
    # fixture. The production control flow, format strings and arguments remain
    # verbatim in the extracted class.
    for old, new in ((r"(?<![A-Za-z0-9_])env_open\(", "spy_open("),
                     (r"(?<![A-Za-z0-9_])fclose\(", "spy_close("),
                     (r"(?<![A-Za-z0-9_])fprintf\(", "spy_fprintf(")):
        body = re.sub(old, new, body)
    return body


HOST_PREFIX = r"""
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
using BOOL = int;
using LPSTR = char *;
using LPCSTR = const char *;
#define MEASURED
static BOOL IsLoggingDisabled;
static FILE *const fake_file = reinterpret_cast<FILE *>(1);
static bool open_fails;
static std::vector<std::string> events;
static std::vector<std::string> formats;
static std::vector<std::string> outputs;
static int fprintf_calls;
static int fprintf_s_calls;
static FILE *spy_open(const char *path, const char *mode) {
    events.push_back(std::string("open:") + mode + ":" + path);
    return open_fails ? nullptr : fake_file;
}
static int spy_close(FILE *file) {
    if (file != fake_file) return -1;
    events.push_back("close");
    return 0;
}
static int spy_fprintf(FILE *file, const char *format, ...) {
    if (file != fake_file) return -1;
    ++fprintf_calls;
    events.push_back("write");
    formats.push_back(format);
    char buffer[256];
    va_list arguments;
    va_start(arguments, format);
    const int written = std::vsnprintf(buffer, sizeof(buffer), format, arguments);
    va_end(arguments);
    outputs.push_back(buffer);
    return written;
}
// Deliberately distinct from the ordinary CRT spy: the source-target damage
// control can compile a restored fprintf_s call and observe the wrong path.
static int fprintf_s(FILE *, const char *, ...) {
    ++fprintf_s_calls;
    events.push_back("secure-wrapper");
    return 0;
}
static void *mem_get(std::size_t bytes) { return std::malloc(bytes); }
static FILE *env_open(const char *path, const char *mode) {
    return spy_open(path, mode);
}
"""


HOST_MAIN = r"""
static int failures;
static void expect(bool ok, const char *name) {
    if (!ok) { std::fprintf(stderr, "failed: %s\n", name); ++failures; }
}
static char *make_name() {
    char *name = static_cast<char *>(std::malloc(9));
    std::memcpy(name, "log.txt", 8);
    return name;
}
static void clear_trace() {
    events.clear(); formats.clear(); outputs.clear();
}
int main() {
    clear_trace();
    {
        Log filename_log("filename.log");
        expect(filename_log.log_file_ && std::strcmp(filename_log.log_file_, "filename.log") == 0,
               "filename constructor stores the copied name");
        expect(events == std::vector<std::string>{"open:wt:filename.log", "close"},
               "filename constructor opens and closes only a successful file");
    }

    clear_trace();
    open_fails = true;
    {
        Log failed_open_log("missing.log");
        expect(failed_open_log.log_file_ && std::strcmp(failed_open_log.log_file_, "missing.log") == 0,
               "failed filename open retains the allocated name");
        expect(events == std::vector<std::string>{"open:wt:missing.log"},
               "failed filename open is not closed");
    }
    open_fails = false;

    clear_trace();
    {
        Log null_name_log(static_cast<LPCSTR>(nullptr));
        expect(null_name_log.log_file_ == nullptr && events.empty(),
               "null filename leaves the file slot null without opening");
    }

    Log log;
    char *name = make_name();
    log.log_file_ = name;

    log.say("decimal", nullptr, -7, 0, 123);
    expect(events == std::vector<std::string>{"open:at:log.txt", "write", "close"},
           "say without second text opens/writes/closes in order");
    expect(formats.size() == 1 && formats[0] == "%s %d %d %d\n",
           "decimal no-second format");
    expect(outputs.size() == 1 && outputs[0] == "decimal -7 0 123\n",
           "decimal no-second values");

    clear_trace();
    log.say("label", "two words", 1, -20, 300);
    expect(events == std::vector<std::string>{"open:at:log.txt", "write", "close"},
           "say with second text opens/writes/closes in order");
    expect(formats.size() == 1 && formats[0] == "%s %s %d %d %d\n",
           "decimal second-text format");
    expect(outputs.size() == 1 && outputs[0] == "label two words 1 -20 300\n",
           "decimal second-text values");

    clear_trace();
    log.say_hex("hex", nullptr, 10, 255, 0xABC);
    expect(events == std::vector<std::string>{"open:at:log.txt", "write", "close"},
           "hex without second text opens/writes/closes in order");
    expect(formats.size() == 1 && formats[0] == "%s %04x %04x %04x\n",
           "hex no-second format");
    expect(outputs.size() == 1 && outputs[0] == "hex 000a 00ff 0abc\n",
           "hex no-second values");

    clear_trace();
    log.say_hex("hex-label", "tail", 1, 0x12, 0x3456);
    expect(events == std::vector<std::string>{"open:at:log.txt", "write", "close"},
           "hex with second text opens/writes/closes in order");
    expect(formats.size() == 1 && formats[0] == "%s %s %04x %04x %04x\n",
           "hex second-text format");
    expect(outputs.size() == 1 && outputs[0] == "hex-label tail 0001 0012 3456\n",
           "hex second-text values");

    clear_trace();
    log.log_file_ = nullptr;
    log.say("gated", nullptr, 1, 2, 3);
    expect(events.empty(), "null filename gates before opening");
    log.log_file_ = name;

    clear_trace();
    log.set_state(0);
    log.say_hex("gated", nullptr, 1, 2, 3);
    expect(events.empty(), "instance-disabled gate precedes open");
    log.set_state(1);

    clear_trace();
    IsLoggingDisabled = 1;
    log.say("gated", nullptr, 1, 2, 3);
    expect(events.empty(), "global-disabled gate precedes open");
    IsLoggingDisabled = 0;

    clear_trace();
    open_fails = true;
    log.say_hex("no-file", "tail", 1, 2, 3);
    expect(events == std::vector<std::string>{"open:at:log.txt"},
           "failed open neither writes nor closes");
    open_fails = false;
    expect(fprintf_calls == 4 && fprintf_s_calls == 0,
           "all successful writes target ordinary fprintf only");

    log.log_file_ = nullptr;
    std::free(name);
    return failures == 0 ? 0 : 1;
}
"""


def compile_run(tmp_path: Path, body: str, name: str, expect_success: bool):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    source = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source.write_text(HOST_PREFIX + body + HOST_MAIN, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", str(source),
                    "-o", str(binary)], check=True, capture_output=True,
                   text=True, timeout=60)
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            timeout=20)
    if expect_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "wrong fprintf_s source target survived"
    return result


def test_pinned_log_calls_reach_fprintf_dependency_entry():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86_const import X86_OP_IMM
    from match_recovery.component_source import parse_component
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = find_pinned_image()
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PIN
    image = PeImage(image_bytes, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    calls = (0x00626147, 0x00626176, 0x006261E7, 0x00626216)
    for address in calls:
        instructions = list(decoder.disasm(image.read(Span(address, address + 5)),
                                            address))
        assert len(instructions) == 1
        instruction = instructions[0]
        assert instruction.mnemonic == "call"
        assert instruction.operands[0].type == X86_OP_IMM
        assert instruction.operands[0].imm == 0x00647815

    manifest = parse_component(
        (ROOT / "src/vendor/vc6-runtime.recovery.h").read_text(encoding="utf-8"))
    binding = next(binding for binding in manifest.bindings
                   if binding.origin.address == 0x00647815)
    assert binding.symbol == "_fprintf"
    assert binding.spans == (Span(0x00647815, 0x00647851),)


def test_actual_log_source_formats_and_preserves_open_write_close_gates(tmp_path):
    body = instrument_class(extract_class() + "\n" + extract_owner_methods())
    compile_run(tmp_path, body, "actual_log_formatting", expect_success=True)


def test_reverting_log_to_fprintf_s_is_detected_by_executed_target(tmp_path):
    body = instrument_class(extract_class() + "\n" + extract_owner_methods())
    assert body.count("spy_fprintf(") == 4
    damaged = body.replace("spy_fprintf(", "fprintf_s(")
    result = compile_run(tmp_path, damaged, "damaged_fprintf_s", expect_success=False)
    assert "failed: all successful writes target ordinary fprintf only" in result.stderr

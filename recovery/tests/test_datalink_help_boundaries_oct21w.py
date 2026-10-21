"""Actual-source controls for the ordinary help_topic forwarding body."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _function_body(source: str, signature: str) -> str:
    match = re.search(signature, source, re.MULTILINE)
    assert match, "missing expected source definition"
    opening = source.index("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 1
            elif char == "/" and following == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start():index + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and following == "/":
            state = "code"
            index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated help_topic body")


def _actual_body() -> str:
    header = (ROOT / "src/datalink.h").read_text()
    source = (ROOT / "src/datalink.cpp").read_text()
    declaration = "MEASURED void __cdecl help_topic(unsigned int topic, int index);"
    assert header.count(declaration) == 1
    assert "MEASURED inline void __cdecl help_topic" not in header
    signature = r"^void __cdecl help_topic\(unsigned int topic, int index\)\s*\{"
    assert len(re.findall(signature, source, re.MULTILINE)) == 1
    assert "HISTORICAL: this body was in datalink.h as `MEASURED inline`." in source
    assert "mixed inline/call-site observation remains unverified" in source
    return _function_body(source, signature)


HARNESS = r"""
#include <stdio.h>
#include <stdlib.h>

#define __cdecl

struct Datalink {
    unsigned int calls;
    unsigned int topic;
    int index;
    void exec(unsigned int next_topic, int next_index);
};

Datalink DatalinkMain = {0, 0, 0};
Datalink OtherDatalink = {0, 0, 0};
Datalink *last_receiver = 0;

void Datalink::exec(unsigned int next_topic, int next_index) {
    ++calls;
    topic = next_topic;
    index = next_index;
    last_receiver = this;
}

void require_at(int condition, int line) {
    if (!condition) {
        fprintf(stderr, "assertion failed at line %d\n", line);
        exit(93);
    }
}
#define require(condition) require_at((condition), __LINE__)

BODY

int main() {
    help_topic(0x80000005U, -12345);
    require(DatalinkMain.calls == 1);
    require(last_receiver == &DatalinkMain);
    require(DatalinkMain.topic == 0x80000005U);
    require(DatalinkMain.index == -12345);
    require(OtherDatalink.calls == 0);

    last_receiver = 0;
    help_topic(0xF1234567U, 0x1234);
    require(DatalinkMain.calls == 2);
    require(last_receiver == &DatalinkMain);
    require(DatalinkMain.topic == 0xF1234567U);
    require(DatalinkMain.index == 0x1234);
    require(OtherDatalink.calls == 0);
    return 0;
}
"""


def _compile_and_run(compiler: str, body: str, directory: Path, name: str):
    source = directory / (name + ".cpp")
    executable = directory / name
    source.write_text(HARNESS.replace("BODY", body))
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        text=True, capture_output=True, check=False)
    if built.returncode != 0:
        return built.returncode, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True,
                         check=False)
    return ran.returncode, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_help_topic_forwarder_and_compile_success_mutants(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    body = _actual_body()
    with tempfile.TemporaryDirectory(prefix="datalink-help-topic-oct21w-") as name:
        directory = Path(name)
        result, output = _compile_and_run(compiler, body, directory, "actual")
        assert result == 0, output

        mutants = {
            "wrong_receiver": body.replace(
                "DatalinkMain.exec(topic, index)",
                "OtherDatalink.exec(topic, index)"),
            "swapped_arguments": body.replace(
                "DatalinkMain.exec(topic, index)",
                "DatalinkMain.exec(index, topic)"),
            "changed_topic": body.replace(
                "DatalinkMain.exec(topic, index)",
                "DatalinkMain.exec(topic + 1U, index)"),
            "changed_index": body.replace(
                "DatalinkMain.exec(topic, index)",
                "DatalinkMain.exec(topic, index + 1)"),
        }
        for mutant_name, mutant in mutants.items():
            assert mutant != body
            result, output = _compile_and_run(
                compiler, mutant, directory, mutant_name)
            assert result == 93, (mutant_name, result, output)

"""Controls for Flic's six ordinary calls to Buffer's no-argument getter."""

from pathlib import Path
import re
import shutil
import subprocess
import textwrap

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/flic.cpp").is_file())
FLIC_CPP = ROOT / "src/flic.cpp"


def _function(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated function {signature}")


def _code(source):
    return re.sub(r"/\*.*?\*/|//[^\n]*", " ", source, flags=re.S)


METHODS = (
    ("int Flic::decode_frame(int x, int y) {", "buffer2_->get_data()", "null"),
    ("int Flic::UNK1(int x, int y) {", "buffer2_->get_data()", "null"),
    ("int Flic::UNK3(int target_frame) {", "buffer2_->get_data()", "present"),
    ("void Flic::decode_byte_run_trans() {", "buffer_.get_data()", "pixels"),
    ("void Flic::decode_delta_flc_trans() {", "buffer_.get_data()", "pixels"),
    ("void Flic::decode_fli_copy_trans() {", "buffer_.get_data()", "pixels"),
)


def test_all_six_live_decoder_bodies_use_ordinary_no_argument_calls():
    source = FLIC_CPP.read_text()
    bodies = [_code(_function(source, signature)) for signature, _expr, _kind in METHODS]
    for body, (_signature, expression, kind) in zip(bodies, METHODS):
        assert body.count(expression) == 1
        assert "Buffer::*get_data" not in body
        assert "->*get_data" not in body
        assert ".*get_data" not in body
        if kind == "null":
            assert re.search(re.escape(expression) + r"\s*==\s*0", body)
        elif kind == "present":
            assert re.search(re.escape(expression) + r"\s*!=\s*0", body)
        else:
            assert re.search(r"static_cast<uint8_t\s*\*>\s*\(\s*"
                             + re.escape(expression) + r"\s*\)", body)

    assert "Buffer::*get_data" not in _code(source)
    assert "HISTORICAL (2026-09)" in source
    assert "header retains only its declaration" in source


def test_actual_getter_expressions_compile_with_declaration_only_visibility(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    source = FLIC_CPP.read_text()
    methods = [_code(_function(source, signature)) for signature, _expr, _kind in METHODS]
    wrappers = []
    for index, (body, (_signature, expected, kind)) in enumerate(zip(methods, METHODS)):
        match = re.search(re.escape(expected), body)
        assert match
        expression = match.group(0)
        if kind in ("null", "present"):
            comparison = "== 0" if kind == "null" else "!= 0"
            wrappers.append(f"bool consumer_{index}(Buffer *buffer2_) {{ "
                            f"return {expression} {comparison}; }}")
        else:
            wrappers.append(f"uint8_t *consumer_{index}(Buffer &buffer_) {{ "
                            f"return static_cast<uint8_t *>({expression}); }}")
    cpp_source = textwrap.dedent(r"""
        #include <cstdint>
        #include <cstdio>
        struct Buffer {
            void *pixels;
            int calls;
            void *get_data();
        };
        void *Buffer::get_data() { ++calls; return pixels; }
    """) + "\n" + "\n".join(wrappers) + textwrap.dedent(r"""
        int main() {
            uint8_t pixels[4] = {1, 2, 3, 4};
            Buffer destination = {0, 0};
            if (!consumer_0(&destination) || !consumer_1(&destination)) return 1;
            destination.pixels = pixels;
            if (consumer_0(&destination) || consumer_1(&destination)) return 2;
            if (!consumer_2(&destination) || destination.calls != 5) return 3;
            Buffer embedded = {pixels, 0};
            if (consumer_3(embedded) != pixels || consumer_4(embedded) != pixels ||
                consumer_5(embedded) != pixels || embedded.calls != 3) return 4;
            return 0;
        }
    """)
    cpp = tmp_path / "flic_get_data_calls.cpp"
    binary = tmp_path / "flic_get_data_calls"
    cpp.write_text(cpp_source)
    compiled = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                               str(cpp), "-o", str(binary)],
                              capture_output=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr.decode(errors="replace")
    run = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert run.returncode == 0, run.stderr.decode(errors="replace")

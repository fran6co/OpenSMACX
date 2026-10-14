"""Actual-source controls for the recovered GraphicWin/Buffer input check."""
from __future__ import annotations

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from match_recovery import source

ROOT = Path(__file__).resolve().parents[2]


def function(text: str, signature: str) -> str:
    hits = list(re.finditer(re.escape(signature), text))
    assert len(hits) == 1, signature
    start = hits[0].start()
    brace = text.find("{", hits[0].end())
    assert brace >= 0
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated source body: {signature}")


def compile_and_run(compiler: str, buffer_body: str, graphicwin_body: str,
                    *, expect_success: bool) -> None:
    prelude = r"""
#include <cassert>
struct Palette {};
struct PcxInputView {
    unsigned char prefix_[0x0C];
    unsigned char *data_;
    unsigned int size_;
};
struct Buffer {
    int check_pcx_input_data(PcxInputView *, int, int, int);
};
struct Win {};
struct GraphicWin : Win, Buffer {
    int check_pcx_input_data(PcxInputView *, int, int, int);
};
"""
    main = r"""
int main() {
    GraphicWin owner;
    PcxInputView input = {};
    assert(owner.check_pcx_input_data(0, 42, -7, 0x12345678) == 3);
    input.data_ = 0;
    input.size_ = 1;
    assert(owner.check_pcx_input_data(&input, 42, -7, 0x12345678) == 7);
    unsigned char payload = 0;
    input.data_ = &payload;
    input.size_ = 0;
    assert(owner.check_pcx_input_data(&input, 42, -7, 0x12345678) == 0);
}
"""
    with tempfile.TemporaryDirectory() as directory:
        cpp = Path(directory) / "actual_owner.cpp"
        exe = Path(directory) / "actual_owner"
        cpp.write_text(prelude + buffer_body + graphicwin_body + main,
                       encoding="utf-8")
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                "-Werror", str(cpp), "-o", str(exe)],
                               text=True, capture_output=True)
        assert built.returncode == 0, built.stderr
        result = subprocess.run([str(exe)], text=True, capture_output=True)
        if expect_success:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0


def test_actual_owner_behaviors_and_field_read_mutant():
    buffer_text = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
    graphicwin_text = (ROOT / "src/graphicwin.cpp").read_text(encoding="utf-8")
    buffer_body = function(buffer_text,
                           "int Buffer::check_pcx_input_data(")
    graphicwin_body = function(graphicwin_text,
                               "int GraphicWin::check_pcx_input_data(")

    compiler_request = os.environ.get("CXX")
    compiler = shutil.which(compiler_request) if compiler_request else None
    compiler = compiler or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host compiler required for actual-source controls"
    compile_and_run(compiler, buffer_body, graphicwin_body, expect_success=True)

    # A plausible but wrong +0x10 read must reject the non-null-data/zero-size
    # case, demonstrating that the behavioral oracle detects the wrong field.
    mutant = buffer_body.replace("source->data_ != 0", "source->size_ != 0")
    assert mutant != buffer_body
    compile_and_run(compiler, mutant, graphicwin_body, expect_success=False)


def test_identity_claims_and_complete_history_survive_transfer():
    leaf_path = ROOT / "src/leaf_recoveries.cpp"
    buffer_path = ROOT / "src/buffer.cpp"
    graphicwin_path = ROOT / "src/graphicwin.cpp"
    assert not leaf_path.exists(), "retired leaf source collection reappeared"
    buffer_text = buffer_path.read_text(encoding="utf-8")
    graphicwin_text = graphicwin_path.read_text(encoding="utf-8")
    buffer_rows = {row.address: row for row in source.read_text(buffer_text, buffer_path)}
    graphicwin_rows = {row.address: row for row in source.read_text(graphicwin_text, graphicwin_path)}

    helper = buffer_rows[0x005D7A10]
    wrapper = graphicwin_rows[0x005D5470]
    assert helper.name == "sub_5d7a10"
    assert helper.image_spans == ((0x005D7A10, 0x005D7A2F),)
    assert helper.byte_exact is True and helper.semantic is False
    assert helper.symbol == "?check_pcx_input_data@Buffer@@QAEHPAUPcxInputView@@HHH@Z"
    assert wrapper.name == "sub_5d5470"
    assert wrapper.image_spans == ((0x005D5470, 0x005D5492),)
    assert wrapper.byte_exact is True and wrapper.semantic is False
    assert wrapper.symbol == "?check_pcx_input_data@GraphicWin@@QAEHPAUPcxInputView@@HHH@Z"
    assert helper.source_role is source.SourceRole.UNCLASSIFIED
    assert wrapper.source_role is source.SourceRole.UNCLASSIFIED
    assert "leaf_005d7a10_redirect" in buffer_text
    assert "leaf_005d5470_redirect" in graphicwin_text
    assert "0x444" in graphicwin_text[graphicwin_text.index("0x005D5470"):]
    assert "source->data_" in buffer_text[
        buffer_text.index("0x005D7A10"):buffer_text.index("0x005D7A10") + 3000]

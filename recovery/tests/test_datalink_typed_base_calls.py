"""Installed-source typed-owner controls; host models do not prove native ABI."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/datalink.cpp"



def extract_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_typed_base_calls_preserve_mapped_behavior() -> None:
    text = SOURCE.read_text()
    records = {record.address: record for record in read_file(SOURCE)}
    assert records[0x0042BD40].image_spans == ((0x0042BD40, 0x0042BD5B),)
    assert records[0x0042BD60].image_spans == ((0x0042BD60, 0x0042BD79),)
    assert records[0x0042BD40].byte_exact and records[0x0042BD60].byte_exact
    owner = (ROOT / "src/datalink.h").read_text()
    graphic = (ROOT / "src/graphicwin.h").read_text()
    win = (ROOT / "src/win.h").read_text()
    assert "class Datalink : public GraphicWin, public SubInterface" in owner
    assert "RECOVERY_BASE: v1 Datalink GraphicWin 0x0 0x00428FC0" in owner
    assert "RECOVERY_BASE: v1 Datalink SubInterface 0xA14 0x00428FC0" in owner
    assert "class GraphicWin : public Win, public Buffer" in graphic
    assert "int is_visible();" in win

    candidate = text
    assert "0x0042BD40 ?show@Datalink@@QAEXH@Z 0x0042BD40-0x0042BD5B BYTE_EXACT" in candidate
    assert "0x0042BD60 ?hide@Datalink@@QAEXXZ 0x0042BD60-0x0042BD79 BYTE_EXACT" in candidate

    show = extract_definition(candidate, "void Datalink::show(int visible)")
    hide = extract_definition(candidate, "void Datalink::hide()")
    assert "reinterpret_cast" not in show + hide
    assert "0xa14" not in show + hide

    compiler = shutil.which("c++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler available for the isolated control")
    harness = r"""
#include <cassert>
class Win {
public:
    int visible;
    int is_visible() { return visible; }
};
class GraphicWin : public Win {};
class SubInterface {
public:
    int set_count = 0;
    int release_count = 0;
    void set_iface_mode() { ++set_count; }
    void release_iface_mode() { ++release_count; }
};
class Datalink : public GraphicWin, public SubInterface {
public:
    void show(int);
    void hide();
};
""" + show + "\n" + hide + r"""
int main() {
    Datalink link{};
    link.visible = 0;
    link.show(91);
    assert(link.set_count == 1);
    link.visible = 1;
    link.show(0);
    assert(link.set_count == 1);
    link.hide();
    assert(link.release_count == 1);
    link.visible = 0;
    link.hide();
    assert(link.release_count == 1);
}
"""
    with tempfile.TemporaryDirectory(prefix="opensmacx-datalink-control-") as directory:
        source = Path(directory) / "control.cpp"
        binary = Path(directory) / "control"
        source.write_text(harness)
        subprocess.run([compiler, "-std=c++11", "-Wno-unused-parameter",
                        str(source), "-o", str(binary)], check=True,
                       capture_output=True, text=True)
        subprocess.run([str(binary)], check=True, capture_output=True, text=True)

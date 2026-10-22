"""Actual-source control for ReportIf's technology-label callback.

The host fixture observes source-level call arguments, order, and C++ base
adjustment only. The pinned listing independently witnesses the original
18-instruction span and helper operands; neither is target-ABI/runtime proof.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.pe_coff import PeImage, Span
from match_recovery.source.asm import Listing
from test_buffer_typed_font_consumers_oct10 import ROOT, extract_definition


SOURCE = ROOT / "src/reportif.cpp"
ADDRESS = "0x004A8440"
SIGNATURE = "int __cdecl labs_write(GraphicWin *window, int technology, int, RECT *, char *)"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


FIXTURE = r'''#define __cdecl
struct RECT { int value; };

struct Win {
    int nonempty_prefix;
};

static int call_count;
static int seen_technology;
static int seen_category;
static int seen_colors[4];
static int call_order[3];
static int call_order_size;
static void *seen_buffer;

struct Buffer {
    int nonempty_buffer_prefix;
    void set_text_color3(int first, int second, int third, int fourth) {
        ++call_count;
        seen_buffer = this;
        seen_colors[0] = first;
        seen_colors[1] = second;
        seen_colors[2] = third;
        seen_colors[3] = fourth;
        call_order[call_order_size++] = 3;
    }
};

struct GraphicWin : Win, Buffer {};

int tech_category(int technology) {
    seen_technology = technology;
    call_order[call_order_size++] = 1;
    return technology == 17 ? 4 : 2;
}

int mandate_color(int category) {
    seen_category = category;
    call_order[call_order_size++] = 2;
    return category * 10 + 3;
}
'''


MAIN = r'''
static int verify_case(int technology, int expected_category, int expected_color) {
    GraphicWin window;
    RECT rect = {0};
    char text[] = "technology";
    call_count = 0;
    seen_technology = -1;
    seen_category = -1;
    call_order_size = 0;
    seen_buffer = 0;

    const int result = labs_write(&window, technology, 0, &rect, text);
    if (result != 0 || call_count != 1) return 1;
    if (seen_technology != technology || seen_category != expected_category) return 2;
    if (seen_colors[0] != expected_color || seen_colors[1] != 0 ||
        seen_colors[2] != 1 || seen_colors[3] != 1) return 3;
    if (call_order_size != 3 || call_order[0] != 1 ||
        call_order[1] != 2 || call_order[2] != 3) return 4;
    if (seen_buffer != static_cast<void *>(static_cast<Buffer *>(&window))) return 5;
    if (seen_buffer == static_cast<void *>(&window)) return 6;
    return 0;
}

int main() {
    if (verify_case(17, 4, 43) != 0) return 10;
    if (verify_case(42, 2, 23) != 0) return 11;
    return 0;
}
'''


MUTANTS = {
    "wrong_return": ("    return 0;", "    return 1;"),
    "wrong_color_flags": ("mandate_color(tech_category(technology)), 0, 1, 1",
                           "mandate_color(tech_category(technology)), 0, 1, 2"),
    "skip_mandate_helper": ("mandate_color(tech_category(technology))",
                             "tech_category(technology)"),
}


def actual_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    return extract_definition(source, ADDRESS, SIGNATURE)


def program(body: str) -> str:
    return FIXTURE + "\n" + body + "\n" + MAIN


def compile_and_run(compiler: str, tmp_path: Path, source: str, name: str) -> subprocess.CompletedProcess:
    executable = tmp_path / name
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-",
         "-o", str(executable)], input=source, text=True, capture_output=True,
        cwd=tmp_path, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          cwd=tmp_path, timeout=10)


def _compiler_names() -> list[str]:
    return [found for name in ("g++", "clang++") if (found := shutil.which(name))]


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_labs_write_body_preserves_receiver_order_arguments_and_return(compiler, tmp_path):
    body = actual_body()
    result = compile_and_run(compiler, tmp_path, program(body), "labs-write")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", _compiler_names())
@pytest.mark.parametrize("mutant", sorted(MUTANTS))
def test_compilable_labs_write_mutants_fail_runtime_control(compiler, mutant, tmp_path):
    body = actual_body()
    before, after = MUTANTS[mutant]
    assert body.count(before) == 1, f"mutant anchor must be unique: {mutant}"
    damaged = body.replace(before, after, 1)
    result = compile_and_run(compiler, tmp_path, program(damaged), "labs-write-mutant")
    assert result.returncode != 0, f"compiled mutant escaped runtime control: {mutant}"


def test_pinned_labs_write_has_18_instructions_and_original_helper_operands():
    assert SOURCE.is_file()
    configured = os.environ.get("OPENSMACX_IMAGE")
    image_path = (Path(configured).expanduser().resolve() if configured else
                  ROOT / ".opensmacx/game/terranx_original.exe")
    if not configured and not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    assert image_path.is_file(), f"pinned original unavailable: {image_path}"
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    listing = Listing(image.read(Span(0x004A8440, 0x004A846E)), 0x004A8440)
    assert len(listing.instructions) == 18
    calls = [(ins.address, ins.op_str) for ins in listing.instructions if ins.mnemonic == "call"]
    assert calls == [(0x004A844D, "0x5b9fe0"),
                     (0x004A8453, "0x50e820"),
                     (0x004A8465, "0x5dad10")]
    by_address = {ins.address: (ins.mnemonic, ins.op_str) for ins in listing.instructions}
    assert [by_address[address] for address in
            (0x004A8446, 0x004A8448, 0x004A844A, 0x004A844C)] == [
                ("push", "1"), ("push", "1"), ("push", "0"), ("push", "eax")]
    assert by_address[0x004A8458] == ("mov", "ecx, dword ptr [ebp + 8]")
    assert by_address[0x004A845E] == ("add", "ecx, 0x444")
    assert by_address[0x004A846A] == ("xor", "eax, eax")

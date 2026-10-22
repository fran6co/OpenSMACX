"""Actual-source control for MessageWin's ListBox color callback.

The host double observes only source-level arguments and C++ base adjustment;
the optional pinned listing is a separate operand witness, not target ABI or
runtime proof.
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


SOURCE = ROOT / "src/messagewin.cpp"
ADDRESS = "0x00471FD0"
SIGNATURE = "int __cdecl MessageWin::listbox_write(GraphicWin *target, int item_id, int,"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


FIXTURE = r'''#ifndef __cdecl
#define __cdecl
#endif

struct RECT { int value; };
struct GraphicWin;
static int MessageWinTextColor = 0xE7;
static int MessageWinTextBackground = 0x91;
static int set_call_count;
static int set_args[2][4];
static void *set_receivers[2];

struct Win {
    int nonempty_win_prefix;
};

struct Buffer {
    int nonempty_buffer_prefix;
    void set_text_color(int first, int second, int third, int fourth) {
        const int index = set_call_count++;
        if (index >= 2) return;
        set_receivers[index] = this;
        set_args[index][0] = first;
        set_args[index][1] = second;
        set_args[index][2] = third;
        set_args[index][3] = fourth;
    }
};

struct GraphicWin : Win, Buffer {};

struct MessageWin {
    static int __cdecl listbox_write(GraphicWin *, int, int, RECT *, char *);
};
'''


MAIN = r'''
static int verify_case(int item_id, int expected_calls) {
    GraphicWin target;
    RECT area = {0};
    char text[] = "message";
    set_call_count = 0;
    set_receivers[0] = set_receivers[1] = 0;

    const int result = MessageWin::listbox_write(&target, item_id, 0, &area, text);
    if (result != 0 || set_call_count != expected_calls) return 1;
    if (set_receivers[0] != static_cast<void *>(static_cast<Buffer *>(&target))) return 2;
    if (set_receivers[0] == static_cast<void *>(&target)) return 3;
    if (set_args[0][0] != MessageWinTextColor ||
        set_args[0][1] != MessageWinTextBackground ||
        set_args[0][2] != 1 || set_args[0][3] != 1) return 4;
    if (expected_calls == 2) {
        if (set_receivers[1] != static_cast<void *>(static_cast<Buffer *>(&target))) return 5;
        if (set_args[1][0] != MessageWinTextColor || set_args[1][1] != -1 ||
            set_args[1][2] != 1 || set_args[1][3] != 1) return 6;
    }
    return 0;
}

int main() {
    if (verify_case(0,          1) != 0) return 10;
    if (verify_case(0x10000000, 1) != 0) return 11;
    if (verify_case(0x10000001, 2) != 0) return 12;
    if (verify_case((-2147483647 - 1), 1) != 0) return 13;
    if (verify_case(-1879048191, 2) != 0) return 14;
    return 0;
}
'''


MUTANTS = {
    "wrong_optional_color": ("set_text_color(MessageWinTextColor, -1, 1, 1)",
                             "set_text_color(MessageWinTextColor, 0, 1, 1)"),
    "weaken_mask_predicate": ("(item_id & 0x10000000) != 0 && (item_id & 0xefffffff) != 0",
                              "(item_id & 0x10000000) != 0"),
    "wrong_return": ("    return 0;", "    return 1;"),
    "extra_optional_color_call": (
        "target->set_text_color(MessageWinTextColor, -1, 1, 1);",
        "target->set_text_color(MessageWinTextColor, -1, 1, 1);\n"
        "        target->set_text_color(MessageWinTextColor, -1, 1, 1);"),
}


def actual_body() -> str:
    return extract_definition(SOURCE.read_text(encoding="utf-8"), ADDRESS, SIGNATURE)


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
def test_actual_messagewin_callback_preserves_mask_color_calls_and_return(compiler, tmp_path):
    result = compile_and_run(compiler, tmp_path, program(actual_body()), "messagewin-write")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", _compiler_names())
@pytest.mark.parametrize("mutant", sorted(MUTANTS))
def test_compilable_messagewin_callback_mutants_fail_runtime_control(compiler, mutant, tmp_path):
    body = actual_body()
    before, after = MUTANTS[mutant]
    assert body.count(before) == 1, f"mutant anchor must be unique: {mutant}"
    damaged = body.replace(before, after, 1)
    result = compile_and_run(compiler, tmp_path, program(damaged), "messagewin-write-mutant")
    assert result.returncode != 0, f"compiled mutant escaped runtime control: {mutant}"


def test_optional_pinned_messagewin_callback_operand_witness():
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        image_path = Path(configured).expanduser().resolve()
        assert image_path.is_file(), f"configured pinned image is missing: {image_path}"
    else:
        image_path = ROOT / ".opensmacx/game/terranx_original.exe"
        if not image_path.is_file():
            pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    listing = Listing(image.read(Span(0x00471FD0, 0x0047201F)), 0x00471FD0)
    assert len(listing.instructions) == 29
    by_address = {ins.address: (ins.mnemonic, ins.op_str) for ins in listing.instructions}
    calls = [(ins.address, ins.op_str) for ins in listing.instructions if ins.mnemonic == "call"]
    assert calls == [(0x00471FF1, "0x5dacb0"), (0x00472015, "0x5dacb0")]
    assert by_address[0x00471FE5] == ("lea", "esi, [eax + 0x444]")
    assert by_address[0x00471FF9] == ("test", "eax, 0x10000000")
    assert by_address[0x00472000] == ("test", "eax, 0xefffffff")
    assert by_address[0x0047201A] == ("xor", "eax, eax")

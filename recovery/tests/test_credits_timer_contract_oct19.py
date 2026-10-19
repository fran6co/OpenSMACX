"""Credits' one-payload timer ABI, borrowed receiver, and pinned registration."""
from __future__ import annotations

import os
from functools import lru_cache
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/credits.cpp"
HEADER = ROOT / "src/credits.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
_IMAGE_CANDIDATES = (
    ROOT / ".opensmacx/game/terranx_original.exe",
    ROOT.parents[2] / ".opensmacx/game/terranx_original.exe",
)
IMAGE = Path(os.environ["OPENSMACX_IMAGE"]) if "OPENSMACX_IMAGE" in os.environ else next(
    (candidate for candidate in _IMAGE_CANDIDATES if candidate.is_file()),
    _IMAGE_CANDIDATES[0],
)
CALLBACK_SIGNATURE = "void __cdecl Credits::timer_callback_daemon(int a1)"
CALLBACK_DECLARATION = "static void __cdecl timer_callback_daemon(int a1);"
NOARG_DECLARATION = "void timer_callback_daemon();"
COMPILE_FLAGS = (
    "-std=c++11", "-Wall", "-Wextra", "-Werror",
    "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
)


def extract_definition(text: str, signature: str) -> str:
    start = text.find(signature)
    while start >= 0:
        opening = text.find("{", start + len(signature))
        semicolon = text.find(";", start + len(signature))
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = text.find(signature, start + len(signature))
    assert start >= 0, f"missing actual definition: {signature}"
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated actual definition: {signature}")


def test_source_registers_static_payload_entry_and_preserves_noarg_member():
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    callback = extract_definition(source, CALLBACK_SIGNATURE)
    assert _callback_declarations(header) == (NOARG_DECLARATION, CALLBACK_DECLARATION)
    assert "void Credits::timer_callback_daemon()" in source
    assert "uint32_t frame_count_;  // 0xA14" in header
    assert "time_.start(Credits::timer_callback_daemon, reinterpret_cast<int>(this), 30, 5);" in source
    assert "credits_timer_tick" not in source
    assert "// ORIGINAL: 0x00428DC0 ?timer_callback_daemon@Credits@@QAEXXZ 0x00428DC0-0x00428DCB BYTE_EXACT" in source
    assert "// ORIGINAL: 0x00428DD0 ?timer_callback_daemon@Credits@@QAAXH@Z 0x00428DD0-0x00428DE7 BYTE_EXACT" in source
    assert "Credits *credits = reinterpret_cast<Credits *>(a1);" in callback
    assert "if (credits != 0)" in callback
    assert callback.index("++credits->frame_count_;") < callback.index("credits->draw_credits();")


def pinned_image():
    if not IMAGE.is_file():
        if "OPENSMACX_IMAGE" in os.environ:
            pytest.fail(f"explicit OPENSMACX_IMAGE does not exist: {IMAGE}")
        pytest.skip("pinned original image required")
    from match_recovery.pe_coff import PeImage
    return PeImage(IMAGE.read_bytes(), PIN)


def test_explicit_missing_pin_path_is_a_failure(monkeypatch, tmp_path):
    from sys import modules

    missing = tmp_path / "missing-original.exe"
    monkeypatch.setenv("OPENSMACX_IMAGE", str(missing))
    monkeypatch.setattr(modules[__name__], "IMAGE", missing)
    with pytest.raises(pytest.fail.Exception, match="explicit OPENSMACX_IMAGE"):
        pinned_image()


def test_pinned_full_callback_and_registration_contract():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span

    image = pinned_image()
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    callback = list(decoder.disasm(image.read(Span(0x428DD0, 0x428DE7)), 0x428DD0))
    assert sum(instruction.size for instruction in callback) == 0x17
    assert [(i.mnemonic, i.op_str) for i in callback] == [
        ("push", "ebp"),
        ("mov", "ebp, esp"),
        ("mov", "ecx, dword ptr [ebp + 8]"),
        ("test", "ecx, ecx"),
        ("je", "0x428de5"),
        ("inc", "dword ptr [ecx + 0xa14]"),
        ("call", "0x428c00"),
        ("pop", "ebp"),
        ("ret", ""),
    ]

    registration = list(decoder.disasm(image.read(Span(0x428B44, 0x428B55)), 0x428B44))
    assert [(i.mnemonic, i.op_str) for i in registration] == [
        ("push", "5"),
        ("push", "0x1e"),
        ("push", "esi"),
        ("push", "0x428dd0"),
        ("mov", "ecx, ebx"),
        ("call", "0x616350"),
    ]


PREAMBLE = r'''#include <assert.h>
#include <new>
#include <stdint.h>
#include <sys/mman.h>
#ifndef __cdecl
#define __cdecl
#endif
class Credits {
public:
    uint32_t frame_count_;
    int draw_count_;
    uint32_t drawn_frame_[6];
CALLBACK_DECLARATIONS
    void draw_credits() {
        assert(draw_count_ < 6);
        drawn_frame_[draw_count_++] = frame_count_;
    }
};
class Time {
public:
    void start(void (__cdecl *callback)(int), int context,
               uint32_t count, uint32_t resolution) {
        assert(count == 30 && resolution == 5);
        callback(context);
    }
};
CALLBACK_BODY
int main() {
    void (__cdecl *callback)(int) = &Credits::timer_callback_daemon;
    Time time;
    time.start(Credits::timer_callback_daemon, 0, 30, 5);
    callback(0);
#if defined(MAP_32BIT)
    void *storage = mmap(0, 4096, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (storage == MAP_FAILED) return 77;
    assert(reinterpret_cast<uintptr_t>(storage) <= INT32_MAX);
    Credits *credits = new(storage) Credits();
    credits->frame_count_ = 4;
    credits->draw_count_ = 0;
    const int context = static_cast<int>(reinterpret_cast<uintptr_t>(credits));
    callback(context);
    assert(credits->frame_count_ == 5);
    assert(credits->draw_count_ == 1 && credits->drawn_frame_[0] == 5);
    callback(context);
    assert(credits->frame_count_ == 6);
    assert(credits->draw_count_ == 2 && credits->drawn_frame_[1] == 6);
    credits->frame_count_ = UINT32_MAX;
    callback(context);
    assert(credits->frame_count_ == 0);
    assert(credits->draw_count_ == 3 && credits->drawn_frame_[2] == 0);
    callback(context);
    assert(credits->frame_count_ == 1);
    assert(credits->draw_count_ == 4 && credits->drawn_frame_[3] == 1);
    credits->~Credits();
    assert(munmap(storage, 4096) == 0);
    return 0;
#else
    return 77;
#endif
}
'''


def _callback_declarations(header: str) -> tuple[str, str]:
    declarations = tuple(
        line.strip() for line in header.splitlines()
        if line.strip() in (NOARG_DECLARATION, CALLBACK_DECLARATION)
    )
    assert declarations.count(NOARG_DECLARATION) == 1
    assert declarations.count(CALLBACK_DECLARATION) == 1
    return declarations


def _mutate_callback(callback: str, damage: str) -> str:
    if damage == "wrong_guard":
        assert "if (credits != 0)" in callback
        callback = callback.replace("if (credits != 0)", "if (credits == 0)", 1)
    elif damage == "wrong_increment":
        assert "++credits->frame_count_;" in callback
        callback = callback.replace("++credits->frame_count_;", "credits->frame_count_ += 2;", 1)
    elif damage == "wrong_order":
        old = "++credits->frame_count_;\n    credits->draw_credits();"
        new = "credits->draw_credits();\n    ++credits->frame_count_;"
        assert old in callback
        callback = callback.replace(old, new, 1)
    else:
        raise AssertionError(f"unknown callback damage: {damage}")
    assert callback != extract_definition(SOURCE.read_text(encoding="utf-8"), CALLBACK_SIGNATURE)
    return callback


@lru_cache(maxsize=None)
def make_harness(callback: str, declarations: tuple[str, str]) -> str:
    harness = PREAMBLE.replace("CALLBACK_DECLARATIONS", "\n".join(declarations))
    return harness.replace("CALLBACK_BODY", callback)


@lru_cache(maxsize=None)
def compile_and_run(compiler: str, harness: str, compile_flags: tuple[str, ...]):
    with tempfile.TemporaryDirectory(prefix="credits-timer-contract-") as temporary:
        directory = Path(temporary)
        path = directory / "credits-timer.cpp"
        executable = directory / "credits-timer"
        path.write_text(harness, encoding="utf-8")
        subprocess.run(
            [compiler, *compile_flags, str(path), "-o", str(executable)],
            check=True, capture_output=True, text=True, timeout=60,
        )
        result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        return result.returncode, result.stdout, result.stderr


def assert_damage_rejected(compiler: str, callback: str, declarations: tuple[str, str], damage: str):
    baseline = compile_and_run(compiler, make_harness(callback, declarations), COMPILE_FLAGS)
    assert baseline[0] == 0, f"unmutated callback fixture failed: {baseline[1]}{baseline[2]}"
    mutant = _mutate_callback(callback, damage)
    result = compile_and_run(compiler, make_harness(mutant, declarations), COMPILE_FLAGS)
    assert result[0] != 0, result[1] + result[2]


@pytest.mark.parametrize("damage", [None, "wrong_guard", "wrong_increment", "wrong_order"])
def test_actual_callback_null_payload_frame_order_and_damage_controls(damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    callback = extract_definition(SOURCE.read_text(encoding="utf-8"), CALLBACK_SIGNATURE)
    declarations = _callback_declarations(HEADER.read_text(encoding="utf-8"))
    if damage is None:
        result = compile_and_run(compiler, make_harness(callback, declarations), COMPILE_FLAGS)
        if result[0] == 77:
            pytest.skip("host cannot provide a native-width int pointer payload")
        assert result[0] == 0, result[1] + result[2]
    else:
        baseline = compile_and_run(compiler, make_harness(callback, declarations), COMPILE_FLAGS)
        if baseline[0] == 77:
            pytest.skip("host cannot provide a native-width int pointer payload")
        assert_damage_rejected(compiler, callback, declarations, damage)


def test_damage_control_refuses_unmutated_fixture_failure(monkeypatch):
    from sys import modules

    monkeypatch.setattr(
        modules[__name__], "compile_and_run",
        lambda *args: (1, "", "simulated unrelated baseline failure"),
    )
    callback = extract_definition(SOURCE.read_text(encoding="utf-8"), CALLBACK_SIGNATURE)
    declarations = _callback_declarations(HEADER.read_text(encoding="utf-8"))
    with pytest.raises(AssertionError, match="unmutated callback fixture failed"):
        assert_damage_rejected("g++", callback, declarations, "wrong_guard")

"""Compiler-free pin checks and host controls for the real Midi slot bodies.

The host fixture extracts the current Midi declarations and definitions. It
does not measure VC6 ABI/codegen, object layout, linker selection, or runtime
audio behavior; those remain coordinator/native-build obligations.
"""
from __future__ import annotations

import os
import hashlib
from pathlib import Path
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.shared_source import read_shared
from match_recovery.source import Mode, read_file
from match_recovery.source_inventory import image_pin


def repository_root(start: Path) -> Path:
    for candidate in (start.resolve(), *start.resolve().parents):
        if ((candidate / "CMakeLists.txt").is_file()
                and (candidate / "src/IMAGE_PIN").is_file()
                and (candidate / "recovery/WORKFLOW.md").is_file()):
            return candidate
    raise RuntimeError(f"cannot locate project root from {start}")


ROOT = repository_root(Path(__file__).parent)
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for index in range(brace, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError(f"unterminated product definition: {signature}")


def declaration(text: str, signature: str) -> str:
    matches = [line.strip().split("//", 1)[0].strip()
               for line in text.splitlines()
               if signature in line and line.lstrip().startswith("virtual int ")]
    assert len(matches) == 1, (signature, matches)
    return matches[0]


def product_host_program(midi_cpp: str, sound_h: str, midi_h: str) -> str:
    sound_decls = "\n".join(
        declaration(sound_h, signature) for signature in
        ("get_game_hwnd", "set_attrib", "get_attrib"))
    midi_decls = "\n".join(
        declaration(midi_h, signature) for signature in
        ("get_game_hwnd", "set_attrib", "get_attrib"))
    definitions = "\n".join(
        definition(midi_cpp, signature) for signature in
        ("int Midi::get_game_hwnd()", "int Midi::set_attrib(unsigned long)",
         "int Midi::get_attrib()"))
    return f'''#include <assert.h>
class Sound {{ public:
{sound_decls}
}};
class Midi : public Sound {{ public:
{midi_decls}
}};
{definitions}
int main() {{
    Midi midi;
    Sound *base = &midi;
    assert(midi.get_game_hwnd() == 0);
    assert(midi.set_attrib(0x12345678UL) == 11);
    assert(midi.get_attrib() == 0);
    assert(base->get_game_hwnd() == 0);
    assert(base->set_attrib(0UL) == 11);
    assert(base->get_attrib() == 0);
    return 0;
}}
'''


def compile_and_run(compiler: str, tmp_path: Path, name: str,
                    program: str) -> subprocess.CompletedProcess:
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(program, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)], capture_output=True, text=True,
        timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def test_pinned_sound_midi_ambience_and_wave_slot_targets_without_native_tools():
    assert image_pin(ROOT) == PIN
    path = Path(os.environ.get("OPENSMACX_IMAGE",
                              ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is not installed")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    expected = {
        (0x66E3C0, 15): 0x645844,
        (0x66E3C0, 27): 0x645844,
        (0x66E3C0, 28): 0x645844,
        (0x66E538, 15): 0x406B30,
        (0x66E538, 27): 0x4C93E0,
        (0x66E538, 28): 0x406B30,
        (0x66E664, 15): 0x406B30,
        (0x66E664, 27): 0x4C93E0,
        (0x66E664, 28): 0x406B30,
        (0x66E44C, 15): 0x4C7010,
        (0x66E44C, 27): 0x4C6F20,
        (0x66E44C, 28): 0x4C6F80,
    }
    for (table, slot), target in expected.items():
        assert int.from_bytes(image.read(Span(table + slot * 4,
                                             table + (slot + 1) * 4)),
                              "little") == target
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    zero_body = list(decoder.disasm(image.read(Span(0x406B30, 0x406B33)), 0x406B30))
    setter_body = list(decoder.disasm(image.read(Span(0x4C93E0, 0x4C93E8)), 0x4C93E0))
    assert [(op.mnemonic, op.op_str) for op in zero_body] == [
        ("xor", "eax, eax"), ("ret", "")]
    assert [(op.mnemonic, op.op_str) for op in setter_body] == [
        ("mov", "eax, 0xb"), ("ret", "4")]


def test_pinned_play_type_null_path_returns_before_the_adjacent_initializer():
    """Decode from the real entry, not from its overlapping historical end.

    This proves the original control-flow/register observation only. A missing
    C++ return does not prove that VC6 preserves the incoming EAX value.
    """
    assert image_pin(ROOT) == PIN
    path = Path(os.environ.get("OPENSMACX_IMAGE",
                              ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is not installed")
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    ops = list(decoder.disasm(image.read(Span(0x4C8050, 0x4C8072)),
                             0x4C8050))
    by_address = {op.address: op for op in ops}
    assert [(by_address[address].mnemonic, by_address[address].op_str)
            for address in (0x4C8050, 0x4C8051, 0x4C8053, 0x4C8056,
                            0x4C8058, 0x4C8066, 0x4C8067)] == [
        ("push", "ebp"), ("mov", "ebp, esp"),
        ("mov", "ecx, dword ptr [ecx + 0x3c]"),
        ("test", "ecx, ecx"), ("je", "0x4c8066"),
        ("pop", "ebp"), ("ret", "4")]
    null_path = (0x4C8050, 0x4C8051, 0x4C8053, 0x4C8056,
                 0x4C8058, 0x4C8066, 0x4C8067)
    for address in null_path:
        _, written = by_address[address].regs_access()
        assert "eax" not in [decoder.reg_name(reg) for reg in written]
    assert by_address[0x4C8067].address + by_address[0x4C8067].size == 0x4C806A
    assert all(by_address[address].mnemonic == "nop"
               for address in range(0x4C806A, 0x4C8070))
    assert (by_address[0x4C8070].mnemonic,
            by_address[0x4C8070].op_str) == ("mov", "eax, ecx")
    assert by_address[0x4C8060].mnemonic == "call"
    assert by_address[0x4C8060].op_str == "dword ptr [eax + 0x11c]"
    midi = next(row for row in read_file(ROOT / "src/midi.cpp")
                if row.address == 0x4C8050)
    adjacent = next(row for row in read_file(ROOT / "src/word_state.cpp")
                    if row.address == 0x4C8070)
    assert midi.image_spans == ((0x4C8050, 0x4C8072),)
    assert adjacent.image_spans == ((0x4C8070, 0x4C8089),)


def test_product_contract_is_pure_on_sound_and_concrete_on_midi():
    sound_h = (ROOT / "src/sound.h").read_text(encoding="utf-8")
    sound_cpp = (ROOT / "src/sound.cpp").read_text(encoding="utf-8")
    midi_h = (ROOT / "src/midi.h").read_text(encoding="utf-8")
    midi_cpp = (ROOT / "src/midi.cpp").read_text(encoding="utf-8")
    assert "virtual int get_game_hwnd() = 0;" in sound_h
    assert "virtual int set_attrib(unsigned long attrib) = 0;" in sound_h
    assert "virtual int get_attrib() = 0;" in sound_h
    assert "int Sound::get_game_hwnd()" not in sound_cpp
    assert "int Sound::set_attrib(" not in sound_cpp
    assert "int Sound::get_attrib()" not in sound_cpp
    assert "class Midi : public Sound" in midi_h
    assert "class Ambience : public Midi" in (ROOT / "src/ambience.h").read_text()
    assert "Ambience g_AMBIENCE" in (ROOT / "src/ambience.cpp").read_text()

    rows = [row for row in read_file(ROOT / "src/midi.cpp")
            if row.address == 0x4C93E0]
    assert len(rows) == 1
    row, = rows
    assert row.mode is Mode.FILE and row.image_spans == ((0x4C93E0, 0x4C93E8),)
    assert row.byte_exact and row.symbol == "?set_attrib@Midi@@UAEHK@Z"

    shares = {row.symbol: row for row in read_shared(ROOT, PIN)}
    for symbol in ("?get_game_hwnd@Midi@@UAEHXZ", "?get_attrib@Midi@@UAEHXZ"):
        assert shares[symbol].span == Span(0x406B30, 0x406B33)
        assert shares[symbol].evidence.address == 0x4C75B0


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_current_midi_bodies_execute_and_each_mutation_is_detected(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    midi_cpp = (ROOT / "src/midi.cpp").read_text(encoding="utf-8")
    sound_h = (ROOT / "src/sound.h").read_text(encoding="utf-8")
    midi_h = (ROOT / "src/midi.h").read_text(encoding="utf-8")
    actual = product_host_program(midi_cpp, sound_h, midi_h)
    run = compile_and_run(compiler, tmp_path, "midi_actual_slots", actual)
    assert run.returncode == 0, run.stdout + run.stderr

    mutations = (
        ("int Midi::get_game_hwnd()", "return 0;", "return 1;"),
        ("int Midi::set_attrib(unsigned long)", "return 0x0B;", "return 0x0A;"),
        ("int Midi::get_attrib()", "return 0;", "return 1;"),
    )
    for index, (signature, before, after) in enumerate(mutations):
        method = definition(midi_cpp, signature)
        assert method.count(before) == 1
        changed = midi_cpp.replace(method, method.replace(before, after, 1), 1)
        rejected = compile_and_run(
            compiler, tmp_path, f"midi_slot_mutation_{index}",
            product_host_program(changed, sound_h, midi_h))
        assert rejected.returncode != 0, f"mutation escaped for {signature}"

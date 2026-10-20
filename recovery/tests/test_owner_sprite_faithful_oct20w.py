"""Actual Sprite source controls for the original change-color count path."""
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.pe_coff import PeImage, Span
from match_recovery.source.asm import Listing


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/sprite.cpp").is_file())
SOURCE = ROOT / "src/sprite.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
CHANGE_COLOR = "int Sprite::change_color(int old_color, int new_color)"


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )


def _method(source):
    masked = _masked(source)
    assert masked.count(CHANGE_COLOR) == 1
    start = masked.index(CHANGE_COLOR)
    opening = masked.index("{", start + len(CHANGE_COLOR))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated Sprite::change_color definition")


def _fixture(method):
    return r'''#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

struct Sprite {
    unsigned char *pcBits_;
    int iSpriteHeight_;
    int iSpriteWidth2_;
    int change_color(int old_color, int new_color);
};

''' + method + r'''
int main(int argc, char **argv) {
    if (argc > 1) {
        const size_t byte_count = static_cast<size_t>(0x80000000ULL);
        const size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        unsigned char *arena = static_cast<unsigned char *>(mmap(
            0, byte_count + page_size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        if (arena == MAP_FAILED) return 77;
        if (mprotect(arena + byte_count, page_size, PROT_NONE) != 0) return 78;
        Sprite large = {arena, 0x40000000, 2};
        // All accessible bytes are demand-zero and cannot match 1.  The old
        // unsigned byte scan ends at the guard; original SAR/LOOP continues.
        return large.change_color(1, 2);
    }

    unsigned char pixels[] = {1, 7, 1, 3, 1, 2, 7, 1, 4};
    Sprite sprite = {pixels, 1, 9};
    if (sprite.change_color(0x101, 0x105) != 0) return 1;
    const unsigned char expected[] = {5, 7, 5, 3, 5, 2, 7, 5, 4};
    if (memcmp(pixels, expected, sizeof(expected)) != 0) return 2;

    unsigned char short_pixels[] = {1, 1, 1};
    Sprite short_sprite = {short_pixels, 1, 3};
    if (short_sprite.change_color(1, 2) != 0) return 3;
    if (short_pixels[0] != 1 || short_pixels[1] != 1 || short_pixels[2] != 1)
        return 4;
    Sprite absent = {0, 4, 4};
    if (absent.change_color(1, 2) != 7) return 5;
    return 0;
}
'''


def _compile(method, compiler, output):
    cpp = output.with_suffix(".cpp")
    cpp.write_text(_fixture(method), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         str(cpp), "-o", str(output)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr


def _run_without_core(executable, *args):
    import resource

    def disable_core():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run(
        [str(executable), *args], capture_output=True, timeout=45,
        preexec_fn=disable_core,
    )


def test_pinned_change_color_group_and_tail_instructions():
    explicit = __import__("os").environ.get("OPENSMACX_IMAGE")
    image = Path(explicit) if explicit else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image.is_file():
        pytest.skip("pinned original not available")
    pe = PeImage(image.read_bytes(), PIN)

    def at(address, length):
        return Listing(pe.read(Span(address, address + length)), address).instructions

    assert [(i.mnemonic, i.op_str) for i in at(0x5EADBF, 4)] == [
        ("imul", "eax, dword ptr [ecx + 0xc]")]
    assert [(i.mnemonic, i.op_str) for i in at(0x5EADCE, 3)] == [
        ("sar", "ecx, 2")]
    assert [(i.mnemonic, i.op_str) for i in at(0x5EADF8, 4)] == [
        ("and", "ecx, ecx"), ("je", "0x5eae44")]
    assert [(i.mnemonic, i.op_str) for i in at(0x5EAE10, 2)] == [
        ("loop", "0x5eadfc")]
    assert [(i.mnemonic, i.op_str) for i in at(0x5EAE31, 2)] == [
        ("loop", "0x5eae15")]


def test_actual_change_color_valid_groups_tail_and_zero_count_behavior(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    method = _method(source)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in compilers if compiler]
    if not available:
        pytest.skip("GCC or Clang required for actual-source Sprite controls")
    for index, compiler in enumerate(available):
        executable = tmp_path / f"sprite-change-{index}"
        _compile(method, compiler, executable)
        run = subprocess.run([str(executable)], capture_output=True, timeout=10)
        assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.skipif(not sys.platform.startswith("linux") or sys.maxsize <= 2**32,
                    reason="2 GiB guarded virtual mapping requires Linux 64-bit")
def test_sar_group_count_crosses_wrapped_byte_count_endpoint(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    actual = _method(source)
    masked = _masked(actual)
    sign_fill = re.compile(
        r"const uint32_t group_count = \(pixel_count_bits >> 2\) \|\s*"
        r"\(\(pixel_count_bits & 0x80000000u\) \? 0xC0000000u : 0u\);"
    )
    assert len(list(sign_fill.finditer(masked))) == 1
    mutant = sign_fill.sub(
        "const uint32_t group_count = pixel_count_bits >> 2;", actual, count=1)

    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC or Clang required for actual-source Sprite controls")
    actual_exe = tmp_path / "sprite-change-sar"
    mutant_exe = tmp_path / "sprite-change-shr-mutant"
    _compile(actual, compiler, actual_exe)
    _compile(mutant, compiler, mutant_exe)

    actual_run = _run_without_core(actual_exe, "high-bit")
    if actual_run.returncode in (77, 78):
        pytest.skip("host could not reserve/protect the bounded 2 GiB virtual mapping")
    assert actual_run.returncode == -signal.SIGSEGV, (
        "pinned signed SAR + LOOP count must continue beyond the 0x80000000-byte endpoint; "
        f"got {actual_run.returncode}: {actual_run.stdout!r} {actual_run.stderr!r}")

    mutant_run = _run_without_core(mutant_exe, "high-bit")
    assert mutant_run.returncode == 0, (
        "logical-shift mutant should finish at the unsigned byte-count endpoint")

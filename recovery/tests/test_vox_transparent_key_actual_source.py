"""Exercise the actual Caviar 16-bit blitter key type and word comparison."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/caviar.cpp").is_file())
PRODUCT = ROOT / "src/caviar.cpp"


def extract(source: str, signature: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source)
    start = masked.find(signature)
    while start >= 0:
        opening = masked.find("{", start)
        semicolon = masked.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = masked.find(signature, start + len(signature))
    assert start >= 0, "missing actual source definition: " + signature
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unclosed actual source body: " + signature)


HOST = r'''#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#define __cdecl
struct CaviarBlitStateCandidate {
    unsigned char * volatile destination_cursor_;
    uint32_t source_row_bytes_;
    uint32_t destination_stride_bytes_;
};
CaviarBlitStateCandidate CaviarBlitState = {0, 0, 0};
static void *low_map(size_t bytes) {
    void *p = mmap(0, bytes, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (p == MAP_FAILED || (uintptr_t)p > 0x7fffffffU) return 0;
    return p;
}
'''


MAIN = r'''
int main() {
    unsigned short *destination = (unsigned short *)low_map(4096);
    unsigned short *source = (unsigned short *)low_map(4096);
    assert(destination && source);
    const unsigned short keys[4] = {0x0000, 0x7fff, 0x8000, 0xffff};
    const unsigned short different[4] = {0x0001, 0x7ffe, 0x8001, 0xfffe};
    for (int test = 0; test < 4; ++test) {
        for (int i = 0; i < 32; ++i) destination[i] = 0xa55a;
        source[0] = keys[test];
        source[1] = different[test];
        source[2] = different[test];
        source[3] = keys[test];
        vox_blit_transparent((unsigned char *)destination, source,
                             1, 1, 2, 2, 0x10000, 2, 4, keys[test]);
        // Nonzero origin and distinct row stride exercise the pinned address
        // calculation. Matching key words preserve destination pixels;
        // nonmatching words overwrite them, including both high-bit keys.
        assert(destination[5] == 0xa55a);
        assert(destination[6] == different[test]);
        assert(destination[9] == different[test]);
        assert(destination[10] == 0xa55a);
        assert(CaviarBlitState.source_row_bytes_ == 4);
        assert(CaviarBlitState.destination_stride_bytes_ == 8);
        assert((uintptr_t)CaviarBlitState.destination_cursor_ ==
               (uintptr_t)(destination + 13));
    }

    // The original outer guards skip all state writes for nonpositive width
    // or height. Retain that behavior and the void return contract.
    CaviarBlitState.destination_cursor_ = (unsigned char *)(destination + 3);
    CaviarBlitState.source_row_bytes_ = 0x1234;
    CaviarBlitState.destination_stride_bytes_ = 0x5678;
    for (int i = 0; i < 32; ++i) destination[i] = 0x3344;
    vox_blit_transparent((unsigned char *)destination, source,
                         0, 0, 0, 2, 0x10000, 2, 4, 0xffff);
    vox_blit_transparent((unsigned char *)destination, source,
                         0, 0, 2, 0, 0x10000, 2, 4, 0xffff);
    vox_blit_transparent((unsigned char *)destination, source,
                         0, 0, -1, 2, 0x10000, 2, 4, 0xffff);
    vox_blit_transparent((unsigned char *)destination, source,
                         0, 0, 2, -1, 0x10000, 2, 4, 0xffff);
    for (int i = 0; i < 32; ++i) assert(destination[i] == 0x3344);
    assert((uintptr_t)CaviarBlitState.destination_cursor_ ==
           (uintptr_t)(destination + 3));
    assert(CaviarBlitState.source_row_bytes_ == 0x1234);
    assert(CaviarBlitState.destination_stride_bytes_ == 0x5678);
    return 0;
}
'''


def compile_and_run(compiler: str, text: str, tmp_path: Path, stem: str):
    source = tmp_path / (stem + ".cpp")
    executable = tmp_path / stem
    source.write_text(text)
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(source), "-o", str(executable)],
        capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_actual_blitter_uses_unsigned_word_key_and_signed_mutant_fails(tmp_path):
    source = PRODUCT.read_text()
    helpers = "\n".join(extract(source, signature) for signature in (
        "static uint32_t caviar_blit_offset(",
        "static unsigned char *caviar_blit_address(",
        "static const char *caviar_blit_source_address(",
    ))
    body = extract(source, "void __cdecl vox_blit_transparent(")
    assert "unsigned short transparent_key" in body
    assert "if (value != transparent_key)" in body
    assert "static_cast<unsigned short>(transparent_key)" not in body
    definitions = helpers + "\n" + body
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "GCC and Clang are required for these host controls"

    for index, compiler in enumerate(compilers):
        result = compile_and_run(compiler, HOST + definitions + MAIN, tmp_path,
                                "unsigned_key_" + str(index))
        assert result.returncode == 0, result.stdout + result.stderr

        # Restore the prior signed declaration in the actual extracted body.
        # The high-bit match assertions above must catch the exact defect.
        mutant = body.replace("unsigned short transparent_key",
                              "short transparent_key", 1)
        assert mutant != body
        result = compile_and_run(compiler, HOST + helpers + "\n" + mutant + MAIN,
                                tmp_path, "signed_key_mutant_" + str(index))
        assert (result.returncode != 0 and "destination[5]" in result.stderr), (
            "signed-key mutant did not fail the expected high-bit match: " +
            result.stdout + result.stderr)

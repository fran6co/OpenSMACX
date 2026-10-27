"""Exercise the complete checked-in Palette timer callback on a host."""

from __future__ import annotations

import shutil
import subprocess
import os
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/palette.cpp"
SIGNATURE = "void __cdecl Palette::timer_callback(int key, void *context)"


def extract_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#define __cdecl

struct PALETTEENTRY { uint8_t peRed, peGreen, peBlue, peFlags; };
struct Palette {
    struct Slot {
        uint32_t key = UINT32_MAX;
        uint8_t first = 0, count = 0, field_A = 0, field_B = 0;
        PALETTEENTRY *colours = nullptr;
    };
    PALETTEENTRY entries_[256]{};
    uint32_t seed_ = 0;
    Slot internal_[5]{};
    static void __cdecl timer_callback(int key, void *context);
};

static int PaletteUsesSystemColours = 0;
static void *PaletteInitialized = reinterpret_cast<void *>(uintptr_t{1});
static int PaletteSeedCache = 0;
struct AnimationCall {
    void *palette;
    unsigned int start;
    unsigned int count;
    PALETTEENTRY *entries;
    int cache_at_call;
};
static std::vector<AnimationCall> animation_calls;
static void AnimatePalette(void *palette, unsigned int start, unsigned int count,
                           PALETTEENTRY *entries) {
    animation_calls.push_back({palette, start, count, entries, PaletteSeedCache});
}

// Complete definition extracted verbatim from src/jackal/palette.cpp follows.
'''


def host_source(callback: str, *, unsigned_mutant: bool = False) -> str:
    if unsigned_mutant:
        # Recreate the previous counter types as a negative control.
        assert callback.count("int cEntries = self->internal_[idx].count;") == 1
        assert callback.count("int i;") == 1
        callback = callback.replace(
            "int cEntries = self->internal_[idx].count;",
            "unsigned int cEntries = self->internal_[idx].count;",
            1,
        ).replace("int i;", "unsigned int i;", 1)
    suffix = r'''
static PALETTEENTRY value(int i) {
    return {static_cast<uint8_t>(i + 10), static_cast<uint8_t>(i + 30),
            static_cast<uint8_t>(i + 50), static_cast<uint8_t>(i ^ 0xA5)};
}
static void initialize(Palette &p) {
    for (int i = 0; i < 256; ++i) p.entries_[i] = value(i);
    p.internal_[0].key = 17;
    p.internal_[0].first = 10;
    p.internal_[0].count = 0;
    p.seed_ = 321;
}

int main() {
    // count==0, with first>0: the original tail store writes RGB to the
    // preceding valid entry, still calls AnimatePalette(count 0), then caches
    // the seed. It deliberately leaves flags and entries_[first] alone.
    Palette empty;
    initialize(empty);
    empty.internal_[0].first = 10;
    empty.internal_[0].count = 0;
    const PALETTEENTRY before_first = empty.entries_[10];
    const PALETTEENTRY before_previous = empty.entries_[9];
    PaletteSeedCache = 44;
    animation_calls.clear();
    Palette::timer_callback(17, &empty);
    assert(empty.entries_[9].peRed == before_first.peRed);
    assert(empty.entries_[9].peGreen == before_first.peGreen);
    assert(empty.entries_[9].peBlue == before_first.peBlue);
    assert(empty.entries_[9].peFlags == before_previous.peFlags);
    assert(std::memcmp(&empty.entries_[10], &before_first, sizeof(PALETTEENTRY)) == 0);
    assert(animation_calls.size() == 1);
    assert(animation_calls[0].palette == PaletteInitialized);
    assert(animation_calls[0].start == 10 && animation_calls[0].count == 0);
    assert(animation_calls[0].entries == &empty.entries_[10]);
    assert(animation_calls[0].cache_at_call == 44 && PaletteSeedCache == 321);

    // A one-entry cycle keeps its RGB and all flags while preserving the API
    // call and post-call seed write.
    Palette one;
    initialize(one);
    one.internal_[0].first = 31;
    one.internal_[0].count = 1;
    const PALETTEENTRY one_before[3] = {one.entries_[30], one.entries_[31], one.entries_[32]};
    PaletteSeedCache = 45;
    animation_calls.clear();
    Palette::timer_callback(17, &one);
    assert(std::memcmp(one.entries_ + 30, one_before, sizeof(one_before)) == 0);
    assert(animation_calls.size() == 1 && animation_calls[0].start == 31 && animation_calls[0].count == 1);
    assert(animation_calls[0].entries == &one.entries_[31] && animation_calls[0].cache_at_call == 45);
    assert(PaletteSeedCache == 321);

    // Ordinary rotation moves RGB forward and wraps the saved first RGB to
    // the last entry; per-entry flags and all surrounding entries stay put.
    Palette ordinary;
    initialize(ordinary);
    ordinary.internal_[0].first = 40;
    ordinary.internal_[0].count = 3;
    const PALETTEENTRY before[5] = {ordinary.entries_[39], ordinary.entries_[40],
                                    ordinary.entries_[41], ordinary.entries_[42],
                                    ordinary.entries_[43]};
    PaletteSeedCache = 46;
    animation_calls.clear();
    Palette::timer_callback(17, &ordinary);
    assert(ordinary.entries_[40].peRed == before[2].peRed);
    assert(ordinary.entries_[40].peGreen == before[2].peGreen);
    assert(ordinary.entries_[40].peBlue == before[2].peBlue);
    assert(ordinary.entries_[41].peRed == before[3].peRed);
    assert(ordinary.entries_[41].peGreen == before[3].peGreen);
    assert(ordinary.entries_[41].peBlue == before[3].peBlue);
    assert(ordinary.entries_[42].peRed == before[1].peRed);
    assert(ordinary.entries_[42].peGreen == before[1].peGreen);
    assert(ordinary.entries_[42].peBlue == before[1].peBlue);
    for (int i = 0; i < 5; ++i) assert(ordinary.entries_[39+i].peFlags == before[i].peFlags);
    assert(std::memcmp(&ordinary.entries_[39], &before[0], sizeof(PALETTEENTRY)) == 0);
    assert(std::memcmp(&ordinary.entries_[43], &before[4], sizeof(PALETTEENTRY)) == 0);
    assert(animation_calls.size() == 1 && animation_calls[0].start == 40 && animation_calls[0].count == 3);
    assert(animation_calls[0].entries == &ordinary.entries_[40] && animation_calls[0].cache_at_call == 46);
    assert(PaletteSeedCache == 321);

    // Existing early returns: null context, system-colour mode, and a key
    // miss stopped by the free-slot sentinel all leave palette/API state alone.
    animation_calls.clear();
    PaletteSeedCache = 50;
    Palette::timer_callback(17, nullptr);
    assert(animation_calls.empty() && PaletteSeedCache == 50);
    PaletteUsesSystemColours = 1;
    Palette::timer_callback(17, &one);
    PaletteUsesSystemColours = 0;
    assert(animation_calls.empty() && PaletteSeedCache == 50);
    Palette miss;
    initialize(miss);
    miss.internal_[0].key = 99;
    Palette::timer_callback(17, &miss);
    assert(animation_calls.empty() && PaletteSeedCache == 50);
    return 0;
}
'''
    return HARNESS + callback + "\n" + suffix


def compiler() -> str:
    found = shutil.which("clang++") or shutil.which("g++") or shutil.which("c++")
    assert found, "a host C++ compiler is required for actual-source controls"
    return found


def compile_source(tmp_path: Path, text: str, *, asan: bool = False):
    cc = compiler()
    unit = tmp_path / ("palette_mutant.cpp" if asan else "palette_actual.cpp")
    binary = tmp_path / ("palette_mutant" if asan else "palette_actual")
    unit.write_text(text, encoding="utf-8")
    flags = ["-std=c++17", "-O0"]
    if asan:
        flags += ["-fsanitize=address", "-fno-omit-frame-pointer"]
    try:
        compiled = subprocess.run([cc, *flags, str(unit), "-o", str(binary)],
                                  text=True, capture_output=True, check=False,
                                  timeout=30)
    except subprocess.TimeoutExpired as exc:
        pytest.fail(f"host compile timed out: {exc}")
    return compiled, binary


def sanitizer_environment() -> dict[str, str]:
    env = os.environ.copy()
    # LeakSanitizer cannot run under the harness's ptrace sandbox; address
    # checking remains enabled for both positive and mutant executions.
    env["ASAN_OPTIONS"] = "detect_leaks=0"
    return env


def test_actual_palette_callback_zero_single_and_ordinary_rotation(tmp_path: Path):
    callback = extract_definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    compiled, binary = compile_source(tmp_path, host_source(callback), asan=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], text=True, capture_output=True,
                            check=False, timeout=5, env=sanitizer_environment())
    assert result.returncode == 0, result.stdout + result.stderr


def test_unsigned_counter_mutant_compiles_but_fails_actual_zero_count_control(tmp_path: Path):
    callback = extract_definition(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    compiled, binary = compile_source(tmp_path, host_source(callback, unsigned_mutant=True), asan=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    try:
        result = subprocess.run([str(binary)], text=True, capture_output=True,
                                check=False, timeout=5, env=sanitizer_environment())
    except subprocess.TimeoutExpired as exc:
        pytest.fail(f"unsigned mutant ran away instead of producing a bounded sanitizer failure: {exc}")
    assert result.returncode != 0, "unsigned-counter mutant passed the zero-count control"
    assert "AddressSanitizer: stack-buffer-overflow" in result.stderr, (
        "unsigned mutant failed without the expected bounded out-of-range access:\n"
        + result.stdout + result.stderr
    )

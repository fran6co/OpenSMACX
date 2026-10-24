"""Executable host-test proposal for the signed Palette::fade_to candidate.

This compiles the complete mapped source body with typed host doubles. It
checks ordinary interpolation/callback behavior separately from a target-style
32-bit wrap witness; the latter is not a portable C++ signed-overflow claim or
a Wine/runtime proof. The wrap witness is explicitly qualified; it does not establish portable overflow semantics.
"""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/palette.cpp"
ADDRESS = 0x005FF930
SYMBOL = "?fade_to@Palette@@QAEHPAV1@HHHH@Z"
SIGNATURE = "int Palette::fade_to(Palette *other, int start, int count, int steps,"


def _actual_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    rows = [row for row in read_file(SOURCE) if row.address == ADDRESS]
    assert len(rows) == 1
    row = rows[0]
    assert (row.symbol or row.name) == SYMBOL
    assert row.image_spans == ((0x005FF930, 0x005FFB0B),)
    assert not row.extract_error and "BODY GOES HERE." not in row.region
    body = _definition(source, SIGNATURE)
    assert body in row.region
    # This is the signed candidate. The old spelling is synthesized below as
    # a deliberate mutant; the mapped implementation itself remains complete.
    return body


def _unsigned_cast_mutant(body: str) -> str:
    mutant = body
    for channel in ("peRed", "peGreen", "peBlue"):
        for values, weight in (("selfCopy[k]", "weight"), ("otherCopy[k]", "frame")):
            old = f"{values}.{channel} * {weight}"
            new = f"static_cast<unsigned int>({values}.{channel}) * {weight}"
            assert mutant.count(old) == 1, old
            mutant = mutant.replace(old, new, 1)
    assert mutant.count("static_cast<unsigned int>") == body.count("static_cast<unsigned int>") + 6
    return mutant


HARNESS = r"""
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
using DWORD = uint32_t;
static_assert(sizeof(int) == 4, "the diagnostic models 32-bit target int");
struct PALETTEENTRY { unsigned char peRed, peGreen, peBlue, peFlags; };
using HPALETTE = void *;

static int PaletteUsesSystemColours;
static HPALETTE PaletteInitialized;
static int palette_token_storage;
static void *palette_token = &palette_token_storage;
static std::vector<char> events;
static std::vector<std::vector<PALETTEENTRY>> frames;
static int copies;
static int random_calls;
static int time_calls;
static uint32_t fake_time;
static bool stop_at_first_animation;
static unsigned char first_diagnostic_red;

struct StopAfterFirstAnimation {};

static void record(char c) { events.push_back(c); }
static void *fixture_memcpy(void *dst, const void *src, std::size_t bytes) {
    ++copies;
    record('C');
    return std::memcpy(dst, src, bytes);
}
DWORD timeGetTime() {
    ++time_calls;
    record('T');
    return fake_time++;
}
int AnimatePalette(HPALETTE palette, unsigned int start, unsigned int count,
                   PALETTEENTRY *entries) {
    if (palette != PaletteInitialized || count > 256 || start > 256 - count)
        return 0;
    record('A');
    frames.emplace_back(entries, entries + count);
    if (stop_at_first_animation) {
        first_diagnostic_red = entries[0].peRed;
        throw StopAfterFirstAnimation{};
    }
    return 1;
}
static int random(int low, int high) {
    if (low != 0 || high != 0xffff) return -1;
    ++random_calls;
    record('R');
    return random_calls == 1 ? 0 : 7;
}

class Palette {
public:
    PALETTEENTRY entries_[256];
    unsigned int seed_;
    int fade_to(Palette *other, int start, int count, int steps,
                int min_frame_ms);
};

#define memcpy fixture_memcpy
__BODY__
#undef memcpy

static void reset_observers() {
    events.clear(); frames.clear(); copies = random_calls = time_calls = 0;
    fake_time = 0; stop_at_first_animation = false;
    first_diagnostic_red = 0; PaletteUsesSystemColours = 0;
    PaletteInitialized = palette_token;
}

static bool ordinary_fade() {
    reset_observers();
    Palette source = {}, destination = {};
    for (int i = 0; i < 256; ++i) {
        source.entries_[i] = {static_cast<unsigned char>(i),
                              static_cast<unsigned char>(i + 1),
                              static_cast<unsigned char>(i + 2),
                              static_cast<unsigned char>(i + 3)};
        destination.entries_[i] = {static_cast<unsigned char>(200 - i),
                                   static_cast<unsigned char>(180 - i),
                                   static_cast<unsigned char>(160 - i),
                                   static_cast<unsigned char>(i ^ 0x5a)};
    }
    source.entries_[1] = {0, 100, 50, 0x11};
    source.entries_[2] = {255, 50, 90, 0x22};
    destination.entries_[1] = {255, 200, 10, 0x31};
    destination.entries_[2] = {0, 250, 200, 0x42};
    // Preserve the complete outside range and all peFlags, not just RGB.
    PALETTEENTRY outside[256];
    std::memcpy(outside, destination.entries_, sizeof(outside));

    if (source.fade_to(&destination, 1, 2, 2, 2) != 0) return false;
    if (copies != 2 || frames.size() != 3 || random_calls != 2) return false;
    if (time_calls != 9 || source.seed_ != 7) return false;
    if (events != std::vector<char>({'C','C','T','A','T','T',
                                     'T','A','T','T','T','A','T','T','R','R'}))
        return false;
    const unsigned char expected[3][2][3] = {
        {{0,100,50}, {255,50,90}},
        {{127,150,30}, {127,150,145}},
        {{255,200,10}, {0,250,200}}
    };
    for (std::size_t frame = 0; frame < frames.size(); ++frame) {
        if (frames[frame].size() != 2) return false;
        for (int index = 0; index < 2; ++index) {
            const PALETTEENTRY &entry = frames[frame][index];
            if (entry.peRed != expected[frame][index][0] ||
                entry.peGreen != expected[frame][index][1] ||
                entry.peBlue != expected[frame][index][2] ||
                entry.peFlags != outside[index + 1].peFlags) return false;
        }
    }
    for (int i = 0; i < 256; ++i) {
        if (i < 1 || i >= 3) {
            if (std::memcmp(&destination.entries_[i], &outside[i],
                            sizeof(PALETTEENTRY)) != 0) return false;
        }
    }
    if (destination.entries_[1].peFlags != outside[1].peFlags ||
        destination.entries_[2].peFlags != outside[2].peFlags) return false;
    return true;
}

static bool guards_and_skipped_steps() {
    Palette source = {}, destination = {};
    reset_observers(); PaletteUsesSystemColours = 1;
    if (source.fade_to(nullptr, 0, 1, 2, 0) != 0 || copies || time_calls || random_calls)
        return false;
    reset_observers(); PaletteInitialized = nullptr;
    if (source.fade_to(nullptr, 0, 1, 2, 0) != 7 || copies || time_calls || random_calls)
        return false;
    reset_observers();
    if (source.fade_to(nullptr, 0, 1, 2, 0) != 0x10 || copies || time_calls || random_calls)
        return false;
    for (int steps : {-1, -2}) {
        reset_observers();
        for (int i = 0; i < 256; ++i)
            destination.entries_[i] = {1,2,3,static_cast<unsigned char>(i)};
        PALETTEENTRY before[256];
        std::memcpy(before, destination.entries_, sizeof(before));
        if (source.fade_to(&destination, 5, 1, steps, 0) != 0) return false;
        if (copies != 2 || !frames.empty() || time_calls != 0 || random_calls != 2)
            return false;
        if (std::memcmp(before, destination.entries_, sizeof(before)) != 0) return false;
    }
    return true;
}

static bool signed_wrap_diagnostic() {
    reset_observers();
    Palette source = {}, destination = {};
    source.entries_[0] = {255,0,0,0};
    destination.entries_[0] = {0,0,0,0};
    stop_at_first_animation = true;
    try {
        source.fade_to(&destination, 0, 1, 10000000, 0);
    } catch (const StopAfterFirstAnimation &) {
        return first_diagnostic_red == __DIAGNOSTIC_EXPECTED__ &&
               frames.size() == 1 && random_calls == 0;
    }
    return false;
}

int main() {
    if (!ordinary_fade()) return 1;
    if (!guards_and_skipped_steps()) return 2;
    if (!signed_wrap_diagnostic()) return 3;
    return 0;
}
"""


def _program(body: str, diagnostic_expected: int) -> str:
    return HARNESS.replace("__BODY__", body).replace(
        "__DIAGNOSTIC_EXPECTED__", str(diagnostic_expected)
    )


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_complete_fade_candidate_and_old_unsigned_mutant(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = _actual_body()
    mutant = _unsigned_cast_mutant(body)
    for name, candidate_body, expected, exit_code in (
        ("signed_candidate", body, 82, 0),
        ("unsigned_violates_original_oracle", mutant, 82, 3),
        ("old_unsigned_explained_control", mutant, 255, 0),
    ):
        source = tmp_path / f"{name}.cpp"
        binary = tmp_path / name
        source.write_text(_program(candidate_body, expected), encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-O0", "-fwrapv", str(source), "-o", str(binary)],
            capture_output=True, text=True, check=False,
        )
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
        assert ran.returncode == exit_code, ran.stderr

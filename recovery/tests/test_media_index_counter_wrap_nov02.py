"""Preserve the index decoder's native wrapped count and first-read phase."""
from pathlib import Path
import subprocess

import pytest

from test_movie_runtime_decoder_ownership_actual_source_oct9 import (
    _compile_run,
    _definition,
)


GENERAL = Path(__file__).resolve().parents[2] / "src/movie_media_decoder.cpp"


def _unit(source: str) -> str:
    functions = "\n".join(
        _definition(source, signature)
        for signature in (
            "static unsigned int media_rotate_right_32(",
            'extern "C" void __cdecl sub_5d439f(',
        )
    )
    return r'''#include <cassert>
#include <climits>
#include <cstring>
#include <initializer_list>
#define __cdecl
static const void *first_input;
static bool stop_at_input;
struct InputRead {};
static void *observed_copy(void *dst, const void *src, size_t size) {
    if (stop_at_input && src == first_input) throw InputRead();
    return std::memcpy(dst, src, size);
}
#define memcpy observed_copy
''' + functions + r'''
#undef memcpy
int main() {
    const unsigned char packed[16] = {
        0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76
    };
    first_input = packed;
    unsigned int output[12];
    // Ordinary block/tail counts, including the zero and negative no-read paths.
    for (int count = -3; count <= 9; ++count) {
        for (unsigned int &word : output) word = 0xdeadbeefu;
        stop_at_input = false;
        sub_5d439f(output, packed, count, 4);
        for (int i = 0; i < 12; ++i) {
            unsigned int expected = 0xdeadbeefu;
            if (i < count) expected = (packed[i / 2] >> ((i & 1) * 4)) & 15;
            assert(output[i] == expected);
        }
    }
    // Native 5D43C2 subtracts four, then JL tests that wrapped signed result.
    // Stop at the first actual input copy: these counts begin a huge decode,
    // rather than returning as a normalized positive-count guard would.
    for (int count : {INT_MIN, INT_MIN + 1, INT_MIN + 2, INT_MIN + 3}) {
        output[0] = 0xdeadbeefu;
        stop_at_input = true;
        bool read = false;
        try { sub_5d439f(output, packed, count, 4); }
        catch (InputRead &) { read = true; }
        assert(read);
        assert(output[0] == 0xdeadbeefu);
    }
    // INT_MIN+4 leaves the decremented native counter negative and returns.
    sub_5d439f(output, packed, INT_MIN + 4, 4);
}
'''


def test_actual_index_decoder_preserves_wrapped_count(tmp_path: Path) -> None:
    _compile_run(tmp_path, _unit(GENERAL.read_text()), "index_counter_wrap")


@pytest.mark.parametrize(
    ("old", "new", "name"),
    [
        (
            "unsigned int bitpos;",
            "if (word_count <= 0) return;\n    unsigned int bitpos;",
            "normalized_negative_guard",
        ),
        ("remaining_bits += 3u;", "remaining_bits += 4u;", "extra_tail_sample"),
    ],
)
def test_counter_control_mutants_are_detected(
    tmp_path: Path, old: str, new: str, name: str,
) -> None:
    source = GENERAL.read_text()
    assert source.count(old) == 1
    with pytest.raises(subprocess.CalledProcessError):
        _compile_run(tmp_path, _unit(source.replace(old, new, 1)), name)

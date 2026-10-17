"""Actual-source controls for the typed motion-index words and row copies."""
from __future__ import annotations

from dataclasses import fields
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text

from test_movie_runtime_decoder_ownership_actual_source_oct9 import (
    _compile_run,
    _definition,
)


ROOT = Path(__file__).resolve().parents[2]
GENERAL = ROOT / "src/general.cpp"
HEADER = ROOT / "src/movie_media.h"
BASE = "5e5dd149b8146859c6854e4505f6db4b520adee1"


def _unit(general: str, main: str) -> str:
    functions = "\n\n".join(
        _definition(general, signature)
        for signature in (
            "static unsigned int media_rotate_right_32(",
            'extern "C" void __cdecl sub_5d439f(',
            'extern "C" void __cdecl sub_5d4448(',
        )
    )
    pixel_block = _definition(HEADER.read_text(), "struct MediaDecodedPixelBlock4x4") + ";"
    return r'''#include <cassert>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#define __cdecl
typedef unsigned char BYTE;
''' + pixel_block + "\n" + functions + "\n" + main


MAIN = r'''int main() {
  // Producer keeps the x86 rotate/mask width behavior, including width 0 and
  // width 32 both selecting a zero mask.
  const unsigned int packed = 0xfedcba98u;
  unsigned int produced[4] = {0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd};
  sub_5d439f(produced, reinterpret_cast<const BYTE *>(&packed), 1, 0);
  assert(produced[0] == 0u);
  produced[0] = 0;
  sub_5d439f(produced, reinterpret_cast<const BYTE *>(&packed), 1, 31);
  assert(produced[0] == (packed & 0x7fffffffu));
  produced[0] = 0xffffffffu;
  sub_5d439f(produced, reinterpret_cast<const BYTE *>(&packed), 1, 32);
  assert(produced[0] == 0u);

  // Two groups at pitch 12 each contain one previous-frame block followed by
  // one decoded block.  The second group's address preserves the flat formula.
  unsigned int mixed_indices[4] = {0u, 1u, 0u, 1u};
  int motion_offsets[1] = {0};
  MediaDecodedPixelBlock4x4 blocks[2] = {};
  for (int i = 0; i < 16; ++i)
    blocks[0].pixels[i] = static_cast<BYTE>(0xa0 + i);
  BYTE previous[128], current[128] = {};
  for (int i = 0; i < 128; ++i) previous[i] = static_cast<BYTE>(i + 1);
  sub_5d4448(mixed_indices, previous, current, 1, motion_offsets,
             blocks, 12, 2, 2);
  for (int group = 0; group < 2; ++group) {
    const int group_start = group == 0 ? 0 : 44;
    for (int row = 0; row < 4; ++row) {
      assert(memcmp(current + group_start + row * 12,
                    previous + group_start + row * 12, 4) == 0);
      assert(memcmp(current + group_start + 4 + row * 12,
                    blocks[0].pixels + row * 4, 4) == 0);
    }
  }

  // A negative raw unsigned word is interpreted as signed -1 and indexes a
  // valid prefixed motion-offset table entry, not the decoded fallback.
  unsigned int negative_index[1] = {0xffffffffu};
  int prefixed_offsets[2] = {0, 0};
  BYTE negative_previous[16], negative_current[16] = {};
  for (int i = 0; i < 16; ++i)
    negative_previous[i] = static_cast<BYTE>(0x30 + i);
  sub_5d4448(negative_index, negative_previous, negative_current, 1,
             prefixed_offsets + 1, blocks, 4, 1, 1);
  assert(memcmp(negative_current, negative_previous, 16) == 0);

  // Exercise the second raw-word load after a first motion copy.  The second
  // negative word remains a signed motion index in the inner run condition.
  unsigned int second_negative[2] = {0u, 0xffffffffu};
  BYTE second_previous[64], second_current[64] = {};
  for (int i = 0; i < 64; ++i) second_previous[i] = static_cast<BYTE>(0x50 + i);
  sub_5d4448(second_negative, second_previous, second_current, 1,
             prefixed_offsets + 1, blocks, 8, 2, 1);
  for (int row = 0; row < 4; ++row) {
    assert(memcmp(second_current + row * 8,
                  second_previous + row * 8, 4) == 0);
    assert(memcmp(second_current + 4 + row * 8,
                  second_previous + 4 + row * 8, 4) == 0);
  }

  // do/while behavior intentionally processes one block for zero/negative
  // run counts and zero/negative height groups.
  unsigned int one_fallback[1] = {0u};
  for (int run_count : {0, -1}) {
    for (int groups : {0, -1}) {
      BYTE output[32] = {};
      sub_5d4448(one_fallback, negative_previous, output, 0,
                 motion_offsets, blocks, 4, run_count, groups);
      assert(memcmp(output, blocks[0].pixels, 16) == 0);
    }
  }

  // Four unaligned fallback loads/stores are sequential.  The first store
  // changes bytes read by the second row, etc.; staging all 16 bytes differs.
  MediaDecodedPixelBlock4x4 overlap = {};
  for (int i = 0; i < 16; ++i) overlap.pixels[i] = static_cast<BYTE>(i + 1);
  sub_5d4448(one_fallback, overlap.pixels + 1, overlap.pixels + 1, 0,
             motion_offsets, &overlap, 1, 1, 1);
  const BYTE ordered_overlap[8] = {1, 1, 4, 9, 13, 14, 15, 16};
  assert(memcmp(overlap.pixels, ordered_overlap, sizeof(ordered_overlap)) == 0);

  // Cross-row aliasing on the previous-frame path also observes each prior
  // row's store before the next row is loaded.
  BYTE alias_arena[40];
  for (int i = 0; i < 40; ++i) alias_arena[i] = static_cast<BYTE>(i + 1);
  unsigned int previous_index[1] = {0u};
  sub_5d4448(previous_index, alias_arena + 4, alias_arena + 8, 1,
             motion_offsets, blocks, 4, 1, 1);
  for (int row = 0; row < 4; ++row)
    assert(memcmp(alias_arena + 8 + row * 4, alias_arena + 4, 4) == 0);
}
'''


def test_actual_motion_index_word_and_ordered_copy_controls(tmp_path: Path) -> None:
    _compile_run(tmp_path, _unit(GENERAL.read_text(), MAIN), "motion_indices")


def test_motion_index_migration_preserves_native_identities_and_lessons() -> None:
    for relative in ("src/general.cpp", "src/movie_media.h", "src/movie_media.cpp"):
        path = ROOT / relative
        old = subprocess.check_output(
            ["git", "show", BASE + ":" + relative], cwd=ROOT, text=True
        )
        current = path.read_text()
        if relative == "src/general.cpp":
            from test_general_owner_split_oct17 import inverse_split, LIVE_PATHS
            current = inverse_split({key: owner.read_bytes() for key, owner in LIVE_PATHS.items()})["general"].decode()
        before, after = read_text(old, path), read_text(current, path)
        assert len(before) == len(after)
        for prior, current in zip(before, after):
            for field in fields(prior):
                if field.name not in {"line", "region", "origin"}:
                    assert getattr(prior, field.name) == getattr(current, field.name)
            lessons = lambda record: [
                (item.kind, item.key, item.prose, tuple(text for _, text in item.lines))
                for item in record.origin
            ]
            assert lessons(prior) == lessons(current)


@pytest.mark.parametrize(
    ("old", "new", "name"),
    [
        (
            "if (idx < run_limit)",
            "if (static_cast<unsigned int>(idx) < "
            "static_cast<unsigned int>(run_limit))",
            "unsigned_compare",
        ),
        (
            "} while (idx < run_limit);",
            "} while (static_cast<unsigned int>(idx) < "
            "static_cast<unsigned int>(run_limit));",
            "unsigned_second_index_compare",
        ),
        (
            "const MediaDecodedPixelBlock4x4 *block =\n"
            "                    reinterpret_cast<const MediaDecodedPixelBlock4x4 *>(entry2);",
            "const MediaDecodedPixelBlock4x4 *block =\n"
            "                    reinterpret_cast<const MediaDecodedPixelBlock4x4 *>(entry2);\n"
            "                const MediaDecodedPixelBlock4x4 staged = *block;\n"
            "                block = &staged;",
            "whole_block_staging",
        ),
        (
            "source_row = block->pixels + 8;\n"
            "                memcpy(&row_word, source_row, sizeof(row_word));\n"
            "                memcpy(dst + stride * 2u, &row_word, sizeof(row_word));",
            "source_row = block->pixels + 8;\n"
            "                memcpy(&row_word, source_row, sizeof(row_word));\n"
            "                memcpy(dst + stride, &row_word, sizeof(row_word));",
            "wrong_row_stride",
        ),
    ],
)
def test_mutated_motion_copy_controls_are_detected(
    tmp_path: Path, old: str, new: str, name: str,
) -> None:
    source = GENERAL.read_text()
    assert source.count(old) == 1
    mutated = source.replace(old, new, 1)
    with pytest.raises(subprocess.CalledProcessError):
        _compile_run(tmp_path, _unit(mutated, MAIN), name)

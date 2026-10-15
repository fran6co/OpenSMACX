"""Actual-body controls for the two ADPCM decoders.

This host projection retains the source decoder bodies and real table
initializer. The older decoder's recovered ABI passes pointers as 32-bit ints;
the harness widens only those two parameters to host pointers so tests can
execute on a 64-bit host. This is not signature/ABI evidence or native-code
verification.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/general.cpp").read_text()
HEADER = (ROOT / "src/general.h").read_text()


def body(signature):
    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start)
    depth = 0
    for end in range(opening, len(SOURCE)):
        depth += (SOURCE[end] == "{") - (SOURCE[end] == "}")
        if depth == 0:
            return SOURCE[start:end + 1]
    raise AssertionError("unterminated " + signature)


def projection(damage):
    struct_start = HEADER.index("struct MediaAdpcmStorage {")
    struct_end = HEADER.index("};", struct_start) + 2
    storage_start = SOURCE.index("#define MEDIA_ADPCM_MAG")
    storage_end = SOURCE.index("#undef MEDIA_ADPCM_MAG", storage_start) + len("#undef MEDIA_ADPCM_MAG")
    helper = body("static int media_adpcm_delta(")
    decode_one = body("void __cdecl sub_5ce640(")
    decode_stereo = body("void __cdecl sub_5ce6e3(")

    # The source ABI intentionally carries pointer parameters as int in this
    # first helper. Widen just the host projection's pointer inputs; retain the
    # function body and separately assert the production declaration remains.
    decode_one = decode_one.replace(
        "void __cdecl sub_5ce640(int first_sample, int codes, int samples,\n                        int sample_total, int *predictor, int *step_index)",
        "void __cdecl sub_5ce640(int first_sample, unsigned char *codes, short *samples,\n                        int sample_total, int *predictor, int *step_index)",
    )
    assert "unsigned char *codes, short *samples" in decode_one

    if damage == "old_decoder_nibble_order":
        changed = decode_one.replace("nibble >>= 4;", "nibble &= 0xF;", 1)
        assert changed != decode_one
        decode_one = changed
    elif damage == "stereo_left_nibble_order":
        changed = decode_stereo.replace(
            "media_adpcm_delta(MediaAdpcmTables.step_index_left, bVar1 >> 4)",
            "media_adpcm_delta(MediaAdpcmTables.step_index_left, bVar1 & 0x0f)",
            1,
        )
        assert changed != decode_stereo
        decode_stereo = changed
    elif damage == "stereo_adjust_index":
        changed = decode_stereo.replace(
            "MediaAdpcmTables.index_adjust[(bVar1 >> 4) & 7]",
            "MediaAdpcmTables.index_adjust[(bVar1 >> 4) & 3]",
            1,
        )
        assert changed != decode_stereo
        decode_stereo = changed
    elif damage == "stereo_upper_clamp":
        changed = decode_stereo.replace(
            "MediaAdpcmTables.step_index_left = 0x1600;",
            "MediaAdpcmTables.step_index_left = 0x15ff;",
            1,
        )
        assert changed != decode_stereo
        decode_stereo = changed

    elif damage == "stereo_skip_nonpositive_count":
        changed = decode_stereo.replace(
            "int iVar2 = 0;",
            "if (sample_total <= 0) return;\n    int iVar2 = 0;", 1,
        )
        assert changed != decode_stereo
        decode_stereo = changed

    return r'''#include <cassert>
#include <stdint.h>
#include <string.h>
#define __cdecl
STRUCT
extern MediaAdpcmStorage MediaAdpcmTables;
STORAGE
HELPER
DECODE_ONE
DECODE_STEREO

int main() {
  // Three samples are intentional: the first decoder alternates high/low
  // nibbles and must leave the odd final high nibble in place.
  unsigned char packed_codes[3] = {0x7f, 0x83, 0x00};
  short mono[4] = {111, 222, 333, 444};
  int predictor = 0, step = 0;
  sub_5ce640(0, packed_codes, mono, 3, &predictor, &step);
  assert(mono[0] == 11 && mono[1] == -19 && mono[2] == -23);
  assert(mono[3] == 444 && predictor == -23 && step == 960);

  // Stereo state is initialized from packed signed halves, updated in the
  // real four-word global state, and packed back without touching neighbors.
  unsigned char stereo_codes[1] = {0x7f};
  short stereo[2] = {0, 0};
  int predictors[2] = {static_cast<int>(0xfffc0005u), 0x13572468};
  int indices[2] = {0, 0x24681357};
  MediaAdpcmTables.predictor_left = 0x12345678;
  MediaAdpcmTables.predictor_right = 0x23456789;
  MediaAdpcmTables.step_index_left = 0x3456789a;
  MediaAdpcmTables.step_index_right = 0x456789ab;
  const int adjust0 = MediaAdpcmTables.index_adjust[0];
  const int adjust7 = MediaAdpcmTables.index_adjust[7];
  const int neighbor = MediaAdpcmTables.lookup_and_delta_words[0];
  sub_5ce6e3(stereo_codes, stereo, 1, predictors, indices);
  assert(stereo[0] == 16 && stereo[1] == -15);
  assert(MediaAdpcmTables.predictor_left == 16);
  assert(MediaAdpcmTables.predictor_right == -15);
  assert(MediaAdpcmTables.step_index_left == 512);
  assert(MediaAdpcmTables.step_index_right == 512);
  assert((uint32_t)predictors[0] == 0xfff10010u);
  assert((uint32_t)indices[0] == 0x02000200u);
  assert(predictors[1] == 0x13572468 && indices[1] == 0x24681357);
  assert(MediaAdpcmTables.index_adjust[0] == adjust0);
  assert(MediaAdpcmTables.index_adjust[7] == adjust7);
  assert(MediaAdpcmTables.lookup_and_delta_words[0] == neighbor);

  // Upper predictor and step clamps retain their original inclusive values.
  unsigned char clamp_codes[1] = {0x77};
  short clamped[2] = {0, 0};
  int clamp_predictors[1] = {static_cast<int>(0x7fff7ff8u)};
  int clamp_indices[1] = {static_cast<int>(0x16001600u)};
  sub_5ce6e3(clamp_codes, clamped, 1, clamp_predictors, clamp_indices);
  assert(clamped[0] == 32767 && clamped[1] == 32767);
  assert((uint32_t)clamp_predictors[0] == 0x7fff7fffu);
  assert((uint32_t)clamp_indices[0] == 0x16001600u);

  // A zero-nibble sample decreases both step offsets and exercises the
  // original lower clamp without adding input validation.
  unsigned char lower_codes[1] = {0x00};
  short lower[2] = {1, 1};
  int lower_predictors[1] = {0};
  int lower_indices[1] = {0};
  sub_5ce6e3(lower_codes, lower, 1, lower_predictors, lower_indices);
  assert(lower[0] == 0 && lower[1] == 0);
  assert((uint32_t)lower_indices[0] == 0u);

  // Both original loops execute once even for nonpositive counts. Supply
  // valid storage so this checks the quirk without an invalid memory access.
  for (int requested = -1; requested <= 0; ++requested) {
    unsigned char once_codes[1] = {0x7f};
    short once_mono[2] = {123, 456};
    int once_predictor = 0, once_step = 0;
    sub_5ce640(0, once_codes, once_mono, requested,
              &once_predictor, &once_step);
    assert(once_mono[0] == 11 && once_mono[1] == 456);
    assert(once_predictor == 11 && once_step == 512);
    short once_stereo[3] = {123, 456, 789};
    int once_predictors = 0, once_indices = 0;
    sub_5ce6e3(once_codes, once_stereo, requested,
              &once_predictors, &once_indices);
    assert(once_stereo[0] == 11 && once_stereo[1] == -11);
    assert(once_stereo[2] == 789);
    assert((uint32_t)once_predictors == 0xfff5000bu);
    assert((uint32_t)once_indices == 0x02000200u);
    assert(MediaAdpcmTables.predictor_left == 11);
    assert(MediaAdpcmTables.predictor_right == -11);
    assert(MediaAdpcmTables.step_index_left == 512);
    assert(MediaAdpcmTables.step_index_right == 512);
  }
}
'''.replace("STRUCT", HEADER[struct_start:struct_end]) \
     .replace("STORAGE", SOURCE[storage_start:storage_end]) \
     .replace("HELPER", helper) \
     .replace("DECODE_ONE", decode_one) \
     .replace("DECODE_STEREO", decode_stereo)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "old_decoder_nibble_order",
                                     "stereo_left_nibble_order",
                                     "stereo_adjust_index", "stereo_upper_clamp",
                                     "stereo_skip_nonpositive_count"])
def test_actual_decoder_bodies_preserve_state_and_packed_outputs(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source, executable = tmp_path / "adpcm.cpp", tmp_path / "adpcm"
    source.write_text(projection(damage))
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                               str(source), "-o", str(executable)],
                              capture_output=True, text=True, timeout=45)
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    assert (run.returncode == 0) == (damage is None), run.stderr


def test_direct_state_members_replace_all_five_pointer_aliases():
    for name in ("AdpcmIndexAdjust", "AdpcmPredictorLeft", "AdpcmPredictorRight",
                 "AdpcmStepIndexLeft", "AdpcmStepIndexRight"):
        assert "*const " + name + " =" not in SOURCE
    assert "void __cdecl sub_5ce640(int first_sample, int codes, int samples," in SOURCE
    assert "void __cdecl sub_5ce6e3(unsigned char *codes, short *samples," in SOURCE

"""Bounded actual-source stereo state ownership, not full codec acceptance."""
from pathlib import Path
import json
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
BASE = "50480d8ad"
FIELDS_BEFORE = """  DWORD decoder_state_3e8;                       // +3e8; 5CE6E3 input/output
  DWORD decoder_state_3ec;                       // +3ec; 5CE6E3 input/output
  DWORD decoder_state_3f0;                       // +3f0; 5CE6E3 input/output
  DWORD decoder_state_3f4;                       // +3f4; 5CE6E3 input/output"""
FIELDS_AFTER = """  // Native5CA3CE/5CA3D4 pass these two words directly to5CE6E3.
  // Each packs signed 16-bit left/right state; adjacent words are untouched.
  int decoder_state_3e8;                         // +3e8; packed predictors
  DWORD decoder_state_3ec;                       // +3ec; uninterpreted neighboring storage
  int decoder_state_3f0;                         // +3f0; packed step byte offsets
  DWORD decoder_state_3f4;                       // +3f4; uninterpreted neighboring storage"""
CALL_BEFORE = """    int predictors[2] = {(int)r->decoder_state_3e8, (int)r->decoder_state_3ec};
    int indices[2] = {(int)r->decoder_state_3f0, (int)r->decoder_state_3f4};
    sub_5ce6e3(input + 0x14, (short *)r->decoded_ring_current, byte_count,
               predictors, indices);
    r->decoder_state_3e8 = predictors[0]; r->decoder_state_3ec = predictors[1];
    r->decoder_state_3f0 = indices[0]; r->decoder_state_3f4 = indices[1];"""
CALL_AFTER = """    sub_5ce6e3(input + 0x14, (short *)r->decoded_ring_current, byte_count,
               &r->decoder_state_3e8, &r->decoder_state_3f0);"""


def restore_stereo_state(text, relative):
    """Project exactly this migration out of earlier immutable controls."""
    before, after = (FIELDS_BEFORE, FIELDS_AFTER) if relative.endswith(".h") else (CALL_BEFORE, CALL_AFTER)
    if after in text:
        assert text.count(after) == 1
        return text.replace(after, before, 1)
    return text


def stereo_branch(source):
    start = source.index("  } else if (r->format_dispatch_60 == 0x0a) {")
    end = source.index("\n  } else {", start)
    return source[source.index("\n", start) + 1:end]


def test_original_source_identity_continuity_and_bounded_edit():
    from match_recovery.source import read_text
    from recovery.tests.test_texture_raster_actual_source import _continuity_fields
    for name in ("movie_media.cpp", "movie_media.h"):
        path = ROOT / "src" / name
        before = subprocess.check_output(["git", "show", BASE + ":src/" + name], text=True)
        after = path.read_text()
        assert restore_stereo_state(after, "src/" + name) == before
        assert [(_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(before, path)] == [
            (_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(after, path)]
        if name.endswith(".cpp"):
            old = stereo_branch(before)
            new = stereo_branch(after)
            assert before.replace(old, new) == after
            assert "int predictors[2]" not in new
            assert "&r->decoder_state_3e8, &r->decoder_state_3f0" in new


def test_native_task_scope_and_committed_baseline():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/media-stereo-state-owner-oct11.json").read_text()))
    assert task.base_revision == subprocess.check_output(["git", "rev-parse", BASE], text=True).strip()
    assert [s.address for s in task.subjects] == [0x5CA310]
    assert set(task.editable_files) == {"src/movie_media.cpp", "src/movie_media.h",
                                      "recovery/tests/test_media_stereo_state_owner_oct11.py",
                                      "recovery/tests/test_media_motion_storage_oct11.py"}


@pytest.mark.parametrize("damage", (None, "copy_vectors", "swap_words"))
def test_actual_caller_passes_owner_words_without_neighbor_writes(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source = (ROOT / "src/movie_media.cpp").read_text()
    branch = stereo_branch(source)
    if damage == "copy_vectors":
        old = subprocess.check_output(["git", "show", BASE + ":src/movie_media.cpp"], text=True)
        branch = stereo_branch(old)
    elif damage == "swap_words":
        branch = branch.replace("&r->decoder_state_3e8, &r->decoder_state_3f0",
                                "&r->decoder_state_3f0, &r->decoder_state_3e8")
    header = (ROOT / "src/movie_media.h").read_text()
    start = header.index("  int decoder_state_3e8;")
    end = header.index("\n  MediaDecoderContext", start)
    fields = header[start:end]
    # This deliberately projects only the complete stereo branch and its four
    # actual declarations. It is not an execution test of other dispatch arms.
    code = r"""
#include <cstring>
#include <cstddef>
typedef unsigned int DWORD;
struct State { char prefix[0x3e8];
/* FIELDS */
};
struct Runtime : State { unsigned char *decoded_ring_base, *decoded_ring_current; };
Runtime *expected;
bool valid = true;
void sub_5ce6e3(unsigned char *, short *, int, int *predictors, int *indices) {
  if (predictors != &expected->decoder_state_3e8 || indices != &expected->decoder_state_3f0) {
    valid = false; return;
  }
  if (static_cast<DWORD>(*predictors) != 0x80007fffu || *indices != 0x16000040) valid = false;
  *predictors = -65535; *indices = 0x00401600;
}
void actual(Runtime *r, unsigned char *input, int byte_count) {
  int output_bytes;
  /* BRANCH */
  if (byte_count != 8) valid = false;
}
int main() {
  if (sizeof(int) != 4 || offsetof(State,decoder_state_3e8) != 0x3e8 ||
      offsetof(State,decoder_state_3ec) != 0x3ec || offsetof(State,decoder_state_3f0) != 0x3f0 ||
      offsetof(State,decoder_state_3f4) != 0x3f4) return 2;
  unsigned char ring[300000] = {}, input[32] = {};
  Runtime r; std::memset(&r, 0, sizeof(r)); expected = &r;
  DWORD bits = 0x80007fffu; std::memcpy(&r.decoder_state_3e8, &bits, 4);
  r.decoder_state_3ec = 0x12345678; r.decoder_state_3f0 = 0x16000040;
  r.decoder_state_3f4 = 0x87654321;
  r.decoded_ring_base = ring; r.decoded_ring_current = ring + 299996;
  actual(&r, input, 2);
  if (r.decoded_ring_current != ring || r.decoder_state_3e8 != -65535 ||
      r.decoder_state_3f0 != 0x00401600 || r.decoder_state_3ec != 0x12345678 ||
      r.decoder_state_3f4 != 0x87654321) valid = false;
  return valid ? 0 : 1;
}
""".replace("/* FIELDS */", fields).replace("/* BRANCH */", branch)
    unit = tmp_path / "stereo.cpp"
    unit.write_text(code)
    exe = tmp_path / "stereo"
    subprocess.run([compiler, "-std=c++98", str(unit), "-o", str(exe)], check=True)
    result = subprocess.run([str(exe)])
    assert (result.returncode == 0) == (damage is None)


def test_pinned_direct_member_addresses_and_single_word_decoder_reads_and_writes():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    image = PeImage((ROOT / ".opensmacx/game/terranx_original.exe").read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for start, end in ((0x5CA3CE, 0x5CA3EB), (0x5CE6E3, 0x5CE719), (0x5CE7DF, 0x5CE80D)):
        instructions.update({i.address: (i.mnemonic, i.op_str)
                             for i in decoder.disasm(image.read(Span(start, end)), start)})
    for address, expected in {
        0x5CA3CE: ("lea", "eax, [esi + 0x3f0]"),
        0x5CA3D4: ("lea", "ecx, [esi + 0x3e8]"),
        0x5CA3DA: ("push", "eax"),
        0x5CA3DE: ("push", "ecx"),
        0x5CA3E6: ("call", "0x5ce6e3"),
        0x5CE6E9: ("mov", "edx, dword ptr [ebp + 0x18]"),
        0x5CE6EC: ("mov", "edx, dword ptr [edx]"),
        0x5CE701: ("mov", "edx, dword ptr [ebp + 0x14]"),
        0x5CE704: ("mov", "edx, dword ptr [edx]"),
        0x5CE7F1: ("mov", "ecx, dword ptr [ebp + 0x14]"),
        0x5CE7F4: ("mov", "dword ptr [ecx], edx"),
        0x5CE808: ("mov", "ecx, dword ptr [ebp + 0x18]"),
        0x5CE80B: ("mov", "dword ptr [ecx], edx"),
    }.items():
        assert instructions[address] == expected

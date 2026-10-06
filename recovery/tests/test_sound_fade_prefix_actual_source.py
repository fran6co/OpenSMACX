from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest

from test_sprite_list_lifecycle_actual_source import extract_definition, repo_root


ROOT = repo_root()


def harness(function: str) -> str:
    return r'''#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <new>
typedef unsigned int uint32_t;
class SoundFadeState {
public:
    SoundFadeState();
    uint32_t root_table_slot_; // host fixture for the original 32-bit vptr
    uint32_t volume_;
    int pan_8_;
    uint32_t state_C_[9];
    uint32_t loop_flag_30_;
    uint32_t delay_;
    uint32_t fade_38_;
};
typedef char fixture_extent[(sizeof(SoundFadeState) == 0x3c) ? 1 : -1];
typedef char volume_offset[(offsetof(SoundFadeState, volume_) == 4) ? 1 : -1];
typedef char untouched_delay_offset[(offsetof(SoundFadeState, delay_) == 0x34) ? 1 : -1];
typedef char untouched_fade_offset[(offsetof(SoundFadeState, fade_38_) == 0x38) ? 1 : -1];
''' + function + r'''
int main() {
    union Storage { unsigned int alignment; unsigned char bytes[0x3c]; } storage;
    memset(storage.bytes, 0xA5, sizeof(storage.bytes));
    SoundFadeState *state = new (storage.bytes) SoundFadeState();
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(state);
    for (int slot = 0; slot != 4; ++slot) assert(bytes[slot] == 0xA5);
    assert(state->volume_ == 0x7Fu);
    assert(state->pan_8_ == 0);
    for (int i = 0; i != 9; ++i) assert(state->state_C_[i] == 0);
    assert(state->loop_flag_30_ == 0);
    // Inspect representations, not indeterminate typed field values.
    for (int tail = 0x34; tail != 0x3C; ++tail) assert(bytes[tail] == 0xA5);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_prefix_constructor_and_untouched_tail(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/sound.cpp").read_text()
    assert "  SoundFadeState();" in (ROOT / "src/sound.h").read_text()
    record_body = extract_definition(source, "SoundFadeState::SoundFadeState()")
    assert "// ORIGINAL: 0x004C8410 ??0Patch@@QAE@XZ" in source
    assert not (ROOT / "src/recovered/units/004c8410.cpp").exists()

    mutant = record_body.replace("loop_flag_30_ = 0;", "loop_flag_30_ = 1;")
    assert mutant != record_body
    for label, body in (("positive", record_body), ("mutant", mutant)):
        cpp = tmp_path / f"{label}.cpp"
        exe = tmp_path / label
        cpp.write_text(harness(body))
        built = subprocess.run(
            [compiler, "-std=c++98", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        if label == "positive":
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "loop-field mutant escaped the field oracle"

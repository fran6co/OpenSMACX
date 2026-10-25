"""Actual-source host controls for the Caviar render-record release order.

The host fixture observes release calls and defined callback mutations; it is
not a native ABI, allocator, or runtime proof.
"""

from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]

from test_caviar_descriptor_ramps_overlay_oct9 import extract


def actual_release(source=None):
    if source is None:
        source = (ROOT / "src/caviar_record.cpp").read_text()
    return extract(source, "void __cdecl vox_release_record(")


FIXTURE = r'''#include <stdint.h>
#include <stddef.h>
#ifndef __cdecl
#define __cdecl
#endif
struct VoxRenderRecord {
  void *colour_table_;
  void *shadow_table_;
  uint16_t **ramp_a_ptr_;
  int32_t *ramp_b_ptr_;
};
struct CaviarOutputWindow { void (__cdecl *io_release_)(void *); };
static CaviarOutputWindow CaviarOutputMessageWindow;
static uint32_t vox_record_count;
static VoxRenderRecord *g_record;
static void *g_events[16];
static int g_event_count;
static void *g_mutation_trigger;
static void *g_shadow_replacement;
static void *g_switch_trigger;
static int g_mutate_count_from_table;
static uint32_t g_table_count_value;
static int g_mutate_count_on_record;
static uint32_t g_record_count_value;
static int g_replacement_calls;
static char g_tokens[8];

static void __cdecl replacement_release(void *value);
static void __cdecl observe_release(void *value) {
  g_events[g_event_count++] = value;
  if (value == g_mutation_trigger) {
    g_record->shadow_table_ = g_shadow_replacement;
    if (g_mutate_count_from_table) vox_record_count = g_table_count_value;
  }
  if (value == g_record && g_mutate_count_on_record)
    vox_record_count = g_record_count_value;
  if (value == g_switch_trigger)
    CaviarOutputMessageWindow.io_release_ = replacement_release;
}
static void __cdecl replacement_release(void *value) {
  g_events[g_event_count++] = value;
  ++g_replacement_calls;
}
'''


MAIN = r'''
int main() {
  uint16_t ramp_row = 0x1234;
  int32_t ramp_b = 0x12345678;
  VoxRenderRecord record;
  g_record = &record;
  CaviarOutputMessageWindow.io_release_ = observe_release;

  // The first callback changes the next table field and the count. The
  // release body must read both later values after that callback returns.
  record.colour_table_ = &g_tokens[0];
  record.shadow_table_ = &g_tokens[1];
  uint16_t *ramp_a_storage = &ramp_row;
  record.ramp_a_ptr_ = &ramp_a_storage;
  record.ramp_b_ptr_ = &ramp_b;
  void *ramp_a_as_void = static_cast<void *>(record.ramp_a_ptr_);
  g_mutation_trigger = record.colour_table_;
  g_shadow_replacement = &g_tokens[4];
  g_mutate_count_from_table = 1;
  g_table_count_value = 2;
  vox_record_count = 1;
  vox_release_record(&record);
  if (g_event_count != 5 ||
      g_events[0] != &g_tokens[0] || g_events[1] != &g_tokens[4] ||
      g_events[2] != ramp_a_as_void || g_events[3] != &ramp_b ||
      g_events[4] != &record || vox_record_count != 1) return 11;

  // Null table members skip their callbacks. The record callback still runs;
  // the guarded decrement leaves zero at zero and decrements positive counts.
  for (uint32_t initial = 0; initial != 3; ++initial) {
    g_event_count = 0;
    g_mutation_trigger = 0;
    g_mutate_count_from_table = 0;
    g_mutate_count_on_record = 0;
    record.colour_table_ = 0;
    record.shadow_table_ = &g_tokens[5];
    record.ramp_a_ptr_ = 0;
    record.ramp_b_ptr_ = &ramp_b;
    vox_record_count = initial;
    vox_release_record(&record);
    const uint32_t expected_count = initial == 0 ? 0 : initial - 1;
    if (g_event_count != 3 || g_events[0] != &g_tokens[5] ||
        g_events[1] != &ramp_b || g_events[2] != &record ||
        vox_record_count != expected_count) return 12;
  }

  // The record release callback itself runs before the guarded count update.
  g_event_count = 0;
  g_mutate_count_on_record = 1;
  g_record_count_value = 2;
  record.colour_table_ = 0;
  record.shadow_table_ = 0;
  record.ramp_a_ptr_ = 0;
  record.ramp_b_ptr_ = 0;
  vox_record_count = 0;
  vox_release_record(&record);
  if (g_event_count != 1 || g_events[0] != &record ||
      vox_record_count != 1) return 13;

  // Replacing the callback from inside its first call affects every later
  // table and record release; the source does not snapshot the function slot.
  g_event_count = 0;
  g_replacement_calls = 0;
  g_mutation_trigger = 0;
  g_switch_trigger = &g_tokens[0];
  g_mutate_count_from_table = 0;
  g_mutate_count_on_record = 0;
  record.colour_table_ = &g_tokens[0];
  record.shadow_table_ = &g_tokens[1];
  record.ramp_a_ptr_ = &ramp_a_storage;
  record.ramp_b_ptr_ = &ramp_b;
  CaviarOutputMessageWindow.io_release_ = observe_release;
  vox_record_count = 1;
  vox_release_record(&record);
  if (g_event_count != 5 || g_replacement_calls != 4 ||
      g_events[0] != &g_tokens[0] || g_events[1] != &g_tokens[1] ||
      g_events[2] != ramp_a_as_void || g_events[3] != &ramp_b ||
      g_events[4] != &record || vox_record_count != 0) return 14;
  return 0;
}
'''


def program(body):
    return FIXTURE + "\n" + body + "\n" + MAIN


def compile_run(tmp_path, compiler, body, stem):
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(program(body))
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_release_order_live_fields_and_guarded_count(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_run(tmp_path, compiler, actual_release(), "release_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_cached_later_table_field_mutant_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = actual_release()
    anchor = "void __cdecl vox_release_record(VoxRenderRecord *record) {"
    assert body.count(anchor) == 1
    body = body.replace(anchor, anchor + "\n    void *cached_shadow = record->shadow_table_;", 1)
    assert body.count("if (record->shadow_table_ != 0)") == 1
    body = body.replace("if (record->shadow_table_ != 0)",
                        "if (cached_shadow != 0)", 1)
    assert body.count("io_release_(record->shadow_table_)") == 1
    body = body.replace("io_release_(record->shadow_table_)",
                        "io_release_(cached_shadow)", 1)
    result = compile_run(tmp_path, compiler, body, "release_cached_shadow_mutant")
    assert result.returncode == 11, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_count_before_record_callback_mutant_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = actual_release()
    original = """    CaviarOutputMessageWindow.io_release_(record);
    if (vox_record_count != 0) {
        --vox_record_count;
    }"""
    damage = """    if (vox_record_count != 0) {
        --vox_record_count;
    }
    CaviarOutputMessageWindow.io_release_(record);"""
    assert body.count(original) == 1
    body = body.replace(original, damage, 1)
    result = compile_run(tmp_path, compiler, body, "release_early_count_mutant")
    assert result.returncode == 13, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_cached_release_callback_mutant_is_rejected(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = actual_release()
    anchor = "void __cdecl vox_release_record(VoxRenderRecord *record) {"
    assert body.count(anchor) == 1
    body = body.replace(
        anchor,
        anchor + "\n    void (__cdecl *cached_release)(void *) = "
        "CaviarOutputMessageWindow.io_release_;",
        1,
    )
    assert body.count("CaviarOutputMessageWindow.io_release_(") == 5
    body = body.replace("CaviarOutputMessageWindow.io_release_(", "cached_release(")
    result = compile_run(tmp_path, compiler, body, "release_cached_callback_mutant")
    assert result.returncode == 14, result.stdout + result.stderr


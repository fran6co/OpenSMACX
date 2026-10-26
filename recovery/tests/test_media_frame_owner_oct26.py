"""Execute frame setup and surface-loop receiver reloads from actual source."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/movie_media.cpp"
from recovery.tests.test_media_output_buffers_oct11 import HARNESS, SIGNATURE, definition


def replacement_once(text: str, old: str, new: str) -> str:
    count = text.count(old)
    assert count == 1, f"expected one fixture anchor, got {count}: {old}"
    return text.replace(old, new, 1)


HOOKS = r"""
static MediaRuntime *callback_runtime;
static MediaDescriptor *create_after[2];
static MediaDescriptor *query_after[2];
static MediaDescriptor *after_is_lost;
static IDirectDrawSurface2 *create_result[2];
static IDirectDrawSurface2 *query_result[2];
static int global_create_calls, query_calls;
static HRESULT is_lost_result;

HRESULT IDirectDrawSurface2::IsLost() {
  ++lost_calls;
  if (after_is_lost) callback_runtime->descriptor = after_is_lost;
  return is_lost_result;
}
HRESULT IDirectDrawSurface2::Restore() { ++restore_calls; return DD_OK; }
HRESULT IDirectDrawSurface2::QueryInterface(int, void **out) {
  const int index = query_calls++;
  assert(index < 2);
  *out = query_result[index];
  if (query_after[index]) callback_runtime->descriptor = query_after[index];
  return DD_OK;
}
HRESULT IDirectDraw2::CreateSurface(DDSURFACEDESC *, IDirectDrawSurface **out,
                                    void *) {
  const int index = global_create_calls++;
  ++this->create_calls;
  assert(index < 4);
  *out = create_result[index < 2 ? index : 1];
  if (index < 2 && create_after[index])
    callback_runtime->descriptor = create_after[index];
  return DD_OK;
}
"""


MAIN = r"""
static void reset_callbacks(MediaRuntime &runtime) {
  callback_runtime = &runtime;
  for (int i = 0; i != 2; ++i) {
    create_after[i] = 0;
    query_after[i] = 0;
    create_result[i] = 0;
    query_result[i] = 0;
  }
  after_is_lost = 0;
  global_create_calls = query_calls = 0;
  is_lost_result = DD_OK;
}

static void test_create_callback_reloads_owner_before_query_interface() {
  BYTE packet[32] = {};
  packet[0] = packet[2] = 16;
  MediaQueueRecord record = {packet, 2};
  MediaRuntime runtime;
  MediaDescriptor entry, after_first_create;
  IDirectDraw2 first_device, second_device;
  IDirectDrawSurface2 first_created, second_created, first_interface,
      second_interface;
  entry.flags = 0x400;
  entry.draw_device_10 = &first_device;
  entry.output_width_64 = entry.output_height_60 = 16;
  after_first_create.flags = 0x400;
  after_first_create.draw_device_10 = &second_device;
  runtime.descriptor = &entry;
  runtime.queue_record_kind = 2;
  reset_callbacks(runtime);
  create_result[0] = &first_created;
  create_result[1] = &second_created;
  create_after[0] = &after_first_create;
  query_result[0] = &first_interface;
  query_result[1] = &second_interface;

  sub_5cc940(&runtime, &record);

  assert(entry.surface_1c == 0 && entry.surface_20 == 0);
  assert(after_first_create.surface_1c == &first_interface);
  assert(after_first_create.surface_20 == &second_interface);
  assert(global_create_calls == 2 && query_calls == 2);
  assert(first_device.create_calls == 1);
  assert(second_device.create_calls == 1);
}

static void test_query_interface_callback_reloads_next_owner() {
  BYTE packet[32] = {};
  packet[0] = packet[2] = 16;
  MediaQueueRecord record = {packet, 2};
  MediaRuntime runtime;
  MediaDescriptor entry, after_query;
  IDirectDraw2 first_device, second_device;
  IDirectDrawSurface2 first_created, second_created, first_interface,
      second_interface;
  entry.flags = 0x400;
  entry.draw_device_10 = &first_device;
  entry.output_width_64 = entry.output_height_60 = 16;
  after_query.flags = 0x400;
  after_query.draw_device_10 = &second_device;
  runtime.descriptor = &entry;
  runtime.queue_record_kind = 2;
  reset_callbacks(runtime);
  create_result[0] = &first_created;
  create_result[1] = &second_created;
  query_result[0] = &first_interface;
  query_result[1] = &second_interface;
  query_after[0] = &after_query;

  sub_5cc940(&runtime, &record);

  assert(entry.surface_1c == &first_interface);
  assert(entry.surface_20 == 0);
  assert(after_query.surface_20 == &second_interface);
  assert(global_create_calls == 2 && query_calls == 2);
  assert(first_device.create_calls == 1);
  assert(second_device.create_calls == 1);
}

static void test_restore_receiver_reloads_after_islost() {
  BYTE packet[32] = {};
  packet[0] = packet[2] = 16;
  MediaQueueRecord record = {packet, 2};
  MediaRuntime runtime;
  MediaDescriptor before_loss, after_loss;
  IDirectDrawSurface2 lost_surface, current_surface;
  before_loss.flags = 0;
  before_loss.output_width_64 = before_loss.output_height_60 = 16;
  before_loss.surface_1c = &lost_surface;
  after_loss.flags = 0;
  after_loss.output_width_64 = after_loss.output_height_60 = 16;
  after_loss.surface_1c = &current_surface;
  runtime.descriptor = &before_loss;
  runtime.queue_record_kind = 2;
  runtime.owned_heap_638 = 0;
  reset_callbacks(runtime);
  is_lost_result = DDERR_SURFACELOST;
  after_is_lost = &after_loss;

  sub_5cc940(&runtime, &record);

  assert(lost_surface.lost_calls == 1);
  assert(lost_surface.restore_calls == 0);
  assert(current_surface.restore_calls == 1);
  assert(runtime.descriptor == &after_loss);
}

int main(int argc, char **argv) {
  assert(argc == 2);
  if (std::strcmp(argv[1], "create") == 0)
    test_create_callback_reloads_owner_before_query_interface();
  else if (std::strcmp(argv[1], "query") == 0)
    test_query_interface_callback_reloads_next_owner();
  else if (std::strcmp(argv[1], "restore") == 0)
    test_restore_receiver_reloads_after_islost();
  else
    assert(!"unknown case");
  return 0;
}
"""


def projected_source(body: str) -> str:
    fixture = HARNESS
    fixture = replacement_once(
        fixture,
        "HRESULT IsLost() { ++lost_calls; return DD_OK; }",
        "HRESULT IsLost();",
    )
    fixture = replacement_once(
        fixture,
        "HRESULT Restore() { ++restore_calls; return DD_OK; }",
        "HRESULT Restore();",
    )
    fixture = replacement_once(
        fixture,
        "HRESULT QueryInterface(int,void**) { assert(false); return -1; }",
        "HRESULT QueryInterface(int,void**);",
    )
    fixture = replacement_once(
        fixture,
        "HRESULT CreateSurface(DDSURFACEDESC*,IDirectDrawSurface**,void*) { assert(false); return -1; }",
        "HRESULT CreateSurface(DDSURFACEDESC*,IDirectDrawSurface**,void*);",
    )
    fixture = replacement_once(
        fixture,
        "struct IDirectDraw2 {\n  HRESULT CreateSurface",
        "struct IDirectDraw2 {\n  int create_calls=0;\n  HRESULT CreateSurface",
    )
    fixture = fixture.replace("ACTUAL_BODY", body)
    fixture = fixture.replace("ACTUAL_SWAP", "void MediaRuntime::sub_5cbbc0() {}")
    fixture = fixture[: fixture.rfind("int main() {")]
    fixture += MAIN
    marker = "static SHORT media_read_signed_word(const BYTE *p)"
    fixture = replacement_once(fixture, marker, HOOKS + "\n" + marker)
    return fixture


def body_for_run() -> str:
    return definition(SOURCE.read_text(), SIGNATURE)


def run_case(tmp_path: Path, body: str, case: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    unit = tmp_path / "movie_media_surface_reload.cpp"
    binary = tmp_path / "movie_media_surface_reload"
    unit.write_text(projected_source(body))
    built = subprocess.run(
        [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", str(unit), "-o", str(binary)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary), case], capture_output=True, text=True)


@pytest.mark.parametrize("case", ["create", "query", "restore"])
def test_actual_source_reloads_surface_owners_at_pinned_boundaries(tmp_path, case):
    result = run_case(tmp_path, body_for_run(), case)
    assert result.returncode == 0, result.stdout + result.stderr


def test_stale_restore_receiver_mutant_is_caught(tmp_path):
    body = body_for_run()
    old = "restore_surface->Restore()"
    assert body.count(old) == 1
    mutant = body.replace(old, "surface->Restore()", 1)
    result = run_case(tmp_path, mutant, "restore")
    assert result.returncode != 0, "restore receiver mutation escaped the host control"
    assert "lost_surface.restore_calls == 0" in result.stderr, result.stderr

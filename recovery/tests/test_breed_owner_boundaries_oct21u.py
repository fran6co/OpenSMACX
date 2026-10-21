from __future__ import annotations

from dataclasses import asdict
from pathlib import Path
import shutil
import subprocess

from match_recovery.source import read_file, read_text
from match_recovery.source.writer import write_body


ROOT = Path(__file__).resolve().parents[2]
BASE_CPP = ROOT / "src/base.cpp"
BASE_H = ROOT / "src/base.h"
GAME_CPP = ROOT / "src/game.cpp"
BASELINE = "33fa72a0ed456dd6946aeb9e90066b8379344acc"
ADDRESS = 0x004E65C0
SIGNATURE = "int __cdecl breed_mod(int base_id, int faction_id)"


def _source_at_revision(path: str) -> str:
    return subprocess.check_output(
        ["git", "show", f"{BASELINE}:{path}"], cwd=ROOT, text=True
    )


def _extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening : end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_body_was_transferred_verbatim_and_owner_is_ordinary_definition():
    old_header = _source_at_revision("src/base.h")
    current_header = BASE_H.read_text()
    current_base = BASE_CPP.read_text()

    original_body = _extract_body(
        old_header, "MEASURED inline int __cdecl breed_mod(int base_id, int faction_id)"
    )
    moved_body = _extract_body(current_base, SIGNATURE)
    assert moved_body == original_body
    assert "MEASURED int __cdecl breed_mod(int base_id, int faction_id);" in current_header
    assert "MEASURED inline int __cdecl breed_mod" not in current_header
    assert current_base.index(SIGNATURE) < current_base.index(
        "int __cdecl worm_mod(int base_id, int faction_id)"
    )


def test_native_body_writer_changes_only_breed_marker_body_path():
    old_cpp = _source_at_revision("src/base.cpp").encode("utf-8")
    old_text = old_cpp.decode("utf-8")
    old_record, = [
        record for record in read_text(old_text, BASE_CPP) if record.address == ADDRESS
    ]
    assert old_record.body == "src/base.h"

    rewritten = write_body(old_cpp, old_record, "src/base.cpp")
    rewritten_records = read_text(rewritten.decode("utf-8"), BASE_CPP)
    moved_record, = [record for record in rewritten_records if record.address == ADDRESS]
    assert moved_record.body == "src/base.cpp"

    old_records = read_text(old_text, BASE_CPP)
    assert len(old_records) == len(rewritten_records)
    for before, after in zip(old_records, rewritten_records):
        left, right = asdict(before), asdict(after)
        for field in ("body", "line", "region", "extract_error"):
            left.pop(field)
            right.pop(field)
        assert left == right

    current_marker, = [record for record in read_file(BASE_CPP) if record.address == ADDRESS]
    assert current_marker.body == "src/base.cpp"
    expected_metadata, actual_metadata = asdict(moved_record), asdict(current_marker)
    for field in ("line", "region", "extract_error"):
        expected_metadata.pop(field)
        actual_metadata.pop(field)
    assert actual_metadata == expected_metadata


def test_repair_phase_uses_direct_helper_and_old_wrapper_is_absent():
    game = GAME_CPP.read_text()
    assert "breed_mod_call" not in game
    repair = _extract_body(game, "void __cdecl repair_phase(int faction_id)")
    assert "if (breed_mod(base_id, Bases[base_id].faction_id_current))" in repair


def _behavior_harness(body: str) -> str:
    return r'''#include <cassert>
typedef unsigned int uint32_t;
enum {
  SP_XENOEMPATYH_DOME = 1, SP_PHOLUS_MUTAGEN = 2, SP_VOICE_OF_PLANET = 3,
  SP_CYBORG_FACTORY = 4,
  FAC_CENTAURI_PRESERVE = 1, FAC_TEMPLE_OF_PLANET = 2,
  FAC_BIOLOGY_LAB = 3, FAC_BIOENHANCEMENT_CENTER = 4
};
static int project_values[5];
static int facility_values[5];
static int calls[16];
static int call_count;
static int expected_base;
static int expected_faction;
static void record(int value) { calls[call_count++] = value; }
static int has_project(int id, int faction_id) {
  assert(faction_id == expected_faction);
  record(100 + id);
  return project_values[id];
}
static int has_fac_built(int id, int base_id) {
  assert(base_id == expected_base);
  record(200 + id);
  return facility_values[id];
}
int breed_mod(int base_id, int faction_id) BODY
static void reset(int base_id, int faction_id) {
  for (int i = 0; i != 5; ++i) { project_values[i] = 0; facility_values[i] = 0; }
  call_count = 0;
  expected_base = base_id;
  expected_faction = faction_id;
}
static void expect_calls(const int *wanted, int count) {
  assert(call_count == count);
  for (int i = 0; i != count; ++i) assert(calls[i] == wanted[i]);
}
int main() {
  reset(17, 4);
  project_values[SP_CYBORG_FACTORY] = 1;
  assert(breed_mod(17, 4) == 1);
  const int fallback[] = {101, 102, 103, 201, 202, 203, 204, 104};
  expect_calls(fallback, 8);

  reset(23, 6);
  project_values[SP_XENOEMPATYH_DOME] = 1;
  project_values[SP_PHOLUS_MUTAGEN] = 1;
  project_values[SP_VOICE_OF_PLANET] = 1;
  project_values[SP_CYBORG_FACTORY] = 1;
  for (int i = 1; i <= 4; ++i) facility_values[i] = 1;
  assert(breed_mod(23, 6) == 7);
  const int fourth_facility_short_circuit[] = {101, 102, 103, 201, 202, 203, 204};
  expect_calls(fourth_facility_short_circuit, 7);
  return 0;
}
'''.replace("BODY", body)


def test_actual_body_host_controls_with_gcc_and_clang(tmp_path):
    body = _extract_body(BASE_CPP.read_text(), SIGNATURE)
    source = tmp_path / "breed_mod_control.cpp"
    source.write_text(_behavior_harness(body))

    for compiler in ("g++", "clang++"):
        executable = shutil.which(compiler)
        assert executable is not None, f"required host compiler is unavailable: {compiler}"
        binary = tmp_path / f"breed_mod_{compiler.replace('+', 'p')}"
        subprocess.run(
            [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
            check=True,
            capture_output=True,
            text=True,
        )
        subprocess.run([str(binary)], check=True, capture_output=True, text=True)


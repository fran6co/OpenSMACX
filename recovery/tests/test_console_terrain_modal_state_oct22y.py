"""Actual-source controls for Console::editor_terrain's modal-state lifetime."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

from match_recovery import binding_source
from match_recovery.source import read_text
from console_sources import console_method_source


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "void Console::editor_terrain()"
BASELINE = "7fb7f072926c248045b4f70eaaf1cf8e3fd456ce"
SOURCE_RELATIVE = "src/console_commands.cpp"


def _definition(text: str) -> str:
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for offset in range(opening, len(text)):
        if text[offset] == "{":
            depth += 1
        elif text[offset] == "}":
            depth -= 1
            if depth == 0:
                return text[start : offset + 1]
    raise AssertionError("unterminated Console::editor_terrain definition")


def _compile_run(tmp_path: Path, definition: str) -> subprocess.CompletedProcess[str]:
    compiler = next((shutil.which(name) for name in ("g++", "clang++") if shutil.which(name)), None)
    if compiler is None:
        raise AssertionError("g++ is required for the actual-source host control")
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "console_terrain.cpp"
    executable = tmp_path / "console_terrain"
    source.write_text(HOST + "\n" + definition + "\n" + MAIN, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        text=True,
        capture_output=True,
        check=False,
        timeout=30,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return subprocess.run(
        [str(executable)], text=True, capture_output=True, check=False, timeout=10
    )


HOST = r"""
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#define nullptr 0

static std::vector<std::string> events;
static int PopupModalState = 0;
static int MapEditorPaintId = 6;
static int popup_result = 4;
static int paint_value = -999;
static int dtor_modal_state = -999;
static int item_count = 0;
static int item_ids[8];
static std::string item_text[8];
static char *item_text_buffers[8];
static int selected_value = -999;
static int start_count = 0;
static int exec_count = 0;
static int paint_count = 0;
static int string_get_count[4];

char PopupStartCaption[] = "POPUP";
char StringTemp[1032];
typedef char string_temp_capacity_must_match[(sizeof(StringTemp) == 1032) ? 1 : -1];
const int MaxTerrainNum = 4;
struct TerrainData { int bit; int name; int name_sea; };
TerrainData Terraforming[4];
char TerrainNames[4][16];
struct Table {
  char *get(int id) {
    assert(id >= 0 && id < 4);
    ++string_get_count[id];
    return TerrainNames[id];
  }
} StringTable;

struct Dialogs {
  void item(char *text, int id) {
    assert(PopupModalState == 6);
    assert(item_count < 8);
    assert(text == StringTemp);
    item_ids[item_count] = id;
    item_text[item_count] = text;
    item_text_buffers[item_count] = text;
    ++item_count;
    events.push_back("item");
  }
};
struct Popup {
  Dialogs list;
  Popup() { events.push_back("popup-ctor"); }
  ~Popup() {
    dtor_modal_state = PopupModalState;
    events.push_back("popup-dtor");
  }
  int start(char *file, const char *label, int value, char *override_text, int flags,
            void *owner) {
    assert(PopupModalState == 6);
    assert(file == PopupStartCaption && std::strcmp(label, "EDITORTERRAIN") == 0);
    assert(value == -1 && override_text == 0 && flags == 0);
    assert(owner == 0);
    ++start_count;
    events.push_back("start");
    return 0;
  }
  void start(char *file, const char *label, int value, char *override_text, int flags) {
    (void)start(file, label, value, override_text, flags, 0);
  }
  Dialogs &dialogs() { return list; }
  void set_value(int value) {
    assert(PopupModalState == 6);
    selected_value = value;
    events.push_back("select");
  }
  int exec(int mode, void *callback) {
    assert(PopupModalState == 6 && mode == 0 && callback == 0);
    ++exec_count;
    events.push_back("exec");
    return popup_result;
  }
};

struct Console {
  void editor_terrain();
  void editor_paints(int paint) {
    assert(PopupModalState == 6);
    paint_value = paint;
    ++paint_count;
    events.push_back("paint");
  }
};

static void reset_fixture() {
  events.clear();
  PopupModalState = 0;
  MapEditorPaintId = 6;
  popup_result = 4;
  paint_value = -999;
  dtor_modal_state = -999;
  item_count = 0;
  selected_value = -999;
  start_count = exec_count = paint_count = 0;
  std::memset(string_get_count, 0, sizeof(string_get_count));
  for (int i = 0; i != 8; ++i) item_text_buffers[i] = 0;
  std::strcpy(StringTemp, "stale buffer contents");
  std::strcpy(TerrainNames[0], "Land");
  std::strcpy(TerrainNames[1], "Sea");
  std::strcpy(TerrainNames[2], "Same");
  std::strcpy(TerrainNames[3], "Unused");
  Terraforming[0].bit = 1; Terraforming[0].name = 0; Terraforming[0].name_sea = 1;
  Terraforming[1].bit = 0; Terraforming[1].name = 2; Terraforming[1].name_sea = 3;
  Terraforming[2].bit = 1; Terraforming[2].name = 2; Terraforming[2].name_sea = 2;
  Terraforming[3].bit = 1; Terraforming[3].name = 3; Terraforming[3].name_sea = 2;
}
"""


MAIN = r"""
int main() {
  Console console;
  reset_fixture();
  MapEditorPaintId = 8;
  console.editor_terrain();
  assert(start_count == 1 && exec_count == 1 && paint_count == 1);
  assert(item_count == 3);
  assert(item_ids[0] == 0 && item_text[0] == "Land/Sea");
  assert(item_ids[1] == 2 && item_text[1] == "Same");
  assert(item_ids[2] == 3 && item_text[2] == "Unused/Same");
  for (int i = 0; i != 3; ++i) assert(item_text_buffers[i] == StringTemp);
  // Per-name totals avoid assuming an order for the two strcmp arguments.
  // Each valid terrain fetches both names for comparison and fetches the sea
  // name again only when the slash form is appended.
  assert(string_get_count[0] == 2 && string_get_count[1] == 2);
  assert(string_get_count[2] == 5 && string_get_count[3] == 2);
  assert(selected_value == 2); // index == MapEditorPaintId - 6
  assert(paint_value == 10);   // popup result + 6
  assert(PopupModalState == 0 && dtor_modal_state == 0);
  assert(events[0] == "popup-ctor" && events[1] == "start");
  assert(events[events.size()-3] == "exec");
  assert(events[events.size()-2] == "paint");
  assert(events[events.size()-1] == "popup-dtor");

  // Negative exec results are forwarded unchanged through the original +6
  // conversion; the modal reset still follows the callback and precedes dtor.
  reset_fixture();
  popup_result = -1;
  MapEditorPaintId = 99;
  console.editor_terrain();
  assert(paint_count == 1 && paint_value == 5);
  assert(selected_value == -999);
  assert(PopupModalState == 0 && dtor_modal_state == 0);
  assert(events[events.size()-3] == "exec");
  assert(events[events.size()-2] == "paint");
  assert(events[events.size()-1] == "popup-dtor");
  return 0;
}
"""


def test_actual_source_resets_modal_state_after_paint_and_before_popup_teardown(tmp_path: Path) -> None:
    source = console_method_source(SIGNATURE)
    definition = _definition(source)
    assert "PopupModalState = 6;" in definition
    assert "editor_paints(result + 6);\n    PopupModalState = 0;" in definition
    result = _compile_run(tmp_path / "actual", definition)
    assert result.returncode == 0, result.stdout + result.stderr


def test_compiling_missing_and_premature_reset_mutants_fail(tmp_path: Path) -> None:
    definition = _definition(console_method_source(SIGNATURE))

    missing, count = definition.replace("    PopupModalState = 0;\n", "", 1), definition.count(
        "    PopupModalState = 0;\n"
    )
    assert count == 1 and missing != definition
    result = _compile_run(tmp_path / "missing-reset", missing)
    assert result.returncode != 0

    ordered = "    editor_paints(result + 6);\n    PopupModalState = 0;"
    premature = definition.replace(
        ordered, "    PopupModalState = 0;\n    editor_paints(result + 6);", 1
    )
    assert premature != definition and ordered in definition
    result = _compile_run(tmp_path / "premature-reset", premature)
    assert result.returncode != 0


def _local_buffer_mutant(definition: str) -> str:
    clear = "        StringTemp[0] = 0;\n"
    assert definition.count(clear) == 1
    mutant = definition.replace(clear, "        char text[256];\n        text[0] = 0;\n", 1)
    mutant = mutant.replace("StringTemp", "text")
    assert "char text[256];" in mutant and "StringTemp" not in mutant
    return mutant


def test_compiling_local_buffer_regression_is_rejected_by_identity_assertion(
    tmp_path: Path,
) -> None:
    definition = _definition(console_method_source(SIGNATURE))
    result = _compile_run(tmp_path / "local-buffer", _local_buffer_mutant(definition))
    assert result.returncode != 0, "local text buffer unexpectedly passed item identity control"
    assert "text == StringTemp" in result.stderr, result.stdout + result.stderr


def test_native_source_reader_preserves_original_identity_claims_lessons_and_binding() -> None:
    prior_text = subprocess.check_output(
        ["git", "show", f"{BASELINE}:{SOURCE_RELATIVE}"],
        cwd=ROOT,
        text=True,
    )
    current_text = (ROOT / SOURCE_RELATIVE).read_text(encoding="utf-8")
    prior_rows = read_text(prior_text, SOURCE_RELATIVE)
    current_rows = read_text(current_text, SOURCE_RELATIVE)
    prior, = (row for row in prior_rows if row.address == 0x004E0B00)
    current, = (row for row in current_rows if row.address == 0x004E0B00)

    # These are source-reader facts; `region` and `line` are navigation
    # metadata expected to move as unrelated source is inserted. Preserve the
    # original identity, mapping, mode, claims, and all prior history.
    for field in (
        "address", "mode", "state", "path", "name", "image_spans",
        "symbol", "body", "context", "kind", "exclusion", "recipe",
        "levers", "ruled_out", "unrecoverable", "deferred", "origin",
    ):
        assert getattr(current, field) == getattr(prior, field), field
    assert not prior.byte_exact or current.byte_exact
    assert not prior.semantic or current.semantic

    prior_binding = binding_source.parse(prior_text, prior_rows)[prior.line]
    current_binding = binding_source.parse(current_text, current_rows)[current.line]
    assert current_binding == prior_binding

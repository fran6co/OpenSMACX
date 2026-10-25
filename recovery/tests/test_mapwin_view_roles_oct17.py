"""Focused controls for the two established MapWin field roles.

Pinned instructions establish the two roles; bounded actual-source game_io
controls preserve transfer order and width. Host record layout does not prove
the native class ABI, full lifetime, original names or runtime/game acceptance.
"""

from hashlib import sha256
from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"

CLEAR_INDEX = bytes.fromhex("89 BE 78 DD 01 00")
CLEAR_ACTIVE_CONDITION = bytes.fromhex("85 FF 0F 94 C1")
CLEAR_ACTIVE = bytes.fromhex("89 8E 74 DD 01 00")
INIT_ACTIVE = bytes.fromhex("C7 86 74 DD 01 00 01 00 00 00")
PLAN_INDEX = bytes.fromhex("89 86 78 DD 01 00")


def _body(text, signature):
    match = re.search(r"(?m)^\s*" + re.escape(signature) + r"\s*\{", text)
    assert match, "missing source body: " + signature
    opening = text.find("{", match.start())
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                i += 1
            elif char == "/" and nxt == "*":
                state = "block"
                i += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[match.start():i + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            i += 1
        elif state in ("string", "char"):
            if char == "\\":
                i += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated source body: " + signature)


def _pinned_image():
    import os

    path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    assert path.is_file(), "pinned game image is required for this evidence control"
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN_SHA256
    return PeImage(raw, PIN_SHA256)


def test_pinned_mapwin_and_planwin_role_witnesses():
    image = _pinned_image()
    assert image.read(Span(0x0046287C, 0x0046287C + len(CLEAR_INDEX))) == CLEAR_INDEX
    assert image.read(Span(0x004628A4, 0x004628A4 + len(CLEAR_ACTIVE_CONDITION))) == CLEAR_ACTIVE_CONDITION
    assert image.read(Span(0x004628AF, 0x004628AF + len(CLEAR_ACTIVE))) == CLEAR_ACTIVE
    assert image.read(Span(0x00470AB1, 0x00470AB1 + len(INIT_ACTIVE))) == INIT_ACTIVE
    assert image.read(Span(0x0048BC79, 0x0048BC79 + len(PLAN_INDEX))) == PLAN_INDEX

    plan_call_clear = image.read(Span(0x0048BC7F, 0x0048BC84))
    assert plan_call_clear[0] == 0xE8
    assert 0x0048BC84 + int.from_bytes(plan_call_clear[1:], "little", signed=True) == 0x00462870
    plan_call_init = image.read(Span(0x0048BC8A, 0x0048BC8F))
    assert plan_call_init[0] == 0xE8
    assert 0x0048BC8F + int.from_bytes(plan_call_init[1:], "little", signed=True) == 0x00470A90

    header = (ROOT / "src/mapwin.h").read_text()
    active = re.search(r"(?m)^\s*uint32_t view_active_;\s*// 0x1DD74\s*$", header)
    index = re.search(r"(?m)^\s*uint32_t map_view_index_;\s*// 0x1DD78\s*$", header)
    assert active and index and active.start() < index.start()
    table_decl = re.search(r"(?m)^static const size_t MapWinTableSlots = 8;", header)
    assert table_decl
    map_source = (ROOT / "src/mapwin.cpp").read_text()
    table_object = re.search(
        r"(?m)^// RECOVERY_OBJECT: v2 0x([0-9A-Fa-f]+) 0x([0-9A-Fa-f]+) "
        r"\?MapWinTable@@3PAPAVMapWin@@A", map_source)
    assert table_object
    plan_source = (ROOT / "src/planwin.cpp").read_text()
    plan_init = _body(plan_source, "void PlanWin::init(char *)")
    assert "map_view_index_ = 1;" in plan_init
    assert plan_init.index("map_view_index_ = 1;") < plan_init.index("MapWin::clear(1);")
    assert plan_init.index("MapWin::clear(1);") < plan_init.index("MapWin::init(3, 0);")
    assert "RECOVERY_OBJECT: v2 0x00834D70" in plan_source
    map_table_start, map_table_end = (int(v, 16) for v in table_object.groups())
    plan_address = 0x00834D70
    assert map_table_end - map_table_start == 8 * 4
    assert not (map_table_start <= plan_address < map_table_end)
    # PlanWindow is its own source-owned PlanWin object, not storage inside
    # the independent eight-pointer MapWinTable.
    assert re.search(r"(?m)^PlanWin PlanWindow;\s*$", plan_source)
    assert "MapWinTable" not in plan_init
    # Being outside pointer-array storage alone would not rule out being a
    # pointee. The current startup instead publishes new MapWins in 1..7 and
    # the existing Console in slot 0; preserve that independent ownership fact.
    startup = _body((ROOT / "src/game.cpp").read_text(), "int __cdecl system_init()")
    assert "MapWinTable[slot] = new MapWin;" in startup
    assert "slot = 1; slot < static_cast<int>(MapWinTableSlots)" in startup
    assert "MapWinTable[0] = &ConsoleGlobal;" in startup
    assert "PlanWindow" not in startup

    clear = _body(map_source, "void MapWin::clear(int index)")
    init = _body(map_source, "void MapWin::init(int mode, int update)")
    assert "map_view_index_ = index;" in clear
    assert "view_active_ = (index == 0);" in clear
    assert "view_active_ = 1;" in init


def _game_io_fixture(body):
    # Replace only the host I/O entry points; the extracted game_io
    # transfer logic and its member-address order remain actual source.
    assert body.count("fwrite(") == 6
    assert body.count("fread(") == 6
    body = body.replace("fwrite(", "plain_write(").replace("fread(", "plain_read(")
    return r'''#include <stdint.h>
#include <string.h>
#define __cdecl
struct FILE {};
struct MapWin {
  uint32_t map_view_index_, view_active_, field_1DD70_;
  uint32_t field_1DD98_;
  int32_t field_1DD9C_;
  uint32_t field_1DDA0_;
};
int GameDataEncryptedMode = 0;
int GameDataWriteMode = 0;
int EventCount = 0, FailAt = -1;
int NextEncrypted[6], NextWrite[6];
struct Event { void *address; int write, encrypted, width, count; };
Event Events[8];
static int transfer(void *p, int write, int encrypted, unsigned int width,
                    unsigned int count) {
  int i = EventCount++;
  Events[i].address = p; Events[i].write = write; Events[i].encrypted = encrypted;
  Events[i].width = (int)width; Events[i].count = (int)count;
  if (!write) {
    uint32_t raw = 0xA5000000U + (uint32_t)i;
    memcpy(p, &raw, sizeof(raw));
  }
  if (i < 5) {
    GameDataEncryptedMode = NextEncrypted[i];
    GameDataWriteMode = NextWrite[i];
  }
  return i == FailAt ? 0 : 1;
}
int encrypt_write(void *p, unsigned int w, unsigned int n, FILE *) {
  return transfer(p, 1, 1, w, n);
}
int encrypt_read(void *p, unsigned int w, unsigned int n, FILE *) {
  return transfer(p, 0, 1, w, n);
}
int plain_write(void *p, unsigned int w, unsigned int n, FILE *) {
  return transfer(p, 1, 0, w, n);
}
int plain_read(void *p, unsigned int w, unsigned int n, FILE *) {
  return transfer(p, 0, 0, w, n);
}
@@GAME_IO_BODY@@
static int run_case(const int encrypted[6], const int writing[6], int fail_at) {
  MapWin record = { 11, 12, 13, 14, 15, 16 };
  FILE file;
  void *expected[6] = { &record.map_view_index_, &record.view_active_,
    &record.field_1DD70_, &record.field_1DD98_, &record.field_1DD9C_,
    &record.field_1DDA0_ };
  uint32_t old_values[6] = { 11, 12, 13, 14, 15, 16 };
  for (int i = 0; i < 6; ++i) {
    NextEncrypted[i] = encrypted[i + 1 < 6 ? i + 1 : i];
    NextWrite[i] = writing[i + 1 < 6 ? i + 1 : i];
  }
  GameDataEncryptedMode = encrypted[0];
  GameDataWriteMode = writing[0];
  EventCount = 0; FailAt = fail_at;
  int result = game_io(&record, &file);
  int expected_count = fail_at < 0 ? 6 : fail_at + 1;
  if (EventCount != expected_count || result != (fail_at < 0 ? 0 : 1)) return 1;
  for (int i = 0; i < expected_count; ++i) {
    if (Events[i].address != expected[i] || Events[i].write != writing[i] ||
        Events[i].encrypted != encrypted[i] || Events[i].width != 4 ||
        Events[i].count != 1) return 2;
    uint32_t raw;
    memcpy(&raw, Events[i].address, sizeof(raw));
    if (writing[i] && raw != old_values[i]) return 3;
    if (!writing[i] && raw != 0xA5000000U + (uint32_t)i) return 4;
  }
  return 0;
}
int main() {
  for (int encrypted = 0; encrypted <= 1; ++encrypted) {
    for (int writing = 0; writing <= 1; ++writing) {
      int encrypted_modes[6] = { encrypted, encrypted, encrypted,
                                 encrypted, encrypted, encrypted };
      int write_modes[6] = { writing, writing, writing, writing, writing, writing };
      if (run_case(encrypted_modes, write_modes, -1) != 0) return 10;
      for (int failure = 0; failure < 6; ++failure)
        if (run_case(encrypted_modes, write_modes, failure) != 0) return 11;
    }
  }
  int changing_encrypted[6] = { 1, 0, 1, 0, 0, 1 };
  int changing_write[6] = { 0, 1, 1, 0, 1, 0 };
  if (run_case(changing_encrypted, changing_write, -1) != 0) return 12;
  for (int failure = 0; failure < 6; ++failure)
    if (run_case(changing_encrypted, changing_write, failure) != 0) return 13;
  return 0;
}
'''.replace("@@GAME_IO_BODY@@", body)


def test_actual_game_io_keeps_six_word_transfer_order(tmp_path):
    game_source = (ROOT / "src/game.cpp").read_text()
    body = _body(game_source, "int __cdecl game_io(MapWin *record, FILE *file)")
    expected = [
        "&record->map_view_index_", "&record->view_active_",
        "&record->field_1DD70_", "&record->field_1DD98_",
        "&record->field_1DD9C_", "&record->field_1DDA0_",
    ]
    assert [body.index(member) for member in expected] == sorted(
        body.index(member) for member in expected)
    fixture = _game_io_fixture(body)
    assert any(shutil.which(name) for name in ("g++", "clang++")), "host compiler required"
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if compiler is None:
            continue
        source_path = tmp_path / ("mapwin_game_io_" + compiler_name + ".cpp")
        exe_path = tmp_path / ("mapwin_game_io_" + compiler_name)
        source_path.write_text(fixture)
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             str(source_path), "-o", str(exe_path)],
            check=True, capture_output=True, text=True, timeout=30)
        run = subprocess.run([str(exe_path)], capture_output=True, text=True, timeout=10)
        assert run.returncode == 0, compiler_name + ": " + run.stdout + run.stderr


def test_game_io_order_and_width_damage_controls(tmp_path):
    game_source = (ROOT / "src/game.cpp").read_text()
    body = _body(game_source, "int __cdecl game_io(MapWin *record, FILE *file)")
    mutants = {
        "swap_role_order": None,
        "widen_first_encrypted_write": (
            "encrypt_write(field, 4, 1, file)",
            "encrypt_write(field, 8, 1, file)"),
    }
    assert any(shutil.which(name) for name in ("g++", "clang++")), "host compiler required"
    for label, mutation in mutants.items():
        if label == "swap_role_order":
            first, second = "record->map_view_index_", "record->view_active_"
            marker = "__MAPWIN_ROLE_SWAP__"
            damaged = body.replace(first, marker).replace(second, first).replace(marker, second)
        else:
            old, new = mutation
            assert body.count(old) == 6, "mutant anchor changed: " + label
            damaged = body.replace(old, new, 1)
        fixture = _game_io_fixture(damaged)
        for compiler_name in ("g++", "clang++"):
            compiler = shutil.which(compiler_name)
            if compiler is None:
                continue
            source_path = tmp_path / ("damage_" + label + "_" + compiler_name + ".cpp")
            exe_path = tmp_path / ("damage_" + label + "_" + compiler_name)
            source_path.write_text(fixture)
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                 str(source_path), "-o", str(exe_path)],
                check=True, capture_output=True, text=True, timeout=30)
            run = subprocess.run(
                [str(exe_path)], capture_output=True, text=True, timeout=10)
            assert run.returncode != 0, (
                label + " unexpectedly survived under " + compiler_name)


def _mapwin_label_font_fixture(body):
    """Compile the actual extracted MapWin::init body with narrow host spies."""
    return r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "popup_fonts.h"

class Font { public: int identity; };
Font *PopupMapLabelFont = 0;
Font *LookupFont = 0;
int FindFontCalls = 0;
Font *find_font(int, int) { ++FindFontCalls; return LookupFont; }

struct RECT { int left, top, right, bottom; };
struct BufferStub { void init(int, int, int, int) {} };
class MapWin;
MapWin *MapWinTable[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
int IsMultiplayerNet = 0;
unsigned int MapPanelDirtyMask = 0;
int WorldClimateSkipTerrainClear = 0;
RECT WorldMapBounds = { 0, 0, 100, 100 };
RECT MapPanelRects[8] = {};
struct BaseWindowStub { RECT rect1_; };
BaseWindowStub TutWinBaseWindow = {};
struct PlayerStub {
  int noun_gender;
  int is_noun_plural;
  char formal_name_faction[32];
};
typedef PlayerStub Player;
PlayerStub Players[8] = {};
struct ConsoleStub { int faction_id_; };
ConsoleStub ConsoleGlobal = { 0 };
int GenderDefault = 0;
int PluralityDefault = 0;
char StringTemp[256] = {};
char *label_get(int) { return (char *)"label"; }
void say_year(char *) {}
char *_itoa(int value, char *buffer, int) {
  sprintf(buffer, "%d", value);
  return buffer;
}

class Win {
 public:
  Font *field_F8_;
  Win() : field_F8_(0) {}
};

class MapWin : public Win {
 public:
  BufferStub buffer_a_;
  unsigned int view_active_;
  unsigned int map_view_index_;
  unsigned int field_1DD70_;
  int field_1DD98_;
  MapWin() : view_active_(0), map_view_index_(0), field_1DD70_(0),
      field_1DD98_(0) {}
  void UNK5(unsigned int) {}
  void init(RECT *, char *, int) {}
  void do_image_buttons() {}
  void standard_caption() {}
  void redraw() {}
  void init(int mode, int update);
};
struct WorldClimateWindowStub { MapWin mapWin_; };
WorldClimateWindowStub WorldClimateWorldWin;
@@MAPWIN_INIT_BODY@@

int main() {
  Font cached = { 1 };
  Font lookup = { 2 };
  MapWin win;
  MapWinTable[1] = 0;
  LookupFont = &lookup;

  // The guarded nonzero-view case must read the typed cache even when its
  // value differs from a fresh lookup result, and must not call find_font.
  win.map_view_index_ = 2;
  win.field_F8_ = 0;
  PopupMapLabelFont = &cached;
  FindFontCalls = 0;
  win.init(1, 1);
  if (win.field_F8_ != &cached || FindFontCalls != 0) return 1;

  // The zero-view path does not write the font field.
  win.map_view_index_ = 0;
  win.field_F8_ = &lookup;
  PopupMapLabelFont = &cached;
  FindFontCalls = 0;
  win.init(1, 1);
  if (win.field_F8_ != &lookup || FindFontCalls != 0) return 2;

  // Preserve the original cache's null value: no lookup or fallback.
  win.map_view_index_ = 2;
  win.field_F8_ = &lookup;
  PopupMapLabelFont = 0;
  FindFontCalls = 0;
  win.init(1, 1);
  if (win.field_F8_ != 0 || FindFontCalls != 0) return 3;
  return 0;
}
'''.replace("@@MAPWIN_INIT_BODY@@", body).replace(
        '"popup_fonts.h"', '"' + str(ROOT / "src/popup_fonts.h") + '"')


def _compile_and_run_mapwin_label_font(body, tmp_path, name, compiler):
    fixture = _mapwin_label_font_fixture(body)
    source_path = tmp_path / ("mapwin_label_font_" + name + ".cpp")
    exe_path = tmp_path / ("mapwin_label_font_" + name)
    source_path.write_text(fixture)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(exe_path)],
        check=True, capture_output=True, text=True, timeout=30)
    return subprocess.run([str(exe_path)], capture_output=True, text=True, timeout=10)


def test_actual_mapwin_init_uses_cached_map_label_font_and_preserves_null(tmp_path):
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = _body(source, "void MapWin::init(int mode, int update)")
    old_lookup = "Win::field_F8_ = find_font(10, 1);"
    cached_read = "Win::field_F8_ = PopupMapLabelFont;"
    assert body.count(cached_read) == 1

    compilers = [(name, shutil.which(name)) for name in ("g++", "clang++")]
    compilers = [(name, path) for name, path in compilers if path]
    assert compilers, "host GCC/Clang required"
    for name, compiler in compilers:
        run = _compile_and_run_mapwin_label_font(body, tmp_path, name, compiler)
        assert run.returncode == 0, name + ": " + run.stdout + run.stderr

    # Causal mutant: restore the old re-lookup. The fixture gives the cache
    # and lookup different Font addresses, so this must fail the nonzero-view
    # expectation and expose the unwanted call.
    damaged = body.replace(cached_read, old_lookup)
    assert damaged != body and damaged.count(old_lookup) == 1
    for name, compiler in compilers:
        run = _compile_and_run_mapwin_label_font(
            damaged, tmp_path, "old_lookup_mutant_" + name, compiler)
        assert run.returncode != 0, (
            "old lookup mutant unexpectedly passed under " + name)

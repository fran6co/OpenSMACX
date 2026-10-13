"""Host controls for pointer-typed Labels and Difficulty consumers."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())
CONSOLE = ROOT / "src/console.cpp"
GAME = ROOT / "src/game.cpp"
NETWIN = ROOT / "src/netwin.cpp"


def _mask(text):
    return COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), text)


def _body(text, signature):
    masked = _mask(text)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for i in range(opening, len(masked)):
        if masked[i] == "{":
            depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unbalanced source body: " + signature)


def _block(text, start):
    masked = _mask(text)
    opening = masked.index("{", start)
    depth = 0
    for i in range(opening, len(masked)):
        if masked[i] == "{":
            depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unbalanced source block")


def _actual_fragments():
    console_source = CONSOLE.read_text()
    game_source = GAME.read_text()
    netwin_source = NETWIN.read_text()
    console_body = _body(console_source, "void Console::custom_my_faction()")
    label_start = console_body.index("LPSTR *const label_keys = Labels.strings_ptr;")
    key_end_marker = "strcpy(name_buf, StringTable.get(key));"
    key_end = console_body.index(key_end_marker, label_start) + len(key_end_marker)
    console_key = console_body[label_start:key_end]
    picker = _body(netwin_source, "void NetWin::pick_diff(int slot)")
    loop_start = picker.index("for (int index = 0; index < 6; ++index)")
    difficulty_loop = _block(picker, loop_start)
    draw = _body(netwin_source, "void NetWin::draw_session_info()")
    opponent_start = draw.index("// ---- opponent difficulty ----")
    prefix_start = draw.index("StringTemp[0] = 0;", opponent_start)
    branch_start = draw.index("if (*NetWinScenarioText == 0)", prefix_start)
    first_branch = _block(draw, branch_start)
    else_start = _mask(draw).index("else", branch_start + len(first_branch))
    else_branch = _block(draw, else_start)
    opponent_fragment = draw[prefix_start:branch_start] + first_branch + draw[
        branch_start + len(first_branch):else_start] + else_branch
    game_label = "StringTable.get(Labels.strings_ptr[0])"
    assert game_label in game_source
    game_fallback = "StringTable.get(0)"
    assert game_fallback in game_source
    say_start = game_source.index(
        "parse_say(0,\n            Difficulty[GameDataStateStorage.fields_.diff_level_current_],")
    say_end = game_source.index("-1, -1);", say_start) + len("-1, -1);")
    game_difficulty_call = game_source[say_start:say_end]
    return (console_key, difficulty_loop, game_label, game_fallback,
            game_difficulty_call, opponent_fragment)


def _harness(console_key, difficulty_loop, game_label, game_fallback,
             game_difficulty_call):
    return r'''#include <assert.h>
#include <stddef.h>
#include <string.h>
typedef char *LPSTR;
struct Label { LPSTR *strings_ptr; } Labels;
struct Strings {
  LPSTR base;
  LPSTR last;
  int calls;
  LPSTR get(LPSTR value) {
    ++calls;
    last = value;
    return value ? value : base;
  }
} StringTable;
LPSTR Difficulty[6];
LPSTR parsed_text;
struct GameFields { int diff_level_current_; };
struct GameState { GameFields fields_; } GameDataStateStorage;
int parse_say(int, LPSTR text, int, int) { parsed_text = text; return 17; }
struct Player { int is_leader_female; };
void console_case(int female, char *out) {
  Player player;
  player.is_leader_female = female;
  char name_buf[64];
''' + console_key + r'''
  strcpy(out, name_buf);
}
struct Dialogs {
  LPSTR text[8];
  int ids[8];
  int count;
  Dialogs() : count(0) {}
  void item(LPSTR value, int id) {
    text[count] = value;
    ids[count++] = id;
  }
};
struct PopMenu {
  Dialogs dialog;
  Dialogs &dialogs() { return dialog; }
};
void netwin_case(PopMenu &pop_menu) {
''' + difficulty_loop + r'''
}
LPSTR game_label_zero() { return ''' + game_label + r'''; }
LPSTR game_null_fallback() { return ''' + game_fallback + r'''; }
void game_difficulty_say() {
''' + game_difficulty_call + r'''
}
int main() {
  char base[] = "BASE", neutral[] = "NEUTRAL", female[] = "FEMALE";
  char out[64];
  LPSTR label_slots[0x2E4];
  memset(label_slots, 0, sizeof(label_slots));
  label_slots[0] = neutral;
  label_slots[0x2E2] = neutral;
  label_slots[0x2E3] = female;
  Labels.strings_ptr = label_slots;
  StringTable.base = base;
  StringTable.calls = 0;
  StringTable.last = 0;
  console_case(0, out);
  assert(strcmp(out, "NEUTRAL") == 0);
  console_case(1, out);
  assert(strcmp(out, "FEMALE") == 0);
  assert(game_label_zero() == neutral);
  assert(game_null_fallback() == base);
  char diff0[] = "D0", diff1[] = "D1", diff2[] = "D2";
  char diff3[] = "D3", diff4[] = "D4", diff5[] = "D5";
  LPSTR values[6] = {diff0, diff1, diff2, diff3, diff4, diff5};
  for (int i = 0; i < 6; ++i) Difficulty[i] = values[i];
  GameDataStateStorage.fields_.diff_level_current_ = 4;
  game_difficulty_say();
  assert(parsed_text == diff4);
  PopMenu pop_menu;
  netwin_case(pop_menu);
  assert(pop_menu.dialog.count == 6);
  for (int i = 0; i < 6; ++i) {
    assert(pop_menu.dialog.text[i] == values[i]);
    assert(pop_menu.dialog.ids[i] == i);
  }
  return 0;
}
'''


def _opponent_harness(fragment):
    harness = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef char *LPSTR;
/* ACTUAL_NETWIN_ROW_AND_SESSION_DECLARATIONS */
struct Label { LPSTR *strings_ptr; } Labels;
LPSTR raw_title, localized_title, raw_fallback, localized_fallback;
struct Strings {
  LPSTR inputs[8];
  int calls;
  LPSTR get(LPSTR value) {
    inputs[calls++] = value;
    if (value == raw_title) return localized_title;
    if (value == raw_fallback) return localized_fallback;
    return value;
  }
} StringTable;
LPSTR label_get(int slot) { return StringTable.get(Labels.strings_ptr[slot]); }
struct SessionOptions { int option_4_; int option_5_; } NetWinSessionOptions;
NetWinSessionBlock NetWinSessionData;
char *NetWinScenarioText = NetWinSessionData.scenario_text_wire_tail_;
char StringTemp[4096];
char * _itoa(int value, char *buffer, int) {
  sprintf(buffer, "%d", value);
  return buffer;
}
void opponent_case(int option) {
  NetWinSessionOptions.option_4_ = option;
''' + fragment + r'''
}
int main() {
  char title_raw[] = "raw title", title_text[] = "localized title";
  char fallback_raw[] = "raw fallback", fallback_text[] = "localized fallback";
  char opponent0[] = "opponent 0", opponent1[] = "opponent 1";
  char opponent31[] = "opponent 31";
  static LPSTR slots[8000];
  raw_title = title_raw; localized_title = title_text;
  raw_fallback = fallback_raw; localized_fallback = fallback_text;
  slots[0xFAC / 4] = raw_title;
  slots[0xFE4 / 4] = raw_fallback;
  slots[(0 * 0x80 + 0x6614) / 4] = opponent0;
  slots[(1 * 0x80 + 0x6614) / 4] = opponent1;
  slots[(31 * 0x80 + 0x6614) / 4] = opponent31;
  Labels.strings_ptr = slots;
  NetWinSessionData.scenario_text_wire_tail_[0] = 0;

  opponent_case(0);
  assert(NetWinSessionOptions.option_4_ == 0);
  assert(StringTable.calls == 1 && StringTable.inputs[0] == raw_title);
  assert(strstr(StringTemp, "localized title") != 0);
  assert(strstr(StringTemp, "opponent 0") != 0);

  StringTable.calls = 0;
  opponent_case(1);
  assert(NetWinSessionOptions.option_4_ == 1);
  assert(StringTable.calls == 1 && StringTable.inputs[0] == raw_title);
  assert(strstr(StringTemp, "opponent 1") != 0);

  StringTable.calls = 0;
  opponent_case(31);
  assert(NetWinSessionOptions.option_4_ == 31);
  assert(StringTable.calls == 1 && StringTable.inputs[0] == raw_title);
  assert(strstr(StringTemp, "opponent 31") != 0);

  StringTable.calls = 0;
  opponent_case(-1);
  assert(NetWinSessionOptions.option_4_ == -1);
  assert(StringTable.calls == 2);
  assert(StringTable.inputs[0] == raw_title);
  assert(StringTable.inputs[1] == raw_fallback);
  assert(strstr(StringTemp, "localized fallback") != 0);

  StringTable.calls = 0;
  opponent_case(-2); // non-sentinel negative clamps to slot zero
  assert(NetWinSessionOptions.option_4_ == -2);
  assert(strstr(StringTemp, "opponent 0") != 0);
  StringTable.calls = 0;
  opponent_case(32); // upper clamp selects slot thirty-one
  assert(NetWinSessionOptions.option_4_ == 32);
  assert(strstr(StringTemp, "opponent 31") != 0);

  NetWinSessionData.scenario_text_wire_tail_[0] = 1;
  StringTable.calls = 0;
  opponent_case(7);
  assert(strstr(StringTemp, "7 X") != 0);
  assert(NetWinSessionOptions.option_4_ == 7);
  return 0;
}
'''
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    declarations = []
    for name in ("NetWinLobbyRow", "NetWinSessionBlock"):
        match = re.search(rf"struct {name} \{{.*?\n\}};", header, re.S)
        assert match is not None, name
        declarations.append(match.group())
    return harness.replace(
        "/* ACTUAL_NETWIN_ROW_AND_SESSION_DECLARATIONS */",
        "\n".join(declarations))


def _run(source, compiler):
    with tempfile.TemporaryDirectory(prefix="strings-label-owner-") as temp:
        cpp = Path(temp) / "control.cpp"
        exe = Path(temp) / "control"
        cpp.write_text(source)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                str(cpp), "-o", str(exe)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def test_actual_pointer_consumers_and_compiling_mutants():
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        import pytest
        pytest.skip("host C++ compiler unavailable")
    (console_key, difficulty_loop, game_label, game_fallback,
     game_difficulty_call, opponent_fragment) = _actual_fragments()
    for path in (CONSOLE, GAME, NETWIN):
        text = path.read_text()
        assert "StringTable.get(reinterpret_cast<int>" not in text
        assert "parse_say(0, reinterpret_cast<int>" not in text
    netwin_source = NETWIN.read_text()
    assert "for (int slot = 0xF84 / 4; slot < 0xF90 / 4; ++slot)" in netwin_source
    assert "static const int kTextSlots[4]" in netwin_source
    assert "LPSTR *labels = Labels.strings_ptr;" in GAME.read_text()
    assert "LPSTR *const label_keys = Labels.strings_ptr;" in CONSOLE.read_text()
    assert "const LPSTR key" in console_key
    assert "StringTable.get(key)" in console_key
    assert "reinterpret_cast<int" not in console_key
    assert "reinterpret_cast<int" not in difficulty_loop
    assert "StringTable.get(Difficulty[index])" in difficulty_loop
    source = _harness(console_key, difficulty_loop, game_label, game_fallback,
                      game_difficulty_call)
    result = _run(source, compiler)
    assert result.returncode == 0, result.stdout + result.stderr

    # Both mutations compile but must fail source-extracted behavior checks.
    wrong_label = console_key.replace("label_keys[0x2E3]", "label_keys[0x2E2]")
    assert wrong_label != console_key
    damaged = _run(_harness(wrong_label, difficulty_loop, game_label,
                            game_fallback, game_difficulty_call), compiler)
    assert damaged.returncode != 0, "wrong-label-slot mutant escaped"

    wrong_difficulty = difficulty_loop.replace(
        "StringTable.get(Difficulty[index])", "StringTable.get(Difficulty[0])")
    assert wrong_difficulty != difficulty_loop
    damaged = _run(_harness(console_key, wrong_difficulty, game_label,
                            game_fallback, game_difficulty_call), compiler)
    assert damaged.returncode != 0, "wrong-difficulty-pointer mutant escaped"

    opponent_source = _opponent_harness(opponent_fragment)
    result = _run(opponent_source, compiler)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "(opponent * 0x80 + 0x6614) / 4" in opponent_fragment

    # These are compiled, in-bounds wrong slot calculations. Distinct decoys
    # make a bad stride or byte offset observably select the wrong text.
    for old, new, label in (
        ("opponent * 0x80", "opponent * 0x84", "wrong-stride"),
        ("+ 0x6614", "+ 0x6618", "wrong-byte-offset"),
    ):
        mutant_fragment = opponent_fragment.replace(old, new)
        assert mutant_fragment != opponent_fragment
        mutant_source = _opponent_harness(mutant_fragment)
        mutant_source = mutant_source.replace(
            "Labels.strings_ptr = slots;",
            """slots[(0 * 0x84 + 0x6614) / 4] = opponent31;
  slots[(1 * 0x84 + 0x6614) / 4] = opponent0;
  slots[(31 * 0x84 + 0x6614) / 4] = opponent1;
  slots[(0 * 0x80 + 0x6618) / 4] = opponent31;
  slots[(1 * 0x80 + 0x6618) / 4] = opponent0;
  slots[(31 * 0x80 + 0x6618) / 4] = opponent1;
  Labels.strings_ptr = slots;""")
        damaged = _run(mutant_source, compiler)
        assert damaged.returncode != 0, label + " mutant escaped"


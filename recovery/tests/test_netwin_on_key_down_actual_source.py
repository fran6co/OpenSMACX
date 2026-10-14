"""Live-body host controls with mock services; not native layout, virtual ABI or full-game proof."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


def _root_from(path: Path) -> Path:
    for parent in path.resolve().parents:
        if (parent / "src" / "netwin.cpp").is_file():
            return parent
    raise AssertionError("could not find repository src/netwin.cpp ancestor")


ROOT = _root_from(Path(__file__))


def _live_method() -> str:
    source = (ROOT / "src/netwin.cpp").read_text()
    signature = "int NetWin::on_key_down(int key) {"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError("unterminated live NetWin::on_key_down body")


FIXTURE_PREFIX = r"""
#include <cstdlib>
#include <stdint.h>

enum {
  EVENT_EDIT_KEY = 1,
  EVENT_RELEASE_MODAL,
  EVENT_FREE_RECORDS,
  EVENT_FREE_COPY,
  EVENT_SPOT_SHUTDOWN,
  EVENT_STRING_CLOSE,
  EVENT_EDIT_CLOSE,
  EVENT_CHECK_CLOSE,
  EVENT_BUTTON_CLOSE,
  EVENT_PID_TO_INDEX,
  EVENT_MESSAGE_CHAT,
  EVENT_EDIT_SET_TEXT
};

static int events[32];
static int event_count;
static int observed_key;
static int observed_edit_receiver;
static int observed_pid;
static int observed_message_type;
static int observed_message_arg;
static char *observed_message_text;
static char *expected_records;
static char *expected_copy;

static void record_event(int value) {
  events[event_count++] = value;
}

static void tracked_free(void *value) {
  if (value == expected_records) {
    record_event(EVENT_FREE_RECORDS);
  } else if (value == expected_copy) {
    record_event(EVENT_FREE_COPY);
  } else {
    record_event(99);
  }
  std::free(value);
}

struct EditBox {
  int identity;
  int answer;
  char text_[256];
  int on_key_down(int key) {
    observed_key = key;
    observed_edit_receiver = identity;
    record_event(EVENT_EDIT_KEY);
    return answer;
  }
  void set_text(char *) { record_event(EVENT_EDIT_SET_TEXT); }
  void close() { record_event(EVENT_EDIT_CLOSE); }
};

struct Spot {
  void shutdown() { record_event(EVENT_SPOT_SHUTDOWN); }
};
struct StringBox {
  void close() { record_event(EVENT_STRING_CLOSE); }
};
struct CheckBox {
  void close() { record_event(EVENT_CHECK_CLOSE); }
};
struct FlatButton {
  void close() { record_event(EVENT_BUTTON_CLOSE); }
};
struct NetDaemon {
  uint32_t local_player_id_, host_player_id_;
  NetDaemon() : local_player_id_(17), host_player_id_(0) {}
  uint32_t local_player_id() const { return local_player_id_; }
  uint32_t host_player_id() const { return host_player_id_; }
  int pid_2_idx(int value) {
    observed_pid = value;
    record_event(EVENT_PID_TO_INDEX);
    return 3;
  }
};

static NetDaemon NetDaemonNet;

static void message_chat(int type, int arg, char *text) {
  observed_message_type = type;
  observed_message_arg = arg;
  observed_message_text = text;
  record_event(EVENT_MESSAGE_CHAT);
}

struct NetWin {
  char prefix_[0x42F4];
  EditBox editBox_;
  EditBox decoy_edit_box_;
  char *faction_records_;
  char *faction_records_copy_;
  uint32_t field_761C_;
  uint32_t field_7620_;
  Spot spot_;
  StringBox stringBox_;
  CheckBox checkBox_;
  FlatButton flatButton1_;

  NetWin() : faction_records_(0), faction_records_copy_(0),
      field_761C_(5), field_7620_(6) {
    editBox_.identity = 1;
    editBox_.answer = -17;
    decoy_edit_box_.identity = 2;
    decoy_edit_box_.answer = -29;
  }
  void release_modal() { record_event(EVENT_RELEASE_MODAL); }
  int on_key_down(int key);
};

#define free tracked_free
"""


FIXTURE_SUFFIX = r"""
#undef free

static void reset_events() {
  event_count = 0;
  observed_key = -1;
  observed_edit_receiver = -1;
  observed_pid = -1;
  observed_message_type = -1;
  observed_message_arg = -1;
  observed_message_text = 0;
  expected_records = 0;
  expected_copy = 0;
}

static int expect_events(const int *expected, int count) {
  if (event_count != count) return 0;
  for (int i = 0; i < count; ++i) {
    if (events[i] != expected[i]) return 0;
  }
  return 1;
}

static int test_forwarded_key() {
  reset_events();
  NetWin window;
  const int answer = window.on_key_down(0x41);
  const int expected[] = { EVENT_EDIT_KEY };
  return answer == -17 && observed_key == 0x41
      && observed_edit_receiver == 1
      && expect_events(expected, 1)
      && window.field_761C_ == 5 && window.field_7620_ == 6;
}

static int test_escape_cleanup_order() {
  reset_events();
  NetWin window;
  expected_records = static_cast<char *>(std::malloc(8));
  expected_copy = static_cast<char *>(std::malloc(8));
  if (!expected_records || !expected_copy) return 0;
  window.faction_records_ = expected_records;
  window.faction_records_copy_ = expected_copy;
  const int answer = window.on_key_down(0x1B);
  const int expected[] = {
    EVENT_RELEASE_MODAL,
    EVENT_FREE_RECORDS,
    EVENT_FREE_COPY,
    EVENT_SPOT_SHUTDOWN,
    EVENT_STRING_CLOSE,
    EVENT_EDIT_CLOSE,
    EVENT_CHECK_CLOSE,
    EVENT_BUTTON_CLOSE
  };
  return answer == 1 && window.faction_records_ == 0
      && window.faction_records_copy_ == 0
      && window.field_761C_ == 0xFFFFFFFFu
      && window.field_7620_ == 0xFFFFFFFFu
      && expect_events(expected, 8);
}

static int test_enter_chat_path() {
  reset_events();
  NetWin window;
  const int answer = window.on_key_down(0x0D);
  const int expected[] = {
    EVENT_PID_TO_INDEX, EVENT_MESSAGE_CHAT, EVENT_EDIT_SET_TEXT
  };
  char *const expected_text = window.editBox_.text_;
  return answer == 1 && observed_pid == 17
      && observed_message_type == 0x1E00 && observed_message_arg == 0
      && observed_message_text == expected_text
      && expect_events(expected, 3);
}

int main() {
  return test_forwarded_key() && test_escape_cleanup_order()
      && test_enter_chat_path() ? 0 : 1;
}
"""


def _compile_and_run(tmp_path: Path, compiler: str, method: str) -> int:
    source = tmp_path / "netwin_on_key_down.cpp"
    executable = tmp_path / "netwin_on_key_down"
    source.write_text(FIXTURE_PREFIX + method + FIXTURE_SUFFIX)
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        capture_output=True,
        text=True,
        check=False,
    )
    if compiled.returncode != 0:
        raise AssertionError(compiled.stdout + compiled.stderr)
    return subprocess.run([str(executable)], check=False).returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_live_method_behavior_and_escape_order(
    tmp_path: Path, compiler_name: str
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    assert _compile_and_run(tmp_path, compiler, _live_method()) == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    ("old", "new"),
    (
        ("return editBox_.on_key_down(key);",
         "return decoy_edit_box_.on_key_down(key);"),
        ("return editBox_.on_key_down(key);",
         "return editBox_.on_key_down(key + 1);"),
        ("        release_modal();\n", "        /* release omitted */\n"),
        ("        flatButton1_.close();\n", "        /* button close omitted */\n"),
    ),
)
def test_mutants_wrong_receiver_key_or_escape_cleanup_are_detected(
    tmp_path: Path, compiler_name: str, old: str, new: str
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    method = _live_method()
    assert old in method
    mutant = method.replace(old, new, 1)
    assert mutant != method
    assert _compile_and_run(tmp_path, compiler, mutant) != 0

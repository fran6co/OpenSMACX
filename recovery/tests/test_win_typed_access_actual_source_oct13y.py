"""Bounded actual-source controls for typed Win fields and flag masks.

The host fixtures compile literal current Win method bodies, not the game ABI.
The immutable task-base comparison protects source identities and obligations;
the two pinned PE-body digests independently anchor the low-byte consumers.
Neither control proves the native Win layout or whole-game behavior.
"""
from dataclasses import fields, replace
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
TASK = "win-typed-access-closure-oct13y"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
CLIENT_SPAN = (0x005EEE70, 0x005EEF56)
BUTTON_SPAN = (0x005F6550, 0x005F670D)
CLIENT_SHA256 = "a8cb2d5fbe4d588221018bfdaece2e5eb71673bc2b69b1200eca6afb95b53db6"
BUTTON_SHA256 = "f3ce47905a1c47f1f7d6089c44d70cefdcd324666f4a8d35b0fa00cb35a0472f"


def extract_function(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        else:
            if char == "/" and nxt == "/":
                state = "line_comment"
                index += 1
            elif char == "/" and nxt == "*":
                state = "block_comment"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated method {signature}")


def protected_history(row):
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line for _, line in lesson.lines))
                 for lesson in row.origin)


def protected_metadata(row):
    return tuple((item.name, getattr(row, item.name)) for item in fields(row)
                 if item.name not in {"line", "region", "origin", "symbol"})


def test_immutable_task_base_identity_claim_and_history_continuity():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == "82ca29fb11830b40b97cf503e04d589853ffce81"
    path = ROOT / "src/win.cpp"
    previous = subprocess.check_output(
        ["git", "show", f"{task.base_revision}:src/win.cpp"],
        cwd=ROOT, text=True, timeout=30)
    before = {row.address: row for row in read_text(previous, path)}
    after = {row.address: row for row in read_text(path.read_text(), path)}
    assert set(after) == set(before)
    for address, old in before.items():
        current = after[address]
        expected = old
        if address in {0x5F1070, 0x5EE9F0}:
            # Native recording in the later immutable claim-only task promotes
            # precisely these two obligations; all other metadata stays fixed.
            assert not old.byte_exact and not old.semantic
            assert current.byte_exact and not current.semantic
            expected = replace(old, byte_exact=True)
        assert protected_metadata(expected) == protected_metadata(current)
        assert protected_history(old) == protected_history(current)
        if address == 0x5F0540:
            assert old.symbol == "?adjust_menus@Win@@SGHPAUHWND__@@PAX@Z"
            assert current.symbol == "?adjust_menus@Win@@SGHPAUHWND__@@J@Z"
        else:
            assert old.symbol == current.symbol


def test_scoped_real_types_and_only_the_proven_same_type_views_are_removed():
    header = (ROOT / "src/win.h").read_text()
    assert re.search(r"\buint32_t\s+iFlags_\s*;", header)
    assert re.search(r"\buint32_t\s+iSomeFlag_\s*;", header)
    assert re.search(r"\bWin\s*\*\s*win_parent_\s*;", header)
    assert re.search(r"\bWin\s*\*\s*poWinBase_\s*;", header)

    source = (ROOT / "src/win.cpp").read_text()
    client = extract_function(source, "void Win::client_to_nonclient(int * points, int * count)")
    button = extract_function(source, "void Win::on_l_button_up(int x, int y, unsigned int keys, int dbl)")
    key = extract_function(source, "void Win::on_key(unsigned int key, long flags, int repeat, unsigned int scan)")
    assert "reinterpret_cast<uint8_t *>(&iFlags_)" not in client
    assert "reinterpret_cast<uint8_t *>(&iSomeFlag_)" not in button
    assert "Win *target;" in key and "void *target;" not in key
    assert "target = list_.current_->focus_owner_;" in key
    assert "target->key_up_event(with_modifiers)" in key
    assert "reinterpret_cast<Win *>(target)" not in key
    assert "if (list_.count_ != 0)" in key
    assert "if (list_.head_ != 0)" in key and "target = 0;" in key
    assert key.index("target = 0;") < key.index("target->key_up_event(with_modifiers)")
    assert "if (target != 0)" not in key and "if (target != nullptr)" not in key

    # The callback's actual SDK type flows directly into EnumThreadWindows.
    # These remaining conversions are genuine foreign-boundary crossings.
    assert "reinterpret_cast<WNDENUMPROC>(Win::adjust_menus)" not in source
    update_menus = extract_function(source, "void Win::update_menus()")
    assert "EnumThreadWindows(GetCurrentThreadId(), Win::adjust_menus, 0);" in update_menus
    assert re.search(r"static\s+int\s+__stdcall\s+adjust_menus\(HWND\s+hwnd,\s*LPARAM\s+unused\)",
                     header)
    assert re.search(r"int\s+__stdcall\s+Win::adjust_menus\(HWND\s+hwnd,\s*LPARAM\s+unused\)",
                     source)
    pos_changed = extract_function(source, "LRESULT Win::on_window_pos_changed(WINDOWPOS* pos)")
    assert "reinterpret_cast<LPARAM>(pos)" in pos_changed
    window_proc = extract_function(source, "LRESULT __stdcall Win::window_proc(")
    assert "reinterpret_cast<Win *>(" in window_proc  # foreign userdata/receiver conversion


def test_pinned_original_low_byte_consumers_are_complete_and_unchanged():
    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    image = PeImage(image_path.read_bytes(), PIN)
    for span, expected in ((CLIENT_SPAN, CLIENT_SHA256), (BUTTON_SPAN, BUTTON_SHA256)):
        raw = image.read(Span(*span))
        assert hashlib.sha256(raw).hexdigest() == expected
    client_raw = image.read(Span(*CLIENT_SPAN))
    button_raw = image.read(Span(*BUTTON_SPAN))
    # The original uses byte loads/tests from the low byte for these masks.
    assert bytes.fromhex("8a 91 98 00 00 00 57 f6 c2 04") in client_raw
    assert bytes.fromhex("f6 81 98 00 00 00 08") in client_raw
    assert bytes.fromhex("f6 81 98 00 00 00 10") in client_raw
    assert bytes.fromhex("f6 86 9c 00 00 00 08") in button_raw
    assert bytes.fromhex("f6 86 9c 00 00 00 01") in button_raw


CLIENT_FIXTURE = r"""
#include <cstdint>
#include <cstdio>
struct Menu { int height; int requested_height() { return height; } };
static const int ScrollDefaultThickness = 3;
struct Win {
  uint32_t iFlags_;
  int border_thickness_;
  int bottom_border_thickness_;
  int caption_height_;
  Menu *menu_;
  void client_to_nonclient(int *, int *);
};
"""


def client_program(body, mode):
    return CLIENT_FIXTURE + "\n" + body + rf"""
int main() {{
  Win window{{}};
  window.bottom_border_thickness_ = -1;
  int points = 10, count = 20;
  const char *label = "{mode}";
  auto require = [label](bool ok, const char *case_name) {{
    if (!ok) {{ std::fprintf(stderr, "FAIL: %s: %s\n", label, case_name); return false; }}
    return true;
  }};
  window.iFlags_ = 4;
  window.client_to_nonclient(&points, &count);
  if (!require(points == 10 && count == 23, "low-bit-4")) return 98;
  points = 10; count = 20; window.iFlags_ = 8;
  window.client_to_nonclient(&points, &count);
  if (!require(points == 13 && count == 20, "low-bit-8")) return 98;
  points = 10; count = 20; window.iFlags_ = 0x10;
  window.caption_height_ = 6; window.border_thickness_ = 2;
  window.client_to_nonclient(&points, &count);
  if (!require(points == 14 && count == 28, "low-bit-10")) return 98;
  points = 10; count = 20; window.iFlags_ = 0x10000000U;
  window.caption_height_ = 0; window.border_thickness_ = 0;
  window.client_to_nonclient(&points, &count);
  if (!require(points == 10 && count == 20, "upper-bits-ignored")) return 98;
  return 0;
}}
"""


def build_and_run(tmp_path, compiler_name, source, expected, diagnostic=None):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    executable = tmp_path / "actual_win"
    built = subprocess.run([compiler, "-std=c++17", "-O2", "-x", "c++", "-", "-o", str(executable)],
                           input=source, text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == expected, result.stderr
    if diagnostic:
        assert diagnostic in result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["none", "wrong-bit", "upper-bit-alias"])
def test_actual_client_to_nonclient_word_masks_and_named_mutants(tmp_path, compiler_name, damage):
    source_text = (ROOT / "src/win.cpp").read_text()
    body = extract_function(source_text, "void Win::client_to_nonclient(int * points, int * count)")
    if damage == "wrong-bit":
        before = "if (iFlags_ & 4)"
        assert body.count(before) == 1
        body = body.replace(before, "if (iFlags_ & 2)", 1)
    elif damage == "upper-bit-alias":
        before = "if (iFlags_ & 4)"
        assert body.count(before) == 1
        body = body.replace(before, "if (iFlags_ & 0x10000004U)", 1)
    source = client_program(body, damage)
    if damage == "none":
        build_and_run(tmp_path, compiler_name, source, 0)
    else:
        build_and_run(tmp_path, compiler_name, source, 98, f"FAIL: {damage}:")


BUTTON_FIXTURE = r"""
#include <cstdint>
#include <cstdio>
struct RECT { int left, top, right, bottom; };
typedef void (*WinMouseEventCallback)(int, int);
struct Sound { int is_playing() { return 0; } void play() {} };
struct Win {
  uint32_t iFlags_, iSomeFlag_;
  RECT outer_rect_;
  Win *win_parent_;
  Sound *val_8_, *val_20_, *val_10_, *val_22_;
  WinMouseEventCallback field_414_, field_41C_;
  int visible_, clicks_, ups_, nonclient_ups_, double_clicks_;
  int is_visible() { return visible_; }
  void on_left_click(int, int) { ++clicks_; }
  void on_left_up(int, int) { ++ups_; }
  void on_nc_left_up(int, int) { ++nonclient_ups_; }
  void vslot_32(int, int) { ++double_clicks_; }
  void on_l_button_up(int, int, unsigned int, int);
};
Win *WinPointerOwner1;
Win *WinPointerOwner2;
Win *WinCallbackWindow;
static int in_box(int, int, RECT *) { return 1; }
static Win *callback_target;
static void set_high_flag(int, int) { callback_target->iSomeFlag_ |= 8U; }
static void observe_callback(int, int) {}
"""


def button_program(body, mode):
    return BUTTON_FIXTURE + "\n" + body + rf"""
int main() {{
  auto require = [](bool ok, const char *name) {{
    if (!ok) {{ std::fprintf(stderr, "FAIL: {mode}: %s\n", name); return false; }}
    return true;
  }};
  Win mutation{{}};
  mutation.visible_ = 1;
  mutation.iSomeFlag_ = 1U;
  mutation.field_414_ = set_high_flag;
  mutation.field_41C_ = observe_callback;
  callback_target = &mutation;
  WinPointerOwner1 = &mutation; WinPointerOwner2 = nullptr;
  mutation.on_l_button_up(2, 3, 0, 0);
  if (!require(mutation.clicks_ == 1 && mutation.ups_ == 0,
               "fresh-flag-after-callback")) return 98;

  Win upper{{}};
  upper.visible_ = 1;
  upper.iSomeFlag_ = 1U | 0x10000000U;
  upper.field_414_ = observe_callback;
  upper.field_41C_ = observe_callback;
  WinPointerOwner1 = &upper; WinPointerOwner2 = nullptr;
  upper.on_l_button_up(2, 3, 0, 0);
  if (!require(upper.clicks_ == 1 && upper.ups_ == 1,
               "upper-bits-ignored")) return 98;
  return 0;
}}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["none", "upper-bit-alias", "stale-callback-flags"])
def test_actual_l_button_up_word_masks_reload_after_callback_and_mutants(tmp_path, compiler_name, damage):
    source_text = (ROOT / "src/win.cpp").read_text()
    body = extract_function(source_text, "void Win::on_l_button_up(int x, int y, unsigned int keys, int dbl)")
    if damage == "upper-bit-alias":
        before = "(iSomeFlag_ & 8) == 0"
        assert body.count(before) == 2
        body = body.replace(before, "(iSomeFlag_ & 0x10000008U) == 0", 1)
    elif damage == "stale-callback-flags":
        before = "(iSomeFlag_ & 8) == 0"
        assert body.count(before) == 2
        opening = body.index("{", body.index("void Win::on_l_button_up")) + 1
        body = body[:opening] + "\n    const uint32_t cached_flags = iSomeFlag_;" + body[opening:]
        body = body.replace(before, "(cached_flags & 8) == 0", 2)
        # Keep the first pre-callback guard live; only the later check is stale.
        body = body.replace("(cached_flags & 8) == 0", before, 1)
    source = button_program(body, damage)
    if damage == "none":
        build_and_run(tmp_path, compiler_name, source, 0)
    else:
        build_and_run(tmp_path, compiler_name, source, 98, f"FAIL: {damage}:")

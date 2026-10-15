"""Host controls for AutoSound's Sound receiver views and BaseButton reloads.

These fixtures compile literal current method bodies against host-only probes.
They exercise receiver/callback behavior, not the 32-bit object ABI or a Sound
provider/lifetime contract. The pinned PE witness is checked independently.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/autosound.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
AUTO_SOURCE = ROOT / "src/autosound.cpp"
AUTO_HEADER = ROOT / "src/autosound.h"
DEFAULTS_HEADER = ROOT / "src/autosound_defaults.h"
BUTTON_SOURCE = ROOT / "src/basebutton.cpp"
WIN_SOURCE = ROOT / "src/win.cpp"


def extract_definition(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    assert masked.count(signature) == 1, f"expected one definition: {signature}"
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


COPY_MAP = (
    ("val_1_", 0, "int"), ("val_3_", 1, "Sound *"),
    ("val_4_", 2, "Sound *"), ("val_2_", 3, "int"),
    ("val_5_", 4, "int"), ("val_6_", 5, "Sound *"),
    ("val_7_", 6, "Sound *"), ("val_8_", 7, "Sound *"),
    ("val_9_", 8, "Sound *"), ("val_10_", 9, "Sound *"),
    ("val_11_", 10, "Sound *"), ("val_12_", 11, "Sound *"),
    ("val_13_", 12, "Sound *"), ("val_14_", 13, "Sound *"),
    ("val_15_", 14, "Sound *"), ("val_16_", 15, "Sound *"),
    ("val_17_", 16, "Sound *"), ("val_18_", 17, "Sound *"),
    ("val_19_", 18, "Sound *"), ("val_20_", 19, "Sound *"),
    ("val_21_", 20, "Sound *"), ("val_22_", 21, "Sound *"),
    ("val_23_", 22, "Sound *"), ("val_24_", 23, "Sound *"),
    ("val_25_", 24, "Sound *"), ("val_26_", 25, "Sound *"),
    ("val_27_", 26, "Sound *"), ("val_28_", 27, "Sound *"),
    ("val_29_", 28, "Sound *"), ("val_30_", 29, "Sound *"),
    ("val_31_", 30, "Sound *"), ("val_32_", 31, "int"),
    ("val_33_", 32, "Sound *"), ("event_window_", 33, "Sound *"),
    ("val_35_", 34, "Sound *"), ("val_36_", 35, "Sound *"),
    ("val_37_", 36, "Sound *"),
)


def auto_members() -> str:
    header = AUTO_HEADER.read_text(encoding="utf-8")
    header = header[header.index("class AutoSound {"):]
    declarations = re.findall(
        r"^\s*(int|Sound\s*\*)\s*(val_\d+_|event_window_);\s*(?://.*)?$",
        header, flags=re.M,
    )
    assert len(declarations) == 37
    assert ("Sound *val_20_;" in header and
            "RECOVERY_MEMBER: v3 AutoSound val_20_ Sound* 0x50 0x005F6550" in header)
    assert ("Sound *val_31_;" in header and
            "RECOVERY_MEMBER: v3 AutoSound val_31_ Sound* 0x7C 0x00607A20" in header)
    assert "static_assert(sizeof(AutoSound) == 0x98" in header
    # The host probe intentionally does not compile that 32-bit ABI assertion.
    return "\n".join(f"  {kind} {name};" for kind, name in declarations)


def defaults_members() -> str:
    header = DEFAULTS_HEADER.read_text(encoding="utf-8")
    match = re.search(r"struct AutoSoundDefaultsBlock\s*\{.*?^\};", header,
                      flags=re.S | re.M)
    assert match
    declarations = re.findall(
        r"^\s*(int|Sound\s*\*)\s*(val_\d+_|event_window_);\s*(?://.*)?$",
        match.group(), flags=re.M)
    assert tuple((name, kind) for kind, name in declarations) == tuple(
        (field, kind) for field, _, kind in COPY_MAP)
    return match.group()


def lifecycle_bodies() -> str:
    source = AUTO_SOURCE.read_text(encoding="utf-8")
    signatures = (
        "AutoSound::AutoSound()", "void AutoSound::close()",
        "void AutoSound::close2()", "void AutoSound::init()",
    )
    bodies = [extract_definition(source, signature) for signature in signatures]
    replacements = (
        ("AutoSound::AutoSound()", "AutoSoundProbe::AutoSoundProbe()"),
        ("void AutoSound::close()", "void AutoSoundProbe::close()"),
        ("void AutoSound::close2()", "void AutoSoundProbe::close2()"),
        ("void AutoSound::init()", "void AutoSoundProbe::init()"),
    )
    for index, body in enumerate(bodies):
        old, new = replacements[index]
        assert body.startswith(old)
        bodies[index] = body.replace(old, new, 1)
    return "\n\n".join(bodies)


def lifecycle_mutants() -> list[tuple[str, str]]:
    bodies = lifecycle_bodies()
    constructor = extract_definition(bodies, "AutoSoundProbe::AutoSoundProbe()")
    mutants = []
    fields = [field for field, _, _ in COPY_MAP]
    for slot, field in enumerate(fields):
        old = f"AutoSoundDefaults.{field}"
        kind = COPY_MAP[slot][2]
        candidates = [name for name, _, other_kind in COPY_MAP if other_kind == kind]
        wrong_field = candidates[(candidates.index(field) + 1) % len(candidates)]
        wrong = f"AutoSoundDefaults.{wrong_field}"
        assert constructor.count(old) == 1
        changed = constructor.replace(old, wrong, 1)
        mutants.append((f"wrong_default_slot_{slot:02d}",
                        bodies.replace(constructor, changed, 1)))
    assert len(mutants) == 37
    return mutants


LIFECYCLE_HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>

static const char *control_name = "/* CONTROL_NAME */";
static void fail(const char *expression) {
  std::fprintf(stderr, "CONTROL_FAIL[%s]: %s\n", control_name, expression);
  std::exit(98);
}
#define CHECK(expression) do { if (!(expression)) fail(#expression); } while (0)

struct Sound {};
// Host-width words allow literal integer-to-pointer casts to compile here.
// Values are opaque sentinels, compared only; no fabricated address is called.
/* DEFAULTS_DECLARATION */
AutoSoundDefaultsBlock AutoSoundDefaults;
class AutoSoundProbe {
public:
/* AUTO_MEMBERS */
  Sound *legacy_sound_20() const { return val_20_; }
  Sound *legacy_sound_31() const { return val_31_; }
  void close(); void close2(); void init();
  AutoSoundProbe();
};
/* LIFECYCLE_BODIES */

static void check_defaults(const AutoSoundProbe &object) {
/* DEFAULT_CHECKS */
}
static void check_zero(const AutoSoundProbe &object) {
/* ZERO_CHECKS */
}
int main() {
  Sound sound_values[37];
/* SET_DEFAULTS */
  AutoSoundProbe object;
  check_defaults(object);
  CHECK(object.legacy_sound_20() == AutoSoundDefaults.val_20_);
  CHECK(object.legacy_sound_31() == AutoSoundDefaults.val_31_);
  object.init(); check_zero(object);
  object.close(); check_defaults(object);
  object.init(); check_zero(object);
  object.close2(); check_defaults(object);
}
'''


def lifecycle_source(mutant: tuple[str, str] | None = None) -> str:
    name, bodies = mutant if mutant else ("actual", lifecycle_bodies())
    fields = []
    zeroes = []
    for field, index, kind in COPY_MAP:
        if kind == "int":
            fields.append(f"  CHECK(object.{field} == AutoSoundDefaults.{field});")
            zeroes.append(f"  CHECK(object.{field} == 0);")
        else:
            fields.append(f"  CHECK(object.{field} == AutoSoundDefaults.{field});")
            zeroes.append(f"  CHECK(object.{field} == nullptr);")
    source = LIFECYCLE_HARNESS.replace("/* CONTROL_NAME */", name)
    source = source.replace("/* AUTO_MEMBERS */", auto_members())
    source = source.replace("/* DEFAULTS_DECLARATION */", defaults_members())
    source = source.replace("/* LIFECYCLE_BODIES */", bodies)
    source = source.replace("/* DEFAULT_CHECKS */", "\n".join(fields))
    source = source.replace("/* ZERO_CHECKS */", "\n".join(zeroes))
    setters = []
    for field, index, kind in COPY_MAP:
        value = f"static_cast<uintptr_t>(0x1100 + {index} * 0x101)"
        if kind == "int":
            setters.append(f"  AutoSoundDefaults.{field} = static_cast<int>({value});")
        else:
            setters.append(f"  AutoSoundDefaults.{field} = &sound_values[{index}];")
    source = source.replace("/* SET_DEFAULTS */", "\n".join(setters))
    assert "/* " not in source
    return source


BUTTON_MUTATIONS = (
    "wrong_sound_slot31", "cache_parent_before_hook",
    "cache_parent_before_group", "cache_id_before_group",
    "publish_after_hook", "skip_initial_publish", "wrong_hook_id",
    "invert_sound_play_test", "skip_sound_play", "stale_group_id",
    "invert_group_admission", "guard_final_callback",
)


def button_bodies(mutant: str | None = None) -> tuple[str, str]:
    button = extract_definition(
        BUTTON_SOURCE.read_text(encoding="utf-8"),
        "void BaseButton::on_left_click(int, int)",
    )
    win = extract_definition(
        WIN_SOURCE.read_text(encoding="utf-8"),
        "void Win::dispatch_19(int a, int b, int alternate)",
    )
    button = button.replace(
        "void BaseButton::on_left_click(int, int)",
        "void BaseButtonProbe::on_left_click(int, int)", 1,
    )
    win = win.replace(
        "void Win::dispatch_19(int a, int b, int alternate)",
        "void Win::dispatch_19(int a, int b, int alternate)", 1,
    )
    if mutant == "wrong_sound_slot31":
        old, new = "legacy_sound_31()", "legacy_sound_20()"
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
    elif mutant == "cache_parent_before_hook":
        old = "    WinCallbackWindow = win_parent_;\n"
        new = "    Win *const sound_parent = win_parent_;\n" + old
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
        old = "        Win *const sound_parent = win_parent_;\n"
        assert button.count(old) == 1
        button = button.replace(old, "        \n", 1)
    elif mutant == "cache_parent_before_group":
        old = "        const int group_id = static_cast<int>(field_A78_);\n"
        new = "        Win *const notification_parent = win_parent_;\n" + old
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
        old = "            Win *const notification_parent = win_parent_;\n"
        assert button.count(old) == 1
        button = button.replace(old, "            \n", 1)
    elif mutant == "cache_id_before_group":
        old = "        const int group_id = static_cast<int>(field_A78_);\n"
        new = "        const int notification_id = static_cast<int>(field_A78_);\n" + old
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
        old = "            const int notification_id = static_cast<int>(field_A78_);\n"
        assert button.count(old) == 1
        button = button.replace(old, "            \n", 1)
    elif mutant == "publish_after_hook":
        old = "    WinCallbackWindow = win_parent_;\n"
        assert button.count(old) == 1
        button = button.replace(old, "", 1)
        old = "        Win *const sound_parent = win_parent_;\n"
        assert button.count(old) == 1
        button = button.replace(old, "        WinCallbackWindow = win_parent_;\n" + old, 1)
    elif mutant == "skip_initial_publish":
        old = "    WinCallbackWindow = win_parent_;\n"
        assert button.count(old) == 1
        button = button.replace(old, "", 1)
    elif mutant == "wrong_hook_id":
        old, new = "if (hook_id == -1)", "if (hook_id == -2)"
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
    elif mutant == "invert_sound_play_test":
        old, new = "owner->is_playing() == 0", "owner->is_playing() != 0"
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
    elif mutant == "skip_sound_play":
        old = "            owner->play();\n"
        assert button.count(old) == 1
        button = button.replace(old, "            \n", 1)
    elif mutant == "stale_group_id":
        old = "        const int group_id = static_cast<int>(field_A78_);"
        new = "        const int group_id = hook_id;"
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
    elif mutant == "invert_group_admission":
        old, new = "button_click(group_id) != 0", "button_click(group_id) == 0"
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
    elif mutant == "guard_final_callback":
        old, new = "if (field_AB0_ != 0)", "if (field_AAC_ == 0 && field_AB0_ != 0)"
        assert button.count(old) == 1
        button = button.replace(old, new, 1)
    elif mutant is not None:
        raise AssertionError(f"unknown named mutation {mutant}")
    return button, win


BUTTON_HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>

static const char *control_name = "/* CONTROL_NAME */";
static void fail(const char *expression) {
  std::fprintf(stderr, "CONTROL_FAIL[%s]: %s\n", control_name, expression);
  std::exit(98);
}
#define CHECK(expression) do { if (!(expression)) fail(#expression); } while (0)

struct Sound {
  int tag, playing, play_calls, status_calls;
  void (*on_play)(Sound *);
  Sound(int value=0): tag(value), playing(0), play_calls(0), status_calls(0), on_play(0) {}
  int is_playing() { ++status_calls; return playing; }
  void play() { ++play_calls; if (on_play) on_play(this); }
};
class AutoSoundProbe {
public:
/* AUTO_MEMBERS */
  Sound *legacy_sound_20() const { return val_20_; }
  Sound *legacy_sound_31() const { return val_31_; }
};

class Win;
static Win *expected_notification_parent=0;
static int expected_notification_id=0;
class Win : public AutoSoundProbe {
public:
  int tag, notification_count, notification_id;
  int iFlags_, iSomeFlag_, left_click_count, alt_count;
  void (*field_414_)(int, int);
  Win(int value=0): tag(value), notification_count(0), notification_id(0),
      iFlags_(0), iSomeFlag_(0), left_click_count(0), alt_count(0), field_414_(0) {}
  void on_button_clicked(int id) {
    CHECK(this==expected_notification_parent && id==expected_notification_id);
    ++notification_count; notification_id=id;
  }
  void on_left_click(int, int) { ++left_click_count; }
  void vslot_32(int, int) { ++alt_count; }
  void dispatch_19(int a, int b, int alternate);
  Sound *legacy_sound_31() const { return AutoSoundProbe::legacy_sound_31(); }
};

typedef void (*WinMouseEventCallback)(int, int);
typedef void (*WinNullaryHook)();
Win *WinCallbackWindow=0;
WinNullaryHook WinOkHook=0;
WinNullaryHook WinCancelHook=0;
class BaseButtonProbe;
BaseButtonProbe *WinPointerOwner3=0;
BaseButtonProbe *WinPointerOwner4=0;

struct ButtonGroupProbe {
  int calls, last_id, result;
  ButtonGroupProbe(): calls(0), last_id(0), result(1) {}
  int button_click(int id);
};
class BaseButtonProbe {
public:
  Win *win_parent_;
  int field_AAC_, field_A78_, lose_capture_calls;
  ButtonGroupProbe *group_;
  void (*field_AB0_)(int);
  BaseButtonProbe(): win_parent_(0), field_AAC_(0), field_A78_(0),
      lose_capture_calls(0), group_(0), field_AB0_(0) {}
  void on_lose_mouse_capture() { ++lose_capture_calls; }
  void on_left_click(int, int);
};

static BaseButtonProbe *active_button=0;
static Win *initial_parent=0;
static int hook_ok_calls=0, hook_cancel_calls=0, final_calls=0, final_id=0;
static Sound *sound_to_mutate=0;
static Win *after_hook_parent=0;
static Win *after_sound_parent=0;
static Win *after_group_parent=0;
static int group_mutation_id=0;
static void hook_ok() {
  CHECK(WinCallbackWindow == initial_parent);
  ++hook_ok_calls;
  active_button->win_parent_ = after_hook_parent;
  active_button->field_A78_ = -2; // hook selection must use its entry snapshot
}
static void hook_cancel() { ++hook_cancel_calls; }
static void sound_play_mutates(Sound *sound) {
  CHECK(sound == sound_to_mutate);
  active_button->win_parent_ = after_sound_parent;
  active_button->field_A78_ = group_mutation_id;
}
int ButtonGroupProbe::button_click(int id) {
  ++calls; last_id=id;
  active_button->win_parent_ = after_group_parent;
  active_button->field_A78_ = expected_notification_id;
  return result;
}
static void final_callback(int id) { ++final_calls; final_id=id; }
/* BASEBUTTON_BODY */
/* WIN_BODY */

static void reset_call_state() {
  WinCallbackWindow=0; WinOkHook=hook_ok; WinCancelHook=hook_cancel;
  WinPointerOwner3=0; WinPointerOwner4=0;
  hook_ok_calls=hook_cancel_calls=final_calls=final_id=0;
  expected_notification_parent=0; expected_notification_id=0;
  sound_to_mutate=0; after_hook_parent=after_sound_parent=after_group_parent=0;
  group_mutation_id=0; active_button=0; initial_parent=0;
}

static void test_basebutton_reloads() {
  reset_call_state();
  Win original_parent(10), hook_parent(11), sound_parent(12), group_parent(13);
  Sound old_sound(1), live_sound(2), wrong_slot_sound(3);
  original_parent.val_31_=&old_sound;
  hook_parent.val_31_=&live_sound;
  hook_parent.val_20_=&wrong_slot_sound;
  live_sound.on_play=sound_play_mutates;
  BaseButtonProbe button; ButtonGroupProbe group;
  button.win_parent_=&original_parent; button.field_A78_=-1;
  button.group_=&group; button.field_AB0_=final_callback;
  active_button=&button; initial_parent=&original_parent;
  after_hook_parent=&hook_parent; after_sound_parent=&sound_parent;
  after_group_parent=&group_parent;
  sound_to_mutate=&live_sound; group_mutation_id=24;
  expected_notification_parent=&group_parent; expected_notification_id=31;
  WinPointerOwner3=&button; WinPointerOwner4=&button;
  button.on_left_click(0,0);
  CHECK(WinCallbackWindow == &original_parent);
  CHECK(hook_ok_calls==1 && hook_cancel_calls==0);
  CHECK(button.lose_capture_calls==1 && WinPointerOwner3==0 && WinPointerOwner4==0);
  CHECK(live_sound.status_calls==1 && live_sound.play_calls==1);
  CHECK(old_sound.play_calls==0 && wrong_slot_sound.play_calls==0);
  CHECK(group.calls==1 && group.last_id==24);
  CHECK(group_parent.notification_count==1 && group_parent.notification_id==31);
  CHECK(original_parent.notification_count==0 && sound_parent.notification_count==0);
  CHECK(final_calls==1 && final_id==31);

  // The guard bypass is the safe original null-parent path; the final callback
  // remains outside that block. Do not exercise the original unguarded null
  // dereference after hook/sound mutation.
  reset_call_state();
  BaseButtonProbe bypass; bypass.field_AAC_=1; bypass.field_A78_=42;
  bypass.win_parent_=0; bypass.field_AB0_=final_callback;
  active_button=&bypass;
  bypass.on_left_click(0,0);
  CHECK(WinCallbackWindow==0);
  CHECK(hook_ok_calls==0 && hook_cancel_calls==0);
  CHECK(final_calls==1 && final_id==42);
}

static void test_win_sound20_receiver() {
  Win window(20); Sound sound20(20), wrong_sound8(8);
  window.val_20_=&sound20; window.val_8_=&wrong_sound8;
  window.dispatch_19(4,5,1);
  CHECK(window.alt_count==1 && window.left_click_count==0);
  CHECK(sound20.status_calls==1 && sound20.play_calls==1);
  CHECK(wrong_sound8.status_calls==0 && wrong_sound8.play_calls==0);
}

int main() {
  test_basebutton_reloads();
  test_win_sound20_receiver();
}
'''


def button_source(mutant: str | None = None) -> str:
    name = mutant or "actual"
    button, win = button_bodies(mutant)
    source = BUTTON_HARNESS.replace("/* CONTROL_NAME */", name)
    source = source.replace("/* AUTO_MEMBERS */", auto_members())
    source = source.replace("/* BASEBUTTON_BODY */", button)
    source = source.replace("/* WIN_BODY */", win)
    assert "/* " not in source
    return source


def compile_and_run(source: str, compiler: str,
                    stem: str, tmp_path: Path) -> subprocess.CompletedProcess:
    cpp, binary = tmp_path / f"{stem}.cpp", tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=20)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_autosound_lifecycle_copies_all_words_and_zeroes(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(lifecycle_source(), compiler, "autosound-lifecycle", tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_74_compile_success_named_wrong_default_slots_exit_98(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    mutants = lifecycle_mutants()
    assert len(mutants) == 37
    for name, bodies in mutants:
        result = compile_and_run(lifecycle_source((name, bodies)), compiler,
                                 f"{compiler_name}-{name}", tmp_path)
        diagnostic = f"CONTROL_FAIL[{name}]:"
        assert result.returncode == 98, f"{name}: {result.returncode} {result.stderr!r}"
        assert diagnostic in result.stderr, f"{name}: missing named failure diagnostic"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_button_reload_and_typed_win_sound20_consumers(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition(BUTTON_SOURCE.read_text(encoding="utf-8"),
                              "void BaseButton::on_left_click(int, int)")
    assert "WinCallbackWindow = win_parent_;" in body
    assert "const int hook_id = static_cast<int>(field_A78_);" in body
    assert "Win *const sound_parent = win_parent_;" in body
    assert "sound_parent->legacy_sound_31()" in body
    assert "const int group_id = static_cast<int>(field_A78_);" in body
    assert "Win *const notification_parent = win_parent_;" in body
    assert "const int notification_id = static_cast<int>(field_A78_);" in body
    assert "reinterpret_cast<Sound" not in body
    assert "WinCallbackWindow = win_parent_;" not in body[body.index("const int hook_id"):]
    assert "val_20_" in extract_definition(
        WIN_SOURCE.read_text(encoding="utf-8"),
        "void Win::dispatch_19(int a, int b, int alternate)")
    result = compile_and_run(button_source(), compiler, "button-actual", tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_24_compile_success_named_callback_mutants_exit_98(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    assert len(BUTTON_MUTATIONS) == 12
    for name in BUTTON_MUTATIONS:
        result = compile_and_run(button_source(name), compiler,
                                 f"{compiler_name}-{name}", tmp_path)
        diagnostic = f"CONTROL_FAIL[{name}]:"
        assert result.returncode == 98, f"{name}: {result.returncode} {result.stderr!r}"
        assert diagnostic in result.stderr, f"{name}: missing named failure diagnostic"


def test_pinned_full_button_span_and_zero_initialized_default_words():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    fallback = next((parent / ".opensmacx/game/terranx_original.exe"
                     for parent in (ROOT, *ROOT.parents)
                     if (parent / ".opensmacx/game/terranx_original.exe").is_file()),
                    ROOT / ".opensmacx/game/terranx_original.exe")
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", fallback))
    if not image_path.is_file():
        pytest.skip("pinned executable unavailable; set OPENSMACX_IMAGE")
    assert image_pin(ROOT) == PIN
    image = PeImage(image_path.read_bytes(), PIN)
    span = Span(0x607A20, 0x607AF3)
    code = image.read(span)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, span.start))
    assert len(code) == 0xD3
    assert len(instructions) == 65
    by_address = {instruction.address: (instruction.mnemonic, instruction.op_str)
                  for instruction in instructions}
    expected = {
        0x607A20: ("mov", "eax, dword ptr [0x9b7abc]"),
        0x607A4D: ("mov", "ecx, dword ptr [esi + 0xc4]"),
        0x607A53: ("mov", "dword ptr [0x9b7ab8], ecx"),
        0x607A63: ("mov", "eax, dword ptr [esi + 0xa78]"),
        0x607A89: ("mov", "edx, dword ptr [esi + 0xc4]"),
        0x607A90: ("mov", "edi, dword ptr [edx + 0x7c]"),
        0x607A9B: ("call", "dword ptr [eax + 0x5c]"),
        0x607AA6: ("call", "dword ptr [edx + 0x1c]"),
        0x607AB4: ("mov", "eax, dword ptr [esi + 0xa78]"),
        0x607ABB: ("call", "0x62b8a0"),
        0x607AC4: ("mov", "ecx, dword ptr [esi + 0xc4]"),
        0x607ACA: ("mov", "eax, dword ptr [esi + 0xa78]"),
        0x607AD3: ("call", "dword ptr [edx + 0xb0]"),
        0x607AD9: ("mov", "eax, dword ptr [esi + 0xab0]"),
        0x607AF0: ("ret", "8"),
    }
    for address, instruction in expected.items():
        assert by_address[address] == instruction
    assert by_address[0x607A75] == ("je", "0x607a89")
    assert by_address[0x607AC2] == ("je", "0x607ad9")
    defaults = image.read_initial(Span(0x9BC080, 0x9BC114))
    assert len(defaults) == 37 * 4
    words = struct.unpack("<37I", defaults)
    assert words == (0,) * 37
    assert words[19] == 0 and words[30] == 0

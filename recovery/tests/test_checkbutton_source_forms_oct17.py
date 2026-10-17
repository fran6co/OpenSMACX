"""Host controls for CheckButton::init's source-owned input/order contract."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "int CheckButton::init(int x, int y, GraphicWin *parent, int toggle_argument)"


def _function(source: str, signature: str) -> str:
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for position in range(opening, len(source)):
        depth += (source[position] == "{") - (source[position] == "}")
        if depth == 0:
            return source[start : position + 1]
    raise AssertionError(f"unclosed function: {signature}")


PRELUDE = r'''
#include <cassert>

enum { READ_UNCHECKED_HEIGHT = 1, READ_CHECKED_WIDTH = 2,
       CALL_INIT = 3, CALL_SHOW = 4 };
static int events[8];
static int event_count;
static void record(int event) { assert(event_count < 8); events[event_count++] = event; }
static void clear_events() { event_count = 0; }
static void expect_events(const int *expected, int count) {
  assert(event_count == count);
  for (int i = 0; i < count; ++i) assert(events[i] == expected[i]);
}

struct Sprite {
  int width_value;
  int height_value;
  int width_reads;
  int height_reads;
  int width_event;
  int height_event;
  int width() const {
    ++const_cast<Sprite *>(this)->width_reads;
    record(width_event);
    return width_value;
  }
  int height() const {
    ++const_cast<Sprite *>(this)->height_reads;
    record(height_event);
    return height_value;
  }
};

struct GraphicWin {
  int configured_result;
  int init_calls;
  int arg_x, arg_y, arg_width, arg_height, arg_zero_a, arg_flags;
  GraphicWin *arg_parent;
  int arg_zero_b, arg_zero_c;
  GraphicWin() : configured_result(0), init_calls(0), arg_x(0), arg_y(0),
      arg_width(0), arg_height(0), arg_zero_a(-1), arg_flags(0),
      arg_parent(0), arg_zero_b(-1), arg_zero_c(-1) {}
  int init(int x, int y, int width, int height, int zero_a, int flags,
           GraphicWin *parent, int zero_b, int zero_c) {
    ++init_calls;
    arg_x = x; arg_y = y; arg_width = width; arg_height = height;
    arg_zero_a = zero_a; arg_flags = flags; arg_parent = parent;
    arg_zero_b = zero_b; arg_zero_c = zero_c;
    record(CALL_INIT);
    return configured_result;
  }
  virtual void show(int) {}
};

struct CheckButton : GraphicWin {
  Sprite *checked_sprite_;
  Sprite *unchecked_sprite_;
  unsigned int toggle_argument_;
  int show_calls;
  int expected_toggle_on_show;
  CheckButton() : checked_sprite_(0), unchecked_sprite_(0),
      toggle_argument_(0), show_calls(0), expected_toggle_on_show(0) {}
  int run_init(int x, int y, GraphicWin *parent, int toggle_argument);
  void show(int value) override {
    assert(value == 0);
    ++show_calls;
    record(CALL_SHOW);
    // This is the state a re-entrant virtual show observer sees on entry.
    assert(toggle_argument_ == static_cast<unsigned int>(expected_toggle_on_show));
  }
};
'''

HARNESS = r'''
int main() {
  Sprite checked{317, 901, 0, 0, 0, 0};
  Sprite unchecked{227, 73, 0, 0, 0, 0};
  checked.width_event = READ_CHECKED_WIDTH;
  checked.height_event = 8;
  unchecked.width_event = 7;
  unchecked.height_event = READ_UNCHECKED_HEIGHT;

  // Null parent returns before reading either sprite or making calls.
  CheckButton null_case;
  null_case.checked_sprite_ = &checked;
  null_case.unchecked_sprite_ = &unchecked;
  null_case.toggle_argument_ = 0x11223344U;
  clear_events();
  assert(null_case.run_init(4, 5, 0, 0x55667788) == 3);
  assert(null_case.toggle_argument_ == 0x11223344U);
  assert(null_case.init_calls == 0 && null_case.show_calls == 0);
  assert(checked.width_reads == 0 && checked.height_reads == 0);
  assert(unchecked.width_reads == 0 && unchecked.height_reads == 0);
  assert(event_count == 0);

  // Failed base init propagates the result and leaves the toggle unpublished.
  CheckButton failed;
  failed.checked_sprite_ = &checked;
  failed.unchecked_sprite_ = &unchecked;
  failed.configured_result = 19;
  failed.toggle_argument_ = 0x11223344U;
  clear_events();
  assert(failed.run_init(11, 12, &failed, 0x55667788) == 19);
  assert(failed.init_calls == 1 && failed.show_calls == 0);
  assert(failed.toggle_argument_ == 0x11223344U);
  assert(failed.arg_x == 11 && failed.arg_y == 12);
  assert(failed.arg_width == 317 && failed.arg_height == 73);
  assert(failed.arg_zero_a == 0 && failed.arg_flags == 0x1020);
  assert(failed.arg_parent == &failed && failed.arg_zero_b == 0 && failed.arg_zero_c == 0);
  const int failed_events[] = {READ_UNCHECKED_HEIGHT, READ_CHECKED_WIDTH, CALL_INIT};
  expect_events(failed_events, 3);

  // On success, the exact checked-width/unchecked-height pair and all init
  // arguments are passed before the virtual show(0), which observes new state.
  CheckButton succeeded;
  succeeded.checked_sprite_ = &checked;
  succeeded.unchecked_sprite_ = &unchecked;
  succeeded.expected_toggle_on_show = 0x55667788;
  succeeded.toggle_argument_ = 0x11223344U;
  clear_events();
  assert(succeeded.run_init(-2, 29, &succeeded, 0x55667788) == 0);
  assert(succeeded.init_calls == 1 && succeeded.show_calls == 1);
  assert(succeeded.toggle_argument_ == 0x55667788U);
  assert(succeeded.arg_x == -2 && succeeded.arg_y == 29);
  assert(succeeded.arg_width == 317 && succeeded.arg_height == 73);
  assert(succeeded.arg_zero_a == 0 && succeeded.arg_flags == 0x1020);
  assert(succeeded.arg_parent == &succeeded && succeeded.arg_zero_b == 0 && succeeded.arg_zero_c == 0);
  const int success_events[] = {READ_UNCHECKED_HEIGHT, READ_CHECKED_WIDTH, CALL_INIT, CALL_SHOW};
  expect_events(success_events, 4);
  assert(checked.width_reads == 2 && checked.height_reads == 0);
  assert(unchecked.width_reads == 0 && unchecked.height_reads == 2);
}
'''


def _program(source: str, damage: str | None = None) -> str:
    body = _function(source, SIGNATURE).replace(SIGNATURE, "int CheckButton::run_init(int x, int y, GraphicWin *parent, int toggle_argument)", 1)
    if damage == "wrong_sprite_members":
        old = "const int height = unchecked_sprite_->height();\n    const int width = checked_sprite_->width();"
        new = "const int height = checked_sprite_->height();\n    const int width = unchecked_sprite_->width();"
    elif damage == "omit_parent_guard":
        old = "if (parent == 0) {\n        return 3;\n    }"
        new = "if (parent != 0) {\n        return 3;\n    }"
    elif damage == "lose_failure_result":
        old = "if (result != 0) {\n        return result;\n    }"
        new = "if (result != 0) {\n        return 0;\n    }"
    elif damage == "publish_toggle_before_init":
        old = "    if (result != 0) {\n        return result;\n    }\n\n    toggle_argument_ = toggle_argument;"
        new = "    toggle_argument_ = toggle_argument;\n    if (result != 0) {\n        return result;\n    }"
    else:
        old = new = ""
    if damage is not None:
        assert body.count(old) == 1, f"damage anchor changed: {damage}"
        body = body.replace(old, new, 1)
    return PRELUDE + "\n" + body + "\n" + HARNESS


def _compile_run(tmp_path: Path, compiler: str, source: str, damage: str | None = None):
    cpp = tmp_path / "checkbutton-init-control.cpp"
    exe = tmp_path / "checkbutton-init-control"
    cpp.write_text(_program(source, damage), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_init_body_preserves_inputs_order_failures_and_publication(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/jackal/checkbutton.cpp").read_text(encoding="utf-8")
    result = _compile_run(tmp_path, compiler, source)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (
    "wrong_sprite_members", "omit_parent_guard", "lose_failure_result",
    "publish_toggle_before_init",
))
def test_actual_init_body_controls_reject_behavioral_mutations(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = (ROOT / "src/jackal/checkbutton.cpp").read_text(encoding="utf-8")
    result = _compile_run(tmp_path, compiler, source, damage)
    assert result.returncode != 0, f"damage control survived: {damage}"


def _continuity_fields(record):
    history = tuple(
        (lesson.kind, lesson.key, lesson.prose,
         tuple(line for _, line in lesson.lines))
        for lesson in (record.origin or ())
    )
    return (
        record.address, record.name, record.mode, record.image_spans,
        record.semantic, record.kind, record.exclusion, record.levers,
        record.ruled_out, record.unrecoverable, record.deferred,
        record.source_role, record.source_role_reason, history,
    )


def _claim_preserving_fields(prior, current):
    from dataclasses import replace

    # Recorder publication may add an obligation; it must not withdraw one.
    assert not prior.byte_exact or current.byte_exact
    return _continuity_fields(replace(current, byte_exact=prior.byte_exact))


def test_original_identity_span_and_claim_continuity():
    from dataclasses import replace
    from match_recovery.source import Mode, read_file

    path = ROOT / "src/jackal/checkbutton.cpp"
    text = path.read_text(encoding="utf-8")
    marker = "// ORIGINAL: 0x006337E0 ?init@CheckButton@@QAEHHHPAUGraphicWin@@H@Z 0x006337E0-0x00633841"
    assert marker in text
    record, = (row for row in read_file(path) if row.address == 0x006337E0)
    assert record.name == "?init@CheckButton@@QAEHHHPAUGraphicWin@@H@Z"
    assert record.mode is Mode.BODY
    assert record.image_spans == ((0x006337E0, 0x00633841),)

    # Both baseline states are supported: a new recorder grant is allowed,
    # while a previously recorded obligation can never be withdrawn.
    for old_claim, new_claim in ((False, False), (False, True), (True, True)):
        prior = replace(record, byte_exact=old_claim)
        current = replace(record, byte_exact=new_claim)
        assert _claim_preserving_fields(prior, current) == _continuity_fields(prior)
        damaged_identity = replace(current, name=current.name + "_damaged")
        assert _claim_preserving_fields(prior, damaged_identity) != _continuity_fields(prior)
    claimed_prior = replace(record, byte_exact=True)
    withdrawn = replace(record, byte_exact=False)
    with pytest.raises(AssertionError):
        _claim_preserving_fields(claimed_prior, withdrawn)

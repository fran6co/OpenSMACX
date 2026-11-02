"""Host controls for ButtonGroup's original Win notification protocol.

The production member body is extracted verbatim from buttongroup.cpp. The
small host model supplies only the concrete members and virtual Win callbacks
that body uses; it checks callback sequencing, receiver reacquisition and the
group-context/prior/clicked argument order. It is not a native ABI or game test.
"""
from __future__ import annotations

import hashlib
from pathlib import Path
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "int ButtonGroup::button_click(int clicked_id)"


def find_pinned_image() -> Path:
    for parent in (ROOT, *ROOT.parents):
        for candidate in (parent / ".opensmacx/game/terranx_original.exe",
                          parent / "game/terranx_original.exe"):
            if candidate.is_file():
                return candidate
    raise AssertionError("pinned terranx_original.exe is unavailable")


def instruction_facts(image: PeImage, start: int, end: int):
    instructions = tuple(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(start, end)), start))
    assert sum(item.size for item in instructions) == end - start
    return [(item.mnemonic, item.op_str) for item in instructions]


def test_pinned_notification_arguments_and_post_callback_parent_reload():
    data = find_pinned_image().read_bytes()
    assert hashlib.sha256(data).hexdigest() == PINNED_SHA256
    image = PeImage(data, PINNED_SHA256)

    # init's first argument is stored as the group's +0x8C context.
    assert instruction_facts(image, 0x0062B832, 0x0062B838) == [
        ("mov", "dword ptr [edx + 0x8c], eax")]
    # First notification: prior active id, clicked id, then slot 47 (+0xBC).
    assert instruction_facts(image, 0x0062B9CE, 0x0062B9E0) == [
        ("mov", "ecx, dword ptr [esi]"), ("push", "edx"),
        ("push", "eax"), ("mov", "ecx, dword ptr [ecx + 0xc4]"),
        ("mov", "edi, dword ptr [ecx]"),
        ("call", "dword ptr [edi + 0xbc]")]
    # After that callback returns, current count is checked and IDs are
    # recomputed. The second path pushes clicked, prior, context, reloads the
    # first button's parent, then calls slot 46 (+0xB8).
    assert instruction_facts(image, 0x0062BA16, 0x0062BA2F) == [
        ("mov", "ecx, dword ptr [esi]"), ("push", "edx"),
        ("mov", "edx, dword ptr [esi + 0x8c]"), ("push", "eax"),
        ("mov", "ecx, dword ptr [ecx + 0xc4]"), ("push", "edx"),
        ("mov", "edi, dword ptr [ecx]"),
        ("call", "dword ptr [edi + 0xb8]")]


def test_pinned_flags_width_and_same_id_repeat_gate():
    data = find_pinned_image().read_bytes()
    assert hashlib.sha256(data).hexdigest() == PINNED_SHA256
    image = PeImage(data, PINNED_SHA256)
    assert instruction_facts(image, 0x0062B900, 0x0062B906) == [
        ("mov", "eax, dword ptr [esi + 0x88]")]
    assert instruction_facts(image, 0x0062B926, 0x0062B936) == [
        ("cmp", "dword ptr [esi + 0x84], edi"),
        ("jne", "0x62b936"), ("test", "al, 1"),
        ("je", "0x62ba2f")]


def extract_body() -> str:
    source = (ROOT / "src/buttongroup.cpp").read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)
    starts = []
    offset = 0
    while True:
        found = masked.find(SIGNATURE, offset)
        if found < 0:
            break
        starts.append(found)
        offset = found + 1
    assert len(starts) == 1, "expected exactly one production ButtonGroup body"
    start = starts[0]
    opening = masked.find("{", start + len(SIGNATURE))
    assert opening >= 0
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:index + 1]
                assert "BODY GOES HERE." not in body
                return body
    raise AssertionError("ButtonGroup::button_click body is unbalanced")


HOST_TYPES = r"""
#include <cstdint>
#include <cstdio>

struct Event { int kind, id, value; };
static Event events[64];
static int event_count;
static int failures;
static void record(int kind, int id, int value = 0) {
    events[event_count++] = Event{kind, id, value};
}
static void check(bool value, const char *label) {
    if (!value) {
        std::printf("DIAGNOSTIC: %s\n", label);
        ++failures;
    }
}

class ButtonGroup;
class Win {
public:
    virtual void on_group_clicked(int active, int clicked) = 0;
    virtual void on_group_clicked(int group, int active, int clicked) = 0;
};

struct BaseButton {
    uint32_t field_A78_;
    uint32_t field_A74_;
    int state;
    Win *parent_;
    BaseButton(uint32_t id, Win *parent)
        : field_A78_(id), field_A74_(0), state(-1), parent_(parent) {}
    void set(int value) {
        state = value;
        record(1, static_cast<int>(field_A78_), value);
    }
    void redraw() { record(2, static_cast<int>(field_A78_)); }
    Win *parent_window() const { return parent_; }
};

class ButtonGroup {
public:
    BaseButton *buttons_[32];
    uint32_t count_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    int button_click(int clicked_id);
    ButtonGroup() : count_(0), field_84_(0), field_88_(0), field_8C_(0),
                    field_90_(0) {
        for (int i = 0; i < 32; ++i) buttons_[i] = 0;
    }
};

class ProbeWin : public Win {
public:
    ButtonGroup *mutate_group;
    BaseButton *replacement_first;
    BaseButton *clicked_button;
    int zero_count_after_first;
    int two_calls;
    int three_calls;
    int two_active;
    int two_clicked;
    int three_group;
    int three_active;
    int three_clicked;
    ProbeWin() : mutate_group(0), replacement_first(0), clicked_button(0),
        zero_count_after_first(0), two_calls(0), three_calls(0),
        two_active(-1), two_clicked(-1), three_group(-1), three_active(-1),
        three_clicked(-1) {}
    void on_group_clicked(int active, int clicked) override {
        ++two_calls;
        two_active = active;
        two_clicked = clicked;
        record(3, active, clicked);
        if (mutate_group != 0) {
            if (replacement_first != 0)
                mutate_group->buttons_[0] = replacement_first;
            if (clicked_button != 0)
                clicked_button->field_A78_ = 88;
            if (zero_count_after_first)
                mutate_group->count_ = 0;
        }
    }
    void on_group_clicked(int group, int active, int clicked) override {
        ++three_calls;
        three_group = group;
        three_active = active;
        three_clicked = clicked;
        record(4, group, active);
        record(5, clicked, 0);
    }
};
"""


HOST_MAIN = r"""
static int run_protocol_cases() {
    // Ordinary click performs both real virtual overloads. The first callback
    // replaces button zero and edits the clicked ID; the second must observe
    // those live values and dispatch through the replacement's parent.
    ProbeWin first_owner;
    ProbeWin replacement_owner;
    BaseButton first(10, &first_owner);
    BaseButton clicked(20, &first_owner);
    BaseButton replacement(70, &replacement_owner);
    ButtonGroup group;
    group.buttons_[0] = &first;
    group.buttons_[1] = &clicked;
    group.count_ = 2;
    group.field_84_ = 10;
    group.field_8C_ = 55;
    clicked.field_A74_ = 1;
    first_owner.mutate_group = &group;
    first_owner.replacement_first = &replacement;
    first_owner.clicked_button = &clicked;

    check(group.button_click(20) == 0, "button_click result");
    check(group.field_84_ == 20, "selected ID publication");
    check(first.state == 0 && clicked.state == 1,
          "active/clicked state transitions");
    check(first_owner.two_calls == 1 && first_owner.three_calls == 0,
          "two-argument callback uses original owner only");
    check(first_owner.two_active == 10 && first_owner.two_clicked == 20,
          "two-argument callback keeps active/clicked order");
    check(replacement_owner.three_calls == 1,
          "three-argument callback reloads the post-callback parent");
    check(replacement_owner.three_group == 55 &&
          replacement_owner.three_active == 70 &&
          replacement_owner.three_clicked == 88,
          "three-argument callback receives context/prior/clicked");
    check(clicked.field_A74_ == 0,
          "dirty button flag is cleared after notifications");
    check(event_count == 8, "normal event count");
    if (event_count == 8) {
        check(events[0].kind == 1 && events[0].id == 10 && events[0].value == 0,
              "active button is cleared before callbacks");
        check(events[1].kind == 2 && events[1].id == 10,
              "active button redraw precedes callbacks");
        check(events[2].kind == 1 && events[2].id == 20 && events[2].value == 1,
              "clicked button is selected before callbacks");
        check(events[3].kind == 2 && events[3].id == 20,
              "clicked button redraw precedes callbacks");
        check(events[4].kind == 3 && events[4].id == 10 && events[4].value == 20,
              "two-argument notification precedes the second notification");
        check(events[5].kind == 4 && events[5].id == 55 && events[5].value == 70,
              "three-argument context and active ID");
        check(events[6].kind == 5 && events[6].id == 88,
              "three-argument clicked ID");
        check(events[7].kind == 2 && events[7].id == 88,
              "tail dirty-button redraw follows both notifications");
    }

    // Count is read after the first virtual callback; zero suppresses the
    // second overload and leaves the remaining button untouched by tail paint.
    event_count = 0;
    ProbeWin zero_owner;
    BaseButton zero_first(1, &zero_owner);
    BaseButton zero_clicked(2, &zero_owner);
    ButtonGroup shrinking;
    shrinking.buttons_[0] = &zero_first;
    shrinking.buttons_[1] = &zero_clicked;
    shrinking.count_ = 2;
    shrinking.field_84_ = 1;
    shrinking.field_8C_ = 9;
    zero_owner.mutate_group = &shrinking;
    zero_owner.zero_count_after_first = 1;
    zero_clicked.field_A74_ = 1;
    shrinking.button_click(2);
    check(zero_owner.two_calls == 1 && zero_owner.three_calls == 0,
          "post-callback zero count suppresses three-argument notification");
    check(zero_clicked.field_A74_ == 1,
          "zero count also suppresses the tail dirty-button pass");

    // Initial empty groups do not look up a parent or notify either overload.
    ProbeWin unused_owner;
    ButtonGroup empty;
    empty.count_ = 0;
    empty.field_90_ = 0;
    empty.button_click(7);
    check(unused_owner.two_calls == 0 && unused_owner.three_calls == 0,
          "initial zero count suppresses both notifications");

    // Independent table for same/different IDs and the two original flag bits.
    // High flag bits must neither alter the gate nor be overwritten.
    const int callbacks[2][4] = {{0, 1, 1, 1}, {1, 1, 1, 1}};
    const int selected[2][4] = {{10, 10, -1, -1}, {20, 20, 20, 20}};
    const int first_state[2][4] = {{-1, 1, 0, 0}, {0, 0, 0, 0}};
    const int second_state[2][4] = {{-1, -1, -1, -1}, {1, 1, 1, 1}};
    const uint32_t high_bits[] = {0u, 0xA5001000u};
    for (int different = 0; different != 2; ++different) {
        for (int mode = 0; mode != 4; ++mode) {
            for (unsigned high = 0; high != 2; ++high) {
                event_count = 0;
                ProbeWin owner;
                BaseButton a(10, &owner), b(20, &owner);
                ButtonGroup repeated;
                repeated.buttons_[0] = &a;
                repeated.buttons_[1] = &b;
                repeated.count_ = 2;
                repeated.field_84_ = 10;
                repeated.field_88_ = high_bits[high] | uint32_t(mode);
                repeated.field_8C_ = 55;
                b.field_A74_ = 1;
                repeated.button_click(different ? 20 : 10);
                check(owner.two_calls == callbacks[different][mode] &&
                      owner.three_calls == callbacks[different][mode],
                      "same/different-ID repeat gate truth table");
                check(repeated.field_84_ == uint32_t(selected[different][mode]),
                      "toggle and repeat selected ID");
                check(a.state == first_state[different][mode] &&
                      b.state == second_state[different][mode],
                      "toggle and repeat button state transitions");
                check(repeated.field_88_ == (high_bits[high] | uint32_t(mode)),
                      "full flag word remains unchanged");
                if (callbacks[different][mode]) {
                    check(owner.two_active == 10 &&
                          owner.two_clicked == selected[different][mode] &&
                          owner.three_group == 55 && owner.three_active == 10 &&
                          owner.three_clicked == selected[different][mode],
                          "repeat callbacks retain prior clicked and context order");
                }
                check(b.field_A74_ == 0 && event_count > 0 &&
                      events[event_count - 1].kind == 2 &&
                      events[event_count - 1].id == 20,
                      "dirty tail follows either repeat gate path");
            }
        }
    }
    return failures == 0 ? 0 : 1;
}

int main() { return run_protocol_cases(); }
"""


def compile_and_run(tmp_path: Path, body: str, name: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    source = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    source.write_text(HOST_TYPES + body + HOST_MAIN, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source), "-o",
         str(binary)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=20)


def test_actual_source_protocol_reloads_parent_and_preserves_callback_order(
        tmp_path: Path):
    result = compile_and_run(tmp_path, extract_body(), "button_group_contract")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("damage", ["old_argument_order", "cached_parent"])
def test_actual_source_controls_reject_each_old_defect_independently(
        tmp_path: Path, damage: str):
    body = extract_body()
    if damage == "old_argument_order":
        old = ("owner_again->on_group_clicked(static_cast<int>(field_8C_),\n"
               "                                               active_again, clicked_again);")
        new = ("owner_again->on_group_clicked(active_again, clicked_again,\n"
               "                                               static_cast<int>(field_8C_));")
    else:
        old = "Win *owner_again = buttons_[0]->parent_window();"
        new = "Win *owner_again = owner;"
    assert body.count(old) == 1
    damaged = body.replace(old, new)
    result = compile_and_run(tmp_path, damaged, f"damaged_{damage}")
    assert result.returncode != 0, (
        f"actual-source controls failed to detect {damage}\n"
        + result.stdout + result.stderr)


def test_actual_source_controls_reject_reversed_repeat_gate(tmp_path: Path):
    body = extract_body()
    needle = "(flags & 1) != 0"
    assert body.count(needle) == 1
    damaged = body.replace(needle, "(flags & 1) == 0", 1)
    result = compile_and_run(tmp_path, damaged, "damaged_repeat_gate")
    assert result.returncode != 0, "reversed repeat gate escaped controls"
    assert "same/different-ID repeat gate truth table" in result.stdout

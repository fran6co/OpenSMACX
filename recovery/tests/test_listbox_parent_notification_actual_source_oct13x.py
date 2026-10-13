"""Host controls for ListBox's real Win-parent callback paths.

The C++ bodies are extracted verbatim from listbox.cpp. The small host model
preserves ListBox's virtual GraphicWin/Dialog bases and gives AutoSound decoy
words so the old +0x50/+0x7c interpretation cannot accidentally pass.
These controls do not establish the native VC6 ABI or whole-game behavior.
"""
from __future__ import annotations

import hashlib
from pathlib import Path
import shutil
import struct
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def find_pinned_image() -> Path:
    for parent in (ROOT, *ROOT.parents):
        for candidate in (parent / ".opensmacx/game/terranx_original.exe",
                          parent / "game/terranx_original.exe"):
            if candidate.is_file():
                return candidate
    raise AssertionError("pinned terranx_original.exe is unavailable")


def read_pe_va(image: bytes, va: int, size: int) -> bytes:
    pe = struct.unpack_from("<I", image, 0x3C)[0]
    section_count = struct.unpack_from("<H", image, pe + 6)[0]
    optional_size = struct.unpack_from("<H", image, pe + 20)[0]
    optional = pe + 24
    image_base = struct.unpack_from("<I", image, optional + 28)[0]
    rva = va - image_base
    sections_at = optional + optional_size
    for index in range(section_count):
        section = sections_at + 40 * index
        virtual_size, virtual_address, raw_size, raw_at = struct.unpack_from(
            "<IIII", image, section + 8)
        if virtual_address <= rva and rva + size <= virtual_address + max(
                virtual_size, raw_size):
            offset = raw_at + rva - virtual_address
            return image[offset:offset + size]
    raise AssertionError(f"VA 0x{va:08X} is not backed by a PE section")


def test_pinned_receiver_chain_and_parent_null_branch():
    image_path = find_pinned_image()
    image = image_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == PINNED_SHA256

    # Complete original spans: the first eight bytes of each scroll body are
    # the receiver adjustor; the remaining bytes read Win+0x98/+0xC4 and
    # dispatch the parent's slots 8/9. The double-click body includes all
    # four receiver reloads and the 0x60C74D -> 0x60C7B6 null exit.
    native_spans = {
        0x0060CED0: bytes.fromhex(
            "2B49FC E908000000 CC CC CC CC CC CC CC CC "
            "8B41B8 8B5004 8D040A 8A4C0A50 F6C140 740C "
            "8B487C 85C9 7405 8B01 FF6020 33C0 C3"),
        0x0060CF10: bytes.fromhex(
            "2B49FC E908000000 CC CC CC CC CC CC CC CC "
            "8B41B8 8B5004 8D040A 8A4C0A50 F6C140 7414 "
            "8B487C 85C9 740D 8B542404 8B01 52 FF5024 C20400 "
            "8B442404 C20400"),
        0x0060C710: bytes.fromhex(
            "56 8BF1 57 8B46B8 8B4804 8B54317C 8915B87A9B00 "
            "8B46F8 85C0 7416 8B46B8 8B4808 8D4C31B8 E816D3FFFF "
            "50 FF56F8 83C404 8B46B8 8B5004 8B7C327C 85FF 7467 "
            "8B4008 53 8B1F 8D4C30B8 E8F2D2FFFF 50 8BCF FF93E0000000 "
            "8B46B8 8B15EC8E9B00 52 8B4804 8B4008 8B7C317C "
            "8D4C30B8 8B1F E8CAD2FFFF 50 8BCF FF93DC000000 "
            "8B4EB8 5B 8B5104 8B44327C 8BB094000000 85F6 7412 "
            "8B16 8BCE FF525C 85C0 7507 8B06 8BCE FF501C 5F5E C20800"),
        0x0060CFC0: bytes.fromhex("2B49FC E948F7FFFF"),
    }
    for address, expected in native_spans.items():
        assert read_pe_va(image, address, len(expected)) == expected, (
            f"pinned original bytes changed at 0x{address:08X}")

    # Constructor table writes anchor the receiver chain: ListBox installs
    # the Win-view and Dialog-view tables, and writes the GraphicWin vtordisp.
    constructor_witnesses = {
        0x00609DE1: bytes.fromhex("C70684056700"),
        0x00609E08: bytes.fromhex("C704311C046700"),
        0x00609E24: bytes.fromhex("C7043208046700"),
        0x00609E33: bytes.fromhex("894C30FC"),
    }
    for address, expected in constructor_witnesses.items():
        assert read_pe_va(image, address, len(expected)) == expected


def extract_function(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]),
        source,
    )
    starts = []
    offset = 0
    while True:
        found = masked.find(signature, offset)
        if found < 0:
            break
        starts.append(found)
        offset = found + 1
    assert len(starts) == 1, f"expected one code-visible body: {signature}"
    start = starts[0]
    opening = masked.find("{", start + len(signature))
    assert opening >= 0
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated C++ function: {signature}")


SIGNATURES = (
    "Scroll *ListBox::on_scroll_create()",
    "Scroll *ListBox::on_scroll_delete(Scroll *scroll)",
    "void ListBox::on_left_double_click(int, int)",
)


def actual_bodies() -> list[str]:
    source = (ROOT / "src/listbox.cpp").read_text(encoding="utf-8")
    bodies = [extract_function(source, signature) for signature in SIGNATURES]
    for body in bodies:
        assert "BODY GOES HERE." not in body
        assert body.rstrip().endswith("}")
    return bodies


HOST_TYPES = r"""
#include <cstdio>

struct Scroll { int tag; };
struct Win;
struct ListBox;
struct Sound {
    int playing;
    int play_calls;
    Sound() : playing(0), play_calls(0) {}
    int is_playing() const { return playing; }
    void play() { ++play_calls; }
};
struct AutoSound {
    unsigned int decoy_flags_50;
    Win *decoy_parent_7c;
    Sound *sound_37;
    int sound_reads;
    AutoSound() : decoy_flags_50(0), decoy_parent_7c(0), sound_37(0),
                  sound_reads(0) {}
    Sound *legacy_sound_37() { ++sound_reads; return sound_37; }
};
struct Buffer {};
struct Dialog {
    int selected_id;
    Dialog() : selected_id(0) {}
    int get_selected_id() { return selected_id; }
};
struct Win : AutoSound {
    unsigned int iFlags_;
    Win *win_parent_;
    Scroll *create_result;
    Scroll *delete_result;
    Scroll *last_deleted;
    int create_calls;
    int delete_calls;
    int single_calls;
    int double_calls;
    int single_id;
    int double_id;
    int double_guard;
    ListBox *mutate_list;
    Win *single_parent_after;
    Win *double_parent_after;
    int single_id_after;
    int double_id_after;
    int single_guard_after;
    int double_guard_after;
    Win() : iFlags_(0), win_parent_(0), create_result(0), delete_result(0),
            last_deleted(0), create_calls(0), delete_calls(0), single_calls(0),
            double_calls(0), single_id(-1), double_id(-1), double_guard(-1),
            mutate_list(0), single_parent_after(0), double_parent_after(0),
            single_id_after(0), double_id_after(0), single_guard_after(0),
            double_guard_after(0) {}
    Win *parent_window() const { return win_parent_; }
    virtual Scroll *on_listbox_scroll_create() {
        ++create_calls; return create_result;
    }
    virtual Scroll *on_listbox_scroll_delete(Scroll *scroll) {
        ++delete_calls; last_deleted = scroll; return delete_result;
    }
    virtual void on_double_clicked(int id);
    virtual void on_double_clicked(int id, int guard);
};
struct GraphicWin : Win, Buffer {};
typedef void (*WinNotificationCallback)(int);
struct ListBox : public virtual GraphicWin, public virtual Dialog {
    WinNotificationCallback double_click_callback_;
    ListBox() : double_click_callback_(0) {}
    virtual Scroll *on_scroll_create();
    virtual Scroll *on_scroll_delete(Scroll *scroll);
    virtual void on_left_double_click(int, int);
};

static Win *WinCallbackWindow = 0;
static int ListBoxClickGuard = 0;
static int null_exit_oracle_calls = 0;
static void missing_null_return_trap() { ++null_exit_oracle_calls; }
static bool good = true;
static void check(bool value, const char *label) {
    if (!value) {
        std::printf("DIAGNOSTIC: %s\n", label);
        good = false;
    }
}

void Win::on_double_clicked(int id) {
    ++single_calls;
    single_id = id;
    if (mutate_list != 0) {
        mutate_list->win_parent_ = single_parent_after;
        mutate_list->selected_id = single_id_after;
        ListBoxClickGuard = single_guard_after;
    }
}
void Win::on_double_clicked(int id, int guard) {
    ++double_calls;
    double_id = id;
    double_guard = guard;
    if (mutate_list != 0) {
        mutate_list->win_parent_ = double_parent_after;
        mutate_list->selected_id = double_id_after;
        ListBoxClickGuard = double_guard_after;
    }
}

struct CallbackPlan {
    ListBox *list;
    Win *initial_parent;
    Win *next_parent;
    int next_id;
    int next_guard;
    int calls;
    int id;
    bool saw_initial_window;
    CallbackPlan() : list(0), initial_parent(0), next_parent(0), next_id(0),
                     next_guard(0), calls(0), id(-1),
                     saw_initial_window(false) {}
};
static CallbackPlan *callback_plan = 0;
static void installed_callback(int id) {
    CallbackPlan &plan = *callback_plan;
    ++plan.calls;
    plan.id = id;
    plan.saw_initial_window = WinCallbackWindow == plan.initial_parent;
    plan.list->win_parent_ = plan.next_parent;
    plan.list->selected_id = plan.next_id;
    ListBoxClickGuard = plan.next_guard;
}

static bool test_scroll_parent_contract() {
    ListBox list;
    Win correct_parent;
    Win decoy_parent;
    Scroll expected_create = { 11 };
    Scroll expected_delete = { 22 };
    Scroll removed = { 33 };
    correct_parent.create_result = &expected_create;
    correct_parent.delete_result = &expected_delete;
    decoy_parent.create_result = &removed;
    decoy_parent.delete_result = &removed;
    list.win_parent_ = &correct_parent;
    list.decoy_parent_7c = &decoy_parent;
    list.iFlags_ = 0x40;
    list.decoy_flags_50 = 0;

    check(list.on_scroll_create() == &expected_create,
          "scroll-create must use Win flags and parent");
    check(correct_parent.create_calls == 1 && decoy_parent.create_calls == 0,
          "scroll-create called the wrong receiver");
    check(list.on_scroll_delete(&removed) == &expected_delete,
          "scroll-delete must return the parent's result");
    check(correct_parent.delete_calls == 1 &&
          correct_parent.last_deleted == &removed &&
          decoy_parent.delete_calls == 0,
          "scroll-delete must forward the exact Scroll through Win parent");

    list.iFlags_ = 0;
    check(list.on_scroll_create() == 0,
          "disabled Win scroll flag must return null");
    check(list.on_scroll_delete(&removed) == &removed,
          "disabled Win scroll flag must return its input");
    list.iFlags_ = 0x40;
    list.win_parent_ = 0;
    check(list.on_scroll_create() == 0,
          "null Win parent must return null for scroll-create");
    check(list.on_scroll_delete(&removed) == &removed,
          "null Win parent must return its input for scroll-delete");
    return good;
}

static bool test_reloaded_parent_chain() {
    ListBox list;
    Win initial_parent;
    Win first_parent;
    Win second_parent;
    Win sound_parent;
    Sound sound;
    list.selected_id = 11;
    list.win_parent_ = &initial_parent;
    list.double_click_callback_ = installed_callback;
    list.decoy_parent_7c = &initial_parent;
    list.decoy_flags_50 = 0;
    // Keep the cached-parent mutant on valid objects: if it dispatches the
    // first notification to the initially acquired parent, that callback
    // advances the chain to the same valid first_parent used below.
    initial_parent.mutate_list = &list;
    initial_parent.single_parent_after = &first_parent;
    initial_parent.single_id_after = 23;
    initial_parent.single_guard_after = 24;

    CallbackPlan plan;
    plan.list = &list;
    plan.initial_parent = &initial_parent;
    plan.next_parent = &first_parent;
    plan.next_id = 22;
    plan.next_guard = 31;
    callback_plan = &plan;

    first_parent.mutate_list = &list;
    first_parent.single_parent_after = &second_parent;
    first_parent.single_id_after = 33;
    first_parent.single_guard_after = 42;
    first_parent.double_parent_after = &second_parent;
    second_parent.sound_37 = &sound;
    second_parent.mutate_list = &list;
    second_parent.double_parent_after = &sound_parent;
    second_parent.double_id_after = 44;
    second_parent.double_guard_after = 53;
    sound_parent.sound_37 = &sound;
    ListBoxClickGuard = 7;
    WinCallbackWindow = 0;

    list.on_left_double_click(4, 5);

    check(plan.calls == 1 && plan.id == 11 && plan.saw_initial_window,
          "installed callback must see the acquired selection and initial window");
    check(first_parent.single_calls == 1 && first_parent.single_id == 22,
          "first notification must reload parent and selection after installed callback");
    check(second_parent.double_calls == 1 && second_parent.double_id == 33 &&
          second_parent.double_guard == 42,
          "second notification must reacquire parent, ID, and guard after first notification");
    check(sound_parent.sound_reads == 1 && sound.play_calls == 1,
          "audio must reload the parent after second notification");
    check(initial_parent.single_calls == 0 && initial_parent.double_calls == 0 &&
          first_parent.double_calls == 0 && second_parent.sound_reads == 0,
          "callbacks must not reuse a stale parent");
    callback_plan = 0;
    return good;
}

static bool test_post_callback_null_exit() {
    ListBox list;
    Win initial_parent;
    Win safe_later_parent;
    list.selected_id = 81;
    list.win_parent_ = &initial_parent;
    list.double_click_callback_ = installed_callback;
    // A stale-parent mutant must remain safe even though the correct source
    // exits after the callback nulls the parent. If it incorrectly notifies
    // this initially acquired parent, redirect later reloads to a valid host
    // object instead of using a null/invalid sentinel as a crash oracle.
    initial_parent.mutate_list = &list;
    initial_parent.single_parent_after = &safe_later_parent;
    initial_parent.single_id_after = 83;
    initial_parent.single_guard_after = 84;
    CallbackPlan plan;
    plan.list = &list;
    plan.initial_parent = &initial_parent;
    plan.next_parent = 0;
    plan.next_id = 82;
    plan.next_guard = 99;
    callback_plan = &plan;
    null_exit_oracle_calls = 0;
    initial_parent.sound_37 = 0;
    list.on_left_double_click(0, 0);
    check(plan.calls == 1 && plan.id == 81 && plan.saw_initial_window,
          "installed callback still runs before the null-parent check");
    check(initial_parent.single_calls == 0 && initial_parent.double_calls == 0 &&
          initial_parent.sound_reads == 0,
          "post-callback null parent must skip the complete notification/audio block");
    check(null_exit_oracle_calls == 0,
          "post-callback null path must take the original early exit");
    callback_plan = 0;
    return good;
}

int main() {
    test_scroll_parent_contract();
    test_reloaded_parent_chain();
    test_post_callback_null_exit();
    return good ? 0 : 98;
}
"""


def mutate_bodies(bodies: list[str], name: str) -> list[str]:
    changed = list(bodies)
    if name == "oldflagword":
        changed[0] = changed[0].replace("iFlags_", "decoy_flags_50")
        changed[1] = changed[1].replace("iFlags_", "decoy_flags_50")
    elif name == "wrongparentword":
        changed[0] = changed[0].replace("win_parent_", "decoy_parent_7c")
        changed[1] = changed[1].replace("win_parent_", "decoy_parent_7c")
    elif name == "removednullreturn":
        changed[2] = changed[2].replace(
            "if (target == 0) return;",
            "if (target == 0) { missing_null_return_trap(); return; }",
            1,
        )
    elif name == "cachedparent":
        changed[2] = changed[2].replace(
            "target = graphic->parent_window();\n    if (target == 0) return;",
            "target = target;\n    if (target == 0) return;",
            1,
        )
    else:
        raise AssertionError(f"unknown mutant: {name}")
    assert changed != bodies, f"mutant did not change source: {name}"
    return changed


def compile_and_run(tmp_path: Path, compiler: str, bodies: list[str],
                    should_pass: bool, label: str) -> None:
    source = HOST_TYPES + "\n\n" + "\n\n".join(bodies)
    cpp = tmp_path / f"{label}.cpp"
    binary = tmp_path / label
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, (
        f"{compiler} failed to compile {label}:\n{built.stdout}\n{built.stderr}")
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if should_pass:
        assert ran.returncode == 0, (
            f"{label} failed: rc={ran.returncode}\n{ran.stdout}\n{ran.stderr}")
    else:
        assert ran.returncode == 98, (
            f"{label} did not reach the named runtime diagnostic: "
            f"rc={ran.returncode}\n{ran.stdout}\n{ran.stderr}")
        assert "DIAGNOSTIC:" in ran.stdout


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_listbox_callbacks_and_named_mutants(tmp_path: Path,
                                                     compiler_name: str):
    compiler = shutil.which(compiler_name)
    assert compiler is not None, f"{compiler_name} is required for this host control"
    bodies = actual_bodies()
    compile_and_run(tmp_path, compiler, bodies, should_pass=True,
                    label=f"{compiler_name}_source")

    for mutant in ("oldflagword", "wrongparentword", "removednullreturn",
                   "cachedparent"):
        compile_and_run(tmp_path, compiler, mutate_bodies(bodies, mutant),
                        should_pass=False,
                        label=f"{compiler_name}_{mutant}")

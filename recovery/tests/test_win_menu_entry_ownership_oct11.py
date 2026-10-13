"""Independent callable entry ownership; not runtime or full-image proof."""
from dataclasses import replace
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text, Mode, State, DecompilationState
from match_recovery.source.writer import write_bytes, write_symbol
from match_recovery.binding_source import PREFIX, parse
from match_recovery.tasks import Task

ROOT = Path(__file__).resolve().parents[2]
BASE = "7850ec596c1a3d2fb37071e4f46634440aefbe0a"
MIGRATION_SNAPSHOT = "82ee8966fae8ea56548cba4a4123d7b931ab0160"


def test_writer_migration_preserves_all_old_spans_claims_and_cpp():
    old = {}
    snapshot = {}
    current = {}
    for name in ("menu", "win"):
        path = ROOT / ("src/" + name + ".cpp")
        before = subprocess.check_output(["git", "show", BASE + ":" + str(path.relative_to(ROOT))], text=True)
        expected = subprocess.check_output(["git", "show", MIGRATION_SNAPSHOT + ":" + str(path.relative_to(ROOT))], text=True)
        after = path.read_text()
        old[name] = read_text(before, path)
        snapshot[name] = read_text(expected, path)
        current[name] = read_text(after, path)
        mask = lambda text: re.sub(r"\s+", "", COMMENTS_OR_STRINGS.sub("", text))
        assert mask(before) == mask(expected)
        bindings = parse(expected, snapshot[name])
        if name == "menu":
            record, = (r for r in old[name] if r.address == 0x5FAEE0)
            emitted = write_bytes(before.encode(), [replace(record, image_spans=((0x5FAEE0, 0x5FAEEE),))]).decode()
            original_binding = before.splitlines()[record.line]
            current_binding = expected.splitlines()[record.line]
            assert original_binding.startswith(PREFIX) and current_binding.startswith(PREFIX)
            prior = json.loads(original_binding[len(PREFIX):])
            actual = bindings[record.line]
            assert actual["evidence"][:len(prior["evidence"])] == prior["evidence"]
            assert actual["reason"].startswith(prior["reason"])
            assert "5F0520..5F0535" in " ".join(actual["evidence"])
            assert emitted.replace(original_binding, current_binding, 1) == expected
        else:
            line = before.splitlines().index("void Win::update_menus() {") + 1
            added = DecompilationState(address=0x5F0520, mode=Mode.FILE, state=State.IMPLEMENTED,
                path=path, line=line, name="sub_5f0520", image_spans=((0x5F0520, 0x5F0535),))
            recorded, = (r for r in snapshot[name] if r.address == 0x5F0520)
            added = replace(added, byte_exact=recorded.byte_exact)
            emitted = write_bytes(before.encode(), [added])
            selected, = (r for r in read_text(emitted.decode(), path) if r.address == 0x5F0520)
            emitted = write_symbol(emitted, selected, "?update_menus@Win@@SAXXZ").decode()
            added_binding = expected.splitlines()[line]
            assert added_binding.startswith(PREFIX)
            assert emitted.replace("// symbol    ?update_menus@Win@@SAXXZ",
                added_binding + "\n// symbol    ?update_menus@Win@@SAXXZ", 1) == expected
    caller, = (r for r in current["menu"] if r.address == 0x5FAEE0)
    callee, = (r for r in current["win"] if r.address == 0x5F0520)
    snapshot_caller, = (r for r in snapshot["menu"] if r.address == 0x5FAEE0)
    snapshot_callee, = (r for r in snapshot["win"] if r.address == 0x5F0520)
    original, = (r for r in old["menu"] if r.address == 0x5FAEE0)
    assert caller.name == original.name and caller.byte_exact == original.byte_exact
    assert caller.semantic == original.semantic
    assert caller.image_spans + callee.image_spans == original.image_spans
    # The independent callee must acquire its own measured native-record
    # obligation before the former combined range is mechanically covered.
    assert callee.byte_exact and not callee.semantic
    assert (caller.name, caller.byte_exact, caller.semantic, caller.image_spans) == (
        snapshot_caller.name, snapshot_caller.byte_exact, snapshot_caller.semantic,
        snapshot_caller.image_spans)
    assert (callee.name, callee.byte_exact, callee.semantic, callee.image_spans) == (
        snapshot_callee.name, snapshot_callee.byte_exact, snapshot_callee.semantic,
        snapshot_callee.image_spans)
    assert {r.address for r in old["menu"]} == {r.address for r in current["menu"]}
    assert {r.address for r in old["win"]} | {0x5F0520} == {r.address for r in current["win"]}


def test_pinned_independent_callee_and_tail_transfer():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ""))
    assert image_path.is_file(), "OPENSMACX_IMAGE must name the pinned executable explicitly"
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    image = PeImage(image_bytes, image_pin(ROOT))
    decode = lambda low, high: [(i.address, i.mnemonic, i.op_str) for i in
        Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(low, high)), low)]
    assert decode(0x5FAEE0, 0x5FAEEE) == [
        (0x5FAEE0, "mov", "eax, dword ptr [esp + 4]"),
        (0x5FAEE4, "mov", "dword ptr [0x9b7b94], eax"),
        (0x5FAEE9, "jmp", "0x5f0520")]
    assert decode(0x5F0520, 0x5F0535) == [
        (0x5F0520, "push", "0"), (0x5F0522, "push", "0x5f0540"),
        (0x5F0527, "call", "dword ptr [0x66913c]"), (0x5F052D, "push", "eax"),
        (0x5F052E, "call", "dword ptr [0x669308]"), (0x5F0534, "ret", "")]


def test_task_both_owners_and_original_baseline():
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/win-menu-entry-ownership-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert {s.address for s in task.subjects} == {0x5F0520, 0x5FAEE0}
    assert {"src/menu.cpp", "src/win.cpp"} <= set(task.editable_files)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_setter_publishes_before_thread_window_callback(compiler_name, tmp_path):
    from recovery.tests.test_texture_sub61df60_actual_source import definition
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    bodies = definition((ROOT / "src/menu.cpp").read_text(), "Menu::set_def_font") + "\n" + (
        definition((ROOT / "src/win.cpp").read_text(), "Win::update_menus"))
    source = """
#include <cassert>
#define __stdcall
#define CALLBACK __stdcall
struct HWND__ {};
typedef HWND__ *HWND;
typedef long LPARAM;
typedef int BOOL;
typedef BOOL (CALLBACK *WNDENUMPROC)(HWND, LPARAM);
struct Font {};
static Font *MenuDefaultFont;
static Font *expected_font;
static int phase;
struct Win {
    static void update_menus();
    static int __stdcall adjust_menus(HWND, LPARAM) { return 1; }
};
struct Menu { static void set_def_font(Font *); };
static unsigned GetCurrentThreadId() { assert(phase==0 && MenuDefaultFont==expected_font); phase=1; return 37; }
static BOOL EnumThreadWindows(unsigned thread, WNDENUMPROC callback, LPARAM context) {
    assert(phase==1 && thread==37 && callback==&Win::adjust_menus && context==0);
    phase=2; return 0;
}
"""
    source += bodies + """
int main() {
    Font first, second;
    expected_font=&first; Menu::set_def_font(&first); assert(phase==2);
    phase=0; expected_font=&second; Menu::set_def_font(&second); assert(phase==2);
    phase=0; expected_font=0; Menu::set_def_font(0); assert(phase==2);
}
"""
    cpp, binary = tmp_path / "control.cpp", tmp_path / "control"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++98", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert run.returncode == 0, run.stderr

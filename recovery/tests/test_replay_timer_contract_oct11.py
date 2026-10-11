"""Original replay callback ABI and actual source bridge, not game proof."""
from dataclasses import fields
import json
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "db92cacc2d789d6c96196a3d0258b02870ae9b83"
FIXTURE = Path(__file__).with_name("replay-timer-contract-oct11.patch")


@pytest.mark.parametrize("relative", ("src/replaywin.cpp", "src/replaywin.h"))
def test_only_authorized_contract_migration(relative):
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT, text=True)
    section = next(p for p in FIXTURE.read_text().split("diff --git ")[1:]
                   if p.startswith("a/" + relative + " b/" + relative + "\n"))
    for hunk in section.split("\n@@ ")[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(l[1:] for l in lines if l.startswith((" ", "-")))
        after = "".join(l[1:] for l in lines if l.startswith((" ", "+")))
        assert before and old.count(before) == 1
        old = old.replace(before, after, 1)
    assert old == (ROOT / relative).read_text()


def test_every_original_identity_span_claim_and_lesson_preserved():
    path = ROOT / "src/replaywin.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/replaywin.cpp"], cwd=ROOT, text=True)
    before = read_text(old, path)
    after = read_text(path.read_text(), path)
    assert [r.address for r in before] == [r.address for r in after]
    for a, b in zip(before, after):
        for field in fields(a):
            if field.name in {"line", "region", "origin"}:
                continue
            expected = "?timer_callback_daemon@ReplayWin@@SAXH@Z" if (
                a.address == 0x5ADBD0 and field.name == "symbol") else getattr(a, field.name)
            assert expected == getattr(b, field.name)
        lessons = lambda r: [(l.kind, l.key, l.prose, tuple(t for _, t in l.lines)) for l in r.origin]
        assert lessons(a) == lessons(b)
    callback = next(r for r in after if r.address == 0x5ADBD0)
    assert callback.byte_exact and callback.image_spans == ((0x5ADBD0, 0x5ADBE1),)
    assert "fn_005adbd0" not in path.read_text()


def test_pinned_callback_entry_stack_null_guard_and_direct_registration():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(decoder.disasm(image.read(Span(0x5ADBD0, 0x5ADBE1)), 0x5ADBD0))
    assert sum(i.size for i in instructions) == 17
    assert [(i.mnemonic, i.op_str) for i in instructions] == [
        ("push", "ebp"), ("mov", "ebp, esp"), ("mov", "ecx, dword ptr [ebp + 8]"),
        ("test", "ecx, ecx"), ("je", "0x5adbdf"), ("call", "0x5ad9e0"),
        ("pop", "ebp"), ("ret", "")]
    registration = list(decoder.disasm(image.read(Span(0x5AD720, 0x5AD99A)), 0x5AD720))
    assert any(i.mnemonic == "push" and i.op_str == "0x5adbd0" for i in registration)
    assert any(i.mnemonic == "call" and i.op_str == "0x616350" for i in registration)


def program():
    callback = extract((ROOT / "src/replaywin.cpp").read_text(),
                       "void __cdecl ReplayWin::timer_callback_daemon(int context)")
    return r'''#include <new>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include <sys/mman.h>
#define __cdecl
class ReplayWin {
public:
    int calls;
    void timer_callback() { ++calls; }
    static void timer_callback_daemon(int context);
};
CALLBACK
static_assert(std::is_same<decltype(&ReplayWin::timer_callback_daemon),
              void (*)(int)>::value, "direct Time callback contract");
int main() {
    void (*callback)(int) = &ReplayWin::timer_callback_daemon;
    callback(0);
#ifdef MAP_32BIT
    void *storage = mmap(0, 4096, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (storage == MAP_FAILED) return 77;
    assert(reinterpret_cast<uintptr_t>(storage) <= INT32_MAX);
    ReplayWin *window = new(storage) ReplayWin();
    const int context = static_cast<int>(reinterpret_cast<uintptr_t>(window));
    callback(context);
    assert(window->calls == 1);
    callback(context);
    assert(window->calls == 2);
    window->~ReplayWin();
    assert(munmap(storage, 4096) == 0);
#else
    return 77;
#endif
}
'''.replace("CALLBACK", callback)


@pytest.mark.parametrize("damage", (None, "null_guard", "dispatch"))
def test_actual_bridge_null_and_borrowed_receiver(tmp_path, damage):
    compiler = shutil.which("g++")
    assert compiler, "actual-source host compiler required"
    text = program()
    if damage == "null_guard":
        text = text.replace("if (window)", "if (!window)", 1)
    elif damage == "dispatch":
        text = text.replace("window->timer_callback();", ";", 1)
    cpp, exe = tmp_path / "callback.cpp", tmp_path / "callback"
    cpp.write_text(text)
    subprocess.run([compiler, "-std=c++11", "-O2", "-fsanitize=undefined",
                    "-fno-sanitize-recover=undefined", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, timeout=60)
    result = subprocess.run([str(exe)], capture_output=True, timeout=30)
    if result.returncode == 77:
        pytest.skip("host cannot provide a native-width pointer payload")
    assert (result.returncode == 0) == (damage is None), result.stderr.decode()


def test_native_authorization_exact_subjects():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/replay-timer-contract-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert {s.address for s in task.subjects} == {0x5AD720, 0x5ADBD0}
    assert all(s.address_space == "pe:va" and s.image_sha256 ==
               "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf" for s in task.subjects)

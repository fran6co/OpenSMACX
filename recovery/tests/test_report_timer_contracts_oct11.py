"""Actual report callback contracts; no null-path normalization or game proof."""
from dataclasses import fields
import json
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from test_caviar_draw_callbacks_actual_source import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "e16e67dcc2adbdfafe12f24d23fcd054cb58071e"
CALLBACKS = {
    0x49FFC0: ("src/reportwin.cpp", "sat_anim_daemon", "ReportWin", "sat_anim", 0x49FE40, 0x49F350),
    0x4A0250: ("src/reportwin.cpp", "sat_expl_daemon", "ReportWin", "exp_anim", 0x4A0100, 0x49FFD0),
    0x4A4050: ("src/reportif.cpp", "bl_daemon", "ReportIf", "bl_anim", 0x4A4060, 0x496E10),
}


@pytest.mark.parametrize("relative", ("src/reportwin.cpp", "src/reportif.cpp"))
def test_exact_authorized_source_and_original_continuity(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT, text=True)
    expected = old
    fixture = Path(__file__).with_name("report-timer-contracts-oct11.patch").read_text()
    section = next(p for p in fixture.split("diff --git ")[1:]
                   if p.startswith("a/" + relative + " b/" + relative + "\n"))
    for hunk in section.split("\n@@ ")[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(l[1:] for l in lines if l.startswith((" ", "-")))
        after = "".join(l[1:] for l in lines if l.startswith((" ", "+")))
        assert before and expected.count(before) == 1
        expected = expected.replace(before, after, 1)
    assert expected == path.read_text()
    before, after = read_text(old, path), read_text(expected, path)
    assert [r.address for r in before] == [r.address for r in after]
    for a, b in zip(before, after):
        for field in fields(a):
            if field.name in {"line", "region", "origin"}:
                continue
            value = "?" + CALLBACKS[a.address][1] + "@@YAXH@Z" if (
                a.address in CALLBACKS and field.name == "symbol") else getattr(a, field.name)
            assert getattr(b, field.name) == value
        lessons = lambda r: [(l.kind, l.key, l.prose, tuple(t for _, t in l.lines)) for l in r.origin]
        assert lessons(a) == lessons(b)
        if a.address in CALLBACKS:
            assert b.byte_exact and b.image_spans == ((a.address, a.address + 13),)


@pytest.mark.parametrize("address", CALLBACKS)
def test_pinned_stack_payload_unguarded_call_and_registration(address):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    _, name, _, _, target, caller = CALLBACKS[address]
    code = list(decoder.disasm(image.read(Span(address, address + 13)), address))
    assert sum(i.size for i in code) == 13
    assert [(i.mnemonic, i.op_str) for i in code] == [
        ("push", "ebp"), ("mov", "ebp, esp"), ("mov", "ecx, dword ptr [ebp + 8]"),
        ("call", hex(target)), ("pop", "ebp"), ("ret", "")]
    report_path = ROOT / "src/reportwin.cpp"
    record = next(r for r in read_text(report_path.read_text(), report_path) if r.address == caller)
    start, end = record.image_spans[0]
    registration = list(decoder.disasm(image.read(Span(start, end)), start))
    assert any(i.mnemonic == "push" and i.op_str == hex(address) for i in registration)
    assert any(i.mnemonic == "call" and i.op_str == "0x616260" for i in registration)
    source = report_path.read_text()
    assert f"reinterpret_cast<void (__cdecl *)(int)>({name})" not in source


def program(address, damage):
    relative, name, owner, method, *_ = CALLBACKS[address]
    callback = extract((ROOT / relative).read_text(), f"void __cdecl {name}(int context)")
    if damage == "dispatch":
        callback = callback.replace("->" + method + "();", "->calls += 0;", 1)
    elif damage == "null_guard":
        callback = callback.replace("{", "{ if (context == 0) return;", 1)
    return r'''#include <new>
#include <cstdint>
#include <type_traits>
#include <sys/mman.h>
#define __cdecl
class OWNER { public: int calls; void METHOD() { ++calls; } };
CALLBACK
static_assert(std::is_same<decltype(&NAME), void (*)(int)>::value, "Time contract");
int main(int argc, char **) {
    void (*callback)(int) = &NAME;
    if (argc > 1) { callback(0); return 0; }
#ifdef MAP_32BIT
    void *storage = mmap(0, 4096, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    if (storage == MAP_FAILED) return 77;
    if (reinterpret_cast<uintptr_t>(storage) > INT32_MAX) return 77;
    OWNER *owner = new(storage) OWNER();
    int context = static_cast<int>(reinterpret_cast<uintptr_t>(owner));
    callback(context);
    if (owner->calls != 1) return 1;
    callback(context);
    if (owner->calls != 2) return 1;
    owner->~OWNER();
    return munmap(storage, 4096) == 0 ? 0 : 1;
#else
    return 77;
#endif
}
'''.replace("OWNER", owner).replace("METHOD", method).replace("CALLBACK", callback).replace("NAME", name)


@pytest.mark.parametrize("address", CALLBACKS)
@pytest.mark.parametrize("damage", (None, "dispatch", "null_guard"))
def test_actual_bridge_borrowed_receiver_and_unguarded_contract(tmp_path, address, damage):
    compiler = shutil.which("g++")
    assert compiler, "actual-source host compiler required"
    cpp, exe = tmp_path / "callback.cpp", tmp_path / "callback"
    cpp.write_text(program(address, damage))
    subprocess.run([compiler, "-std=c++11", "-O2", "-fsanitize=undefined",
                    "-fno-sanitize-recover=undefined", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, timeout=60)
    valid = subprocess.run([str(exe)], cwd=tmp_path, capture_output=True, timeout=30)
    if valid.returncode == 77:
        pytest.skip("host cannot provide native-width receiver payload")
    null = subprocess.run([str(exe), "null"], cwd=tmp_path, capture_output=True, timeout=30)
    assert (valid.returncode == 0 and null.returncode != 0) == (damage is None)


def test_native_exact_authorization():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/report-timer-contracts-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert {s.address for s in task.subjects} == set(CALLBACKS) | {r[-1] for r in CALLBACKS.values()}
    assert all(s.address_space == "pe:va" and s.image_sha256 ==
               "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf" for s in task.subjects)

"""Bounded typed Win scrollbar callback recovery controls.

The host executable exercises the actual setter definitions, not hand-written
equivalents. It proves assignment behavior only; target ABI and linked build
remain coordinator/native checks.
"""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.binding_source import parse as parse_binding_evidence
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import Mode, read_text
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
BASELINE = "b922c7a6e8e960d7f6089f1c752b97ee35fcbd73"
SIGNATURES = (
    "void Win::set_scrolled_callback(ScrollCallback callback)",
    "void Win::set_scrolling_callback(ScrollCallback callback)",
)
ADDRESSES = (0x005EE130, 0x005EE160)


def _body(text: str, signature: str) -> str:
    assert text.count(signature) == 1, signature
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _actual_bodies() -> tuple[str, str]:
    source = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    rows = read_text(source, ROOT / "src/win.cpp")
    bodies = []
    for address, signature in zip(ADDRESSES, SIGNATURES):
        row, = [row for row in rows if row.address == address]
        assert row.mode is Mode.BODY
        assert row.image_spans == ((address, address + 0x27),)
        assert signature in row.region
        bodies.append(_body(source, signature))
    return tuple(bodies)


PRELUDE = r'''
#include <cassert>
#define __cdecl
struct Scroll { void (__cdecl *field_2144_)(int,int); void (__cdecl *field_2148_)(int,int); };
struct Win {
    using ScrollCallback = void (__cdecl *)(int,int);
    Scroll *scroll_vert_; Scroll *scroll_horz_;
    void set_scrolled_callback(ScrollCallback callback);
    void set_scrolling_callback(ScrollCallback callback);
};
static int calls_a, calls_b;
static void cb_a(int, int) { ++calls_a; }
static void cb_b(int, int) { ++calls_b; }
'''

HARNESS = r'''
static void seed(Scroll &s, void (__cdecl *first)(int,int), void (__cdecl *second)(int,int)) {
    s.field_2144_ = first; s.field_2148_ = second;
}
static void both(Win &w, Scroll &v, Scroll &h, void (__cdecl *v2144)(int,int),
                 void (__cdecl *v2148)(int,int), void (__cdecl *h2144)(int,int),
                 void (__cdecl *h2148)(int,int)) {
    seed(v, v2144, v2148); seed(h, h2144, h2148);
    w.set_scrolled_callback(cb_a);
    w.set_scrolling_callback(cb_b);
    assert(v.field_2144_ == cb_a && v.field_2148_ == cb_b);
    assert(h.field_2144_ == cb_a && h.field_2148_ == cb_b);
    assert(calls_a == 0 && calls_b == 0);
    w.set_scrolled_callback(nullptr);
    w.set_scrolling_callback(nullptr);
    assert(!v.field_2144_ && !v.field_2148_ && !h.field_2144_ && !h.field_2148_);
    assert(calls_a == 0 && calls_b == 0);
}
int main() {
    Scroll v{cb_b, cb_b}, h{cb_b, cb_b};
    void (__cdecl *sentinel)(int,int) = cb_b;
    for (int mask=0; mask<4; ++mask) {
        Win w{(mask&1) ? &v : nullptr, (mask&2) ? &h : nullptr};
        seed(v, sentinel, sentinel); seed(h, sentinel, sentinel);
        w.set_scrolled_callback(cb_a);
        assert(v.field_2144_ == ((mask&1) ? cb_a : sentinel));
        assert(h.field_2144_ == ((mask&2) ? cb_a : sentinel));
        assert(v.field_2148_ == sentinel && h.field_2148_ == sentinel);
        w.set_scrolling_callback(cb_b);
        assert(v.field_2148_ == ((mask&1) ? cb_b : sentinel));
        assert(h.field_2148_ == ((mask&2) ? cb_b : sentinel));
        assert(v.field_2144_ == ((mask&1) ? cb_a : sentinel));
        assert(h.field_2144_ == ((mask&2) ? cb_a : sentinel));
        assert(calls_a == 0 && calls_b == 0);
    }
    Win attached{&v,&h};
    both(attached, v, h, cb_b, cb_b, cb_b, cb_b);
    Win alias{&v, &v};
    seed(v, cb_b, cb_b);
    alias.set_scrolled_callback(cb_a);
    alias.set_scrolling_callback(cb_b);
    assert(v.field_2144_ == cb_a && v.field_2148_ == cb_b);
    assert(calls_a == 0 && calls_b == 0);
}
'''


def _compile_and_run(tmp_path: Path, compiler: str, bodies: tuple[str, str],
                     harness: str = HARNESS) -> subprocess.CompletedProcess[str]:
    program = tmp_path / "callback_control.cpp"
    binary = tmp_path / "callback_control"
    program.write_text(PRELUDE + "\n".join(bodies) + harness, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(program), "-o", str(binary)], check=True, timeout=30,
                   capture_output=True, text=True)
    return subprocess.run([str(binary)], timeout=10, capture_output=True, text=True)


def test_typed_api_and_actual_setters_cover_optional_independent_slots(tmp_path):
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert "typedef void (__cdecl *ScrollCallback)(int, int);" in header
    assert "void set_scrolled_callback(ScrollCallback callback);" in header
    assert "void set_scrolling_callback(ScrollCallback callback);" in header
    bodies = _actual_bodies()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is available for host controls")
    for index, compiler in enumerate(compilers):
        case_path = tmp_path / str(index)
        case_path.mkdir()
        result = _compile_and_run(case_path, compiler, bodies)
        assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("damage", ("wrong_slot", "omit_horizontal", "else_guard", "invoke_early"))
def test_host_controls_reject_setter_mutations(tmp_path, damage):
    bodies = list(_actual_bodies())
    if damage == "wrong_slot":
        bodies[0] = bodies[0].replace("field_2144_", "field_2148_")
    elif damage == "omit_horizontal":
        bodies[0] = bodies[0].replace("if (scroll_horz_)", "if (false && scroll_horz_)")
    elif damage == "else_guard":
        bodies[0] = bodies[0].replace("    if (scroll_horz_)", "    else if (scroll_horz_)")
    else:
        bodies[0] = bodies[0].replace("void Win::set_scrolled_callback(ScrollCallback callback) {",
                                      "void Win::set_scrolled_callback(ScrollCallback callback) {\n    callback(1, 2);")
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("no host C++ compiler is available")
    tmp_path.mkdir(exist_ok=True)
    result = _compile_and_run(tmp_path, compiler, tuple(bodies))
    assert result.returncode != 0, f"damage control survived: {damage}"


def test_pinned_complete_setter_instruction_spans_and_independent_gates():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for setter instruction verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    expected = {
        0x005EE130: (("mov", "eax, dword ptr [ecx + 0x43c]"),
                     ("mov", "edx, dword ptr [esp + 4]"), ("test", "eax, eax"),
                     ("je", "0x5ee144"), ("mov", "dword ptr [eax + 0x2144], edx"),
                     ("mov", "ecx, dword ptr [ecx + 0x440]"), ("test", "ecx, ecx"),
                     ("je", "0x5ee154"), ("mov", "dword ptr [ecx + 0x2144], edx"),
                     ("ret", "4")),
        0x005EE160: (("mov", "eax, dword ptr [ecx + 0x43c]"),
                     ("mov", "edx, dword ptr [esp + 4]"), ("test", "eax, eax"),
                     ("je", "0x5ee174"), ("mov", "dword ptr [eax + 0x2148], edx"),
                     ("mov", "ecx, dword ptr [ecx + 0x440]"), ("test", "ecx, ecx"),
                     ("je", "0x5ee184"), ("mov", "dword ptr [ecx + 0x2148], edx"),
                     ("ret", "4")),
    }
    for address in ADDRESSES:
        rows = read_text((ROOT / "src/win.cpp").read_text(encoding="utf-8"), ROOT / "src/win.cpp")
        row, = [record for record in rows if record.address == address]
        assert len(row.image_spans) == 1
        low, high = row.image_spans[0]
        assert (low, high) == (address, address + 0x27)
        instructions = list(decoder.disasm(image.read(Span(low, high)), low))
        cursor = low
        for instruction in instructions:
            assert instruction.address == cursor
            cursor += instruction.size
        assert cursor == high
        assert tuple((instruction.mnemonic, instruction.op_str) for instruction in instructions) == expected[address]
        assert sum(instruction.size for instruction in instructions) == 39


def test_catalogue_identity_claims_and_authored_history_are_preserved():
    source_path = ROOT / "src/win.cpp"
    text = source_path.read_text(encoding="utf-8")
    records = read_text(text, source_path)
    bindings = parse_binding_evidence(text, records)
    current = {row.address: row for row in records}
    baseline_text = subprocess.run(["git", "show", f"{BASELINE}:src/win.cpp"], cwd=ROOT,
                                   check=True, capture_output=True, text=True).stdout
    old = {row.address: row for row in read_text(baseline_text, source_path)}
    for address in ADDRESSES:
        before, after = old[address], current[address]
        assert (after.address, after.name, after.mode, after.image_spans, after.byte_exact,
                after.semantic, after.kind, after.exclusion, after.levers, after.ruled_out,
                after.unrecoverable, after.deferred, after.source_role,
                after.source_role_reason) == (
                before.address, before.name, before.mode, before.image_spans, before.byte_exact,
                before.semantic, before.kind, before.exclusion, before.levers, before.ruled_out,
                before.unrecoverable, before.deferred, before.source_role,
                before.source_role_reason)
        history = lambda row: tuple((lesson.kind, lesson.key, lesson.prose,
                                     tuple(line for _, line in lesson.lines))
                                    for lesson in (row.origin or ()))
        assert history(after) == history(before)
    # The coordinator observed these definitions in a fresh native VC6 COFF
    # and published the overrides through source.writer.write_symbol. These
    # are current emitted bindings, never authenticated original spellings.
    expected_symbols = {
        0x005EE130: "?set_scrolled_callback@Win@@QAEXP6AXHH@Z@Z",
        0x005EE160: "?set_scrolling_callback@Win@@QAEXP6AXHH@Z@Z",
    }
    for address, symbol in expected_symbols.items():
        assert current[address].symbol == symbol
        assert current[address].line in bindings
        assert bindings[current[address].line]["evidence"]

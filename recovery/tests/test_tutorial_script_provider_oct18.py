"""Pinned pointer-provider evidence and narrow metadata/call-expression controls.

Host fixtures observe source expressions and do not claim native ABI, linking,
Win32 callback behavior, or game runtime equivalence.
"""

from __future__ import annotations

from pathlib import Path
import hashlib
import os
import re
import shutil
import struct
import subprocess

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86_const import X86_REG_EDX, X86_REG_ESI
import pytest

from match_recovery.model import Span
from match_recovery.literal_source import read_literals
from match_recovery.pe_coff import PeImage
from match_recovery.source.reader import read_text


ROOT = Path(__file__).resolve().parents[2]
BASELINE = "0e9b8378eedfcb84e950b2f9b01ad5612ca6bad1"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPACE = "pe:va"
SUBJECTS = {
    0x53CAF0: ((0x53CAF0, 0x53CE05),),
    0x53CE10: ((0x53CE10, 0x53D158),),
    0x5BB000: ((0x5BB000, 0x5BCB55), (0x662710, 0x662A1E)),
    0x4BDFE0: ((0x4BDFE0, 0x4BE643), (0x659CE0, 0x659CF2)),
    0x4BC6E0: ((0x4BC6E0, 0x4BDCBE),),
}


def _disassemble(image: PeImage, start: int, end: int):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(image.read(Span(start, end)), start))
    cursor = start
    result = {}
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:08X}"
        cursor += instruction.size
        result[instruction.address] = instruction
    assert instructions and cursor == end
    return result


def _expect(instructions, address: int, mnemonic: str, operands: str) -> None:
    instruction = instructions[address]
    assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands), (
        f"{address:08X}: {instruction.mnemonic} {instruction.op_str}"
    )


def _git_source(revision: str, path: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{revision}:{path}"], cwd=ROOT,
        check=True, capture_output=True, text=True, timeout=15,
    )
    return result.stdout


def _one_record(text: str, address: int, source_path: str):
    matches = [row for row in read_text(text, source_path)
               if row.address == address]
    assert len(matches) == 1, f"expected one mapped record at {address:08X}"
    return matches[0]


def test_original_caption_cell_and_both_callsite_argument_sequences() -> None:
    configured_image = os.environ.get("OPENSMACX_IMAGE")
    image_candidates = ([Path(configured_image)] if configured_image else []) + [
        ROOT / ".opensmacx/game/terranx_original.exe",
        ROOT.parents[2] / ".opensmacx/game/terranx_original.exe",
    ]
    image_path = next((candidate for candidate in image_candidates
                       if candidate.is_file()), image_candidates[0])
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)

    cell = 0x00691B14
    pointee = struct.unpack("<I", image.read(Span(cell, cell + 4)))[0]
    assert pointee == 0x00691B8C
    assert image.read(Span(pointee, pointee + 6)) == b"TUTOR\0"

    # PopupStartCaption is a separate zero-filled array in original .bss,
    # while the tutorial provider is a pointer cell to the TUTOR string.
    popup_array = 0x009B8AA8
    assert image.read_initial(Span(popup_array, popup_array + 256)) == bytes(256)
    assert popup_array != cell and popup_array != pointee
    basepop = (ROOT / "src/basepop.cpp").read_text()
    assert re.search(r"char\s+PopupStartCaption\[256\]\s*;\s*//\s*0x009B8AA8", basepop)
    console = (ROOT / "src/console.cpp").read_text()
    assert 'char TutorialPopupCaptionText[6] = "TUTOR";' in console
    assert "char *TutorialPopupCaption = TutorialPopupCaptionText;" in console
    header = (ROOT / "src/tutorial_script.h").read_text()
    assert re.search(r"\bextern\s+char\s*\*\s*TutorialPopupCaption\s*;", header)

    pact = _disassemble(image, *SUBJECTS[0x53CAF0][0])
    _expect(pact, 0x53CD5D, "xor", "esi, esi")
    _expect(pact, 0x53CDE8, "mov", "edx, dword ptr [0x691b14]")
    _expect(pact, 0x53CDF1, "push", "esi")  # Original null third argument.
    _expect(pact, 0x53CDF2, "push", "0x68d5ec")
    _expect(pact, 0x53CDF7, "push", "edx")
    _expect(pact, 0x53CDF8, "call", "0x5bf330")
    assert all(X86_REG_ESI not in instruction.regs_access()[1]
               for address, instruction in pact.items()
               if 0x53CD5D < address < 0x53CDF1)
    assert all(X86_REG_EDX not in instruction.regs_access()[1]
               for address, instruction in pact.items()
               if 0x53CDE8 < address < 0x53CDF7)
    assert image.read(Span(0x68D5EC, 0x68D5F1)) == b"PACT\0"
    assert image.read(Span(0x68D600, 0x68D609)) == b"COMMERCE\0"

    treaty = _disassemble(image, *SUBJECTS[0x53CE10][0])
    _expect(treaty, 0x53D0D1, "mov", "eax, dword ptr [0x691b14]")
    _expect(treaty, 0x53D0D6, "push", "0")
    _expect(treaty, 0x53D0D8, "push", "0x68d600")
    _expect(treaty, 0x53D0DD, "push", "eax")
    _expect(treaty, 0x53D0DE, "call", "0x5bf330")

    # TutWin::tut_win independently reloads the provider for both calls; the
    # callsite argument stack is pinned rather than inferred from source text.
    tutorial = _disassemble(image, 0x4BE128, 0x4BE136)
    _expect(tutorial, 0x4BE128, "mov", "eax, dword ptr [0x691b14]")
    _expect(tutorial, 0x4BE12D, "push", "edx")
    _expect(tutorial, 0x4BE12E, "push", "eax")
    _expect(tutorial, 0x4BE12F, "mov", "ecx, esi")
    _expect(tutorial, 0x4BE131, "call", "0x406380")
    tutorial_again = _disassemble(image, 0x4BE43F, 0x4BE452)
    _expect(tutorial_again, 0x4BE43F, "mov", "eax, dword ptr [0x691b14]")
    _expect(tutorial_again, 0x4BE449, "push", "edx")
    _expect(tutorial_again, 0x4BE44A, "push", "eax")
    _expect(tutorial_again, 0x4BE44D, "call", "0x406380")

    tour = _disassemble(image, 0x4BCA6D, 0x4BCA7E)
    _expect(tour, 0x4BCA6D, "mov", "edx, dword ptr [0x691b14]")
    _expect(tour, 0x4BCA73, "push", "0x687924")
    _expect(tour, 0x4BCA78, "push", "edx")
    _expect(tour, 0x4BCA79, "call", "0x5fd550")
    assert image.read(Span(0x687924, 0x68792F)) == b"DIRECTIONS\0"

    # The separately admitted TREETIME path has three independent gates,
    # then its parser and provider-backed popup; its result is ignored and
    # control continues at the original prototype path.
    name_offset = _disassemble(image, 0x5BBBFE, 0x5BBC07)
    _expect(name_offset, 0x5BBBFE, "mov", "edi, dword ptr [ebp + 0xc]")
    _expect(name_offset, 0x5BBC01, "imul", "edi, edi, 0x2c")
    _expect(name_offset, 0x5BBC04, "mov", "dword ptr [ebp + 0x10], edi")
    # VC6 reuses the third argument slot for the technology byte offset;
    # the later [ebp+0x10] read is not the original faction_id_with value.
    treetime = _disassemble(image, 0x5BBC1C, 0x5BBC6C)
    _expect(treetime, 0x5BBC1C, "mov", "edx, dword ptr [ebp + 8]")
    _expect(treetime, 0x5BBC1F, "mov", "eax, dword ptr [0x939284]")
    _expect(treetime, 0x5BBC24, "cmp", "edx, eax")
    _expect(treetime, 0x5BBC26, "jne", "0x5bbeff")
    _expect(treetime, 0x5BBC2C, "mov", "eax, dword ptr [ebp + 0xc]")
    _expect(treetime, 0x5BBC2F, "mov", "ecx, dword ptr [0x691900]")
    _expect(treetime, 0x5BBC35, "cmp", "eax, ecx")
    _expect(treetime, 0x5BBC37, "jne", "0x5bbc6c")
    _expect(treetime, 0x5BBC39, "mov", "eax, dword ptr [0x93f660]")
    _expect(treetime, 0x5BBC40, "jne", "0x5bbc6c")
    _expect(treetime, 0x5BBC42, "mov", "ecx, dword ptr [ebp + 0x10]")
    _expect(treetime, 0x5BBC4F, "push", "edx")
    _expect(treetime, 0x5BBC50, "push", "0")
    _expect(treetime, 0x5BBC52, "call", "0x625e50")
    _expect(treetime, 0x5BBC57, "mov", "eax, dword ptr [0x691b14]")
    _expect(treetime, 0x5BBC5C, "push", "0")
    _expect(treetime, 0x5BBC5E, "push", "0x69178c")
    _expect(treetime, 0x5BBC63, "push", "eax")
    _expect(treetime, 0x5BBC64, "call", "0x5bf330")
    _expect(treetime, 0x5BBC49, "mov", "edx, dword ptr [ecx + 0x94f35c]")
    assert image.read(Span(0x69178C, 0x691795)) == b"TREETIME\0"


def test_diplomacy_subject_metadata_and_claims_match_task_baseline() -> None:
    sources = {
        "src/faction.cpp": (ROOT / "src/faction.cpp").read_text(),
        "src/tutwin.cpp": (ROOT / "src/tutwin.cpp").read_text(),
        "src/technology.cpp": (ROOT / "src/technology.cpp").read_text(),
    }
    retained_fields = (
        "address", "mode", "state", "name", "image_spans", "symbol", "body",
        "context", "kind", "exclusion", "recipe",
        "levers", "ruled_out", "unrecoverable", "deferred", "source_role",
        "source_role_reason", "function_fragment",
    )
    for source_path in sources:
        baseline = _git_source(BASELINE, source_path)
        current = sources[source_path]
        for address, expected in SUBJECTS.items():
            owners = {
                "src/faction.cpp": {0x53CAF0, 0x53CE10},
                "src/tutwin.cpp": {0x4BDFE0, 0x4BC6E0},
                "src/technology.cpp": {0x5BB000},
            }
            if address not in owners[source_path]:
                continue
            old = _one_record(baseline, address, source_path)
            live = _one_record(current, address, source_path)
            expected_spans = expected
            assert old.image_spans == expected_spans
            assert live.image_spans == expected_spans
            # Body/context are binding paths, not function body text. Preserve
            # them and fragment relations while permitting later native claims.
            assert tuple(getattr(live, field) for field in retained_fields) == tuple(
                getattr(old, field) for field in retained_fields
            ), f"metadata/claim continuity changed at {address:08X}"
            assert not old.byte_exact or live.byte_exact
            assert not old.semantic or live.semantic


def test_forest_prompt_owns_its_independent_original_label_binding() -> None:
    bindings = [row for row in read_literals(ROOT, PIN)
                if row.unit == "src/technology.cpp"
                and row.origin.address == 0x69178C]
    assert len(bindings) == 1
    binding = bindings[0]
    assert binding.span == Span(0x69178C, 0x691795)
    assert binding.symbol == "??_C@_08BPDN@TREETIME?$AA@"
    assert binding.evidence.address == 0x5BB000


def _closing_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and next_char == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif ((state == "string" and char == '"')
                  or (state == "char" and char == "'")):
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line"
            index += 1
        elif char == "/" and next_char == "*":
            state = "block"
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
                return index
        index += 1
    raise AssertionError("unterminated actual method")


def _method_body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    return text[start:_closing_brace(text, opening) + 1]


def test_tutwin_caption_call_expressions_reload_source_provider(tmp_path: Path) -> None:
    """Bounded call-expression control only; not a full TutWin method proof."""
    source = (ROOT / "src/tutwin.cpp").read_text()
    callout = _method_body(source, "int TutWin::tut_win(")
    tour = _method_body(source, "void TutWin::tour()")
    start_pattern = re.compile(
        r"start\(TutorialPopupCaption,\s*text,\s*-1,\s*0,\s*popup_flags,\s*0\);"
    )
    start_calls = [match.group(0) for match in start_pattern.finditer(callout)]
    text_open = re.search(
        r"text_open\(TutorialPopupCaption,\s*\"DIRECTIONS\"\)", tour
    )
    assert len(start_calls) == 2
    assert text_open is not None

    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    fixture = r'''#include <cstdio>
#include <cstdlib>
#include <cstring>
static char backing_a[] = "TUTOR";
static char backing_b[] = "REPLACED";
static char popup_start[256] = "POPUP";
char *TutorialPopupCaption = backing_a;
struct Spy {
  const char *caption; const char *text; int flags; int calls;
  Spy() : caption(0), text(0), flags(0), calls(0) {}
  void start(char *c,const char *t,int,int,int f,int) {
    caption=c; text=t; flags=f; ++calls;
  }
};
static const char *opened_caption=0;
static const char *opened_path=0;
int text_open(char *c,const char *path) {
  opened_caption=c; opened_path=path; return 1;
}
struct TutWinCallouts : Spy {
  const char *text; int popup_flags;
  TutWinCallouts() : text("EDITOR"), popup_flags(0x40) {}
  void first() { ACTUAL_START_1 }
  void second() { ACTUAL_START_2 }
};
int run_tour_open() { return ACTUAL_TEXT_OPEN; }
void demand(bool value,const char *message) {
  if(!value){std::fprintf(stderr,"%s\n",message);std::abort();}
}
int main() {
  TutWinCallouts callouts;
  TutorialPopupCaption=backing_a; callouts.first();
  demand(callouts.caption==backing_a && callouts.Spy::text==callouts.text
         && callouts.flags==callouts.popup_flags && callouts.calls==1,"tut-start-1");
  TutorialPopupCaption=backing_b; callouts.second();
  demand(callouts.caption==backing_b && callouts.Spy::text==callouts.text
         && callouts.flags==callouts.popup_flags && callouts.calls==2,"tut-start-2");
  TutorialPopupCaption=backing_b; (void)run_tour_open();
  demand(opened_caption==backing_b && std::strcmp(opened_path,"DIRECTIONS")==0,
         "tour-text-open");
  demand(callouts.caption!=popup_start,"separate-popup-storage");
  return 0;
}
'''
    program = (fixture.replace("ACTUAL_START_1", start_calls[0])
               .replace("ACTUAL_START_2", start_calls[1])
               .replace("ACTUAL_TEXT_OPEN", text_open.group(0)))
    cpp = tmp_path / "tutwin_caption_expression.cpp"
    binary = tmp_path / "tutwin_caption_expression"
    cpp.write_text(program)
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)], capture_output=True, text=True, timeout=30
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr

"""Actual-source controls for Win's signed aspect-ratio operands."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPAN = (0x005F53A0, 0x005F5480)
SIGNATURE = "long Win::on_window_pos_changing(WINDOWPOS * pos)"
SCALE_FIELDS = ("iVertScaleDenom_", "iVertScaleNum_")
OTHER_FIELDS = ("iFlags_", "field_134_", "field_138_")


def _definition(source: str) -> str:
    assert source.count(SIGNATURE) == 1
    start = source.index(SIGNATURE)
    opening = source.index("{", start + len(SIGNATURE))
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated Win position handler")


def _field(header: str, name: str, override: dict[str, str] | None = None) -> str:
    matches = re.findall(r"^\s*(uint32_t|int)\s+" + re.escape(name) + r"\s*;", header, re.M)
    assert len(matches) == 1, (name, matches)
    ctype = (override or {}).get(name, matches[0])
    return f"    {ctype} {name};"


def _actual_body(overrides: dict[str, str] | None = None) -> tuple[str, str]:
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    declarations = [_field(header, name, overrides) for name in SCALE_FIELDS + OTHER_FIELDS]
    assert _field(header, "iVertScaleNum_").startswith("    int "), "scale numerator must use actual signed declaration"
    assert _field(header, "iVertScaleDenom_").startswith("    int "), "scale denominator must use actual signed declaration"
    body = _definition((ROOT / "src/win.cpp").read_text(encoding="utf-8"))
    return body, "\n".join(declarations)


HOST_PREFIX = r"""
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#define __cdecl
using HWND = void *;
using UINT = unsigned int;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using LRESULT = intptr_t;
struct WINDOWPOS { HWND hwnd; HWND hwndInsertAfter; int x, y, cx, cy; UINT flags; };
struct GraphicWin {};
static std::vector<std::string> events;
static HWND HandleMain = reinterpret_cast<HWND>(0x1234);
static int client_dx, client_dy, nonclient_dx, nonclient_dy;
static int getter_calls;
static LRESULT def_result = 0x55;
static HWND posted_hwnd;
static UINT posted_message;
static WPARAM posted_wparam;
static LPARAM posted_lparam;
BOOL PostMessageA(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    events.push_back("post"); posted_hwnd=hwnd; posted_message=message;
    posted_wparam=wparam; posted_lparam=lparam; return 1;
}
LRESULT DefWindowProcA(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    events.push_back("def"); assert(hwnd==HandleMain && message==0x46);
    assert(wparam==0); return def_result;
}
struct Win {
    SCALE_DECLARATIONS
    GraphicWin *graphic;
    virtual GraphicWin *get_graphic_win() { ++getter_calls; return graphic; }
    void nonclient_to_client(int *x, int *y) {
        events.push_back("n2c"); *x -= client_dx; *y -= client_dy;
    }
    void client_to_nonclient(int *x, int *y) {
        events.push_back("c2n"); *x += nonclient_dx; *y += nonclient_dy;
    }
    long on_window_pos_changing(WINDOWPOS *pos);
};
"""

HOST_MAIN = r"""
static void reset() {
    events.clear(); getter_calls=0; client_dx=2; client_dy=4;
    nonclient_dx=2; nonclient_dy=4; posted_hwnd=0; posted_message=0;
    posted_wparam=99; posted_lparam=99;
}
static WINDOWPOS pos(int cx, int cy, UINT flags=0, HWND after=reinterpret_cast<HWND>(1)) {
    WINDOWPOS p{}; p.cx=cx; p.cy=cy; p.flags=flags; p.hwndInsertAfter=after; return p;
}
int main() {
    GraphicWin graphic{};
    Win win{}; win.graphic=&graphic;
    win.field_134_=6; win.field_138_=8;
    win.iVertScaleNum_=3; win.iVertScaleDenom_=2;
    win.iFlags_=0x800000 | 0x4000;
    WINDOWPOS p=pos(12,20,0,0);
    reset(); assert(win.on_window_pos_changing(&p)==def_result);
    assert(p.cx==12 && p.cy==19); // (12-2)*3/2 + 4
    assert(getter_calls==1);
    assert((events==std::vector<std::string>{"n2c","c2n","post","def"}));
    assert((p.flags & 4) && posted_hwnd==HandleMain && posted_message==3);
    assert(posted_wparam==0 && posted_lparam==0);

    // Signed negative numerator, with representable multiply/divide.
    win.iVertScaleNum_=-3; win.iVertScaleDenom_=2; p=pos(6,20);
    reset(); win.on_window_pos_changing(&p);
    assert(p.cy==-2); // (-3 * (6-2)) / 2 + 4
    assert((events==std::vector<std::string>{"n2c","c2n","def"}));

    // Signed negative denominator also uses truncation toward zero.
    win.iVertScaleNum_=3; win.iVertScaleDenom_=-2; p=pos(6,20);
    reset(); win.on_window_pos_changing(&p);
    assert(p.cy==-2);

    // Negative client width is not clamped without a graphic window.
    win.graphic=nullptr; win.iVertScaleNum_=3; win.iVertScaleDenom_=2;
    p=pos(-6,20); reset(); win.on_window_pos_changing(&p);
    assert(getter_calls==1 && p.cx==-6 && p.cy==-8); // 3*(-8)/2 + 4
    assert((events==std::vector<std::string>{"n2c","c2n","def"}));

    // Getter success enables minimum extents before aspect scaling.
    win.graphic=&graphic; win.iVertScaleNum_=1; win.iVertScaleDenom_=1;
    p=pos(1,2); reset(); win.on_window_pos_changing(&p);
    assert(p.cx==6 && p.cy==8); // clamp, then -2/-4 then +2/+4

    // Resize-disabled skips both minimum and aspect paths, but keeps flags,
    // PostMessage and DefWindowProc behavior.
    win.iFlags_=0x4000 | 0x800000; p=pos(1,2,1,0); reset();
    assert(win.on_window_pos_changing(&p)==def_result);
    assert(p.cx==1 && p.cy==2 && getter_calls==0 && (p.flags & 4));
    assert((events==std::vector<std::string>{"post","def"}));

    // Aspect-disabled still performs virtual resize lookup and minimum clamp,
    // with no coordinate conversion calls.
    win.iFlags_=0; p=pos(1,2); reset(); win.on_window_pos_changing(&p);
    assert(p.cx==6 && p.cy==8 && getter_calls==1);
    assert((events==std::vector<std::string>{"def"}));

    // Non-null insert-after suppresses PostMessage; bit 0x4000 sets flag 4.
    win.iFlags_=0x4000; p=pos(10,10,0,reinterpret_cast<HWND>(1)); reset();
    win.on_window_pos_changing(&p);
    assert((p.flags & 4) && posted_message==0);
    assert((events==std::vector<std::string>{"def"}));
}
"""


def _compile_and_run(compiler: str, body: str, declarations: str, *, expect_success: bool) -> None:
    source = HOST_PREFIX.replace("SCALE_DECLARATIONS", declarations) + "\n" + body + "\n" + HOST_MAIN
    # Windows API stand-ins are minimal service doubles, not a replacement
    # implementation of the method under test.
    source = source.replace("using HWND = void *;", "using HWND = void *;\nusing BOOL = int;")
    with tempfile.TemporaryDirectory(prefix="osmx-win-scale-") as temp:
        src = Path(temp) / "actual_win_handler.cpp"
        exe = Path(temp) / "actual_win_handler"
        src.write_text(source)
        compiled = subprocess.run([compiler, "-std=c++17", "-O0", str(src), "-o", str(exe)], capture_output=True, text=True, timeout=45)
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        if expect_success:
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "unsigned field mutant passed signed behavior controls"


def test_actual_handler_signed_behavior_and_unsigned_mutants():
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither GCC nor Clang is installed for host controls"
    body, declarations = _actual_body()
    for compiler in compilers:
        _compile_and_run(compiler, body, declarations, expect_success=True)
        for field in SCALE_FIELDS:
            _, mutant_declarations = _actual_body({field: "uint32_t"})
            _compile_and_run(compiler, body, mutant_declarations, expect_success=False)


def test_pinned_signed_division_instruction_witnesses():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for signed divide witnesses")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    assert image_pin(ROOT) == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(decoder.disasm(image.read(Span(*SPAN)), SPAN[0]))
    assert sum(instruction.size for instruction in instructions) == SPAN[1] - SPAN[0]
    cdq, = [instruction for instruction in instructions
            if instruction.mnemonic == "cdq"]
    idiv, = [instruction for instruction in instructions
             if instruction.mnemonic == "idiv"]
    assert cdq.address == 0x005F541B
    assert idiv.address == 0x005F541C
    assert idiv.op_str == "dword ptr [edi + 0xac]"

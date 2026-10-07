"""Actual-source/history controls for the three Console adjustor rows.

Reads the owner C++ and pinned image; it does not infer or assign emitted
COFF symbols.
"""
from __future__ import annotations

import hashlib
import re
import subprocess
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
from match_recovery.pe_coff import PeImage, Span
from match_recovery.source import Mode, SourceRole, read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
IMAGE = ROOT / ".opensmacx/game/terranx_original.exe"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPECS = (
    (0x0051DEF0, 31, "on_nc_mouse_move", 0x00510110,
     "7937cba18d6e8aac8c8591100a989f43fdd77ca7f84573d36a51b64fa5368e86"),
    (0x0051DF10, 34, "on_nc_left_up", 0x0050FE70,
     "37ddaa38b6a53d790767396d5fc2a32d2594f42d6df20cf9c5b4b9f51becf7f8"),
    (0x0051DF30, 60, "on_post_redraw_nc_buffer", 0x0050F960,
     "44e403a97bb3aef98912293e94a5ac00a173c73c97f77849f3d6bbfdba9f9e09"),
)
SEED_SOURCE_SHA256 = "15413b6171d7e5cbcaf410d7d03726fa38f9c79dfd96425413bdb2063b85d20f"
SEED_COMMIT = "d1bbb0e1b"


def canonical_seed_comment(address: int):
    raw = subprocess.check_output(
        ["git", "show", f"{SEED_COMMIT}:src/adjustor_thunks.cpp"], cwd=ROOT)
    assert hashlib.sha256(raw).hexdigest() == SEED_SOURCE_SHA256
    text = raw.decode("utf-8", "surrogateescape")
    marker = f"// ORIGINAL: 0x{address:08X} "
    assert text.count(marker) == 1
    at = text.index(marker)
    start = text.rfind("/*", 0, at)
    close = text.find("*/", at)
    assert start >= 0 and close >= 0
    full = text[start:close + 2] + "\n"
    normalized = re.sub(r"(?m)^// ORIGINAL:", "// original marker:", full)
    assert all(fact in normalized for fact in
               ("Purpose:", "RECOVERY_ROLE: SCAFFOLD", "Return Value:", "Status:"))
    return normalized


def rows(path: Path):
    return [row for row in read_file(path)]


def record(path: Path, address: int):
    matches = [row for row in rows(path) if row.address == address]
    assert len(matches) == 1, f"expected one 0x{address:08X} identity in {path}"
    return matches[0]


def archived_history(path: Path, address: int, expected_digest: str):
    text = path.read_text(encoding="utf-8")
    token = f"RECOVERY_HISTORY: v1 address=0x{address:08X} "
    assert text.count(token) == 1
    start = text.rfind("/*", 0, text.index(token))
    end = text.find("\n */", text.index(token))
    assert start >= 0 and end >= 0
    archive = text[start:end + 4]
    header = text[text.index(token):text.index("\n", text.index(token))]
    assert f"source_sha256={SEED_SOURCE_SHA256}" in header
    assert f"envelope_sha256={expected_digest}" in header
    payload = []
    for line in archive.splitlines()[2:-1]:
        assert line == " * [ARCHIVE]" or line.startswith(" * [ARCHIVE] ")
        payload.append("" if line == " * [ARCHIVE]" else line[len(" * [ARCHIVE] "):])
    normalized = "\n".join(payload).replace("* /", "*/") + "\n"
    assert hashlib.sha256(normalized.encode("utf-8", "surrogateescape")).hexdigest() == expected_digest
    assert normalized == canonical_seed_comment(address)
    assert f"// original marker: 0x{address:08X} " in normalized
    assert f"// ORIGINAL: 0x{address:08X} " not in normalized


def test_three_original_identities_live_once_in_the_real_console_owner():
    owner = ROOT / "src/console.cpp"
    shim = ROOT / "src/adjustor_thunks.cpp"
    header = ROOT / "src/adjustor_thunks.h"
    for address, _slot, method, _target, history_digest in SPECS:
        row = record(owner, address)
        assert row.mode is Mode.FILE
        assert row.source_role is not SourceRole.SCAFFOLD
        assert not row.symbol or "adjust_" not in row.symbol
        assert not [r for r in rows(shim) if r.address == address]
        assert method not in header.read_text()
        archived_history(owner, address, history_digest)
    source = owner.read_text()
    assert "RECOVERY_GENERATED: v1 vtable 0x0066EC18" in source


def test_virtual_contracts_and_dispatch_callers_preserve_the_three_slot_apis():
    win_h = (ROOT / "src/win.h").read_text()
    console_h = (ROOT / "src/console.h").read_text()
    win_cpp = (ROOT / "src/win.cpp").read_text()
    graphic_cpp = (ROOT / "src/graphicwin.cpp").read_text()
    graphic_mock = (ROOT / "recovery/tests/graphicwin_paint_owner_control.cpp").read_text()
    expected = (
        ("on_nc_mouse_move", r"virtual\s+void\s+on_nc_mouse_move\s*\(int\s*=\s*0,\s*int\s*=\s*0\)\s*\{\s*\}",
         r"void\s+on_nc_mouse_move\s*\(int\s+\w+,\s*int\s+\w+\)\s+override\s*;"),
        ("on_nc_left_up", r"virtual\s+void\s+on_nc_left_up\s*\(int\s*=\s*0,\s*int\s*=\s*0\)\s*\{\s*\}",
         r"void\s+on_nc_left_up\s*\(int\s+\w+,\s*int\s+\w+\)\s+override\s*;"),
        ("on_post_redraw_nc_buffer", r"virtual\s+void\s+on_post_redraw_nc_buffer\s*\(Buffer\s*\*\s*=\s*0,\s*int\s*=\s*0\)\s*\{\s*\}",
         r"void\s+on_post_redraw_nc_buffer\s*\(Buffer\s*\*\w+,\s*int\s+\w+\)\s+override\s*;"),
    )
    for name, base_contract, derived_override in expected:
        assert re.search(base_contract, win_h), name
        assert re.search(derived_override, console_h), name
    assert win_cpp.count("this->on_nc_mouse_move(x, y);") == 1
    assert win_cpp.count("on_nc_mouse_move(a, b);") == 1
    assert win_cpp.count("this->on_nc_left_up(x, y);") == 1
    assert win_cpp.count("on_nc_left_up(a, b);") == 1
    assert win_cpp.count("this->on_post_redraw_nc_buffer(buf, ") == 4
    assert graphic_cpp.count("on_post_redraw_nc_buffer(buffer, ") == 4
    assert not re.search(r"\bvslot_(31|34|60)\b", win_h + win_cpp + graphic_cpp)
    assert "virtual void on_post_redraw_nc_buffer(Buffer *, int side)" in graphic_mock
    assert '"HOOK", "CAP", "V0"' in graphic_mock
    assert '"F4", "D3@0,-10", "D4@34,-11", "T8@5,-8:29,2", "V2"' in graphic_mock


def test_constructor_addresspoint_and_original_slot_cells_select_these_thunks():
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    win_table = 0x0066EC18
    for address, slot, _method, target, _digest in SPECS:
        selected = int.from_bytes(image.read(Span(win_table + slot * 4,
                                                  win_table + slot * 4 + 4)), "little")
        base_target = int.from_bytes(image.read(Span(0x0066FDD0 + slot * 4,
                                                    0x0066FDD0 + slot * 4 + 4)), "little")
        assert selected == address
        assert base_target == 0x00404260
        thunk = image.read(Span(address, address + 8))
        assert thunk[:4] == bytes.fromhex("2b49fce9")
        rel = int.from_bytes(thunk[4:8], "little", signed=True)
        assert address + 8 + rel == target

    ctor = record(ROOT / "src/console.cpp", 0x0050F460)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    installed = False
    for low, high in ctor.image_spans:
        for insn in decoder.disasm(image.read(Span(low, high)), low):
            if (insn.mnemonic == "mov" and len(insn.operands) == 2
                    and insn.operands[0].type == X86_OP_MEM
                    and insn.operands[1].type == X86_OP_IMM
                    and insn.address == 0x0050F562
                    and (insn.operands[1].imm & 0xFFFFFFFF) == win_table):
                installed = True
    assert installed, "pinned Console constructor must install the selected address point"


def test_api_behavior_is_virtual_dispatch_not_a_manual_receiver_callback():
    win_h = (ROOT / "src/win.h").read_text()
    console_h = (ROOT / "src/console.h").read_text()
    # Base defaults remain no-ops; dispatch sites call the public virtual API.
    # This guards callback behavior/signature without compiling a host-only
    # replica of product code or guessing the current physical symbol.
    assert "virtual void on_nc_mouse_move(int = 0, int = 0) {}" in win_h
    assert "virtual void on_nc_left_up(int = 0, int = 0) {}" in win_h
    assert "virtual void on_post_redraw_nc_buffer(Buffer * = 0, int = 0) {}" in win_h
    for method in ("on_nc_mouse_move", "on_nc_left_up", "on_post_redraw_nc_buffer"):
        assert re.search(rf"\b{method}\b[^;]*override\s*;", console_h)
    assert "reinterpret_cast<Console *>" not in (ROOT / "src/adjustor_thunks.cpp").read_text()

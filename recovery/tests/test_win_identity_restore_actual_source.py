"""Actual-source controls for the two restored Win virtual identities.

Promotion destination: ``recovery/tests/test_win_identity_restore_actual_source.py``.
  UV_CACHE_DIR=/tmp/osmx-uv-cache uv run pytest \
    recovery/tests/test_win_identity_restore_actual_source.py
"""
from __future__ import annotations

import hashlib
import json
import os
import re
import subprocess
from pathlib import Path

import pytest

from match_recovery.source import Mode, read_text
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())
WIN_CPP = ROOT / "src/win.cpp"
WIN_HEADER = ROOT / "src/win.h"
PINNED_IMAGE = ROOT / ".opensmacx/game/terranx_original.exe"
BUILD_ENV = os.environ.get("OPENSMACX_WIN_OWNER_BUILD")
BUILD = Path(BUILD_ENV).resolve() if BUILD_ENV else None
WIN_OBJECT = (BUILD / "CMakeFiles/OpenSMACX.dir/src/win.cpp.obj") if BUILD else None
SEED_REV = "3cb99d431^"
BODY_BASELINE = "9f6968cb580396f124711469d6b9dce933892cbd"

IDENTITIES = (
    (0x005F6550, "src/recovered/units/005f6550.cpp",
     "?on_l_button_up@Win@@QAEXHHIH@Z",
     "?on_l_button_up@Win@@UAEXHHIH@Z", "void Win::on_l_button_up("),
    (0x005F2CF0, "src/unrecovered/005f2cf0.cpp",
     "?on_redraw_nc@Win@@QAEXPAURECT@@H@Z",
     "?on_redraw_nc@Win@@UAEXPAUtagRECT@@H@Z", "void Win::on_redraw_nc("),
)

EXPECTED_IMAGE_SHA256 = (
    "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
)
WIN_VTABLE_VA = 0x0066FDD0
TABLE_CELLS = {82: (0x0066FF18, 0x005F6550),
               67: (0x0066FEDC, 0x005F2CF0)}


def source_row(text: str, path: Path, address: int):
    matches = [row for row in read_text(text, path) if row.address == address]
    assert len(matches) == 1, (
        f"expected exactly one mapping for 0x{address:08X}; found {len(matches)}")
    return matches[0]


def seed(path: str, address: int):
    raw = subprocess.check_output(["git", "show", f"{SEED_REV}:{path}"], cwd=ROOT)
    text = raw.decode("utf-8", "surrogateescape")
    rows = [row for row in read_text(text, ROOT / path)
            if row.address == address]
    assert len(rows) == 1
    return raw, text, rows[0]


def archived_lines(address: int, region: str) -> list[str]:
    selected = []
    for line in region.splitlines():
        stripped = line.strip()
        if stripped.startswith(("// ORIGINAL:", "// size", "// prototype",
                               "// callers", "// kind", "// flags",
                               "// calls", "// indirect", "// TRIED:")):
            selected.append(line)
        elif address == 0x005F6550 and stripped in {
            "// PRESERVED UNIT - measured MISMATCH.",
            "// address        0x005F6550",
            "// measured tier  MISMATCH",
            "// divergence     0",
        }:
            selected.append(line)
    assert selected
    return [re.sub(r"^// ORIGINAL:", "// original marker:", line)
            for line in selected]


def archive_for(text: str, address: int) -> tuple[str, list[str]]:
    head = re.compile(
        rf"(?ms)^/\*\n \* RECOVERY_HISTORY: v1 address=0x{address:08X} "
        rf"source_sha256=([0-9a-f]{{64}}) envelope_sha256=([0-9a-f]{{64}})\n"
        rf"(?P<body>.*?)^ \*/\n?"
    )
    matches = list(head.finditer(text))
    assert len(matches) == 1, f"expected one history archive for {address:08X}"
    match = matches[0]
    lines = re.findall(r"^ \* \[ARCHIVE\] (.*)$", match.group("body"), re.M)
    return match.group(1), lines


def fresh_coff(path: Path) -> Coff:
    receipt_path = path.with_name(path.name + ".compile.json")
    assert path.is_file() and receipt_path.is_file()
    receipt = json.loads(receipt_path.read_text())
    assert receipt["exit_code"] == 0
    assert receipt["output"]["path"] == str(path)
    assert not receipt["output"]["existed_before"]
    raw = path.read_bytes()
    assert receipt["output"]["after"] == {
        "sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}
    inputs = receipt.get("inputs")
    assert inputs, "compile receipt must carry source/header inputs"
    source_seen = False
    for item in inputs:
        input_path = Path(item["path"])
        assert input_path.is_file()
        assert item.get("before") == item.get("after")
        assert hashlib.sha256(input_path.read_bytes()).hexdigest() == item["after"]["sha256"]
        if input_path.resolve() == WIN_CPP.resolve():
            source_seen = True
            assert item["after"]["sha256"] == hashlib.sha256(WIN_CPP.read_bytes()).hexdigest()
    assert source_seen, "receipt did not compile the current src/win.cpp"
    return Coff(raw)


def function_definition(text: str, signature: str) -> str:
    starts = [match.start() for match in re.finditer(re.escape(signature), text)]
    assert len(starts) == 1, f"expected one definition of {signature}"
    start = starts[0]
    opening = text.find("{", start)
    assert opening >= 0
    depth, i, state = 0, opening, "code"
    while i < len(text):
        char = text[i]
        next_char = text[i + 1] if i + 1 < len(text) else ""
        if state == "line-comment":
            if char == "\n":
                state = "code"
        elif state == "block-comment":
            if char == "*" and next_char == "/":
                state = "code"
                i += 1
        elif state in ("string", "character"):
            if char == "\\":
                i += 1
            elif (state == "string" and char == '"') or (
                    state == "character" and char == "'"):
                state = "code"
        elif char == "/" and next_char == "/":
            state = "line-comment"
            i += 1
        elif char == "/" and next_char == "*":
            state = "block-comment"
            i += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "character"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
        i += 1
    raise AssertionError(f"unterminated function body for {signature}")


def test_restored_rows_match_seed_identity_claims_and_digest_bound_archives():
    text = WIN_CPP.read_text(encoding="utf-8", errors="surrogateescape")
    for address, seed_path, original_name, emitted_symbol, _signature in IDENTITIES:
        raw, _seed_text, old = seed(seed_path, address)
        current = source_row(text, WIN_CPP, address)
        assert current.mode is old.mode is Mode.FILE
        assert (current.name, current.image_spans) == (old.name, old.image_spans)
        assert current.name == original_name
        assert current.image_spans == ((address, 0x005F670D if address == 0x005F6550
                                        else 0x005F37BA),)
        assert current.symbol == emitted_symbol
        assert (current.byte_exact, current.semantic) == (old.byte_exact, old.semantic)
        assert not current.byte_exact and not current.semantic
        assert (current.kind, current.levers, current.ruled_out,
                current.unrecoverable, current.deferred) == (
                    old.kind, old.levers, old.ruled_out,
                    old.unrecoverable, old.deferred)
        assert hashlib.sha256(raw).hexdigest() == archive_for(text, address)[0]
        _source_digest, archived = archive_for(text, address)
        expected = archived_lines(address, old.region)
        assert archived == expected
        envelope = ("\n".join(archived) + "\n").encode("utf-8", "surrogateescape")
        block = re.search(
            rf"(?ms)^/\*\n \* RECOVERY_HISTORY: v1 address=0x{address:08X} "
            r"source_sha256=[0-9a-f]{64} envelope_sha256=([0-9a-f]{64})\n",
            text)
        assert block is not None
        assert hashlib.sha256(envelope).hexdigest() == block.group(1)


def test_pinned_win_primary_table_selects_both_original_entries():
    raw = PINNED_IMAGE.read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    assert digest == EXPECTED_IMAGE_SHA256
    image = PeImage(raw, digest)
    for slot, (expected_cell, expected_entry) in TABLE_CELLS.items():
        cell = WIN_VTABLE_VA + slot * 4
        assert cell == expected_cell
        actual_entry = int.from_bytes(image.read(Span(cell, cell + 4)), "little")
        assert actual_entry == expected_entry


def test_win_header_contracts_and_original_dispatch_orders_are_retained():
    header = WIN_HEADER.read_text(encoding="utf-8")
    assert re.search(r"virtual void on_redraw_nc\(RECT \*area, int flags\);\s*"
                     r"// slot 67\s+0x005F2CF0", header)
    assert re.search(r"virtual void on_l_button_up\(int x, int y, unsigned int keys, int dbl\);"
                     r"\s*// slot 82\s+0x005F6550", header)
    source = WIN_CPP.read_text(encoding="utf-8")
    click = function_definition(source, "void Win::on_l_button_up(")
    ordered_click = (
        "Win *captured = WinPointerOwner1;", "captured = WinPointerOwner2;",
        "WinPointerOwner1 = nullptr;", "WinPointerOwner2 = 0;",
        "if (isSelf)", "if (dbl == 0)", "field_414_", "cb(x, y);",
        "this->on_left_click(x, y);", "this->vslot_32(x, y);",
        "if (*reinterpret_cast<uint8_t *>(&iSomeFlag_) & 1)",
        "field_41C_", "this->on_left_up(x, y);", "this->on_nc_left_up(x, y);")
    assert [click.index(fragment) for fragment in ordered_click] == sorted(
        click.index(fragment) for fragment in ordered_click)

    redraw = function_definition(source, "void Win::on_redraw_nc(")
    quadrants = ("edge 0: top caption bar (buffer1_)",
                 "edge 2: bottom border (buffer4_)",
                 "edge 3: right border (buffer3_)",
                 "edge 1: left border (buffer2_)")
    assert [redraw.index(fragment) for fragment in quadrants] == sorted(
        redraw.index(fragment) for fragment in quadrants)
    for edge, buf, quadrant in ((0, "buffer1_", 0), (2, "buffer4_", 2),
                                (3, "buffer3_", 3), (1, "buffer2_", 1)):
        marker = f"edge {edge}:"
        start = redraw.index(marker)
        end = min((redraw.find(f"edge {next_edge}:", start + len(marker))
                   for next_edge in range(4) if redraw.find(f"edge {next_edge}:", start + len(marker)) >= 0),
                  default=len(redraw))
        section = redraw[start:end]
        assert f"this->{buf}" in section
        assert f"on_post_redraw_nc_buffer(buf, {quadrant})" in section


def test_cpp_function_bodies_are_unchanged_from_pre_annotation_checkpoint():
    current = WIN_CPP.read_text(encoding="utf-8", errors="surrogateescape")
    baseline = subprocess.check_output(
        ["git", "show", f"{BODY_BASELINE}:src/win.cpp"], cwd=ROOT
    ).decode("utf-8", "surrogateescape")
    for *_identity, signature in IDENTITIES:
        assert function_definition(current, signature) == function_definition(baseline, signature)


@pytest.mark.skipif(BUILD is None,
                    reason="optional fresh native object evidence not supplied")
def test_optional_fresh_object_has_both_published_external_definitions():
    """Qualify only definitions present in a fresh current-source COFF receipt."""
    coff = fresh_coff(WIN_OBJECT)
    text = WIN_CPP.read_text(encoding="utf-8", errors="surrogateescape")
    for address, _seed_path, _original, _observed, _signature in IDENTITIES:
        row = source_row(text, WIN_CPP, address)
        matches = [symbol for symbol in coff.symbols.values()
                   if symbol.name == row.symbol and symbol.storage == 2
                   and symbol.section > 0 and symbol.type & 0x20]
        assert len(matches) == 1, (
            f"{row.symbol}: expected one physical external function definition")

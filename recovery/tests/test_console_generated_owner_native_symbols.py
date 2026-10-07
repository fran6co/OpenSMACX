"""Post-build-only source-symbol/physical-COFF qualification.

Run only after setting OPENSMACX_CONSOLE_OWNER_BUILD to the fresh retained
application build directory. This is deliberately separate from pre-native
source/history checks and never predicts a compiler mangling.
"""
from __future__ import annotations

import json
import hashlib
import os
from pathlib import Path

from match_recovery.pe_coff import Coff, PeImage
from match_recovery.model import Span
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.source import read_file
import pytest

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
BUILD_ENV = os.environ.get("OPENSMACX_CONSOLE_OWNER_BUILD")
BUILD = Path(BUILD_ENV).resolve() if BUILD_ENV else None
OWNER_OBJ = BUILD / "CMakeFiles/OpenSMACX.dir/src/console.cpp.obj" if BUILD else None
RECEIPT = OWNER_OBJ.with_name(OWNER_OBJ.name + ".compile.json") if OWNER_OBJ else None
SPECS = (
    (0x0051DEF0, 31, 0x00510110),
    (0x0051DF10, 34, 0x0050FE70),
    (0x0051DF30, 60, 0x0050F960),
)


def record(path: Path, address: int):
    matches = [row for row in read_file(path) if row.address == address]
    assert len(matches) == 1
    return matches[0]


def fresh_coff(path):
    receipt_path = path.with_name(path.name + ".compile.json")
    assert path.is_file() and receipt_path.is_file()
    receipt = json.loads(receipt_path.read_text())
    assert receipt["exit_code"] == 0
    assert receipt["output"]["path"] == str(path)
    assert not receipt["output"]["existed_before"]
    raw = path.read_bytes()
    assert receipt["output"]["after"] == {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}
    assert receipt.get("inputs"), "compile receipt must carry source/header inputs"
    for item in receipt["inputs"]:
        path = Path(item["path"])
        assert path.is_file()
        assert item.get("before") == item.get("after")
        assert item.get("after", {}).get("sha256")
        assert hashlib.sha256(path.read_bytes()).hexdigest() == item["after"]["sha256"]
    return Coff(raw)


def test_actual_emitted_symbols_are_selected_by_fresh_console_coff_vftable():
    if BUILD is None:
        pytest.skip("post-native qualification: set OPENSMACX_CONSOLE_OWNER_BUILD")
    coff = fresh_coff(OWNER_OBJ)
    table_name = "??_7Console@@6BWin@@@"
    tables = [s for s in coff.symbols.values()
              if s.name == table_name and s.storage == 2 and s.section > 0]
    assert len(tables) == 1
    table = tables[0]
    _, table_relocs, _ = coff.section_subject(table.section)

    for address, slot, target_address in SPECS:
        owner = record(ROOT / "src/console.cpp", address)
        target = record(ROOT / "src/console.cpp", target_address)
        assert owner.symbol and "adjust_" not in owner.symbol
        assert target.symbol
        relocations = [rel for rel in table_relocs
                       if rel.offset == table.value + slot * 4
                       and str(rel.target).isdigit()]
        assert len(relocations) == 1, f"ambiguous Console Win slot {slot} relocation"
        thunk = coff.symbols[int(relocations[0].target)]
        assert thunk.name == owner.symbol, "source emitted-symbol fact must come from this physical COFF row"
        assert thunk.value == 0 and thunk.storage == 2 and thunk.type & 0x20
        section = coff.sections[thunk.section - 1]
        assert section.flags & 0x1000, "expected a compiler-owned COMDAT"
        definitions = [s for s in coff.symbols.values()
                       if s.section == thunk.section and s.storage == 2 and s.type & 0x20]
        assert [s.name for s in definitions] == [thunk.name]
        body, thunk_relocs, _ = coff.subject(thunk)
        assert len(body) == 8 and body[:4] == bytes.fromhex("2b49fce9")
        tails = [coff.symbols[int(rel.target)] for rel in thunk_relocs
                 if str(rel.target).isdigit() and rel.offset == thunk.value + 4]
        assert len(tails) == 1 and tails[0].name == target.symbol


def test_linked_console_table_and_tail_targets_agree_with_source_and_coff():
    # First establish freshness of the actual owner inputs and physical slots.
    test_actual_emitted_symbols_are_selected_by_fresh_console_coff_vftable()
    raw = (BUILD / "OpenSMACX.exe").read_bytes()
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    placements = parse_link_map((BUILD / "OpenSMACX.map").read_bytes())
    placements.validate(image)
    fixups = bound_fixups(image, (BUILD / "OpenSMACX.pdb").read_bytes())

    def placed(symbol):
        matches = [item for item in placements.publics if item.symbol == symbol]
        assert len(matches) == 1, symbol
        return matches[0]

    table_name = "??_7Console@@6BWin@@@"
    table_row = placed(table_name)
    # Header-generated COMDATs may be selected from another application TU.
    # Observe that actual producer rather than assigning it to console.cpp.
    assert Path(table_row.owner).name == table_row.owner and table_row.owner.endswith(".cpp.obj")
    selected = fresh_coff(BUILD / "CMakeFiles/OpenSMACX.dir/src" / table_row.owner)
    table, = [s for s in selected.symbols.values()
              if s.name == table_name and s.storage == 2 and s.section > 0]
    _, table_relocs, _ = selected.section_subject(table.section)
    table_address = table_row.address
    for address, slot, target_address in SPECS:
        owner = record(ROOT / "src/console.cpp", address)
        target = record(ROOT / "src/console.cpp", target_address)
        thunk_row = placed(owner.symbol)
        implementation_row = placed(target.symbol)
        assert implementation_row.owner.lower().endswith("console.cpp.obj")
        thunk_address = thunk_row.address
        implementation_address = implementation_row.address
        relocation, = [r for r in table_relocs if r.offset == table.value + slot * 4]
        selected_thunk = selected.symbols[int(relocation.target)]
        assert selected_thunk.name == owner.symbol
        selected_body, selected_relocs, _ = selected.subject(selected_thunk)
        assert selected_thunk.value == 0 and len(selected_body) == 8
        assert selected_body[:4] == bytes.fromhex("2b49fce9")
        tail, = [r for r in selected_relocs if r.offset == 4]
        assert selected.symbols[int(tail.target)].name == target.symbol
        cell = table_address + slot * 4
        assert int.from_bytes(image.read(Span(cell, cell + 4)), "little") == thunk_address
        assert image.absolute_sites(Span(cell, cell + 4)) == frozenset({0})
        cell_fixup = fixups[cell - image.base]
        assert cell_fixup.kind == 6 and cell_fixup.target_rva == thunk_address - image.base
        body = image.read(Span(thunk_address, thunk_address + 8))
        assert body[:4] == bytes.fromhex("2b49fce9")
        assert thunk_address + 8 + int.from_bytes(body[4:], "little", signed=True) == implementation_address
        tail_fixup = fixups[thunk_address + 4 - image.base]
        assert tail_fixup.kind == 20 and tail_fixup.target_rva == implementation_address - image.base

"""Source migration and pinned call controls, not original-TU/linked proof.

The frozen predecessor is the source transport witness. Future body recovery
must explicitly revise its transport-only assertions, not weaken identity or
behavior checks to claim a different source change was a verbatim migration.
"""
from collections import Counter
from dataclasses import asdict
import os
from pathlib import Path
import re
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM

from match_recovery.binding_source import parse as parse_bindings
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.model import Span
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source import function_line, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "779d87aaca10f80fbe7856245c43e77f033fc02a"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
CORE = "src/console.cpp"
COMMANDS = "src/console_commands.cpp"
HEADER = "src/console_commands.h"
REFRESH_CALLERS = (0x4E0120, 0x4E0210, 0x4D7DE0, 0x4D9970, 0x4CF7C0, 0x4D3470)


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT).decode()


def mask(source):
    return COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)


def method_body(source, record, ordered):
    method = re.match(r"\?([^@]+)@Console@@", record.symbol or record.name)
    assert method is not None
    following = next((r.line for r in ordered if r.line > record.line), len(source.splitlines()) + 1)
    line = function_line(source, "Console::" + method[1],
                         after_line=record.line - 1, before_line=following)
    start = sum(map(len, source.splitlines(keepends=True)[:line - 1]))
    code = mask(source)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated actual method")


def facts(record):
    return {key: getattr(record, key) for key in record.__dataclass_fields__
            if key not in {"path", "line", "region", "origin"}}


def lessons(record):
    return tuple((x.kind, x.key, x.prose, tuple(s for _, s in x.lines)) for x in record.origin or ())


def source_records(path, source=None):
    return read_text((ROOT / path).read_text() if source is None else source, ROOT / path)


def test_complete_original_identity_claim_and_lesson_transport():
    old = source_records(CORE, baseline(CORE))
    current = [*source_records(CORE), *source_records(COMMANDS)]
    assert len(old) == len(current) == 207
    assert Counter(r.address for r in current) == Counter(r.address for r in old)
    before = {r.address: r for r in old}
    for record in current:
        assert facts(record) == facts(before[record.address])
        assert lessons(record) == lessons(before[record.address])


def test_all_76_existing_order_and_editor_method_bodies_move_verbatim():
    before_text = baseline(CORE)
    before = sorted(source_records(CORE, before_text), key=lambda r: r.line)
    after_text = (ROOT / COMMANDS).read_text()
    after = sorted(source_records(COMMANDS), key=lambda r: r.line)
    selected = [r for r in before if 0x4C0000 <= r.address < 0x500000]
    by_address = {r.address: r for r in after}
    assert len(selected) == 76
    for record in selected:
        assert method_body(before_text, record, before) == method_body(after_text, by_address[record.address], after)


def test_existing_cleanup_mappings_follow_their_real_editor_lifetime():
    old = source_records(CORE, baseline(CORE))
    expected = {r.address for r in old if 0x65BFB1 <= r.address < 0x65C0BE}
    moved = source_records(COMMANDS)
    assert len(expected) == 25
    assert len(moved) == 76 + 25
    assert expected <= {r.address for r in moved}
    assert not expected & {r.address for r in source_records(CORE)}
    bindings = parse_bindings((ROOT / COMMANDS).read_text(), moved)
    for record in moved:
        if record.address in expected:
            assert "console.cpp" not in " ".join(bindings[record.line]["evidence"])
            assert "standalone" in bindings[record.line]["reason"] or "generated cleanup" in bindings[record.line]["reason"]


def test_object_bindings_types_and_initializers_are_preserved_once():
    original = baseline(CORE)
    texts = {unit: (ROOT / unit).read_text() for unit in (CORE, COMMANDS)}
    old_objects = parse_objects(original, CORE, PIN, address_space="pe:va")
    new_objects = [row for unit, text in texts.items()
                   for row in parse_objects(text, unit, PIN, address_space="pe:va")]
    def identity(row):
        return {key: value for key, value in asdict(row).items() if key not in {"unit", "line"}}
    assert len(old_objects) == len(new_objects)
    old_by = {row.origin.entity_id: identity(row) for row in old_objects}
    assert Counter(row.origin.entity_id for row in new_objects) == Counter(old_by.keys())
    for row in new_objects:
        assert identity(row) == old_by[row.origin.entity_id]
        # Exact declaration spelling protects array extent, signedness, pointer
        # versus object, and passive initialization. It is not layout proof.
        declaration = texts[row.unit].splitlines()[row.line]
        assert original.splitlines().count(declaration) == 1
        assert sum(text.splitlines().count(declaration) for text in texts.values()) == 1


def test_ui_and_nontrivial_lifetimes_remain_in_original_owner():
    core = mask((ROOT / CORE).read_text())
    commands = mask((ROOT / COMMANDS).read_text())
    for name in ("Console::update_data", "Console::set_move", "Console::on_key_click",
                 "Console::Console"):
        assert name in core
        assert name not in commands
    for text in ("Console ConsoleGlobal", "Time ConsoleTimer"):
        assert text in core
        assert text not in commands
    assert "int ConsoleGroupAction;" in core
    assert "int ConsoleControlTurnActive" in core
    assert (ROOT / "src/console.h").read_text() == baseline("src/console.h")


def test_shared_header_has_one_private_formatter_not_a_new_original_entry():
    header = (ROOT / HEADER).read_text()
    assert "static void parse_field_number(int index, int value) {" in header
    assert "ORIGINAL:" not in header
    assert "typedef int (__cdecl *NullCallback)(void);" in header
    for unit in (CORE, COMMANDS):
        source = (ROOT / unit).read_text()
        assert '#include "console_commands.h"' in source
        assert "parse_field_number(" in source
        assert "static void parse_field_number(" not in source


def test_cmake_adds_only_the_new_real_owner_in_existing_profile():
    current = (ROOT / "CMakeLists.txt").read_text()
    assert current.count("    src/console_commands.cpp\n") == 2
    assert current.replace("    src/console_commands.cpp\n", "") == baseline("CMakeLists.txt")
    assert current.count("    src/console.cpp\n") == 2


def test_original_six_command_callers_directly_call_ui_refresh():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    image = PeImage(path.read_bytes(), PIN)
    records = {r.address: r for r in source_records(COMMANDS)}
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for address in REFRESH_CALLERS:
        start, end = records[address].image_spans[0]
        calls = [ins.operands[0].imm for ins in decoder.disasm(image.read(Span(start, end)), start)
                 if ins.mnemonic == "call" and ins.operands and ins.operands[0].type == X86_OP_IMM]
        assert 0x514880 in calls, hex(address)


def test_identity_control_rejects_missing_or_duplicate_generated_cleanup():
    source = (ROOT / COMMANDS).read_text()
    records = source_records(COMMANDS, source)
    old_addresses = Counter(r.address for r in records)
    marker = "// ORIGINAL: 0x0065BFBC "
    assert source.count(marker) == 1
    damaged = source.replace(marker, "// Removed identity: 0x0065BFBC ", 1)
    assert Counter(r.address for r in source_records(COMMANDS, damaged)) != old_addresses

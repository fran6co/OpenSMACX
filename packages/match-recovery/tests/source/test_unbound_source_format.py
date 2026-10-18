"""Source-core format contract for bodyless original mappings."""

from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.source import (DecompilationState, Mode, Recipe, SourceRole,
                                  State, read_text, write, write_body,
                                  write_context, write_symbol)


HOST = Path("src/unbound_functions.h")
TEXT = """// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401004;0x00401010-0x00401014 UNBOUND BYTE_EXACT SEMANTIC
// kind game
// LEVER: old-key prior evidence remains a note
// TRIED: tested spelling
// DEFERRED: real owner is not established
int incidental_function() { return 1; }
// ORIGINAL: 0x00401020 ?g@@YAHXZ 0x00401020-0x00401024
int g() { return 2; }
"""


def test_unbound_roundtrip_retains_identity_claims_lessons_and_does_not_absorb_code():
    rows = read_text(TEXT, HOST)
    assert len(rows) == 2
    unbound, body = rows
    assert (unbound.address, unbound.name, unbound.image_spans) == (
        0x00401000, "?f@@YAHXZ", ((0x00401000, 0x00401004), (0x00401010, 0x00401014)))
    assert (unbound.mode, unbound.state, unbound.recipe) == (
        Mode.UNBOUND, State.UNBOUND, Recipe.UNBOUND)
    assert unbound.byte_exact and unbound.semantic
    assert unbound.region == unbound.symbol == unbound.body == unbound.context == ""
    assert not unbound.extract_error and unbound.kind == "game"
    assert unbound.levers == (("old-key", "prior evidence remains a note"),)
    assert unbound.ruled_out == ("tested spelling",)
    assert unbound.deferred == ("real owner is not established",)
    assert body.mode is Mode.BODY and body.region.endswith("int g() { return 2; }\n")
    rewritten = write(TEXT, rows)
    assert rewritten == TEXT
    reread = read_text(rewritten, HOST)
    assert [(row.address, row.mode, row.state, row.recipe, row.image_spans,
             row.byte_exact, row.semantic, row.levers, row.ruled_out, row.deferred)
            for row in reread] == [
                (row.address, row.mode, row.state, row.recipe, row.image_spans,
                 row.byte_exact, row.semantic, row.levers, row.ruled_out, row.deferred)
                for row in rows]


@pytest.mark.parametrize("marker", [
    "// ORIGINAL: 0x00401000 UNBOUND",
    "// ORIGINAL: 0x00401000 ?f@@YAHXZ UNBOUND",
    "// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401000 UNBOUND",
    "// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401010-0x00401018 UNBOUND",
    "// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401010-0x00401018;0x00401008-0x00401009 UNBOUND",
])
def test_unbound_requires_nonempty_valid_identity_extent(marker):
    with pytest.raises(ValueError, match="UNBOUND"):
        read_text(marker + "\n", HOST)


@pytest.mark.parametrize("tail", [
    "UNBOUN", "UNBOUND FILE", "UNBOUND EXCLUDED S1", "UNBOUND UNBOUND",
    "FILE UNBOUND", "EXCLUDED S1 UNBOUND", "UNBOUND FUTURE_MODE",
])
def test_unbound_typos_duplicates_and_mixed_modes_fail_closed(tail):
    marker = f"// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401008 {tail}\n"
    with pytest.raises(ValueError, match="UNBOUND"):
        read_text(marker, HOST)


@pytest.mark.parametrize("fact", ["symbol ?f@@YAHXZ", "body src/f.cpp", "compile_context src/f.cpp"])
def test_unbound_rejects_current_emission_candidate_facts(fact):
    text = ("// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401008 UNBOUND\n"
            f"// {fact}\n")
    with pytest.raises(ValueError, match="UNBOUND"):
        read_text(text, HOST)


def test_unbound_rejects_role_and_fragment_relations():
    role = ("// RECOVERY_ROLE: SCAFFOLD temporary adapter\n"
            "// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401008 UNBOUND\n")
    with pytest.raises(ValueError, match="UNBOUND"):
        read_text(role, HOST)
    fragment = ("// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401008 UNBOUND\n"
                '// RECOVERY_FRAGMENT: v1 {"parent_address":"0x00401000",'
                '"parent_symbol":"?f@@YAHXZ","reason":"test","evidence":["test"]}\n')
    with pytest.raises(ValueError, match="FILE"):
        read_text(fragment, HOST)


def test_programmatic_records_and_writers_cannot_turn_unbound_into_a_body_or_emission():
    row, = read_text("// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401008 UNBOUND BYTE_EXACT\n", HOST)
    raw = ("// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401008 UNBOUND BYTE_EXACT\n").encode()
    with pytest.raises(ValueError, match="UNBOUND"):
        DecompilationState(0x00401000, Mode.UNBOUND, State.UNBOUND, HOST, 1,
                          "?f@@YAHXZ", (), recipe=Recipe.UNBOUND)
    with pytest.raises(ValueError, match="UNBOUND"):
        replace(row, mode=Mode.BODY)
    with pytest.raises(ValueError, match="UNBOUND"):
        replace(row, symbol="?f@@YAHXZ")
    for invalid_name in ("?f@@YAHXZ trailing", " ?f@@YAHXZ", "?f@@YAHXZ//comment",
                         "name*tail", "name\x7ftail", "name\x80tail"):
        with pytest.raises(ValueError, match="UNBOUND"):
            replace(row, name=invalid_name)
    with pytest.raises(ValueError, match="UNBOUND"):
        replace(row, source_role=SourceRole.SCAFFOLD, source_role_reason="adapter")
    for field, value in (("mode", "unbound"), ("state", "unbound"),
                         ("recipe", "unbound")):
        with pytest.raises(ValueError, match="UNBOUND"):
            replace(row, **{field: value})
    # Records are mutable, so the writer repeats the invariant check too.
    row.region = "int f() { return 1; }"
    with pytest.raises(ValueError, match="UNBOUND"):
        write(raw.decode(), [row])
    row.region = ""
    for update in (write_symbol, write_body, write_context):
        with pytest.raises(ValueError, match="UNBOUND"):
            update(raw, row, "src/f.cpp")

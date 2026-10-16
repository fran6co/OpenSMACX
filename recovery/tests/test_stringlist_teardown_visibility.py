"""Shared source visibility and bounded teardown-order controls."""

from match_recovery.source import read_file
from match_recovery.source.model import Mode
from test_pact_ascent import ROOT, definition


def test_shared_header_owns_real_walk_and_both_destructor_stages():
    header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    base = definition(header, r"~StringStruct\s*\(\)")
    derived = definition(header, r"~StringList\s*\(\)")
    walk = definition(header, r"void\s+remove_all\s*\(\)")

    # Retain the source's list walk order and loop-bound reread. This checks
    # shared C++ definitions, not VC6 code generation or EH-state equivalence.
    ordered = (
        "if (!head_)",
        "int index = 0;",
        "if (entry_count_ > 0)",
        "current_ = entry->next;",
        "on_remove(payload);",
        "delete payload;",
        "head_->payload = 0;",
        "delete head_;",
        "++index;",
        "head_ = current_;",
        "while (index < entry_count_);",
        "head_ = 0;",
        "current_position_ = 0;",
        "entry_count_ = 0;",
    )
    offsets = [walk.index(token) for token in ordered]
    assert offsets == sorted(offsets)
    assert "remove_all();" in base
    assert "remove_all();" in derived
    assert "current_position_ = 0;" in base
    assert "current_position_ = 0;" in derived


def test_mapped_teardown_entries_and_inline_walk_identity_are_retained():
    base_rows = read_file(ROOT / "src/stringstruct.cpp")
    derived_rows = read_file(ROOT / "src/stringlist.cpp")
    base_dtor, = (row for row in base_rows if row.address == 0x00401060)
    derived_dtor, = (row for row in derived_rows if row.address == 0x004066C0)
    remove_all, = (row for row in base_rows if row.address == 0x00402970)
    allocation_dtor, = (row for row in base_rows if row.address == 0x00402310)
    assert base_dtor.mode is Mode.FILE and derived_dtor.mode is Mode.FILE
    assert remove_all.mode is Mode.FILE and remove_all.byte_exact
    for row in (base_dtor, derived_dtor, remove_all):
        assert "// body      src/stringstruct.h" in row.region
    assert allocation_dtor.mode is Mode.BODY and allocation_dtor.byte_exact
    assert "StringAllocationHeap = allocation_owner_;" in allocation_dtor.region

    header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    assert "void remove_all() {" in header

"""Shared source visibility and bounded teardown-order controls."""

import re

from match_recovery.source import read_file
from match_recovery.source.model import Mode
from test_pact_ascent import ROOT, definition
from owner_transfer_test_support import current_record_by_address
from test_secret_project_rules_owner_actual_source import code_only


def assert_full_destructor_operation(header, walk):
    """Qualify the actual full operation against the unchanged public walk.

    The public empty-head return must not skip the destructor's final reset.
    This is source-composition proof, separate from emitted ABI and host probes.
    """
    operation = definition(header, r"static\s+void\s+destroy_owned_entries\s*\(")

    def tokens(text):
        return re.findall(r"\w+|[^\s]", code_only(text))

    public_body = walk[walk.index("{") + 1:walk.rindex("}")]
    public_tokens = tokens(public_body)
    guard = tokens("if (!head_) { return; }")
    assert public_tokens[:len(guard)] == guard
    expected = (tokens("if (head_ != 0) {") + public_tokens[len(guard):]
                + tokens("} current_position_ = 0;"))
    actual = operation[operation.index("{") + 1:operation.rindex("}")]
    assert tokens(actual.replace("list->", "")) == expected
    return operation


def test_shared_header_owns_real_walk_and_both_destructor_stages():
    header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    source = (ROOT / "src/stringstruct.cpp").read_text(encoding="utf-8")
    base = definition(header, r"~StringStruct\s*\(\)")
    derived = definition(header, r"~StringList\s*\(\)")
    walk = definition(source, r"void\s+StringStruct::remove_all\s*\(\)")

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
    assert_full_destructor_operation(header, walk)
    actual = base[base.index("{") + 1:base.rindex("}")]
    assert actual.strip() == "destroy_owned_entries(this);"
    actual = derived[derived.index("{") + 1:derived.rindex("}")]
    assert actual.strip() == "StringStruct::remove_all();\n    current_position_ = 0;"
    derived_header = header[header.index("class StringList : public StringStruct"):]
    derived_clear = definition(derived_header, r"void\s+remove_all\s*\(\)")
    assert derived_clear[derived_clear.index("{"):] == walk[walk.index("{"):]


def test_mapped_teardown_entries_and_inline_walk_identity_are_retained():
    derived_rows = read_file(ROOT / "src/stringlist.cpp")
    base_rows = read_file(ROOT / "src/stringstruct.cpp")
    base_dtor, = (row for row in base_rows if row.address == 0x00401060)
    derived_dtor, = (row for row in derived_rows if row.address == 0x004066C0)
    remove_all, = (row for row in base_rows if row.address == 0x00402970)
    allocation_dtor = current_record_by_address(ROOT, 0x00402310)
    assert base_dtor.mode is Mode.FILE and derived_dtor.mode is Mode.FILE
    assert remove_all.mode is Mode.FILE and remove_all.byte_exact
    for row in (base_dtor, derived_dtor):
        assert row.body == "src/stringstruct.h"
    assert remove_all.body == "src/stringstruct.cpp"
    assert remove_all.context == "src/stringstruct.cpp"
    assert allocation_dtor.mode is Mode.FILE and allocation_dtor.byte_exact
    assert allocation_dtor.body == "src/stringstruct.h"
    assert allocation_dtor.context == "src/alphanet.cpp"
    base_header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    allocation_body = definition(
        base_header, r"virtual\s+~StringAllocationBase\s*\(\)")
    assert "StringAllocationHeap = allocation_owner_;" in allocation_body

    header = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    base_header = header[:header.index("class StringList : public StringStruct")]
    assert "void remove_all();" in base_header
    assert "void remove_all() {" not in base_header

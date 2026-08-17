"""The read -> write -> read loop.

Fixtures exercise every feature of the annotation grammar, and the last
test closes the loop over the whole tree. Everything is proved against
`src/` and purpose-built text alone - the package is NOT compared against
the `tools/` copies here; that comparison is the drift check in
`python -m decomp`.

Run with `uv run pytest decomp/tests`.
"""

from pathlib import Path

import pytest

from decomp import (DecompilationState, Mode, State, read, read_file, remove,
                    remove_file, write, write_file)
from decomp.writer import roundtrip_tree

FIXTURE = Path("fixture.cpp")


def record(address, line, mode=Mode.BODY, state=State.IMPLEMENTED, **kwargs):
    return DecompilationState(address=address, mode=mode, state=state,
                              path=FIXTURE, line=line, **kwargs)


def loop(text):
    """read -> write -> read; the records the loop must preserve."""
    records = read(text, FIXTURE)
    return records, read(write(text, records), FIXTURE)


def keys(records):
    from decomp.writer import _key
    return [_key(r) for r in records]


# ------------------------------------------------------------- round trips

BODY = """// ORIGINAL: 0x00401000
void __cdecl f() {
    return;
}
"""


def test_body_marker():
    records, reread = loop(BODY)
    assert len(records) == 1
    assert records[0].address == 0x00401000
    assert records[0].mode is Mode.BODY
    assert records[0].state is State.IMPLEMENTED
    assert keys(records) == keys(reread)
    # Canonical in, byte-identical out.
    assert write(BODY, records) == BODY


FILE_TEXT = """// ORIGINAL: 0x00402000 FILE
// BODY GOES HERE.
"""


def test_file_mode():
    records, reread = loop(FILE_TEXT)
    assert len(records) == 1
    assert records[0].mode is Mode.FILE
    assert records[0].state is State.PLACEHOLDER   # derived from the sentinel
    assert keys(records) == keys(reread)
    assert write(FILE_TEXT, records) == FILE_TEXT


EXCLUDED_TEXT = """// ORIGINAL: 0x00403000 EXCLUDED S1
"""


def test_excluded():
    records, reread = loop(EXCLUDED_TEXT)
    assert records[0].state is State.EXCLUDED
    assert records[0].exclusion == "S1"
    assert keys(records) == keys(reread)
    assert write(EXCLUDED_TEXT, records) == EXCLUDED_TEXT


MATCHED_TEXT = """// ORIGINAL: 0x00404000 BYTE_EXACT
void __cdecl g() {
}
"""


def test_byte_exact():
    records, reread = loop(MATCHED_TEXT)
    assert records[0].byte_exact is True
    assert records[0].state is State.IMPLEMENTED
    assert keys(records) == keys(reread)
    assert write(MATCHED_TEXT, records) == MATCHED_TEXT


LESSON_TEXT = """// ORIGINAL: 0x00405000
// LEVER: fp-1234 the cast that landed it
// RULED-OUT: plain ret 4 leaves ECX live
// UNRECOVERABLE: reads three registers live-in
// DEFERRED: needs the vtable order first
void __cdecl h() {
}
"""


def test_lessons():
    records, reread = loop(LESSON_TEXT)
    assert records[0].levers == (("fp-1234", "the cast that landed it"),)
    assert records[0].ruled_out == ("plain ret 4 leaves ECX live",)
    assert records[0].unrecoverable == ("reads three registers live-in",)
    assert records[0].deferred == ("needs the vtable order first",)
    assert keys(records) == keys(reread)
    assert write(LESSON_TEXT, records) == LESSON_TEXT


WRAPPED_LESSON = """// ORIGINAL: 0x00405100
// RULED-OUT: plain ret 4 leaves ECX live,
//            so the stub must zero it first
void __cdecl h() {
}
"""


def test_wrapped_lesson_canonicalises():
    records = read(WRAPPED_LESSON, FIXTURE)
    # The reader joins the continuation with a space.
    assert records[0].ruled_out == \
        ("plain ret 4 leaves ECX live, so the stub must zero it first",)
    rewritten = write(WRAPPED_LESSON, records)
    assert rewritten != WRAPPED_LESSON          # one canonical line now
    reread = read(rewritten, FIXTURE)
    assert keys(records) == keys(reread)
    # The fixed point: canonical is stable.
    assert write(rewritten, reread) == rewritten


MULTI = """// ORIGINAL: 0x00406000
void __cdecl a() {
}

// ORIGINAL: 0x00406010
void __cdecl b() {
}
"""


def test_several_markers():
    records, reread = loop(MULTI)
    assert [r.address for r in records] == [0x00406000, 0x00406010]
    assert keys(records) == keys(reread)
    assert write(MULTI, records) == MULTI


# ------------------------------------------- write trusts the records it gets

TWO_WITH_LESSONS = """// ORIGINAL: 0x00407000
// LEVER: fp-1 what landed A
void __cdecl a() {
}

// ORIGINAL: 0x00407010
// RULED-OUT: the spelling that failed B
void __cdecl b() {
}
"""


def test_unclaimed_markers_pass_through():
    records = read(TWO_WITH_LESSONS, FIXTURE)
    described = [r for r in records if r.address == 0x00407000]
    rewritten = write(TWO_WITH_LESSONS, described)
    reread = read(rewritten, FIXTURE)
    # A is rewritten canonically; B was not described, so it is untouched.
    assert [r.address for r in reread] == [0x00407000, 0x00407010]
    assert reread[0].levers == (("fp-1", "what landed A"),)
    assert "0x00407010" in rewritten
    assert "the spelling that failed B" in rewritten


def test_empty_records_leave_the_text_untouched():
    assert write(TWO_WITH_LESSONS, []) == TWO_WITH_LESSONS


def test_append_past_the_last_line():
    text = "void __cdecl f() {\n}\n"
    made = [record(0x00408100, line=len(text.splitlines()) + 1)]
    rewritten = write(text, made)
    assert rewritten.splitlines()[-1] == "// ORIGINAL: 0x00408100"


# ------------------------------------------------- remove is a statement


def test_remove_deletes_named_records():
    records = read(TWO_WITH_LESSONS, FIXTURE)
    drop = [r for r in records if r.address == 0x00407010]
    rewritten = remove(TWO_WITH_LESSONS, drop)
    reread = read(rewritten, FIXTURE)
    assert [r.address for r in reread] == [0x00407000]
    # B's marker and lesson run are gone; B's code and all of A remain.
    assert "0x00407010" not in rewritten
    assert "the spelling that failed B" not in rewritten
    assert "void __cdecl b() {" in rewritten
    assert "// ORIGINAL: 0x00407000" in rewritten
    assert "what landed A" in rewritten


def test_remove_all_leaves_only_the_code():
    records = read(TWO_WITH_LESSONS, FIXTURE)
    rewritten = remove(TWO_WITH_LESSONS, records)
    assert "ORIGINAL:" not in rewritten
    assert "LEVER:" not in rewritten
    assert "RULED-OUT:" not in rewritten
    assert "void __cdecl a() {" in rewritten
    assert "void __cdecl b() {" in rewritten


def test_remove_refusals():
    with pytest.raises(ValueError, match="marker-addressable"):
        remove("anything\n", [record(0x409000, line=0)])
    with pytest.raises(ValueError, match="past the end"):
        remove("one line\n", [record(0x409000, line=99)])


# ----------------------------------------------------------------- refusals


def test_refuses_filename_derived_records():
    with pytest.raises(ValueError, match="marker-addressable"):
        write("anything\n", [record(0x409000, line=0)])


def test_refuses_file_and_excluded_together():
    bad = record(0x409000, line=1, mode=Mode.FILE, state=State.EXCLUDED)
    with pytest.raises(ValueError, match="mutually exclusive"):
        write("anything\n", [bad])


def test_refuses_a_line_past_the_end():
    with pytest.raises(ValueError, match="past the end"):
        write("one line\n", [record(0x409000, line=99)])


def test_refuses_two_records_on_one_line():
    with pytest.raises(ValueError, match="claimed by another"):
        write("anything\n",
              [record(0x409000, line=1), record(0x409001, line=1)])


def test_write_file_uses_the_paths_records_carry(tmp_path):
    path = tmp_path / "unit.cpp"
    path.write_text(WRAPPED_LESSON)
    write_file(read_file(path))
    # The wrapped lesson is canonicalised on disk, one line.
    assert ("// RULED-OUT: plain ret 4 leaves ECX live, "
            "so the stub must zero it first") in path.read_text()


def test_write_file_groups_records_by_path(tmp_path):
    a, b = tmp_path / "a.cpp", tmp_path / "b.cpp"
    a.write_text(WRAPPED_LESSON)
    b.write_text(BODY)
    write_file(read_file(a) + read_file(b))
    assert "so the stub must zero it first" in a.read_text()
    assert b.read_text() == BODY


def test_remove_file(tmp_path):
    path = tmp_path / "unit.cpp"
    path.write_text(BODY)
    remove_file(read_file(path))
    assert "ORIGINAL:" not in path.read_text()
    assert "void __cdecl f() {" in path.read_text()


# ------------------------------------------------------------- the loop


def test_tree_roundtrip():
    """Every annotated file in src/ survives read -> write -> read.

    Measured zero skips when this landed: a new skip is an event worth
    investigating - a legacy inline spelling landed, or the writer and the
    reader disagreeing about what an annotation owns.
    """
    looped, skipped = roundtrip_tree()
    assert looped > 3000, looped
    assert skipped == 0, skipped

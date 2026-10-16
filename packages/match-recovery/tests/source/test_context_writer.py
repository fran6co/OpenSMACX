from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.source import read_text, write_context


def parsed(raw):
    rows = read_text(raw.decode(), Path("src/owner.cpp"))
    assert len(rows) == 1
    return rows[0]


def test_context_prose_with_or_without_path_is_not_treated_as_a_fact():
    raw = (b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\n"
           b"// context from the first token rather than behind a file read. This\n"
           b"// context from src/legacy.cpp in ordinary prose.\n"
           b"void f() {}\n")
    assert parsed(raw).context == ""


def test_context_roundtrip_inserts_and_updates_only_context():
    raw = (b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004 BYTE_EXACT\n"
           b"// TRIED: historical note\nvoid f() {}\n")
    before = parsed(raw)
    changed = write_context(raw, before, "src/emitter.cpp")
    after = parsed(changed)
    assert after.context == "src/emitter.cpp"
    assert after.address == before.address and after.image_spans == before.image_spans
    assert after.byte_exact and after.ruled_out == before.ruled_out
    assert b"// compile_context src/emitter.cpp\n" in changed
    assert b"// TRIED: historical note\n" in changed
    assert write_context(changed, after, "src/other.cpp") == changed.replace(
        b"src/emitter.cpp", b"src/other.cpp")


@pytest.mark.parametrize("value", ["../outside.cpp", "/tmp/unit.cpp", "src\\unit.cpp",
                                    "src/a b.cpp"])
def test_context_writer_rejects_unsafe_or_non_tu_paths(value):
    raw = b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\nvoid f() {}\n"
    with pytest.raises(ValueError):
        write_context(raw, parsed(raw), value)


def test_context_writer_accepts_canonical_single_component_path_syntax():
    raw = b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\nvoid f() {}\n"
    changed = write_context(raw, parsed(raw), "unit.cpp")
    assert parsed(changed).context == "unit.cpp"


def test_context_writer_rejects_duplicate_malformed_and_stale_facts():
    duplicate = (b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\n"
                 b"// compile_context src/one.cpp\n// compile_context src/two.cpp\nvoid f() {}\n")
    with pytest.raises(ValueError, match="duplicate"):
        parsed(duplicate)
    original = b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\n// compile_context src/old.cpp\nvoid f() {}\n"
    with pytest.raises(ValueError):
        write_context(duplicate, parsed(original), "src/new.cpp")
    malformed = (b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\n"
                 b"// compile_context ../outside.cpp\nvoid f() {}\n")
    with pytest.raises(ValueError):
        write_context(malformed, parsed(malformed), "src/new.cpp")
    raw = b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401004\nvoid f() {}\n"
    with pytest.raises(ValueError, match="current record snapshot"):
        write_context(raw, replace(parsed(raw), context="src/fabricated.cpp"), "src/new.cpp")

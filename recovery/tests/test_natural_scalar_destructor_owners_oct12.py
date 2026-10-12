"""Continuity controls for scalar deleting destructors in real class owners.

These checks cover source annotations and C++ ownership against a fixed source
baseline. They do not prove fresh compiler output or linked selection.
"""
from __future__ import annotations

from pathlib import Path
import re
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import Mode, read_text


ROOT = Path(__file__).resolve().parents[2]
BASELINE = "92d876fe14fd75508d7df4031cbd8c9a90d58a11"
OWNERS = {
    0x00406F20: ("src/scroll.cpp", "src/scroll.h", "Scroll",
                "??_GScroll@@UAEPAXI@Z", "?scroll_scalar_dtor_redirect@@YIPAXPAVScroll@@PAXI@Z",
                "Scroll::~Scroll()"),
    0x004C92D0: ("src/sound.cpp", "src/sound.h", "Sound",
                "??_GSound@@UAEPAXI@Z", "?sound_scalar_dtor_redirect@@YIPAXPAVSound@@PAXI@Z",
                "Sound::~Sound()"),
    0x004C9300: ("src/wave.cpp", "src/wave.h", "Wave",
                "??_GWave@@UAEPAXI@Z", "?wave_scalar_dtor_redirect@@YIPAXPAVWave@@PAXI@Z",
                "Wave::~Wave()"),
}
CONTINUITY_FIELDS = (
    "name", "image_spans", "kind", "byte_exact", "semantic", "exclusion",
    "levers", "ruled_out", "unrecoverable", "deferred",
)


def _baseline(relative: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{BASELINE}:{relative}"], cwd=ROOT, check=True,
        capture_output=True, text=True, timeout=30,
    )
    return result.stdout


def _record_map(text: str, path: str):
    return read_text(text, ROOT / path)


def _assert_mapping(text: str, path: str, address: int, emitted: str, mode: Mode):
    matches = [row for row in _record_map(text, path) if row.address == address]
    assert len(matches) == 1, f"expected one parsed mapping for {address:08X}"
    row = matches[0]
    assert row.mode is mode
    assert row.symbol == emitted
    return row


def _annotation_comment(text: str, address: int) -> str:
    marker = f"// ORIGINAL: 0x{address:08X}"
    assert text.count(marker) == 1, f"expected one active marker for {address:08X}"
    at = text.index(marker)
    start = text.rfind("/*", 0, at)
    end = text.find("*/", at)
    assert start >= 0 and end >= 0, f"missing annotation block for {address:08X}"
    return text[start:end + 2]


def _masked(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )


def _extract_cpp_block(text: str, signature: str) -> str:
    masked = _masked(text)
    definitions = []
    offset = 0
    while (at := masked.find(signature, offset)) >= 0:
        offset = at + len(signature)
        opening = offset
        while opening < len(masked) and masked[opening].isspace():
            opening += 1
        if opening < len(masked) and masked[opening] == "{":
            definitions.append((at, opening))
    assert len(definitions) == 1, f"expected one definition for {signature}"
    at, opening = definitions[0]
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return text[at:index + 1]
    raise AssertionError(f"unterminated body after {signature}")


def _class_declaration(text: str, class_name: str) -> str:
    masked = _masked(text)
    match = re.search(rf"\bclass\s+{re.escape(class_name)}\b", masked)
    assert match, f"missing class {class_name}"
    opening = masked.find("{", match.end())
    assert opening >= 0, f"missing class body for {class_name}"
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                end = masked.find(";", index)
                assert end >= 0, f"missing class terminator for {class_name}"
                return text[match.start():end + 1]
    raise AssertionError(f"unterminated class {class_name}")


def test_original_annotations_and_historical_notes_survive_in_real_owners():
    product_rows = {}
    for address, (cpp, _header, _class, emitted, _wrapper, _dtor) in OWNERS.items():
        old_text = _baseline(cpp)
        old_rows = [row for row in _record_map(old_text, cpp) if row.address == address]
        current_text = (ROOT / cpp).read_text(encoding="utf-8")
        assert len(old_rows) == 1, f"fixed baseline lacks unique {address:08X}"
        old = old_rows[0]
        assert old.mode is Mode.BODY
        current = _assert_mapping(current_text, cpp, address, emitted, Mode.FILE)
        for field in CONTINUITY_FIELDS:
            assert getattr(current, field) == getattr(old, field), (hex(address), field)
        assert current.path == ROOT / cpp

        old_comment = _annotation_comment(old_text, address)
        old_marker = next(line for line in old_comment.splitlines()
                          if line.startswith(f"// ORIGINAL: 0x{address:08X}"))
        new_marker = old_marker.replace(" BYTE_EXACT", " FILE BYTE_EXACT")
        old_symbol = next(line for line in old_comment.splitlines()
                          if line.startswith("// symbol"))
        new_symbol = f"// symbol    {emitted}"
        expected = old_comment.replace(old_marker, new_marker, 1).replace(
            old_symbol, new_symbol, 1)
        assert _annotation_comment(current_text, address) == expected
        product_rows[address] = current

    assert set(product_rows) == set(OWNERS)


def test_generated_entries_remain_owned_by_unchanged_class_lifetimes():
    for _address, (cpp, header, class_name, emitted, _wrapper, destructor) in OWNERS.items():
        current_header = (ROOT / header).read_text(encoding="utf-8")
        baseline_header = _baseline(header)
        assert _class_declaration(current_header, class_name) == _class_declaration(
            baseline_header, class_name)

        current_cpp = (ROOT / cpp).read_text(encoding="utf-8")
        baseline_cpp = _baseline(cpp)
        assert _extract_cpp_block(current_cpp, destructor) == _extract_cpp_block(
            baseline_cpp, destructor)
        assert emitted in product_rows_text(current_cpp, _address)

    sound_class = _class_declaration((ROOT / "src/sound.h").read_text(), "Sound")
    assert "virtual ~Sound();" in sound_class


def product_rows_text(text: str, address: int) -> str:
    """Return the parser-owned annotation region for an owner integrity check."""
    matches = [row for row in _record_map(text, "owner.cpp") if row.address == address]
    assert len(matches) == 1
    return matches[0].region


@pytest.mark.parametrize("damage", ("removed", "wrong-symbol"))
def test_source_parser_rejects_removed_or_renamed_generated_mappings(damage):
    address = 0x00406F20
    emitted = "??_GScroll@@UAEPAXI@Z"
    path = "src/scroll.cpp"
    text = (ROOT / path).read_text(encoding="utf-8")
    if damage == "removed":
        text = text.replace("// ORIGINAL: 0x00406F20", "// historical original: 0x00406F20", 1)
    else:
        text = text.replace(f"// symbol    {emitted}", "// symbol    ??_GWrong@@UAEPAXI@Z", 1)
    with pytest.raises(AssertionError):
        _assert_mapping(text, path, address, emitted, Mode.FILE)


def test_no_active_scalar_destructor_redirects_or_prototypes_remain():
    pattern = re.compile(r"\b(?:scroll|sound|wave)_scalar_dtor_redirect\b")
    source_paths = [
        path for path in (ROOT / "src").rglob("*")
        if path.is_file() and path.suffix.lower() in {".c", ".cc", ".cpp", ".h", ".hpp", ".cxx"}
    ]
    assert source_paths
    for path in source_paths:
        active_code = _masked(path.read_text(encoding="utf-8", errors="replace"))
        assert not pattern.search(active_code), f"active wrapper/prototype in {path.relative_to(ROOT)}"

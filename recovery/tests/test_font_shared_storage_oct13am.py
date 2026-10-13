"""Bounded original identities for Font's three independent shared objects."""
from pathlib import Path
import subprocess

from match_recovery.model import Origin, Span
from match_recovery.object_source import ObjectBinding, parse_objects
from match_recovery.object_writer import write_object_bytes


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/jackal/font.cpp"
BASE = "6b3990cdc5738bda3f775c663f87406c820a0d7a"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
UNIT = "src/jackal/font.cpp"
ADDRESS_SPACE = "pe:va"
EVIDENCE = 0x006195B0
NOTE = (
    b"// These three process-wide Font objects have independent, loader-zeroed identities.\n"
    b"// Font::init_font_class publishes FontDefault/HDC and adjusts the shared count;\n"
    b"// close_font_class clears the HDC/default pointer while preserving its signed\n"
    b"// counter and failure behavior. Address adjacency does not prove an aggregate.\n"
)
EXPECTED = {
    0x009BB484: ("?FontDefault@@3PAVFont@@A", "Font *FontDefault;"),
    0x009BB488: ("?FontHDC@Font@@2PAUHDC__@@A", "HDC Font::FontHDC;"),
    0x009BB48C: ("?FontInitCount@Font@@2HA", "int Font::FontInitCount;"),
}


def _baseline_bytes():
    return subprocess.check_output(
        ["git", "show", f"{BASE}:{UNIT}"], cwd=ROOT, timeout=30
    )


def _bindings_for_writer(raw):
    lines = raw.decode("utf-8").splitlines()
    declarations = {
        "Font *FontDefault;": 0x009BB484,
        "HDC Font::FontHDC;": 0x009BB488,
        "int Font::FontInitCount;": 0x009BB48C,
    }
    bindings = []
    for declaration, address in declarations.items():
        matches = [index + 1 for index, line in enumerate(lines) if line == declaration]
        assert len(matches) == 1, f"expected unique independent definition: {declaration}"
        symbol, _ = EXPECTED[address]
        bindings.append(ObjectBinding(
            origin=Origin(PIN, ADDRESS_SPACE, address),
            symbol=symbol,
            unit=UNIT,
            line=matches[0],
            span=Span(address, address + 4),
            evidence=Origin(PIN, ADDRESS_SPACE, EVIDENCE),
        ))
    return tuple(bindings)


def test_three_exact_bounded_records_bind_independent_font_definitions():
    source = SOURCE.read_text(encoding="utf-8")
    rows = parse_objects(source, UNIT, PIN, address_space=ADDRESS_SPACE)
    assert len(rows) == 3
    assert {row.origin.address for row in rows} == set(EXPECTED)
    lines = source.splitlines()
    for row in rows:
        symbol, declaration = EXPECTED[row.origin.address]
        assert row.symbol == symbol
        assert row.unit == UNIT
        assert row.origin == Origin(PIN, ADDRESS_SPACE, row.origin.address)
        assert row.span == Span(row.origin.address, row.origin.address + 4)
        assert row.evidence == Origin(PIN, ADDRESS_SPACE, EVIDENCE)
        assert lines[row.line - 1] == (
            f"// RECOVERY_OBJECT: v2 0x{row.origin.address:08X} "
            f"0x{row.origin.address + 4:08X} {symbol} 0x{EVIDENCE:08X}"
        )
        assert lines[row.line] == declaration, "each marker sits directly above its definition"


def test_native_object_writer_roundtrip_matches_only_three_records_and_note():
    raw = _baseline_bytes()
    additions = _bindings_for_writer(raw)
    written = write_object_bytes(
        raw,
        expected=(),
        additions=additions,
        path=UNIT,
        pin=PIN,
        address_space=ADDRESS_SPACE,
    )
    first_marker = (
        b"// RECOVERY_OBJECT: v2 0x009BB488 0x009BB48C "
        b"?FontHDC@Font@@2PAUHDC__@@A 0x006195B0\n"
    )
    assert written.count(first_marker) == 1
    expected = written.replace(first_marker, NOTE + first_marker, 1)
    assert SOURCE.read_bytes() == expected, (
        "font.cpp differs from the exact native-writer records and essential note"
    )


def test_font_loader_producers_remain_independent_storage_operations():
    source = SOURCE.read_text(encoding="utf-8")
    assert "HDC Font::FontHDC;" in source
    assert "int Font::FontInitCount;" in source
    assert "Font *FontDefault;" in source
    init = source[source.index("int __cdecl Font::init_font_class(Font *font)"):]
    init = init[:init.index("\n}") + 2]
    close_start = source.index("void __cdecl Font::close_font_class()")
    close = source[close_start:source.index("\n}", close_start) + 2]
    assert "if (++FontInitCount <= 1)" in init
    assert "FontHDC = CreateCompatibleDC(NULL);" in init
    assert "FontDefault = font;" in init
    assert "if (--FontInitCount <= 0)" in close
    assert "DeleteDC(FontHDC);" in close
    assert "FontHDC = 0;" in close
    assert "FontDefault = NULL;" in close
    assert "return 2;" in init and "return 3;" in init

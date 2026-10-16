"""Source-order and pinned-edge controls for the terrain helper boundary."""

from hashlib import sha256
import os
from pathlib import Path
import re

from match_recovery.pe_coff import PeImage
from match_recovery.model import Span
from match_recovery.source import read_file
from match_recovery.source.asm import Listing
from match_recovery.source.calls import call_sites


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ALT_PUT_DETAIL = 0x00591260
ALT_NATURAL = 0x005918A0
ALT_SET_BOTH = 0x005918F0


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def test_helpers_have_one_ordered_definition_after_direct_world_callers():
    source = (ROOT / "src/map.cpp").read_text()
    header = (ROOT / "src/map.h").read_text()

    caller_names = (
        "world_alt_put_detail", "world_polar_caps",
        "world_linearize_contours", "world_rivers", "world_erosion",
    )
    caller_positions = [
        source.index(f"void __cdecl {name}(") for name in caller_names
    ]
    put_definition = source.index("void __cdecl alt_put_detail(int x, int y, int detail) {")
    natural_definition = source.index("int __cdecl alt_natural(int x, int y) {")
    setter_definition = source.index(
        "void __cdecl alt_set_both(int x, int y, int altitude_natural) {")
    assert max(caller_positions) < min(put_definition, natural_definition)
    assert put_definition < setter_definition and natural_definition < setter_definition

    # Header declarations expose one real owner; there is no inline body or
    # call-shaping wrapper that would duplicate the helper implementation.
    assert "void __cdecl alt_put_detail(int x, int y, int detail);" in header
    assert "int __cdecl alt_natural(int x, int y);" in header
    assert "void __cdecl alt_put_detail_call" not in header
    assert not re.search(
        r"\b(?:void\s+)?__cdecl\s+alt_put_detail_call\s*\(", source
    )
    assert "MEASURED inline void __cdecl alt_put_detail" not in header
    assert "MEASURED inline int __cdecl alt_natural" not in header

    polar = extract_body(source, "void __cdecl world_polar_caps()")
    assert len(re.findall(r"(?<![A-Za-z0-9_])alt_put_detail\(", polar)) == 6
    world_alt = extract_body(source, "void __cdecl world_alt_put_detail(int x, int y)")
    assert "alt_put_detail(x, y, AltNatural[3]);" in world_alt
    assert "static_cast<uint8_t>(AltNatural[3])" not in world_alt

    linearize = extract_body(source, "void __cdecl world_linearize_contours()")
    assert "alt_natural(x, y)" in linearize
    assert "alt_put_detail(x, y, static_cast<int32_t>(result));" in linearize
    for name in ("world_rivers", "world_erosion"):
        assert "alt_put_detail(" in extract_body(source, f"void __cdecl {name}()")

    setter = extract_body(source, "void __cdecl alt_set_both(int x, int y, int altitude_natural)")
    assert "alt_natural(x, y)" in setter
    assert "alt_put_detail(x, y," in setter


def test_mappings_parse_at_the_single_map_owner():
    rows = {
        row.address: row
        for row in read_file(ROOT / "src/map.cpp")
        if row.address in {0x005C58C0, 0x005C58E0, 0x005C5AE0,
                           0x005C38B0, 0x005C2CB0, ALT_PUT_DETAIL,
                           ALT_NATURAL, ALT_SET_BOTH}
    }
    for address in (0x005C58C0, 0x005C58E0, 0x005C5AE0,
                    ALT_PUT_DETAIL, ALT_NATURAL, ALT_SET_BOTH):
        assert address in rows
        assert not rows[address].extract_error
    for address in (0x005C58C0, ALT_PUT_DETAIL, ALT_NATURAL):
        assert rows[address].body == "src/map.cpp"
    assert rows[0x005C58E0].body == ""
    assert rows[0x005C5AE0].body == ""
    assert rows[ALT_SET_BOTH].body == ""
    # These historical claims were granted by the native recorder after
    # measuring the canonical bodies; this parser test does not measure them.
    assert rows[ALT_PUT_DETAIL].byte_exact
    assert rows[0x005C58C0].byte_exact
    assert rows[ALT_SET_BOTH].region.find("alt_natural(x, y)") >= 0
    assert rows[ALT_PUT_DETAIL].region.find("map_loc(x, y)->contour = detail;") >= 0
    assert rows[ALT_NATURAL].region.find("while (contour < AltNatural[natural] && natural)") >= 0


def test_pinned_spans_retain_direct_calls_and_the_setter_inline_exception():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE",
        ROOT / ".opensmacx/game/terranx_original.exe",
    ))
    if not image_path.is_file():
        import pytest
        pytest.skip("pinned executable is not provisioned for this host test")
    raw = image_path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    rows = {row.address: row for row in read_file(ROOT / "src/map.cpp")}

    def instructions(address: int):
        row = rows[address]
        output = []
        for low, high in row.image_spans:
            listing = Listing(image.read(Span(low, high)), low)
            decoded = listing.instructions
            assert sum(instruction.size for instruction in decoded) == high - low
            output.extend(decoded)
        return output

    def targets(address: int) -> list[int]:
        output = []
        for low, high in rows[address].image_spans:
            listing = Listing(image.read(Span(low, high)), low)
            assert sum(instruction.size for instruction in listing.instructions) == high - low
            output.extend(site.target for site in call_sites(listing)
                          if site.form == "direct")
        return output

    world_alt = instructions(0x005C58C0)
    assert next(item for item in world_alt if item.address == 0x005C58C3).op_str == (
        "eax, dword ptr [0x68fb58]"
    )
    assert targets(0x005C58C0) == [ALT_PUT_DETAIL]
    assert targets(0x005C58E0).count(ALT_PUT_DETAIL) == 6
    linearize = targets(0x005C5AE0)
    assert ALT_NATURAL in linearize and ALT_PUT_DETAIL in linearize
    assert ALT_PUT_DETAIL in targets(0x005C38B0)
    assert ALT_PUT_DETAIL in targets(0x005C2CB0)

    # The original setter calls alt_set and rand, but not either helper:
    # alt_natural's scan and alt_put_detail's byte write are expanded there.
    setter_targets = targets(ALT_SET_BOTH)
    assert ALT_NATURAL not in setter_targets
    assert ALT_PUT_DETAIL not in setter_targets
    assert 0x00591290 in setter_targets
    assert 0x0064601D in setter_targets

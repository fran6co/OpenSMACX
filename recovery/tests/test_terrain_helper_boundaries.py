"""Source-order and pinned-edge controls for the terrain helper boundary."""

from hashlib import sha256
import json
import os
from pathlib import Path
import re
import subprocess

import pytest

from match_recovery.pe_coff import PeImage
from match_recovery.model import Span
from match_recovery.source import function_line, read_file
from match_recovery.source.reader import _extract_forward_text
from match_recovery.source.asm import Listing
from match_recovery.source.calls import call_sites


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ALT_PUT_DETAIL = 0x00591260
ALT_NATURAL = 0x005918A0
ALT_SET_BOTH = 0x005918F0
WORLD_OWNER_BASE = "61ba1b3c96f2f12a7c025e74e599fa9f1130546a"


def test_altitude_owner_has_its_measured_inline_recipe():
    project = json.loads((ROOT / "recovery/project.json").read_text())
    unit_flags = project["measurement"]["unit_flag_sets"]
    assert unit_flags["src/map_grid.cpp"]["flags"] == unit_flags["src/map.cpp"]["flags"]


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def helper_definition(source: str, name: str) -> str:
    line = function_line(source, name)
    return _extract_forward_text(source.splitlines(), line, name).strip()


def test_site_helpers_move_unchanged_to_shared_header_with_one_definition():
    baseline = subprocess.check_output(
        ["git", "show", WORLD_OWNER_BASE + ":src/map.cpp"], cwd=ROOT,
    ).decode("utf-8")
    current_header = (ROOT / "src/map_coordinates.h").read_text(encoding="utf-8")
    current_map = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    current_world = (ROOT / "src/world.cpp").read_text(encoding="utf-8")

    for name in ("site_xrange", "site_tile"):
        original = helper_definition(baseline, name)
        assert helper_definition(current_header, name) == original
        for implementation in (current_map, current_world):
            with pytest.raises(ValueError, match=r"0 definitions found"):
                helper_definition(implementation, name)

    # The helpers are shared by both the resident map consumers and the moved
    # world_site consumer; references remain valid, but definitions are unique.
    assert "site_xrange(" in current_map and "site_xrange(" in current_world
    assert "site_tile(" in current_map and "site_tile(" in current_world


@pytest.mark.parametrize(("name", "old", "new"), [
    ("site_xrange", "!(MapIsFlat & 1)", "!(MapIsFlat != 0)"),
    ("site_tile", "(x >> 1)", "(x / 2)"),
])
def test_site_helper_identity_rejects_semantic_mutants(name, old, new):
    baseline = subprocess.check_output(
        ["git", "show", WORLD_OWNER_BASE + ":src/map.cpp"], cwd=ROOT,
    ).decode("utf-8")
    current_header = (ROOT / "src/map_coordinates.h").read_text(encoding="utf-8")
    mutated = current_header.replace(old, new, 1)
    assert mutated != current_header
    assert helper_definition(mutated, name) != helper_definition(baseline, name)


@pytest.mark.parametrize("name", ["site_xrange", "site_tile"])
def test_site_helper_identity_rejects_duplicate_nonheader_definition(name):
    current_header = (ROOT / "src/map_coordinates.h").read_text(encoding="utf-8")
    duplicate = helper_definition(current_header, name)
    with pytest.raises(ValueError, match=r"2 definitions found"):
        helper_definition(duplicate + "\n" + duplicate, name)


def test_near_landmark_has_one_world_owner():
    header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    world = (ROOT / "src/world.cpp").read_text(encoding="utf-8")
    current = helper_definition(world, "near_landmark")
    assert "BOOL __cdecl near_landmark(int x, int y);" in header
    with pytest.raises(ValueError, match=r"0 definitions found"):
        helper_definition(header, "near_landmark")
    with pytest.raises(ValueError, match=r"2 definitions found"):
        helper_definition(current + "\n" + current, "near_landmark")
    record, = [row for row in read_file(ROOT / "src/world.cpp")
               if row.address == 0x005C5BD0]
    assert not record.body and not record.extract_error


def test_altitude_helpers_have_one_canonical_owner_and_world_callers():
    source = (ROOT / "src/world.cpp").read_text()
    owner = (ROOT / "src/map_grid.cpp").read_text()
    header = (ROOT / "src/map.h").read_text()

    put_definition = owner.index("void __cdecl alt_put_detail(int x, int y, int detail) {")
    natural_definition = owner.index("int __cdecl alt_natural(int x, int y) {")
    setter_definition = owner.index(
        "void __cdecl alt_set_both(int x, int y, int altitude_natural) {")
    assert put_definition < natural_definition < setter_definition
    assert "alt_natural(x, y)" not in extract_body(
        source, "void __cdecl world_alt_set(int x, int y, int altitude, BOOL is_set_both)")
    assert "void __cdecl alt_put_detail(int x, int y, int detail) {" not in source
    assert "void __cdecl alt_set_both(int x, int y, int altitude_natural) {" not in source

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

    setter = extract_body(owner, "void __cdecl alt_set_both(int x, int y, int altitude_natural)")
    assert "alt_natural(x, y)" in setter
    assert "alt_put_detail(x, y," in setter


def test_mappings_parse_at_the_canonical_altitude_owner():
    assert "    src/map_grid.cpp\n" in (ROOT / "CMakeLists.txt").read_text()
    rows = {
        row.address: row
        for row in (read_file(ROOT / "src/world.cpp") + read_file(ROOT / "src/map.cpp")
                    + read_file(ROOT / "src/map_grid.cpp"))
        if row.address in {0x005C58C0, 0x005C58E0, 0x005C5AE0,
                           0x005C38B0, 0x005C2CB0, ALT_PUT_DETAIL,
                           ALT_NATURAL, ALT_SET_BOTH, 0x005C2020}
    }
    for address in (0x005C58C0, 0x005C58E0, 0x005C5AE0,
                    ALT_PUT_DETAIL, ALT_NATURAL, ALT_SET_BOTH, 0x005C2020):
        assert address in rows
        assert not rows[address].extract_error
    assert rows[0x005C58C0].body == "src/world.cpp"
    assert rows[0x005C2020].body == ""
    for address in (ALT_PUT_DETAIL, ALT_NATURAL):
        assert rows[address].body == "src/map_grid.cpp"
    assert rows[0x005C58E0].body == ""
    assert rows[0x005C5AE0].body == ""
    assert rows[ALT_SET_BOTH].body == ""
    # These historical claims were granted by the native recorder after
    # measuring the canonical bodies; this parser test does not measure them.
    assert rows[ALT_PUT_DETAIL].byte_exact
    assert rows[0x005C58C0].byte_exact
    assert rows[ALT_SET_BOTH].region.find("alt_natural(x, y)") >= 0
    assert rows[ALT_PUT_DETAIL].region.find("map_loc(x, y)->contour = detail;") >= 0
    # This checks the canonical source owner, not the historical unsigned
    # predicate spelling. Actual-source controls verify the signed operation.
    actual_natural = extract_body((ROOT / "src/map_grid.cpp").read_text(),
                                  "int __cdecl alt_natural(int x, int y)")
    assert actual_natural in rows[ALT_NATURAL].region


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
    rows = {row.address: row for row in (read_file(ROOT / "src/world.cpp")
                                         + read_file(ROOT / "src/map.cpp")
                                         + read_file(ROOT / "src/map_grid.cpp"))}

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

    def direct_sites(address: int):
        output = []
        for low, high in rows[address].image_spans:
            listing = Listing(image.read(Span(low, high)), low)
            assert sum(instruction.size for instruction in listing.instructions) == high - low
            output.extend(site for site in call_sites(listing) if site.form == "direct")
        return output

    # The standalone landmark scan and the volcano's inlined copy both
    # stop at a fixed 289 four-byte offset entries, independent of RadiusRange.
    for owner, site in ((0x005C5BD0, 0x005C5C48),
                        (0x005C6DB0, 0x005C6EED)):
        bound = next(item for item in instructions(owner) if item.address == site)
        assert (bound.mnemonic, bound.op_str) == ("cmp", "esi, 0x484")

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

    world_alt_set = direct_sites(0x005C2020)
    assert [(site.at, site.target) for site in world_alt_set
            if site.target == ALT_SET_BOTH] == [
                (0x005C2073, ALT_SET_BOTH),
                (0x005C2148, ALT_SET_BOTH),
                (0x005C22C3, ALT_SET_BOTH),
            ]
    assert ALT_NATURAL not in [site.target for site in world_alt_set]

"""Conserve the actual terrain asset definitions and original obligations."""
from pathlib import Path
from dataclasses import replace
import re
import subprocess
import pytest

from match_recovery.source import read, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "20e1fdbeb252c22708ae380f489107486445b4ef"


def baseline():
    return subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"], cwd=ROOT, text=True)


def test_all_asset_definitions_and_storage_bindings_move_in_order():
    old = baseline()
    objects = (ROOT / "src/interface_assets.cpp").read_text()
    methods_path = ROOT / "src/texture.cpp"
    methods = methods_path.read_text()
    from recovery.tests.texture_direct_storage_host import texture_asset_segment, _cpp_tokens
    block = old[old.index("// ===== MANAGED GLOBALS"):old.index("// Shared Texture-owned storage")]
    segment = texture_asset_segment(objects)
    assert _cpp_tokens(segment) == _cpp_tokens(block)
    definitions = r"^Texture g_\w+(?:\[\d+\])?;"
    assert len(re.findall(definitions, segment, re.MULTILINE)) == 20
    assert not re.findall(definitions, methods, re.MULTILINE)
    # This checkpoint predates later reviewed Texture improvements. Preserve
    # every historical identity and claim across the live method owner plus
    # the moved storage segment without freezing unrelated current behavior.
    historical = read_text(old, methods_path)
    asset_path = ROOT / "src/interface_assets.cpp"
    old_lifetime_markers = [
        line for line in old[:old.index('#include "stdafx.h"')].splitlines()
        if line.startswith("// ORIGINAL:")
    ]
    assert len(old_lifetime_markers) == 40
    historical_asset_addresses = {int(line.split()[2], 16) for line in old_lifetime_markers}
    asset_rows = [row for row in read(asset_path) if row.address in historical_asset_addresses]
    assert len(asset_rows) == 40
    current = read(methods_path) + asset_rows
    by_address = {row.address: row for row in current}
    assert len(by_address) == len(current)
    assert {row.address for row in historical} <= set(by_address)
    from recovery.tests.test_texture_raster_actual_source import _continuity_fields
    for row in historical:
        actual = by_address[row.address]
        assert not row.byte_exact or actual.byte_exact
        actual = replace(actual, byte_exact=row.byte_exact)
        assert _continuity_fields(actual) == _continuity_fields(row)
    raster = by_address[0x0061F0D0]
    assert raster.region and not raster.extract_error
    assert not re.findall(definitions, methods, re.MULTILINE)


def test_lifetime_identities_spans_and_historical_claims_follow_assets():
    old = baseline()
    old_markers = [line for line in old[:old.index('#include "stdafx.h"')].splitlines()
                   if line.startswith("// ORIGINAL:")]
    assert len(old_markers) == 40
    objects = (ROOT / "src/interface_assets.cpp").read_text()
    asset_path = ROOT / "src/interface_assets.cpp"
    asset_addresses = {int(marker.split()[2], 16) for marker in old_markers}
    records = [row for row in read(asset_path) if row.address in asset_addresses]
    assert len(records) == 40
    # Native recording may add claims, but cannot drop baseline claims/spans.
    current = {line.split()[2]: line for line in objects.splitlines()
               if line.startswith("// ORIGINAL:")}
    for marker in old_markers:
        actual = current[marker.split()[2]]
        assert actual == marker or actual == marker + " BYTE_EXACT"
    assert not {r.address for r in records} & {r.address for r in read(ROOT / "src/texture.cpp")}


def test_relative_asset_contribution_order_and_out_of_line_lifetime():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    owners = ["interface_assets", "sprite_objects", "caviar_renderer_objects"]
    indices = [cmake.index("    src/" + owner + ".cpp\n") for owner in owners]
    assert indices == sorted(indices)
    objects = (ROOT / "src/interface_assets.cpp").read_text()
    assert '#include "texture.h"' in objects
    assert "Texture::Texture()" not in objects
    assert "Texture::~Texture()" not in objects


@pytest.mark.parametrize("damage", ["missing", "reordered", "duplicate", "extra_code"])
def test_historical_projection_refuses_damaged_asset_owner(damage):
    from recovery.tests.texture_direct_storage_host import restore_asset_block_for_comparison
    current = (ROOT / "src/texture.cpp").read_text()
    assets = (ROOT / "src/interface_assets.cpp").read_text()
    first, second = "Texture g_RADIUS1_TEXTURE;", "Texture g_RADIUS2_TEXTURE;"
    assert assets.count(first) == assets.count(second) == 1
    if damage == "missing":
        assets = assets.replace(first, "", 1)
    elif damage == "reordered":
        assets = assets.replace(first, "TEMP_DECL", 1).replace(second, first, 1).replace("TEMP_DECL", second, 1)
    elif damage == "extra_code":
        assets = assets.replace(first, first + "\nint extra = side_effect();", 1)
    else:
        assets += "\n" + first + "\n"
    with pytest.raises(AssertionError):
        restore_asset_block_for_comparison(current, baseline(), assets)

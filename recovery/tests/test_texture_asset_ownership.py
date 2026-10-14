"""Conserve the actual terrain asset definitions and original obligations."""
from pathlib import Path
import re
import subprocess
import pytest

from match_recovery.source import read

ROOT = Path(__file__).resolve().parents[2]
BASE = "20e1fdbeb252c22708ae380f489107486445b4ef"


def baseline():
    return subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"], cwd=ROOT, text=True)


def test_all_asset_definitions_and_storage_bindings_move_in_order():
    old = baseline()
    objects = (ROOT / "src/texture_objects.cpp").read_text()
    methods = (ROOT / "src/texture.cpp").read_text()
    block = old[old.index("// ===== MANAGED GLOBALS"):old.index("// Shared Texture-owned storage")]
    assert block in objects
    definitions = r"^Texture g_\w+(?:\[\d+\])?;"
    assert len(re.findall(definitions, objects, re.MULTILINE)) == 20
    assert not re.findall(definitions, methods, re.MULTILINE)
    # All non-asset implementation text stays untouched, including methods,
    # TU-local raster storage and its original order.
    remainder = old[:old.index("// ORIGINAL:")] + old[old.index('#include "stdafx.h"'):]
    remainder = remainder.replace(block, "", 1)
    assert methods == remainder


def test_lifetime_identities_spans_and_historical_claims_follow_assets():
    old = baseline()
    old_markers = [line for line in old[:old.index('#include "stdafx.h"')].splitlines()
                   if line.startswith("// ORIGINAL:")]
    assert len(old_markers) == 40
    records = read(ROOT / "src/texture_objects.cpp")
    assert len(records) == 40
    objects = (ROOT / "src/texture_objects.cpp").read_text()
    # Native recording may add claims, but cannot drop baseline claims/spans.
    current = {line.split()[2]: line for line in objects.splitlines()
               if line.startswith("// ORIGINAL:")}
    for marker in old_markers:
        actual = current[marker.split()[2]]
        assert actual == marker or actual == marker + " BYTE_EXACT"
    assert not {r.address for r in records} & {r.address for r in read(ROOT / "src/texture.cpp")}


def test_relative_asset_contribution_order_and_out_of_line_lifetime():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    owners = ["caviar_objects", "texture_objects", "sprite_objects", "caviar_renderer_objects"]
    indices = [cmake.index("    src/" + owner + ".cpp\n") for owner in owners]
    assert indices == sorted(indices)
    objects = (ROOT / "src/texture_objects.cpp").read_text()
    assert '#include "texture.h"' in objects
    assert "Texture::Texture()" not in objects
    assert "Texture::~Texture()" not in objects


@pytest.mark.parametrize("damage", ["missing", "reordered", "duplicate", "extra_code"])
def test_historical_projection_refuses_damaged_asset_owner(damage):
    from recovery.tests.texture_direct_storage_host import restore_asset_block_for_comparison
    current = (ROOT / "src/texture.cpp").read_text()
    assets = (ROOT / "src/texture_objects.cpp").read_text()
    first, second = "Texture g_RADIUS1_TEXTURE;", "Texture g_RADIUS2_TEXTURE;"
    assert assets.count(first) == assets.count(second) == 1
    if damage == "missing":
        assets = assets.replace(first, "", 1)
    elif damage == "reordered":
        assets = assets.replace(first, "TEMP_DECL", 1).replace(second, first, 1).replace("TEMP_DECL", second, 1)
    elif damage == "extra_code":
        assets = "int extra = side_effect();\n" + assets
    else:
        assets += first + "\n"
    with pytest.raises(AssertionError):
        restore_asset_block_for_comparison(current, baseline(), assets)

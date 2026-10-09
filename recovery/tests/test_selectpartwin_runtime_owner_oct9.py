"""Host controls for SelectPartWin's recovered game-owned asset inputs."""

import os
from pathlib import Path
import re
import subprocess


ROOT = (Path(os.environ["OPENSMACX_ROOT"]).resolve()
        if "OPENSMACX_ROOT" in os.environ
        else Path(__file__).resolve().parents[2])


def test_selectpartwin_uses_owned_palette_and_sprite_tables(tmp_path: Path) -> None:
    source = (ROOT / "src/selectpartwin.cpp").read_text()
    game_header = (ROOT / "src/game.h").read_text()
    sprite_header = (ROOT / "src/sprite.h").read_text()

    assert "ArmorTransparentColorTables[24][256]" in game_header
    assert "g_VEH_SPRITES[152]" in sprite_header
    assert "// 0x007777A0" in sprite_header
    assert "render_multi_table_dest(buf444, midX, midY, 0x18,\n                ArmorTransparentColorTables[0], 6)" in source
    assert "SelectPartTile->draw(&backdrop, SelectPartTile->transparent_index()," in source
    assert "SelectPartTileColour" not in source
    assert "WeaponSpriteIndexSlot = (signed char *)0x009AB9CE" in source
    assert "if (have_sprite_idx && sprite_idx >= 0)" in source

    sprite_rhs = re.search(
        r"Sprite \*const spr = (&g_VEH_SPRITES\[72 \+ sprite_idx \* 8\]);",
        source,
    )
    assert sprite_rhs is not None
    harness = f"""
#include <cassert>
struct Sprite {{ int tag; }};
unsigned char ArmorTransparentColorTables[24][256] = {{}};
Sprite g_VEH_SPRITES[152] = {{}};
Sprite *select_weapon_sprite(int sprite_idx) {{ return {sprite_rhs.group(1)}; }}
int main() {{
    ArmorTransparentColorTables[0][0] = 17;
    ArmorTransparentColorTables[1][0] = 29;
    assert(ArmorTransparentColorTables[0][0] == 17);
    assert(select_weapon_sprite(0) == &g_VEH_SPRITES[72]);
    assert(select_weapon_sprite(1) == &g_VEH_SPRITES[80]);
    assert(select_weapon_sprite(9) == &g_VEH_SPRITES[144]);
}}
"""
    source_path = tmp_path / "selectpart_assets.cpp"
    binary = tmp_path / "selectpart_assets"
    source_path.write_text(harness)
    subprocess.run(
        ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)

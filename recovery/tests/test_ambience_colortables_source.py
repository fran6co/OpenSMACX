"""Product-side source-extraction controls, transferred to recovery/tests."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
if not (ROOT / "src").is_dir():
    ROOT = Path(__file__).resolve().parents[2]


def extract(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError(f"unterminated function: {signature}")


def product_bodies():
    ambience_src = (ROOT / "src/ambience.cpp").read_text()
    game_src = (ROOT / "src/game.cpp").read_text()
    return (
        extract(ambience_src, "int __cdecl ambience(int ambience_id)"),
        extract(game_src,
                "void __cdecl colortables_init_faction(int faction_id)"),
        extract(game_src, "void __cdecl load_faction_art(int faction_id)"),
    )


def test_native_ambience_state_cells_keep_the_pinned_initial_values():
    source = (ROOT / "src/ambience.cpp").read_text()
    assert "int AmbienceCurrentEffect = -1;" in source
    assert "int AmbienceTransitionMarker = 0;" in source


def test_real_ambience_dispatch_and_lifecycle_are_source_owned():
    ambience, _, _ = product_bodies()
    assert "PENDING_BODY" not in ambience
    assert "GamePreferences & 0x800" in ambience
    assert "AmbienceCurrentEffect == ambience_id" in ambience
    for event in (
        "general", "tech", "popup1", "eot", "hostility", "basewin_show",
        "basewin_hide", "terraform", "new_base", "energy_resources",
        "design_window_show", "design_window_hide", "base_liberated",
    ):
        assert f"FactionMusic->{event}()" in ambience
    assert "case 0xC8: FactionMusic->general()" in ambience
    assert "case 0xD0: FactionMusic->general()" in ambience
    assert "g_FX.effect(previous_effect).fade()" in ambience
    assert "g_FX.effect(ambience_id).is_playing()" in ambience
    assert "g_FX.effect(AmbienceCurrentEffect).stop()" in ambience
    assert "g_FX.play(AmbienceCurrentEffect)" in ambience


def test_real_color_cache_has_exact_native_extents_and_owned_palette():
    _, tables, _ = product_bodies()
    for fragment in (
        'strcat(StringTemp, ".pcx")', "get_pixel(0xA1, 0x2EB)",
        "get_pixel(0xA1, 0x2F5)", "get_pixel(0x1B3, 0x2E9)",
        "cache_file.close(saved + 12)",
        "cache_file.create(StringTemp, 0xC100, 1)",
        "memcpy(TerritoryColorTables[faction_id], table_view, 0x100)",
        "memcpy(BodyTintColorTables[faction_id],\n                   static_cast<unsigned char *>(table_view) + 0x100,\n                   0xC000)",
        "g_VBP_CAVIARDATA.record()->palette_ + 0x302",
        "if (table_view != nullptr && cache_file.get_size() >= 0xC100)",
        "values_view = cache_file.create(StringTemp, 0x80, 1)",
        "DWORD now = start_time", "now = timeGetTime()",
        "for (int row = 0; row < 24; ++row)",
        "for (int color = 0; color < 256; ++color)",
    ):
        assert fragment in tables, fragment
    assert "reinterpret_cast" not in tables
    assert "0x0078E970" not in tables
    assert tables.index("get_pixel(0xA1, 0x2EB)") < tables.index(
        "get_pixel(0xA1, 0x2F5)") < tables.index("get_pixel(0x1B3, 0x2E9)")
    assert "if (territory_sample == WinFillColour)" not in tables
    assert "wrapped_territory_delta) * 100) / 255" in tables
    assert "if (now - start_time >= 0xBB8)" in tables
    assert tables.index("Palette local_palette;") < tables.index(
        "Filemap cache_file;") < tables.index("int weight_by_faction[8]") < tables.index(
        "Buffer source_buffer;") < tables.index("compare_palettes()")
    assert tables.rindex("values_view = cache_file.create(StringTemp, 0x80, 1)") > tables.rindex(
        "cache_file.create(StringTemp, 0xC100, 1)")


def test_real_caller_populates_faction_art_and_color_bank_members():
    _, _, art = product_bodies()
    assert "colortables_init_faction(faction_id);" in art
    assert "PENDING_BODY" not in art
    for bank in (
        "InfoFactionTextColors", "InfoFactionShadowColors", "ChatItemIdTable",
        "ChatShadowColors",
    ):
        assert f"{bank}[faction_id] = canvas.get_pixel" in art
    assert "FactionArt &art = FactionsArt[faction_id]" in art
    for member in (
        "base[set][row][column]", "council_logo[council_logo_index]", "council_small",
        "council_big", "diplomacy", "diplomacy_logo",
        "diplomacy_landscape", "datalink", "report_logo[0]",
        "report_logo[1]", "report_logo_small[report_logo_index]",
    ):
        assert member in art
    assert "canvas.close()" in art


def test_mutations_of_the_actual_extracted_bodies_break_the_guards():
    ambience, tables, art = product_bodies()
    mutants = (
        (ambience, "GamePreferences & 0x800", "GamePreferences & 0x400"),
        (ambience, "g_FX.effect(previous_effect).fade()",
         "g_FX.effect(previous_effect).stop()"),
        (tables, "cache_file.close(saved + 12)",
         "cache_file.close(saved + 8)"),
        (tables, "for (int row = 0; row < 24; ++row)",
         "for (int row = 0; row < 23; ++row)"),
        (tables, ") * 100) / 255", ") * 100) / 100"),
        (art, "colortables_init_faction(faction_id);", "/* omitted */"),
        (art, "if (canvas.load_pcx(StringTemp, nullptr, 10, 0xEC) != 0)",
         "if (canvas.load_pcx(StringTemp, nullptr, 10, 0xEC) == 0)"),
    )
    for body, old, new in mutants:
        assert old in body
        assert new not in body
        assert body.replace(old, new, 1) != body

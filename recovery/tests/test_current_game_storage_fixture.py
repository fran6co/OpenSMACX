"""Guard exact storage extraction rather than old scalar stand-ins."""

import pytest

from current_game_storage_fixture import _definition, current_game_storage_fixture


def test_type_extraction_does_not_select_longer_name_prefix():
    source = "struct ExampleFields { int field; };\nstruct Example { ExampleFields fields_; };"
    assert _definition(source, "struct Example") == "struct Example { ExampleFields fields_; };"
    with pytest.raises(AssertionError, match="missing current product declaration"):
        _definition("struct ExampleFields { int field; };", "struct Example")


def test_fixture_owns_actual_types_and_definitions_once():
    source = current_game_storage_fixture(scenario=True)
    assert source.count("struct GameDataStateWindowFields {") == 1
    assert source.count("struct GameDataStateWindow {") == 1
    assert source.count("GameDataStateWindow GameDataStateStorage;") == 1
    assert source.count("ScenarioSaveState ScenarioSave;") == 1
    assert "uint32_t ranking_faction_id_unk2_;" in source
    assert "#define GameState" not in source

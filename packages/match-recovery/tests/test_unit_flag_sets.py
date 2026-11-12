import json

import pytest

from match_recovery.measurement import Measurement
from match_recovery.project import initialize


def configured(tmp_path, policy):
    initialize(tmp_path, "unit-profiles", "unused-compiler")
    unit = tmp_path / "src/owner.cpp"
    unit.parent.mkdir(parents=True, exist_ok=True)
    unit.write_text("void owner() {}\n")
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["flag_sets"] = [["/O2"], ["/Od"]]
    config["measurement"]["unit_flag_sets"] = policy
    path.write_text(json.dumps(config))
    return Measurement.read(tmp_path)


def test_sparse_unit_profiles_select_one_exact_global_trial_and_keep_default(tmp_path):
    settings = configured(tmp_path, {"src/owner.cpp": [1]})
    assert settings.flags_for("src/owner.cpp") == (("/Od",),)
    assert settings.flags_for("src/other.cpp") == (("/O2",), ("/Od",))
    assert settings.effective_flags("src/owner.cpp", ("/O2",)) == ("/Od",)
    assert settings.effective_flags("src/other.cpp", ("/O2",)) == ("/O2",)
    assert settings.witness_grid(["src/owner.cpp", "src/other.cpp"]) == {
        "src/other.cpp": [["/O2"], ["/Od"]], "src/owner.cpp": [["/Od"]]}


def test_explicit_unit_recipe_is_exact_and_used_by_all_flag_consumers(tmp_path):
    settings = configured(tmp_path, {"src/owner.cpp": {"flags": ["/Od", "/Gy"]}})
    assert settings.flags_for("src/owner.cpp") == (("/Od", "/Gy"),)
    assert settings.flags_for("src/other.cpp") == (("/O2",), ("/Od",))
    assert settings.effective_flags("src/owner.cpp", ("/O2",)) == ("/Od", "/Gy")
    assert settings.effective_flags("src/other.cpp", ("/O2",)) == ("/O2",)
    assert settings.witness_grid(["src/owner.cpp", "src/other.cpp"], ("/GR-",)) == {
        "src/other.cpp": [["/O2", "/GR-"], ["/Od", "/GR-"]],
        "src/owner.cpp": [["/Od", "/Gy", "/GR-"]],
    }


@pytest.mark.parametrize("policy", [
    {"src/owner.cpp": []},
    {"src/owner.cpp": [True]},
    {"src/owner.cpp": [2]},
    {"src/owner.cpp": [0, 1]},
    {"src/owner.cpp": [0, 0]},
    {"src/*.cpp": [0]},
    {"src/missing.cpp": [0]},
    {"src/owner.h": [0]},
    {"src/owner.cpp": {"flags": []}},
    {"src/owner.cpp": {"flags": [""]}},
    {"src/owner.cpp": {"flags": ["/Od", True]}},
    {"src/owner.cpp": {"flags": ["/Od", 2]}},
    {"src/owner.cpp": {"flags": ["/Od"], "index": 1}},
    {"src/owner.cpp": {"other": ["/Od"]}},
    {"src/missing.cpp": {"flags": ["/Od"]}},
    {"src/owner.h": {"flags": ["/Od"]}},
])
def test_invalid_unit_profiles_are_refused(tmp_path, policy):
    with pytest.raises(ValueError):
        configured(tmp_path, policy)


def test_cross_tu_profile_selection_is_single_valued(tmp_path):
    settings = configured(tmp_path, {"src/owner.cpp": [1]})
    assert settings.effective_flags("src/owner.cpp", ("/O2",)) == ("/Od",)


def test_case_aliased_unit_paths_are_refused_for_explicit_recipes(tmp_path):
    with pytest.raises(ValueError, match="duplicate or aliased"):
        configured(tmp_path, {
            "src/owner.cpp": {"flags": ["/Od"]},
            "src/OWNER.cpp": {"flags": ["/O2"]},
        })


def test_unit_profile_policy_is_part_of_the_input_fingerprint(tmp_path):
    first = configured(tmp_path, {})
    before = first.snapshot(tmp_path, {})
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["unit_flag_sets"] = {"src/owner.cpp": [1]}
    path.write_text(json.dumps(config))
    second = Measurement.read(tmp_path)
    assert second.snapshot(tmp_path, {}) != before


def test_explicit_unit_recipe_is_part_of_the_input_fingerprint(tmp_path):
    first = configured(tmp_path, {})
    before = first.snapshot(tmp_path, {})
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["unit_flag_sets"] = {
        "src/owner.cpp": {"flags": ["/Od", "/Gy"]}}
    path.write_text(json.dumps(config))
    second = Measurement.read(tmp_path)
    assert second.snapshot(tmp_path, {}) != before


def required(tmp_path, policy, *, require=True, database=("src/owner.cpp",)):
    """A project whose measurement may require a recipe per compiled unit."""
    initialize(tmp_path, "unit-profiles", "unused-compiler")
    for name in ("src/owner.cpp", "src/other.cpp", "src/owner.h"):
        unit = tmp_path / name
        unit.parent.mkdir(parents=True, exist_ok=True)
        unit.write_text("void f() {}\n")
    if database is not None:
        (tmp_path / "build").mkdir(exist_ok=True)
        (tmp_path / "build/compile_commands.json").write_text(json.dumps([
            {"directory": str(tmp_path / "build"), "file": str(tmp_path / name),
             "command": f"cl /O2 /Oy- -c {tmp_path / name}"}
            for name in (*database, "outside/tool.cpp")]))
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["flag_sets"] = [["/O2"], ["/O2", "/Oy-"]]
    config["measurement"]["unit_flag_sets"] = policy
    config["measurement"]["require_unit_flag_sets"] = require
    path.write_text(json.dumps(config))
    return Measurement.read(tmp_path)


def test_required_unit_recipes_refuse_a_compiled_unit_left_to_the_global_trials(tmp_path):
    # An unpinned unit is scored as the best of every global trial, so a unit
    # built with /Oy- could be ranked and reported under the frame-pointer-
    # omitting trial. The policy makes that drift a configuration error.
    with pytest.raises(ValueError, match=r"require_unit_flag_sets.*src/other\.cpp"):
        required(tmp_path, {"src/owner.cpp": {"flags": ["/O2", "/Oy-"]}},
                 database=("src/owner.cpp", "src/other.cpp"))


def test_required_unit_recipes_accept_explicit_and_indexed_pins(tmp_path):
    settings = required(tmp_path, {"src/owner.cpp": {"flags": ["/O2", "/Oy-"]},
                                   "src/other.cpp": [1]},
                        database=("src/owner.cpp", "src/other.cpp", "src/owner.h"))
    assert settings.flags_for("src/owner.cpp") == (("/O2", "/Oy-"),)
    assert settings.flags_for("src/other.cpp") == (("/O2", "/Oy-"),)


def test_required_unit_recipes_ignore_units_the_database_does_not_build(tmp_path):
    # Only database membership is checked; outside-root entries and headers
    # are not measurable translation units.
    settings = required(tmp_path, {"src/owner.cpp": [0]})
    assert settings.flags_for("src/other.cpp") == (("/O2",), ("/O2", "/Oy-"))


def test_unit_recipes_stay_optional_without_the_policy(tmp_path):
    settings = required(tmp_path, {}, require=False, database=("src/owner.cpp", "src/other.cpp"))
    assert settings.flags_for("src/owner.cpp") == (("/O2",), ("/O2", "/Oy-"))


def test_required_unit_recipe_policy_must_be_boolean(tmp_path):
    with pytest.raises(ValueError, match="boolean"):
        required(tmp_path, {"src/owner.cpp": [0]}, require="yes")


def test_required_unit_recipes_defer_a_missing_database_to_recipe_selection(tmp_path):
    settings = required(tmp_path, {}, database=None)
    assert settings.flags_for("src/owner.cpp") == (("/O2",), ("/O2", "/Oy-"))

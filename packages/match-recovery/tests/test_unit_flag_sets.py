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


@pytest.mark.parametrize("policy", [
    {"src/owner.cpp": []},
    {"src/owner.cpp": [True]},
    {"src/owner.cpp": [2]},
    {"src/owner.cpp": [0, 1]},
    {"src/owner.cpp": [0, 0]},
    {"src/*.cpp": [0]},
    {"src/missing.cpp": [0]},
    {"src/owner.h": [0]},
])
def test_invalid_unit_profiles_are_refused(tmp_path, policy):
    with pytest.raises(ValueError):
        configured(tmp_path, policy)


def test_cross_tu_profile_selection_is_single_valued(tmp_path):
    settings = configured(tmp_path, {"src/owner.cpp": [1]})
    assert settings.effective_flags("src/owner.cpp", ("/O2",)) == ("/Od",)


def test_unit_profile_policy_is_part_of_the_input_fingerprint(tmp_path):
    first = configured(tmp_path, {})
    before = first.snapshot(tmp_path, {})
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["unit_flag_sets"] = {"src/owner.cpp": [1]}
    path.write_text(json.dumps(config))
    second = Measurement.read(tmp_path)
    assert second.snapshot(tmp_path, {}) != before

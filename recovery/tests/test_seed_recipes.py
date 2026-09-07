"""Seed hints use an explicit local donor, never a product-recipe escape hatch."""

import json

import pytest

from recovery.environment.recipe import application_command, MissingRecipe


@pytest.fixture
def project(tmp_path):
    for name in ("code", "seeds", "build", "recovery/environment"):
        (tmp_path / name).mkdir(parents=True)
    for name in ("code/donor.cc", "code/missing.cc", "seeds/f.cc"):
        (tmp_path / name).write_text("// Compile input\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "source": {
        "image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code", "seeds"], "seed_roots": ["seeds"]}}))
    (tmp_path / "recovery/environment/recipes.json").write_text(json.dumps({
        "schema_version": 1, "seed_donor": "code/donor.cc"}))
    (tmp_path / "build/compile_commands.json").write_text(json.dumps([{
        "directory": str(tmp_path), "file": "code/donor.cc", "arguments": ["cl", "/DSEED_CONTEXT=1", "code/donor.cc"]}]))
    return tmp_path


def test_seed_uses_the_explicit_product_donor(project):
    assert application_command(project, project / "seeds/f.cc") == ["cl", "/nologo", "/DSEED_CONTEXT=1"]


def test_missing_product_recipe_cannot_borrow(project):
    with pytest.raises(MissingRecipe):
        application_command(project, project / "code/missing.cc")


@pytest.mark.parametrize("kind", ["conflicting", "malformed"])
def test_bad_seed_recipe_cannot_borrow(project, kind):
    path = project / "build/compile_commands.json"
    rows = json.loads(path.read_text())
    rows.extend({"directory": str(project), "file": "seeds/f.cc", "arguments": ["cl", flag, "seeds/f.cc"]}
                for flag in (["/Dx=1", "/Dx=2"] if kind == "conflicting" else ["@hidden-options"]))
    path.write_text(json.dumps(rows))
    with pytest.raises(ValueError) as error:
        application_command(project, project / "seeds/f.cc")
    assert not isinstance(error.value, MissingRecipe)


@pytest.mark.parametrize("policy", [None, {}, {"schema_version": True, "seed_donor": "code/donor.cc"},
    {"schema_version": 1, "seed_donor": "seeds/f.cc"}, {"schema_version": 1, "seed_donor": "../donor.cc"},
    {"schema_version": 1, "seed_donor": "code/missing.cc"}])
def test_donor_policy_must_be_explicit_valid_and_resolvable(project, policy):
    path = project / "recovery/environment/recipes.json"
    if policy is None:
        path.unlink()
    else:
        path.write_text(json.dumps(policy))
    with pytest.raises((ValueError, OSError)):
        application_command(project, project / "seeds/f.cc")


def test_existing_seed_recipe_takes_precedence_over_donor(project):
    path = project / "build/compile_commands.json"
    rows = json.loads(path.read_text())
    rows.append({"directory": str(project), "file": "seeds/f.cc", "arguments": ["cl", "/DOWN_RECIPE=1", "seeds/f.cc"]})
    path.write_text(json.dumps(rows))
    assert application_command(project, project / "seeds/f.cc") == ["cl", "/nologo", "/DOWN_RECIPE=1"]

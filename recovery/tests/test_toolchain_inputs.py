"""Local tool locations follow the compiler wrapper; hashing belongs to package."""

import pytest

from recovery.environment.adapter import toolchain_inputs
from recovery.environment import wine


def test_default_and_overridden_sdk_search_paths(monkeypatch, tmp_path):
    monkeypatch.setattr(wine, "VC6_ROOT", tmp_path)
    monkeypatch.delenv("VC6_INCLUDE", raising=False)
    monkeypatch.delenv("VC6_LIB", raising=False)
    assert toolchain_inputs() == {"schema_version": 1, "files": {}, "directories": {
        "compiler": str(tmp_path / "BIN"), "include-0": str(tmp_path / "INCLUDE"), "library-0": str(tmp_path / "LIB")}}
    monkeypatch.setenv("VC6_INCLUDE", str(tmp_path / "headers") + ":" + str(tmp_path / "sdk"))
    monkeypatch.setenv("VC6_LIB", str(tmp_path / "libraries"))
    assert toolchain_inputs()["directories"] == {"compiler": str(tmp_path / "BIN"),
        "include-0": str(tmp_path / "headers"), "include-1": str(tmp_path / "sdk"), "library-0": str(tmp_path / "libraries")}


@pytest.mark.parametrize("variable", ["VC6_INCLUDE", "VC6_LIB"])
@pytest.mark.parametrize("observe", [toolchain_inputs, wine.environment])
def test_relative_search_paths_refuse_in_discovery_and_execution(monkeypatch, variable, observe):
    monkeypatch.setenv(variable, "relative/sdk")
    with pytest.raises(ValueError, match="absolute"):
        observe()


def test_discovery_and_compiler_share_canonical_sdk_paths_across_working_directories(monkeypatch, tmp_path):
    canonical = tmp_path / "sdk"
    canonical.mkdir()
    alias = tmp_path / "sdk-alias"
    alias.symlink_to(canonical, target_is_directory=True)
    monkeypatch.setenv("VC6_INCLUDE", str(alias))
    monkeypatch.setenv("VC6_LIB", str(alias))
    described = toolchain_inputs()["directories"]
    work = tmp_path / "temporary-compile"
    work.mkdir()
    monkeypatch.chdir(work)
    environment = wine.environment()
    expected = "Z:" + str(canonical).replace("/", "\\")
    assert described["include-0"] == described["library-0"] == str(canonical)
    assert environment["INCLUDE"] == environment["LIB"] == expected

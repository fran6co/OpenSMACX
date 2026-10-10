"""Local tool locations follow the compiler wrapper; hashing belongs to package."""

import pytest

from recovery.environment.adapter import toolchain_inputs
from recovery.environment import intrinsics, wine


@pytest.fixture(autouse=True)
def ordinary_frontend(monkeypatch):
    # SDK controls exercise the explicitly ordinary-only configuration and
    # must not discover a compiler that their project has not registered.
    monkeypatch.setattr(intrinsics, "registration",
                        lambda: {"enabled": False, "compiler": "unregistered-tool"})


def test_disabled_frontend_never_discovers_intrinsic_tools(monkeypatch):
    monkeypatch.setattr(intrinsics.shutil, "which",
                        lambda _: pytest.fail("ordinary-only project discovered an intrinsic compiler"))
    assert toolchain_inputs()["files"] == {}


def test_registered_frontend_inputs_are_merged_not_substituted(monkeypatch, tmp_path):
    monkeypatch.setattr(wine, "VC6_ROOT", tmp_path)
    monkeypatch.delenv("VC6_INCLUDE", raising=False)
    monkeypatch.delenv("VC6_LIB", raising=False)
    declared = {"files": {"intrinsic-compiler": str(tmp_path / "clang-cl")},
                "directories": {"intrinsic-headers": str(tmp_path / "clang-headers")}}
    monkeypatch.setattr(intrinsics, "toolchain_inputs", lambda: declared)
    assert toolchain_inputs() == {"schema_version": 1, "files": declared["files"],
        "directories": {**declared["directories"], "compiler": str(tmp_path / "BIN"),
                        "include-0": str(tmp_path / "INCLUDE"), "library-0": str(tmp_path / "LIB")}}


def test_enabled_registry_without_its_compiler_refuses(monkeypatch):
    monkeypatch.setattr(intrinsics, "registration",
                        lambda: {"enabled": True, "compiler": "missing-registered-tool"})
    monkeypatch.setattr(intrinsics.shutil, "which", lambda _: None)
    with pytest.raises(ValueError, match="requires clang-cl"):
        toolchain_inputs()


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

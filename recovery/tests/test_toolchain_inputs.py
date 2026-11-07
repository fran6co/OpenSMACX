"""Local tool locations follow the compiler wrapper; hashing belongs to package."""

import pytest

from recovery.environment.adapter import toolchain_inputs
from recovery.environment import intrinsics, wine


@pytest.fixture(autouse=True)
def ordinary_frontend(monkeypatch, tmp_path):
    # SDK controls exercise the explicitly ordinary-only configuration and
    # must not discover a compiler that their project has not registered.
    monkeypatch.setattr(intrinsics, "registration",
                        lambda: {"enabled": False, "compiler": "unregistered-tool"})
    # The assembler is always a declared input, wherever ML_ROOT points.
    monkeypatch.setattr(wine, "ML_ROOT", tmp_path / "masm")
    # So is the VC5 compiler that builds the Caviar library objects.
    monkeypatch.setattr(wine, "VC5_ROOT", tmp_path / "vc5")


def vc5(tmp_path):
    return {"compiler-vc5": str(tmp_path / "vc5" / "BIN")}


def assembler(tmp_path):
    return {"assembler": str(tmp_path / "masm" / "bin" / "ML.EXE")}


def test_disabled_frontend_never_discovers_intrinsic_tools(monkeypatch, tmp_path):
    monkeypatch.setattr(intrinsics.shutil, "which",
                        lambda _: pytest.fail("ordinary-only project discovered an intrinsic compiler"))
    assert toolchain_inputs()["files"] == assembler(tmp_path)


def test_registered_frontend_inputs_are_merged_not_substituted(monkeypatch, tmp_path):
    monkeypatch.setattr(wine, "VC6_ROOT", tmp_path)
    monkeypatch.delenv("VC6_INCLUDE", raising=False)
    monkeypatch.delenv("VC6_LIB", raising=False)
    declared = {"files": {"intrinsic-compiler": str(tmp_path / "clang-cl")},
                "directories": {"intrinsic-headers": str(tmp_path / "clang-headers")}}
    monkeypatch.setattr(intrinsics, "toolchain_inputs", lambda: declared)
    assert toolchain_inputs() == {"schema_version": 1, "files": {**declared["files"], **assembler(tmp_path)},
        "directories": {**declared["directories"], "compiler": str(tmp_path / "BIN"), **vc5(tmp_path),
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
    assert toolchain_inputs() == {"schema_version": 1, "files": assembler(tmp_path), "directories": {
        "compiler": str(tmp_path / "BIN"), **vc5(tmp_path), "include-0": str(tmp_path / "INCLUDE"), "library-0": str(tmp_path / "LIB")}}
    monkeypatch.setenv("VC6_INCLUDE", str(tmp_path / "headers") + ":" + str(tmp_path / "sdk"))
    monkeypatch.setenv("VC6_LIB", str(tmp_path / "libraries"))
    assert toolchain_inputs()["directories"] == {"compiler": str(tmp_path / "BIN"), **vc5(tmp_path),
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

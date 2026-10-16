"""Historical claim parsing is cached as evidence, never as a verdict."""

from dataclasses import replace
import json
from pathlib import Path
import shutil
import subprocess
from importlib.machinery import PathFinder

import pytest

from match_recovery import claim_integrity, source_inventory
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_history import SourceVersion


FIRST = "// ORIGINAL: 0x00000010 _first 0x00000010-0x00000018 BYTE_EXACT\nint first() {return 1;}\n"
SECOND = "// ORIGINAL: 0x00000020 _second 0x00000020-0x00000028\nint second() {return 2;}\n"


def git(root, *args):
    return subprocess.run(["git", *args], cwd=root, check=True,
                          capture_output=True, text=True).stdout.strip()


def commit(root, message):
    git(root, "add", ".")
    git(root, "commit", "-m", message)
    return git(root, "rev-parse", "HEAD")


@pytest.fixture
def project(tmp_path):
    git(tmp_path, "init", "-b", "main")
    git(tmp_path, "config", "user.name", "Control")
    git(tmp_path, "config", "user.email", "control@example.test")
    for name in ("recovery", "engine", "seeds", "original"):
        (tmp_path / name).mkdir()
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "engine/first.cc").write_text(FIRST)
    (tmp_path / "engine/second.cc").write_text(SECOND)
    (tmp_path / "seeds/keep.hpp").write_text("// seed evidence\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "custom:offset",
                   "roots": ["engine", "seeds"], "extensions": [".cc", ".hpp"],
                   "seed_roots": ["seeds"]}}))
    base = commit(tmp_path, "Source baseline")
    policy = json.loads((tmp_path / "recovery/project.json").read_text())
    policy["source_history"] = {
        "baseline_revision": base,
        "evidence": ["Existing source obligations at recovery start"]}
    (tmp_path / "recovery/project.json").write_text(json.dumps(policy))
    commit(tmp_path, "Declare immutable source history baseline")
    return tmp_path


def enable_cache(root):
    # The production helper writes only beneath an ignored, untracked directory.
    (root / ".gitignore").write_text("build/\n")
    assert git(root, "check-ignore", "-q", "build/recovery/parse_cache/") == ""


def test_parse_cache_requires_ignored_untracked_storage(project, monkeypatch):
    monkeypatch.delenv("RECOVERY_PARSE_CACHE_DIR", raising=False)
    monkeypatch.delenv("RECOVERY_PARSE_CACHE", raising=False)
    assert source_inventory._parse_cache_directory(project) is None
    enable_cache(project)
    cache = source_inventory._parse_cache_directory(project)
    assert cache == project / "build/recovery/parse_cache"
    monkeypatch.setenv("RECOVERY_PARSE_CACHE", "off")
    assert source_inventory._parse_cache_directory(project) is None


@pytest.mark.parametrize("value", ["relative/cache", "", "/"])
def test_shared_parse_cache_requires_a_canonical_absolute_nonroot_directory(project, monkeypatch, value):
    monkeypatch.delenv("RECOVERY_PARSE_CACHE", raising=False)
    monkeypatch.setenv("RECOVERY_PARSE_CACHE_DIR", value)
    with pytest.raises(ValueError, match="RECOVERY_PARSE_CACHE_DIR"):
        source_inventory._parse_cache_directory(project)


def test_explicit_shared_parse_cache_is_opt_in_and_off_wins(project, monkeypatch, tmp_path):
    monkeypatch.delenv("RECOVERY_PARSE_CACHE", raising=False)
    shared = tmp_path.parent / f"{tmp_path.name}-trusted-shared-cache"
    monkeypatch.setenv("RECOVERY_PARSE_CACHE_DIR", str(shared))
    assert source_inventory._parse_cache_directory(project) == shared
    monkeypatch.setenv("RECOVERY_PARSE_CACHE", "off")
    monkeypatch.setenv("RECOVERY_PARSE_CACHE_DIR", "not/absolute")
    assert source_inventory._parse_cache_directory(project) is None


def test_explicit_shared_parse_cache_rejects_unignored_worktree_storage(project, monkeypatch):
    monkeypatch.delenv("RECOVERY_PARSE_CACHE", raising=False)
    unsafe = project / "unsafe-cache"
    unsafe.mkdir()
    monkeypatch.setenv("RECOVERY_PARSE_CACHE_DIR", str(unsafe))
    with pytest.raises(ValueError, match="ignored and untracked"):
        source_inventory._parse_cache_directory(project)


def test_parse_cache_is_content_path_configuration_and_parser_specific(project, monkeypatch):
    enable_cache(project)
    configuration = SourceConfiguration.read(project)
    version = SourceVersion("a" * 40, "engine/first.cc", "b" * 40,
                            FIRST.encode(), configuration)
    calls = []
    original = claim_integrity.read_text

    def counted(text, path):
        calls.append((text, str(path)))
        return original(text, path)

    monkeypatch.setattr(claim_integrity, "read_text", counted)
    cache = project / "build/recovery/parse_cache"
    pin = "a" * 64
    parser = "1" * 64
    projection = "2" * 64

    rows = claim_integrity._historical_claim_rows(
        project, version, pin, parser, projection, cache)
    assert rows == [[0x10, 1, True, False]]
    assert claim_integrity._historical_claim_rows(
        project, version, pin, parser, projection, cache) == rows
    assert len(calls) == 1

    variants = (
        replace(version, blob="c" * 40),
        replace(version, content=FIRST.replace("return 1", "return 2").encode()),
        replace(version, path="engine/renamed.cc"),
        replace(version, configuration=replace(configuration, address_space="other:offset")),
    )
    for changed in variants:
        claim_integrity._historical_claim_rows(
            project, changed, pin, parser, projection, cache)
    claim_integrity._historical_claim_rows(
        project, version, pin, "3" * 64, projection, cache)
    claim_integrity._historical_claim_rows(
        project, version, pin, parser, "4" * 64, cache)
    claim_integrity._historical_claim_rows(
        project, version, "5" * 64, parser, projection, cache)
    assert len(calls) == 8


def test_identical_historical_blob_reuses_explicit_cache_across_worktrees(project, monkeypatch, tmp_path):
    monkeypatch.delenv("RECOVERY_PARSE_CACHE", raising=False)
    shared = tmp_path.parent / f"{tmp_path.name}-trusted-shared-cache"
    monkeypatch.setenv("RECOVERY_PARSE_CACHE_DIR", str(shared))
    configuration = SourceConfiguration.read(project)
    version = SourceVersion("a" * 40, "engine/first.cc", "b" * 40,
                            FIRST.encode(), configuration)
    calls = []
    original = claim_integrity.read_text

    def counted(text, path):
        calls.append(str(path))
        return original(text, path)

    monkeypatch.setattr(claim_integrity, "read_text", counted)
    first_cache = source_inventory._parse_cache_directory(project)
    first = claim_integrity._historical_claim_rows(
        project, version, "a" * 64, "1" * 64, "2" * 64, first_cache)
    other_root = tmp_path / "other-worktree"
    other_root.mkdir()
    second_cache = source_inventory._parse_cache_directory(other_root)
    second = claim_integrity._historical_claim_rows(
        other_root, version, "a" * 64, "1" * 64, "2" * 64, second_cache)
    assert first == second == [[0x10, 1, True, False]]
    assert calls == [str(project / "engine/first.cc")]


def test_shared_history_cache_distinguishes_proved_store_path_semantics(
        project, monkeypatch, tmp_path):
    monkeypatch.delenv("RECOVERY_PARSE_CACHE", raising=False)
    shared = tmp_path.parent / f"{tmp_path.name}-trusted-shared-cache"
    monkeypatch.setenv("RECOVERY_PARSE_CACHE_DIR", str(shared))
    proved_root = tmp_path / "proved-worktree"
    ordinary_root = tmp_path / "ordinary-worktree"
    proved_root.mkdir()
    ordinary_root.mkdir()
    (proved_root / "recovered").mkdir()
    (ordinary_root / "elsewhere").mkdir()
    try:
        (ordinary_root / "recovered").symlink_to(
            ordinary_root / "elsewhere", target_is_directory=True)
    except OSError as error:
        pytest.skip(f"directory symlinks unavailable: {error}")
    configuration = SourceConfiguration.read(project)
    version = SourceVersion("a" * 40, "recovered/00000010.cpp", "b" * 40,
                            FIRST.encode(), configuration)
    calls = []
    original = claim_integrity.read_text

    def counted(text, path):
        calls.append(str(path))
        return original(text, path)

    monkeypatch.setattr(claim_integrity, "read_text", counted)
    cache = source_inventory._parse_cache_directory(project)
    claim_integrity._historical_claim_rows(
        proved_root, version, "a" * 64, "1" * 64, "2" * 64, cache)
    claim_integrity._historical_claim_rows(
        ordinary_root, version, "a" * 64, "1" * 64, "2" * 64, cache)
    assert len(calls) == 2


def test_cached_rows_are_defensive_corruption_recomputes_and_parse_errors_are_not_cached(
        project, monkeypatch):
    enable_cache(project)
    configuration = SourceConfiguration.read(project)
    version = SourceVersion("a" * 40, "engine/first.cc", "b" * 40,
                            FIRST.encode(), configuration)
    cache = project / "build/recovery/parse_cache"
    calls = []
    original = claim_integrity.read_text

    def counted(text, path):
        calls.append(True)
        return original(text, path)

    monkeypatch.setattr(claim_integrity, "read_text", counted)
    args = (project, version, "a" * 64, "1" * 64, "2" * 64, cache)
    rows = claim_integrity._historical_claim_rows(*args)
    rows[0][0] = 0xDEADBEEF
    assert claim_integrity._historical_claim_rows(*args) == [[0x10, 1, True, False]]
    assert len(calls) == 1

    memo_file, = cache.rglob("*.json")
    memo_file.write_text("not valid memo JSON")
    assert claim_integrity._historical_claim_rows(*args) == [[0x10, 1, True, False]]
    assert len(calls) == 2

    memo_file.write_text("{}")
    def fail(*_args):
        calls.append(True)
        raise ValueError("parser rejected input")
    monkeypatch.setattr(claim_integrity, "read_text", fail)
    with pytest.raises(ValueError, match="parser rejected"):
        claim_integrity._historical_claim_rows(*args)
    monkeypatch.setattr(claim_integrity, "read_text", counted)
    assert claim_integrity._historical_claim_rows(*args) == [[0x10, 1, True, False]]
    assert len(calls) == 4


def parser_fixture(root):
    files = {
        "__init__.py": "from . import source\n",
        "source/__init__.py": "from . import grammar, mangled, model, reader, writer\n",
        "source/reader.py": (
            "from .grammar import PATTERN\nfrom .model import Record\n"
            "from ..records import parse_json_value\n"),
        "source/grammar.py": "PATTERN = 'first'\n",
        "source/model.py": "class Record: pass\n",
        "source/writer.py": "from .model import Record\n",
        "source/mangled.py": "VALUE = 1\n",
        "records.py": "from .model import canonical\ndef parse_json_value(x): return x\n",
        "binding_source.py": "from .records import parse_json\n",
        "memo.py": "from .model import digest\n",
        "model.py": "def canonical(x): return x\ndef digest(x): return x\n",
        "source_inventory.py": "from .source_config import Config\nfrom . import binding_source\n",
        "source_config.py": "from .model import Config\n",
        "source/eh_regions.py": "UNRELATED = 1\n",
        "source/asm.py": "UNRELATED = 2\n",
    }
    for name, content in files.items():
        path = root / "match_recovery" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)
    return root / "match_recovery"


def parser_snapshot(package_root):
    inputs, mode = source_inventory._parser_implementation_inputs(
        package_root, source_inventory._PARSER_ROOTS)
    return source_inventory.digest({"inputs": inputs, "mode": mode})


def test_installed_parser_closure_tracks_real_dependencies_only(tmp_path):
    import match_recovery

    original = Path(match_recovery.__file__).resolve().parent
    package = tmp_path / "match_recovery"
    shutil.copytree(original, package)
    inputs, mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert mode == "local_import_closure"
    assert "package/source_inventory.py" in inputs
    assert "package/source_config.py" in inputs
    assert "package/source/reader.py" in inputs
    assert "package/source/grammar.py" in inputs
    assert "package/source/model.py" in inputs
    assert "package/records.py" in inputs
    assert not any(name.endswith(("/eh_regions.py", "/asm.py")) for name in inputs)

    before_unrelated = parser_snapshot(package)
    for name in ("source/eh_regions.py", "source/asm.py"):
        path = package / name
        path.write_text(path.read_text() + "\n# unrelated implementation change\n")
    assert parser_snapshot(package) == before_unrelated

    for name in ("source/reader.py", "source/grammar.py", "source/model.py",
                 "records.py", "source_inventory.py", "source_config.py"):
        before = parser_snapshot(package)
        path = package / name
        path.write_text(path.read_text() + "\n# dependency fingerprint control\n")
        assert parser_snapshot(package) != before, name


def test_parser_key_uses_local_import_closure_and_tracks_parser_dependencies(tmp_path):
    package = parser_fixture(tmp_path / "installed")
    inputs, mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert mode == "local_import_closure"
    assert "package/source/reader.py" in inputs
    assert "package/source/grammar.py" in inputs
    assert "package/source/model.py" in inputs
    assert "package/records.py" in inputs
    assert "package/source/eh_regions.py" not in inputs
    assert "package/source/asm.py" not in inputs

    for name in ("source/reader.py", "source/grammar.py", "source/model.py", "records.py",
                 "source_inventory.py", "source_config.py"):
        before = parser_snapshot(package)
        path = package / name
        path.write_text(path.read_text() + "# changed parser dependency\n")
        assert parser_snapshot(package) != before, name


@pytest.mark.parametrize("statement", [
    "from . import extra",
    "from .extra import parse",
    "from match_recovery.source import extra",
    "import match_recovery.source.extra",
])
def test_new_local_import_is_discovered_without_persistent_graph(tmp_path, statement):
    package = parser_fixture(tmp_path / "installed")
    reader = package / "source/reader.py"
    before = parser_snapshot(package)
    reader.write_text(reader.read_text() + "\n" + statement + "\n")
    (package / "source/extra.py").write_text("VALUE = 1\n")
    inputs, mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert mode == "local_import_closure"
    assert "package/source/extra.py" in inputs
    assert parser_snapshot(package) != before


def test_nested_import_closure_includes_intermediate_package_initializers(tmp_path):
    package = parser_fixture(tmp_path / "installed")
    reader = package / "source/reader.py"
    reader.write_text(reader.read_text() + "\nfrom .plugins import parser\n")
    (package / "source/plugins").mkdir()
    (package / "source/plugins/__init__.py").write_text("from . import parser\n")
    (package / "source/plugins/parser.py").write_text("VALUE = 1\n")
    inputs, mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert mode == "local_import_closure"
    assert "package/source/plugins/__init__.py" in inputs
    assert "package/source/plugins/parser.py" in inputs


def test_package_initializer_collision_matches_python_import_precedence(tmp_path):
    package = parser_fixture(tmp_path / "installed")
    (package / "source.py").write_text("WRONG_MODULE = True\n")
    spec = PathFinder.find_spec("source", [str(package)])
    assert spec is not None
    assert spec.origin == str(package / "source/__init__.py")

    inputs, mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert mode == "local_import_closure"
    assert "package/source/__init__.py" in inputs
    assert "package/source.py" not in inputs
    before = parser_snapshot(package)
    (package / "source.py").write_text("WRONG_MODULE = False\n")
    assert parser_snapshot(package) == before
    initializer = package / "source/__init__.py"
    initializer.write_text(initializer.read_text() + "\n# actual imported package\n")
    assert parser_snapshot(package) != before


def test_unresolved_or_dynamic_import_falls_back_and_new_package_files_invalidate(tmp_path):
    package = parser_fixture(tmp_path / "installed")
    reader = package / "source/reader.py"
    reader.write_text(reader.read_text() + "\nfrom .missing import parse\n")
    before = parser_snapshot(package)
    inputs, mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert mode == "full_package_fallback"
    (package / "source/new_plugin.py").write_text("VALUE = 1\n")
    assert parser_snapshot(package) != before

    reader.write_text(reader.read_text().replace(
        "from .missing import parse", "import importlib\nplugin = importlib.import_module('x')"))
    dynamic_inputs, dynamic_mode = source_inventory._parser_implementation_inputs(
        package, source_inventory._PARSER_ROOTS)
    assert dynamic_mode == "full_package_fallback"
    assert "package/source/new_plugin.py" in dynamic_inputs

    for snippet in ("loader = __import__\nloader('x')",
                    "import importlib as il\nloader = il.import_module\nloader('x')"):
        reader.write_text(reader.read_text() + "\n" + snippet + "\n")
        _, alias_mode = source_inventory._parser_implementation_inputs(
            package, source_inventory._PARSER_ROOTS)
        assert alias_mode == "full_package_fallback"


def test_warm_cache_still_checks_full_merge_history_and_policy_refusals(project, monkeypatch):
    enable_cache(project)
    git(project, "switch", "-c", "side")
    second = project / "engine/second.cc"
    second.write_text("// ORIGINAL: 0x00000020 _second 0x00000020-0x00000028 BYTE_EXACT\nint second() {return 2;}\n")
    git(project, "add", ".")
    git(project, "commit", "-m", "Side branch introduces claim")
    second.write_text("// ORIGINAL: 0x00000020 _second 0x00000020-0x00000028\nint second() {return 2;}\n")
    git(project, "add", ".")
    git(project, "commit", "-m", "Side branch removes claim")
    git(project, "switch", "main")
    git(project, "merge", "--no-ff", "side", "-m", "Merge side history")

    history_calls = []
    original_versions = claim_integrity.source_versions
    def observed_versions(*args, **kwargs):
        history_calls.append(True)
        return original_versions(*args, **kwargs)
    monkeypatch.setattr(claim_integrity, "source_versions", observed_versions)

    first = claim_integrity.check_claims(project)
    second_result = claim_integrity.check_claims(project)
    assert len(history_calls) == 2
    assert first == second_result
    assert first["result"] == "fail"
    assert first["missing"][0]["address"] == "0x00000020"

    policy = json.loads((project / "recovery/project.json").read_text())
    policy["source_history"]["baseline_revision"] = git(project, "rev-parse", "HEAD")
    (project / "recovery/project.json").write_text(json.dumps(policy))
    with pytest.raises(ValueError, match="baseline changed"):
        claim_integrity.check_claims(project)


def test_complete_report_matches_cold_warm_and_cache_off(project, monkeypatch):
    enable_cache(project)
    calls = []
    original = claim_integrity.read_text

    def counted(text, path):
        calls.append(True)
        return original(text, path)

    monkeypatch.setattr(claim_integrity, "read_text", counted)
    cold = claim_integrity.check_claims(project)
    assert cold["result"] == "pass"
    cold_count = len(calls)
    assert cold_count > 0

    calls.clear()
    warm = claim_integrity.check_claims(project)
    assert warm == cold
    assert calls == []

    monkeypatch.setenv("RECOVERY_PARSE_CACHE", "off")
    calls.clear()
    uncached = claim_integrity.check_claims(project)
    assert uncached == cold
    assert len(calls) == cold_count

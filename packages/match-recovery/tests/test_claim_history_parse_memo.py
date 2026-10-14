"""Historical claim parsing is cached as evidence, never as a verdict."""

from dataclasses import replace
import json
from pathlib import Path
import subprocess

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
    assert source_inventory._parse_cache_directory(project) is None
    enable_cache(project)
    cache = source_inventory._parse_cache_directory(project)
    assert cache == project / "build/recovery/parse_cache"
    monkeypatch.setenv("RECOVERY_PARSE_CACHE", "off")
    assert source_inventory._parse_cache_directory(project) is None


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
    assert len(calls) == 7


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

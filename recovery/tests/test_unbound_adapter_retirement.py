"""Retirement preserves all mapped identities as explicit pending originals."""

from __future__ import annotations

import subprocess
import json
from pathlib import Path

import pytest

from match_recovery.source import Mode, Recipe, SourceRole, State, Tier, read, read_text
from match_recovery.source.asm import compare_unit
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_inventory import source_snapshot
from match_recovery.symbol_check import check_symbols


ROOT = Path(__file__).resolve().parents[2]
BASE_REVISION = "a6233980e9b1b61491bea3749203d6804abb3cc0"
OLD_FILES = (
    "src/field_accessors.cpp", "src/field_accessors.h",
    "src/nullsub_thunks.cpp", "src/nullsub_thunks.h",
)
PENDING_FILE = "src/unbound_functions.h"
OLD_CPP = ("src/field_accessors.cpp", "src/nullsub_thunks.cpp")


def _historical(path: str) -> str:
    result = subprocess.run(
        ["git", "show", f"{BASE_REVISION}:{path}"], cwd=ROOT,
        capture_output=True, text=True, check=False, timeout=20)
    assert result.returncode == 0, result.stderr
    return result.stdout


def _lessons(record):
    return tuple((lesson.kind, lesson.key, lesson.prose)
                 for lesson in (record.origin or ()))


def _records_for_source(path: str):
    return read_text(_historical(path), ROOT / path)


@pytest.fixture(scope="module")
def transfer():
    historical = [record for path in OLD_CPP for record in _records_for_source(path)]
    current = read(ROOT / PENDING_FILE)
    return historical, current


def test_all_52_adapter_identities_are_transferred_without_claim_or_lesson_loss(transfer):
    historical, current = transfer
    assert len(historical) == 52
    assert len(current) == 52
    assert len({record.address for record in historical}) == 52
    assert len({record.address for record in current}) == 52

    old_by_address = {record.address: record for record in historical}
    new_by_address = {record.address: record for record in current}
    assert set(new_by_address) == set(old_by_address)
    for address, old in old_by_address.items():
        new = new_by_address[address]
        assert (new.name, new.image_spans, new.kind) == (old.name, old.image_spans, old.kind)
        assert (new.byte_exact, new.semantic) == (old.byte_exact, old.semantic)
        assert (new.levers, new.ruled_out, new.unrecoverable, new.deferred) == (
            old.levers, old.ruled_out, old.unrecoverable, old.deferred)
        assert _lessons(new) == _lessons(old), f"lesson loss at 0x{address:08X}"
        assert new.mode is Mode.UNBOUND and new.state is State.UNBOUND
        assert new.recipe is Recipe.UNBOUND
        assert new.symbol == new.body == new.context == ""
        assert new.source_role is SourceRole.UNCLASSIFIED
        assert new.source_role_reason == ""


def test_retired_files_and_build_inputs_are_absent_and_no_product_duplicate_owns_them(transfer):
    historical, current = transfer
    expected = {record.address for record in historical}
    assert all(not (ROOT / path).exists() for path in OLD_FILES)
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    assert all(Path(path).name not in cmake for path in OLD_CPP)

    configuration = SourceConfiguration.read(ROOT)
    snapshot = source_snapshot(ROOT)
    owners = [row for row in snapshot["records"]
              if row["candidate_kind"] == "source" and not configuration.is_seed(row["path"])
              and row["origin"]["address"] in expected]
    assert len(owners) == 52
    assert {row["origin"]["address"] for row in owners} == expected
    assert all(row["path"] == PENDING_FILE for row in owners)
    assert len(current) == len(owners)


def test_pending_measurement_and_symbol_checks_refuse_without_compiler_calls(transfer, tmp_path):
    _, current = transfer
    calls = []

    def forbidden_compiler(*args, **kwargs):
        calls.append((args, kwargs))
        raise AssertionError("UNBOUND originals must never request compilation")

    result = compare_unit(current, ROOT / "missing-original-image.exe", ROOT / PENDING_FILE,
                          (), ("not-a-real-compiler-trial",), compiler=forbidden_compiler)
    assert result.compile_attempts == result.reused_attempts == 0
    assert len(result.results) == 52
    assert all(observation.verdict is Tier.REFUSED
               and "UNBOUND" in observation.diagnostic
               for _, observation in result.results)

    for directory in ("src", "recovery", "build", "original"):
        (tmp_path / directory).mkdir()
    (tmp_path / "src/unbound_functions.h").write_text(
        (ROOT / PENDING_FILE).read_text(encoding="utf-8"), encoding="utf-8")
    (tmp_path / "src/decoy.cc").write_text("void decoy() {}\n")
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "build/compile_commands.json").write_text(json.dumps([
        {"directory": str(tmp_path), "file": "src/decoy.cc",
         "arguments": ["not-run", "-c", "src/decoy.cc"]}]))
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "pe:va",
                   "roots": ["src"], "extensions": [".h", ".cc"],
                   "interface_extensions": [".h"], "seed_roots": [],
                   "include_paths": ["src"], "symbol_format": "msvc"},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "fixture",
                        "flag_sets": [["not-a-real-compiler-trial"]], "layout_flags": [],
                        "unit_extensions": [".cc"], "inputs": [], "input_roots": []},
        "mappings": {"compile_commands": "build/compile_commands.json",
                     "forwarder_macros": []},
    }))
    symbols = check_symbols(tmp_path, files=("src/unbound_functions.h",),
                            compiler=forbidden_compiler)
    assert symbols["result"] == "unknown"
    assert symbols["compile_attempts"] == 0
    assert symbols["counts"] == {"unknown": 52}
    assert len(symbols["functions"]) == 52
    assert all(row["result"] == "unknown" and row["symbol"] == ""
               and row["mapping_disposition"] == "unbound_pending"
               for row in symbols["functions"])
    assert calls == []


def test_pending_header_has_no_adapter_declarations_or_symbol_role_fallbacks():
    text = (ROOT / PENDING_FILE).read_text(encoding="utf-8")
    assert "RECOVERY_ROLE:" not in text
    assert not any(line.lstrip().startswith("// symbol") for line in text.splitlines())
    assert "_redirect(" not in text
    assert len([line for line in text.splitlines() if line.startswith("// ORIGINAL:")]) == 52

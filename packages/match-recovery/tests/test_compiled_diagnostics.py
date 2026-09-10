"""Portable unlinked diagnostics: synthetic COFF/PE, no external compiler."""

from dataclasses import replace
import hashlib
import json
import sys

import pytest

from match_recovery import compiled_diagnostics as diagnostic
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.cli import main
from match_recovery.model import Origin, Span, canonical
from match_recovery.pe_coff import Coff
from test_binary import coff_fixture
from test_multispan import fixture
from test_variants import Compiler, deck, measure, project, renamed_deck


def inspect(obj, binding=None, budget=256 * 1024):
    binding = binding or FunctionBinding(Origin("a" * 64, "pe:va", 0x401000),
                                         (Span(0x401000, 0x401001),), "_f", "code/unit.cc")
    return diagnostic.compiled_diagnostic(obj, binding, name="trial", flags=("-O2",), budget=budget)


def test_raw_extent_and_relocation_are_not_original_addresses_or_verdicts():
    obj = Coff(coff_fixture(b"\xe8\x04\0\0\0\xc3" + b"\0" * 16, relocation=(1, 1, 20)))
    report = inspect(obj)
    part, = report["sections"]
    assert report["status"] == "observed" and "result" not in report
    assert part["raw_size"] == len(obj.sections[0].data)
    assert bytes.fromhex(part["raw_bytes_hex"]) == obj.sections[0].data
    assert part["sha256"] == hashlib.sha256(obj.sections[0].data).hexdigest()
    call = part["instructions"][0]
    assert call == {"offset": 0, "bytes": "e804000000", "mnemonic": "call",
                    "operands_unlinked": "9", "relocation_offsets": [1]}
    relocation, = part["relocations"]
    assert relocation["symbol"] == "_g" and relocation["raw_addend"] == 4
    assert relocation["target_section"] == 0 and relocation["target_value"] == 0
    assert "00401000" not in canonical(report).decode()
    # The RET is not used to invent an extent; raw table/padding bytes survive.
    assert part["reported_bytes"] > 6


def test_owned_companions_survive_missing_original_span_without_guessing_siblings():
    binding, objects, image = fixture(extra=(b".text$y", b"\xc3", (), 0, 2, 0x60501020))
    binding = replace(binding, spans=binding.spans[:1])
    obj = objects[binding.unit]
    before, = check_objects([binding], objects, image)
    assert before["result"] == "unknown"
    report = inspect(obj, binding)
    assert [part["section"] for part in report["sections"]] == [1, 2]
    assert report["sections"][0]["relocations"][0]["symbol"] == "$tail2"
    assert check_objects([binding], objects, image) == [before]


@pytest.mark.parametrize("storage", [3, 105])
@pytest.mark.parametrize("kind,offset", [(6, 1), (20, 99), (7, 1)])
def test_relocation_metadata_retains_static_weak_and_unsupported_targets(kind, offset, storage):
    obj = Coff(coff_fixture(b"\xa1\0\0\0\0\xc3", relocation=(offset, 1, kind)))
    obj.symbols[1] = replace(obj.symbols[1], storage=storage, name="_target")
    entry, = inspect(obj)["sections"][0]["relocations"]
    assert entry["symbol"] == "_target" and entry["storage_class"] == storage
    assert entry["coff_type"] == kind and entry["offset"] == offset
    assert entry["raw_addend"] == (0 if kind == 6 else None)
    assert "binding_address" not in entry


def test_invalid_decode_tail_is_visible_and_instruction_limit_is_not_complete(monkeypatch):
    obj = Coff(coff_fixture(b"\xc3"))
    # Precisely sized synthetic section, with an incomplete two-byte opcode.
    obj.sections = (replace(obj.sections[0], data=b"\xc3\x0f"),)
    part, = inspect(obj)["sections"]
    assert part["raw_bytes_hex"] == "c30f"
    assert part["decoded_bytes"] == 1 and part["undecoded_reported_bytes"] == 1
    assert part["decode_status"] == "stopped"
    monkeypatch.setattr(diagnostic, "MAX_INSTRUCTIONS", 1)
    obj.sections = (replace(obj.sections[0], data=b"\x90\xc3"),)
    part, = inspect(obj)["sections"]
    assert part["decoded_instruction_count"] == 1 and part["undecoded_reported_bytes"] == 1
    assert part["decode_status"] == "instruction_limit"


def test_missing_or_ambiguous_target_is_unavailable_not_an_inferred_extent():
    assert inspect(None)["status"] == "unavailable"
    obj = Coff(coff_fixture())
    obj.symbols[77] = replace(obj.symbols[0], index=77)
    assert inspect(obj)["status"] == "unavailable"


def test_absent_optional_decoder_retains_bytes_without_claiming_coverage(monkeypatch):
    monkeypatch.setitem(sys.modules, "capstone", None)
    obj = Coff(coff_fixture())
    part, = inspect(obj)["sections"]
    assert bytes.fromhex(part["raw_bytes_hex"]) == obj.sections[0].data
    assert part["decode_status"] == "unavailable"
    assert part["decoded_bytes"] == part["decoded_instruction_count"] == 0
    assert part["undecoded_reported_bytes"] == part["reported_bytes"]


def test_decoder_initialization_failure_keeps_ordinary_measurements(project, monkeypatch):
    import capstone

    plain = measure(project)

    def unavailable(*args, **kwargs):
        raise capstone.CsError(capstone.CS_ERR_ARCH)

    monkeypatch.setattr(capstone, "Cs", unavailable)
    detailed = measure(project, compiled_diagnostics=True)
    extra = detailed.pop("compiled_diagnostics")
    assert detailed == plain
    for entry in extra["entries"]:
        for part in entry["sections"]:
            assert part["decode_status"] == "unavailable"
            assert part["raw_bytes_hex"]
            assert part["undecoded_reported_bytes"] == part["reported_bytes"]


def test_decoder_failure_after_partial_output_keeps_observations(monkeypatch):
    import capstone

    class InterruptedDecoder:
        def disasm_lite(self, *args, **kwargs):
            yield 0, 1, "nop", ""
            raise capstone.CsError(capstone.CS_ERR_MEM)

    monkeypatch.setattr(capstone, "Cs", lambda *args: InterruptedDecoder())
    obj = Coff(coff_fixture(b"\x90\xc3"))
    part, = inspect(obj)["sections"]
    assert part["decode_status"] == "decode_error"
    assert part["decoded_bytes"] == 1
    assert part["listed_instruction_count"] == 1
    assert part["undecoded_reported_bytes"] == part["reported_bytes"] - 1
    assert bytes.fromhex(part["raw_bytes_hex"]) == obj.sections[0].data


def test_per_trial_and_report_limits_include_long_names_and_metadata(monkeypatch):
    monkeypatch.setattr(diagnostic, "MAX_REPORT_BYTES", 4096)
    obj = Coff(coff_fixture(b"\x90" * 6000))
    entry = inspect(obj, budget=1800)
    assert entry is not None and entry["truncated"]
    assert len(canonical(entry)) <= 1800
    assert entry["sections"][0]["omitted_bytes"] > 0
    huge = replace(FunctionBinding(Origin("a" * 64, "pe:va", 0x401000),
                                   (Span(0x401000, 0x401001),), "_f", "code/unit.cc"), symbol="x" * 5000)
    assert inspect(obj, huge, budget=1800) is None
    report = diagnostic.diagnostic_report([entry, entry, None, entry], 4)
    assert len(canonical(report)) <= 4096
    assert report["omitted_trials"] > 0 and 2 in report["omitted_trial_indices"]
    assert report["omitted_trials"] == len(report["omitted_trial_indices"]) + report["unlisted_omitted_trials"]


def test_opt_in_reuses_objects_without_changing_default_report_or_compile_count(project):
    class Counted(Compiler):
        def __init__(self):
            self.calls = 0
        def __call__(self, *args):
            self.calls += 1
            return super().__call__(*args)
        def overlay(self, *args):
            self.calls += 1
            return super().overlay(*args)
    plain_compiler, detailed_compiler = Counted(), Counted()
    plain = measure(project, compiler=plain_compiler)
    detailed = measure(project, compiler=detailed_compiler, compiled_diagnostics=True)
    extra = detailed.pop("compiled_diagnostics")
    assert detailed == plain
    assert plain_compiler.calls == detailed_compiler.calls == plain["compile_attempts"] == 5
    assert [entry["name"] for entry in extra["entries"]] == ["baseline", "exact", "different"]
    assert extra["omitted_trials"] == 0
    assert all("compiled_diagnostics" not in row["target"] for row in plain["candidates"])


def test_parallel_budget_is_stable_and_extent_failures_keep_listings(project, monkeypatch):
    monkeypatch.setattr(diagnostic, "MAX_REPORT_BYTES", 6000)
    class Large(Compiler):
        def variant(self, source, contents, command, flags):
            if b"for (;;)" in contents or b"baseline" in contents:
                return super().variant(source, contents, command, flags)
            return coff_fixture(b"\x90" * 10000)
    serial = measure(project, compiler=Large(), compiled_diagnostics=True)
    parallel = measure(project, compiler=Large(), compiled_diagnostics=True, jobs=2)
    assert serial["compiled_diagnostics"] == parallel["compiled_diagnostics"]
    assert serial["rankings"] == parallel["rankings"]
    assert len(canonical(serial["compiled_diagnostics"])) <= 6000
    assert all(row["target"]["boundary"] == "fail" for row in serial["candidates"])
    assert all(entry["status"] == "observed" for entry in serial["compiled_diagnostics"]["entries"])
    assert any(entry["truncated"] for entry in serial["compiled_diagnostics"]["entries"])


def test_cli_flag_is_explicit_and_uses_same_local_compiler(project, monkeypatch, capsys):
    root, task = project
    (root / "deck.json").write_text(json.dumps(deck(project)))
    monkeypatch.setattr("match_recovery.variants.Compiler", lambda *args, **kwargs: Compiler())
    assert main(["tasks", "variants", task.task_id, "--root", str(root), "--image", "image.exe",
                 "--deck", "deck.json", "--compiled-diagnostics"]) == 0
    assert json.loads(capsys.readouterr().out)["compiled_diagnostics"]["requested_trials"] == 3


@pytest.mark.parametrize("renamed", [False, True])
def test_candidate_listing_uses_candidate_binding_not_baseline_symbol(project, renamed):
    class Renamed(Compiler):
        def variant(self, source, contents, *args):
            raw = super().variant(source, contents, *args)
            return raw.replace(b"_f", b"_r") if renamed and b"// symbol" in contents else raw
    report = measure(project, renamed_deck(project), compiler=Renamed(), compiled_diagnostics=True)
    baseline, candidate = report["compiled_diagnostics"]["entries"]
    assert baseline["symbol"] == "_f" and candidate["symbol"] == "_r"
    assert candidate["status"] == ("observed" if renamed else "unavailable")
    assert report["candidates"][0]["target"]["result"] == ("pass" if renamed else "unknown")


def test_freshness_still_covers_diagnostic_generation(project, monkeypatch):
    original = diagnostic.compiled_diagnostic
    def changed(*args, **kwargs):
        result = original(*args, **kwargs)
        (project[0] / "code/unit.cc").write_text("changed during diagnostic generation")
        return result
    monkeypatch.setattr(diagnostic, "compiled_diagnostic", changed)
    with pytest.raises(ValueError, match="changed"):
        measure(project, compiled_diagnostics=True)


@pytest.mark.parametrize("value", [1, "yes", None])
def test_non_boolean_option_refuses(project, value):
    with pytest.raises(ValueError, match="boolean"):
        measure(project, compiled_diagnostics=value)

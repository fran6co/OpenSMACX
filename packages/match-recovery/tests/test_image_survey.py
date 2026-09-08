"""Synthetic rooted-disassembly controls; no proprietary fixtures or inferred extents."""

import hashlib
import json
import struct
import sys

import pytest

from match_recovery.cli import main
from match_recovery.image_survey import survey, survey_image
from match_recovery.pe_coff import PeImage
from match_recovery.project import initialize
from test_binary import pe_fixture
from test_imports import image_imports


def executable(raw, *, entry=0x1000, executable=True, size=None):
    raw = bytearray(raw)
    struct.pack_into("<I", raw, 0x98 + 16, entry)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020 if executable else 0x40000040)
    if size is not None:
        struct.pack_into("<I", raw, 0x178 + 16, size)
    return bytes(raw)


def observe(raw, **kwargs):
    pin = hashlib.sha256(raw).hexdigest()
    return survey(PeImage(raw, pin), pin, **kwargs)


def test_rooted_calls_are_candidates_but_jumps_and_unreachable_bytes_are_not_functions():
    # call +16; jmp +8; unreachable call-looking bytes; two returns.
    code = b"\xe8\x0b\0\0\0\xeb\x01\xe8\xc3" + bytes(7) + b"\xc3"
    report = observe(executable(pe_fixture(code)))
    assert report["entrypoint"] == 0x401000
    assert {row["address"] for row in report["instructions"]} == {0x401000, 0x401005, 0x401008, 0x401010}
    assert {row["origin"]["address"] for row in report["candidates"]} == {0x401000, 0x401010}
    assert report["candidates"][1]["evidence"] == [{"kind": "decoded_direct_call", "source": 0x401000}]
    assert all(row["original_extent"] == row["ownership"] == "unknown" for row in report["candidates"])
    assert report["historical_tu_boundaries"] == "not_inferred"
    assert report["verification"] == "not_measured" and not report["ready_for_collection"]


@pytest.mark.parametrize("code", [b"\x75\x01\xc3\xc3", b"\xe2\x01\xc3\xc3", b"\xe3\x01\xc3\xc3"])
def test_conditional_paths_include_fallthrough_and_target_without_new_function_candidates(code):
    report = observe(executable(pe_fixture(code)))
    assert {row["address"] for row in report["instructions"]} == {0x401000, 0x401002, 0x401003}
    assert report["candidate_count"] == 1


def test_cycles_and_budgets_terminate_with_explicit_pending_coverage():
    cycle = observe(executable(pe_fixture(b"\xeb\xfe")))
    assert cycle["instruction_count"] == 1 and not cycle["budget_exhausted"]
    budget = observe(executable(pe_fixture(b"\x90\x90\xc3")), max_instructions=1)
    assert budget["instruction_count"] == 1 and budget["budget_exhausted"]
    assert budget["pending_addresses"] == [0x401001]
    assert budget["coverage"] == "partial_static_reachability"


def test_call_to_interior_retains_conflicting_interpretations_not_extent_claims():
    report = observe(executable(pe_fixture(b"\xe8\xfc\xff\xff\xff\xc3")))  # call start+1
    assert report["unresolved"][0]["reason"] == "overlapping instruction interpretations"
    assert all(row["decode_status"] == "conflict" for row in report["candidates"])


def test_iat_is_a_dependency_slot_not_a_local_function_candidate():
    report = observe(executable(image_imports()))
    edge, = report["edges"]
    assert edge["target"] is None and edge["slot"] == 0x401160
    assert edge["import"]["module"] == "DEMO.dll" and edge["import"]["name"] == "Paint"
    assert report["candidate_count"] == 1
    assert report["unresolved"][0]["reason"] == "indirect control flow not followed"


@pytest.mark.parametrize("code", [b"\xff\xe0", b"\xff\x20"])
def test_unknown_indirect_jump_stops_instead_of_falling_into_neighboring_bytes(code):
    report = observe(executable(pe_fixture(code + b"\xc3")))
    assert report["instruction_count"] == 1
    assert report["edges"][0]["target"] is None and len(report["unresolved"]) == 1


@pytest.mark.parametrize("damage", ["nonexec", "virtual_tail", "truncated_instruction", "zero_entry"])
def test_no_linear_sweep_into_data_unbacked_bytes_or_missing_entry(damage):
    raw = executable(pe_fixture(b"\xe8"), executable=damage != "nonexec",
                     entry=0 if damage == "zero_entry" else 0x1001 if damage == "virtual_tail" else 0x1000,
                     size=1 if damage in ("truncated_instruction", "virtual_tail") else None)
    report = observe(raw)
    assert report["instruction_count"] == 0
    assert report["metadata_gaps"] if damage == "zero_entry" else report["unresolved"]


def test_explicit_analysis_hint_works_without_a_pe_entry_but_remains_a_hint():
    report = observe(executable(pe_fixture(b"\xc3"), entry=0), starts=[0x401000])
    assert report["candidate_count"] == report["instruction_count"] == 1
    assert report["candidates"][0]["evidence"] == [{"kind": "supplied_start_hint"}]


@pytest.mark.parametrize("options", [{"starts": [-1]}, {"starts": [True]}, {"starts": [0x401000, 0x401000]},
                                     {"max_instructions": 0}, {"max_instructions": True}, {"max_instructions": 1000001}])
def test_invalid_limits_and_addresses_refuse(options):
    with pytest.raises(ValueError):
        observe(executable(pe_fixture()), **options)


def project(tmp_path):
    image = tmp_path / "original.exe"
    image.write_bytes(executable(pe_fixture(b"\xc3")))
    initialize(tmp_path, "survey-control", sys.executable, image=image)
    return image


def test_cli_starts_from_executable_with_no_recovered_source(tmp_path, capsys):
    project(tmp_path)
    before = {path: path.read_bytes() for path in tmp_path.rglob("*") if path.is_file()}
    assert main(["survey", "--root", str(tmp_path), "--image", "original.exe", "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["unmapped_candidate_count"] == report["candidate_count"] == 1
    assert report["candidates"][0]["source_bindings"] == [] and not report["persisted"]
    assert "instructions" not in report and "edges" not in report
    assert before == {path: path.read_bytes() for path in tmp_path.rglob("*") if path.is_file()}


def test_source_correlations_keep_all_competing_seeds_without_picking_a_symbol(tmp_path):
    image = project(tmp_path)
    config_path = tmp_path / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["seed_roots"] = ["src/seeds"]
    config_path.write_text(json.dumps(config))
    (tmp_path / "src/seeds").mkdir()
    for file, name in (("src/f.cpp", "_f"), ("src/seeds/f.cpp", "_guess")):
        (tmp_path / file).write_text(f"// ORIGINAL: 0x00401000 {name} 0x00401000-0x00401001\nvoid f() {{}}\n")
    report = survey_image(tmp_path, image)
    candidate, = report["candidates"]
    assert {row["candidate_kind"] for row in candidate["source_bindings"]} == {"source", "legacy_seed"}
    assert candidate["ownership"] == "unknown" and report["unmapped_candidate_count"] == 0


def test_declared_upstream_binding_is_not_an_unmapped_application_recovery(tmp_path):
    from match_recovery.component_source import render_component
    from test_binary import component_fixture
    image = project(tmp_path)
    (tmp_path / "src/vendor.recovery.h").write_text(render_component(component_fixture(image.read_bytes())))
    report = survey_image(tmp_path, image)
    candidate, = report["candidates"]
    assert candidate["component_bindings"][0]["component"] == "fixture"
    assert candidate["next_action"] == "verify_dependency"
    assert report["declared_dependency_candidate_count"] == 1 and report["unmapped_candidate_count"] == 0
    assert candidate["ownership"] == "unknown"  # Declaration is not fresh dependency proof.


def test_malformed_imports_are_reported_as_missing_evidence_not_silently_empty():
    raw = bytearray(executable(image_imports()))
    struct.pack_into("<I", raw, 0x320 + 16, 0)
    report = observe(bytes(raw))
    assert report["imports"] == []
    assert report["metadata_gaps"][0]["kind"] == "ordinary_imports"
    assert report["edges"][0]["target"] is None and "import" not in report["edges"][0]


@pytest.mark.parametrize("damage", ["source", "image"])
def test_changed_inputs_during_survey_are_refused(tmp_path, monkeypatch, damage):
    import match_recovery.image_survey as module
    image = project(tmp_path)
    native = module.survey
    def changing(*args, **kwargs):
        report = native(*args, **kwargs)
        if damage == "source":
            (tmp_path / "src/new.cpp").write_text("void changed() {}\n")
        else:
            image.write_bytes(image.read_bytes() + b"changed")
        return report
    monkeypatch.setattr(module, "survey", changing)
    with pytest.raises(ValueError, match="changed"):
        survey_image(tmp_path, image)

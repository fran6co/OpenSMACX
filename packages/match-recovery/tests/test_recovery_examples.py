from __future__ import annotations

import importlib.util
from hashlib import sha256
import json
import sys
from dataclasses import dataclass
from pathlib import Path
from types import SimpleNamespace

import pytest


PACKAGE_ROOT = Path(__file__).resolve().parents[1]
MODULE_PATH = PACKAGE_ROOT / "src" / "match_recovery" / "recovery_examples.py"
spec = importlib.util.spec_from_file_location("recovery_examples", MODULE_PATH)
recovery_examples = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = recovery_examples
spec.loader.exec_module(recovery_examples)


@dataclass(frozen=True)
class Mapping:
    address: int
    path: Path
    line: int
    name: str
    symbol: str
    image_spans: tuple[tuple[int, int], ...]

    @property
    def address_hex(self):
        return f"0x{self.address:08X}"


class FakeCandidate:
    def __init__(self, candidate_id, instructions, **kwargs):
        self.candidate_id, self.instructions = candidate_id, instructions
        self.opcodes = instructions
        self.kwargs = kwargs

    @classmethod
    def create(cls, candidate_id, instructions, **kwargs):
        return cls(candidate_id, instructions, **kwargs)


class FakeReference(FakeCandidate):
    @property
    def reference_id(self):
        return self.candidate_id

    @classmethod
    def create(cls, reference_id, instructions, **kwargs):
        return cls(reference_id, instructions, **kwargs)


def _records(root, *, candidate_role="body"):
    rows = []
    for entity_id, address, line, role in (
        ("candidate", 0x401000, 10, candidate_role),
        ("reference", 0x401010, 20, "body"),
    ):
        rows.append({
            "entity_id": entity_id, "path": "src/sample.cpp", "line": line,
            "origin": {"address": address}, "spans": [[address, address + 2]],
            "candidate_kind": "source", "source_role": role,
            "legacy_byte_exact": True, "emitted_symbol_hint": f"?{entity_id}@@",
            "catalogued_symbol": entity_id,
        })
    return rows


def _install_fakes(monkeypatch, tmp_path, *, candidate_role="body", measurement=None,
                   similarity_api=None):
    root = tmp_path / "project"
    root.mkdir()
    (root / "src").mkdir()
    source = root / "src/sample.cpp"
    source.write_text("void candidate() {}\nvoid reference() {}\n")
    image = root / "original.exe"
    image.write_bytes(b"not-a-real-pe")
    rows = _records(root, candidate_role=candidate_role)
    facts = {row["entity_id"]: row for row in rows}
    snapshot = {
        "revision": "abc123", "image_sha256": sha256(image.read_bytes()).hexdigest(),
        "files": {"src/sample.cpp": {"sha256": "source-digest"}}, "records": rows,
    }
    records = {
        "candidate": SimpleNamespace(path=source, line=10, address=0x401000,
                                      address_hex="0x00401000", region="void candidate() { return; }",
                                      state=SimpleNamespace(value="implemented")),
        "reference": SimpleNamespace(path=source, line=20, address=0x401010,
                                      address_hex="0x00401010", region="void reference() { return; }",
                                      state=SimpleNamespace(value="implemented")),
    }
    mappings = [Mapping(row["origin"]["address"], source, row["line"], row["catalogued_symbol"],
                        row["emitted_symbol_hint"], tuple(map(tuple, row["spans"]))) for row in rows]

    class Config:
        address_space = "pe:va"
        symbol_format = "msvc"

        @classmethod
        def read(cls, ignored_root):
            return cls()

        @staticmethod
        def path(base, relative):
            return base / relative

    def select(_mappings, targets, _host, _symbol_format):
        selected = []
        for target in targets:
            match = [m for m in mappings if target in (m.name, m.symbol, m.address_hex)]
            if len(match) != 1:
                raise ValueError(f"target {target} has {len(match)} matches")
            selected.extend(match)
        return selected

    monkeypatch.setattr(recovery_examples.SourceConfiguration, "read", Config.read)
    monkeypatch.setattr(recovery_examples, "source_snapshot", lambda *_args: snapshot)
    monkeypatch.setattr(recovery_examples, "project_view", lambda *_args: ({
        "groups": [{"selector": "class:Sample", "entities": [{"entity_id": key} for key in facts]}]
    }, snapshot))
    monkeypatch.setattr(recovery_examples, "snapshot_mappings", lambda *_args: mappings)
    monkeypatch.setattr(recovery_examples, "select_records", select)
    monkeypatch.setattr(recovery_examples, "selected_source_records",
                        lambda _root, _snapshot, selected: [records[m.name] for m in selected])
    monkeypatch.setattr(recovery_examples, "quality_report", lambda *_args: {"functions": []})
    monkeypatch.setattr(recovery_examples, "assert_snapshot_current", lambda *_args: None)

    class FakePeImage:
        def __init__(self, _data, _fingerprint):
            pass

        def read(self, span):
            assert span.end - span.start == 2
            return b"\x90\xc3"

    monkeypatch.setattr(recovery_examples, "PeImage", FakePeImage)
    worker_calls = {}

    @dataclass
    class Match:
        reference_id: str
        reference_group: str | None
        score: float
        sequence_ratio: float
        length_ratio: float
        opcode_jaccard: float
        provenance: str
        prefiltered: bool
        provisional_provenance: bool
        tentative: bool

    @dataclass
    class RankedCandidate:
        candidate_id: str
        ownership_group: str
        best_score: float
        matches: tuple
        unmatched: bool

    @dataclass
    class Ranking:
        advisory_only: bool = True
        equivalence_established: bool = False
        acceptance_established: bool = False
        candidate_pool_size: int = 1
        returned_count: int = 1
        omitted_count: int = 0
        excluded_reference_count: int = 0
        candidates: tuple = ()

    def rank(candidates, references, **kwargs):
        worker_calls.update(candidates=candidates, references=references, kwargs=kwargs)
        rows = []
        for candidate in candidates[:kwargs["limit"]]:
            matches = []
            for reference in references[:kwargs["top_k"]]:
                score = 1.0 if reference.opcodes == candidate.opcodes else 0.0
                matches.append(Match(reference.reference_id, reference.kwargs.get("ownership_group"),
                                     score, score, 1.0, 1.0, reference.kwargs["provenance"],
                                     False, reference.kwargs["provisional_provenance"], False))
            rows.append(RankedCandidate(candidate.candidate_id,
                                        kwargs["candidate_groups"][candidate.candidate_id],
                                        matches[0].score if matches else 0.0,
                                        tuple(matches), not bool(matches)))
        return Ranking(candidates=tuple(rows), candidate_pool_size=len(candidates),
                       returned_count=len(rows), omitted_count=len(candidates) - len(rows))

    fake_similarity = similarity_api or SimpleNamespace(
        OpcodeCandidate=FakeCandidate, OpcodeReference=FakeReference,
        rank_ownership_groups=rank)
    monkeypatch.setitem(sys.modules, "match_recovery.similarity", fake_similarity)

    measured_calls = []

    def measure(_root, targets, *, image):
        measured_calls.append((list(targets), image))
        value = measurement or {
            "verification": "historical_masked_bytes_only",
            "measurements": [{"path": "src/sample.cpp", "line": 20,
                              "result": "pass", "verdict": "BYTE_EXACT"}],
        }
        return value

    return root, image, measured_calls, worker_calls, measure


def test_freshly_measured_reference_uses_pinned_opcodes_and_masks_only(monkeypatch, tmp_path):
    root, image, measured, calls, measure = _install_fakes(monkeypatch, tmp_path)
    result = recovery_examples.recommend_examples(
        root, image, ["candidate"], ["reference"], measure_source=measure)

    assert measured == [(["0x00401010"], image)]  # only explicit exemplars are measured
    assert calls["references"][0].instructions == ("nop", "ret")
    assert calls["candidates"][0].instructions == ("nop", "ret")
    assert calls["references"][0].kwargs["verified"] is True
    assert calls["references"][0].kwargs["provisional_provenance"] is True
    assert result["reference_context"]["reference"]["verification_level"] == "masked_byte_exact"
    assert result["measurement_kind"] == "masked"
    assert result["advisory_only"] is True
    assert result["equivalence_established"] is False
    json.dumps(result)


def test_scaffold_candidate_remains_investigable_but_cannot_be_reference(monkeypatch, tmp_path):
    root, image, measured, calls, measure = _install_fakes(
        monkeypatch, tmp_path, candidate_role="scaffold")
    result = recovery_examples.recommend_examples(
        root, image, ["candidate"], ["candidate"], measure_source=measure)

    assert len(calls["candidates"]) == 1
    assert result["candidate_context"]["candidate"]["scaffold_investigation_target"] is True
    assert calls["references"] == []
    exclusion = next(row for row in result["unresolved"] if row.get("status") == "reference excluded")
    assert "scaffold" in exclusion["reason"]
    assert measured == []


def test_ambiguous_reference_is_reported_not_guessed_or_measured(monkeypatch, tmp_path):
    root, image, measured, calls, measure = _install_fakes(monkeypatch, tmp_path)

    def ambiguous(_mappings, targets, _host, _format):
        if targets == ["ambiguous"]:
            raise ValueError("two source mappings")
        return [Mapping(0x401000, root / "src/sample.cpp", 10, "candidate", "?candidate@@",
                        ((0x401000, 0x401002),))]

    monkeypatch.setattr(recovery_examples, "select_records", ambiguous)
    result = recovery_examples.recommend_examples(
        root, image, ["candidate"], ["ambiguous"], measure_source=measure)

    assert measured == []
    assert calls["references"] == []
    assert result["unresolved"][0]["status"] == "unresolved or ambiguous reference target"


def test_component_target_is_reported_as_excluded_not_ranked(monkeypatch, tmp_path):
    root, image, _measured, calls, _measure = _install_fakes(monkeypatch, tmp_path)
    monkeypatch.setattr(recovery_examples, "project_view", lambda *_args: ({
        "groups": [
            {"selector": "component:vendor", "entities": [{"entity_id": "candidate"}]},
            {"selector": "class:Sample", "entities": [{"entity_id": "reference"}]},
        ]
    }, recovery_examples.source_snapshot(root, image)))

    result = recovery_examples.recommend_examples(root, image, ["candidate"])

    assert result["candidate_context"] == {}
    assert result["rankings"] is None
    assert calls == {}
    assert result["unresolved"][0]["status"] == "excluded third-party/component candidate"


def test_strict_mode_requires_injected_strict_measurement(monkeypatch, tmp_path):
    root, image, _measured, _calls, measure = _install_fakes(monkeypatch, tmp_path)
    with pytest.raises(ValueError, match="injected strict"):
        recovery_examples.recommend_examples(
            root, image, ["candidate"], ["reference"], measurement_kind="strict")

    strict = lambda root, targets, *, image: {
        "measurement_kind": "strict",
        "measurements": [{"path": "src/sample.cpp", "line": 20,
                          "result": "pass", "verdict": "BYTE_EXACT"}],
    }
    result = recovery_examples.recommend_examples(
        root, image, ["candidate"], ["reference"], measurement_kind="strict",
        measure_source=strict)
    assert result["measurement_kind"] == "strict"


def test_real_similarity_report_is_json_serializable_when_api_is_present(monkeypatch, tmp_path):
    try:
        from match_recovery import similarity
    except ImportError:
        pytest.skip("similarity worker is not yet integrated into this package checkout")
    root, image, _measured, _calls, measure = _install_fakes(
        monkeypatch, tmp_path, similarity_api=similarity)

    result = recovery_examples.recommend_examples(
        root, image, ["candidate"], ["reference"], measure_source=measure)

    encoded = json.dumps(result)
    assert encoded
    assert result["rankings"][0]["matches"][0]["reference_group"] == "class:Sample"

"""Candidate callees remain inspectable when original layout cannot yet match."""

from dataclasses import replace
import hashlib

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from test_binary import coff_fixture, pe_fixture
from test_multispan import fixture as multispan_fixture


@pytest.mark.parametrize("target", [None, 0x402000])
def test_size_failure_still_exposes_emitted_callee_without_claiming_reference_agreement(target):
    raw = pe_fixture(b"\x90" * 5 + b"\xc3")
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    binding = FunctionBinding(origin, (Span(0x401000, 0x401006),), "_f", "code/f.cc")
    obj = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3" + b"\x90" * 16, relocation=(1, 1, 20)))
    row, = check_objects((binding,), {binding.unit: obj}, PeImage(raw, origin.image_sha256),
                         external_targets={} if target is None else {"_g": target})
    assert row["result"] == row["boundary"] == "fail"
    assert row["references"] == "unknown"
    entry, = row["diagnostic"]["emitted_references"]
    assert entry == {"section": 1, "section_name": ".text", "offset": 1,
                     "coff_type": 20, "symbol": "_g", "storage_class": 2,
                     "binding_address": None if target is None else "0x00402000"}


def test_missing_original_companion_extent_does_not_hide_emitted_edges():
    binding, objects, image = multispan_fixture()
    binding = replace(binding, spans=binding.spans[:1])
    row, = check_objects((binding,), objects, image)
    assert row["result"] == "unknown"
    entry, = row["diagnostic"]["emitted_references"]
    assert entry["symbol"] == "$tail2" and entry["binding_address"] is None


@pytest.mark.parametrize("offset,expected", [(5, "fail"), (14, "unknown")])
def test_late_candidate_reference_distinguishes_wrong_extent_from_malformed_object(offset, expected):
    raw = pe_fixture(b"\xc3")
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    binding = FunctionBinding(origin, (Span(0x401000, 0x401001),), "_f", "code/f.cc")
    obj = Coff(coff_fixture(b"\x90" * 4 + b"\xe8\0\0\0\0\xc3", relocation=(offset, 1, 20)))
    row, = check_objects((binding,), {binding.unit: obj}, PeImage(raw, origin.image_sha256))
    assert row["result"] == expected
    entry, = row["diagnostic"]["emitted_references"]
    assert entry["offset"] == offset and entry["symbol"] == "_g"
    if expected == "fail":
        assert row["boundary"] == "fail" and row["references"] == "unknown"
        assert row["original_size"] == 1 and row["candidate_size"] == 16
        assert row["candidate_relocations_outside_original"] == [offset]
    else:
        assert "out-of-bound" in row["reason"]


def test_diagnostic_cap_does_not_skip_reference_damage(monkeypatch):
    monkeypatch.setattr("match_recovery.bound_objects.MAX_REFERENCE_DIAGNOSTICS", 0)
    binding, objects, image = multispan_fixture()
    binding = replace(binding, spans=(binding.spans[0], Span(0x401030, 0x401031)))
    row, = check_objects((binding,), objects, image)
    assert row["diagnostic"]["emitted_references"] == []
    assert row["diagnostic"]["omitted_references"] == 1
    assert row["result"] == "fail" and row["spans"][0]["references"] == "fail"


def test_weak_symbol_diagnostic_does_not_borrow_an_external_binding():
    binding, objects, image = multispan_fixture()
    obj = objects[binding.unit]
    obj.symbols[5] = replace(obj.symbols[5], name="_weak", section=0, storage=105)
    row, = check_objects((binding,), objects, image, external_targets={"_weak": 0x401020})
    entry, = row["diagnostic"]["emitted_references"]
    assert entry["symbol"] == "_weak" and entry["binding_address"] is None
    assert row["result"] == "unknown"

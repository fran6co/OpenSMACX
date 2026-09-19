"""Relocation equality cannot erase independent target-storage bounds."""

import pytest

from match_recovery.binary import BoundedTarget, Relocation, RelocationKind, compare_bound_code
from match_recovery.model import Span


@pytest.mark.parametrize("kind", list(RelocationKind))
@pytest.mark.parametrize("addend,expected", [(-1, "fail"), (0, "pass"), (1, "pass"), (3, "pass"), (4, "fail")])
def test_address_equality_keeps_pointer_bounds_for_absolute_and_relative_fields(kind, addend, expected):
    span = Span(0x401000, 0x401004)
    target = BoundedTarget(Span(0x402000, 0x402003))
    value = target.span.start + addend - (span.end if kind == RelocationKind.RELATIVE32 else 0)
    original = (value & 0xFFFFFFFF).to_bytes(4, "little")
    candidate = (addend & 0xFFFFFFFF).to_bytes(4, "little")
    report = compare_bound_code(span, original, candidate, (Relocation(0, kind, "literal"),), {"literal": target},
        original_absolute_sites=frozenset({0}) if kind == RelocationKind.ABSOLUTE32 else frozenset())
    assert report["bytes"] == "pass"
    assert report["references"] == report["result"] == expected
    assert report["relocations"][0]["binding"] == {"kind": "bounded_address", "start": 0x402000, "end": 0x402003}
    if expected == "fail":
        assert "escapes" in report["relocations"][0]["reason"]


def test_bounded_address_still_needs_original_loader_relocation():
    report = compare_bound_code(Span(0x401000, 0x401004), b"\0\x20\x40\0", bytes(4),
        (Relocation(0, RelocationKind.ABSOLUTE32, "literal"),), {"literal": BoundedTarget(Span(0x402000, 0x402003))})
    assert report["bytes"] == "pass" and report["references"] == "fail"
    assert report["extra_absolute_relocations"] == [0]


@pytest.mark.parametrize("span", [None, (1, 2), Span(0xFFFFFFFF, 0x100000000)])
def test_invalid_bounds_cannot_be_supplied_as_i386_targets(span):
    with pytest.raises(ValueError):
        BoundedTarget(span)


def test_consumption_revalidates_a_mutated_bounded_target():
    target = BoundedTarget(Span(1, 3))
    object.__setattr__(target.span, "end", 0)
    with pytest.raises(ValueError):
        compare_bound_code(Span(0x401000, 0x401004), bytes(4), bytes(4), (), {"literal": target})

"""Literal correspondence stays source-owned and translation-unit qualified."""

from dataclasses import asdict, replace
import json

import pytest

from match_recovery.literal_source import LiteralBinding, parse_literals, read_literals, validate_literals
from match_recovery.model import Origin, Span, digest


PIN = "a" * 64
ANNOTATION = "// RECOVERY_LITERAL: v1 0x1000 0x1003 local_literal 0x2000"


def binding(**changes):
    item, = parse_literals(ANNOTATION, "code/a.cc", PIN, address_space="custom:offset")
    return replace(item, **changes)


def test_parser_keeps_exact_source_bounds_evidence_and_location():
    item, = parse_literals("unrelated\n  " + ANNOTATION + " \n", "code/a.cc", PIN,
                           address_space="custom:offset")
    assert item == LiteralBinding(Origin(PIN, "custom:offset", 0x1000), Span(0x1000, 0x1003),
                                 "local_literal", "code/a.cc", 2, Origin(PIN, "custom:offset", 0x2000))
    assert item.interpretation == {"kind": "tu_private_literal", "origin": asdict(item.origin),
        "span": asdict(item.span), "symbol": "local_literal", "unit": "code/a.cc",
        "original_evidence": asdict(item.evidence)}
    assert item.interpretation_id == digest({"entity_id": item.evidence.entity_id, "value": item.interpretation})


@pytest.mark.parametrize("symbol", ["plain_symbol", "$local", "??_C@_02JKAF@rb?$AA@"])
def test_symbol_grammar_does_not_assume_a_compiler(symbol):
    item, = parse_literals(ANNOTATION.replace("local_literal", symbol), "code/a.cc", PIN,
                           address_space="custom:offset")
    assert item.symbol == symbol


@pytest.mark.parametrize("annotation", [
    "v2 0x1000 0x1003 local_literal 0x2000", "v1 0x1000 local_literal 0x2000",
    "v1 0x1000 0x1000 local_literal 0x2000", "v1 0x1003 0x1000 local_literal 0x2000",
    "v1 4096 4099 local_literal 0x2000", "v1 0x1000 0x1003 local_literal -1",
    "v1 0x1000 0x1003 two symbols 0x2000", "v1 0x1000 0x1003 local_literal 0x2000 extra",
])
def test_malformed_or_unbounded_annotations_refuse(annotation):
    with pytest.raises(ValueError):
        parse_literals("// RECOVERY_LITERAL: " + annotation, "code/a.cc", PIN, address_space="custom:offset")


@pytest.mark.parametrize("changes", [
    {"unit": "../a.cc"}, {"unit": "/a.cc"}, {"unit": "code/./a.cc"},
    {"symbol": ""}, {"symbol": "two symbols"}, {"line": 0}, {"line": True},
    {"span": Span(0x1001, 0x1003)}, {"span": None},
    {"evidence": Origin("b" * 64, "custom:offset", 0x2000)},
    {"evidence": Origin(PIN, "other:space", 0x2000)},
])
def test_direct_api_cannot_bypass_source_identity_constraints(changes):
    with pytest.raises(ValueError):
        binding(**changes)


def test_rehoming_or_reinterpreting_keeps_a_distinct_history_identity():
    item = binding()
    assert replace(item, line=30).interpretation_id == item.interpretation_id
    for changed in (replace(item, unit="code/b.cc"), replace(item, symbol="other"),
                    replace(item, span=Span(0x1000, 0x1004)),
                    replace(item, evidence=Origin(PIN, "custom:offset", 0x2001)),
                    replace(item, origin=Origin(PIN, "custom:offset", 0x1003), span=Span(0x1003, 0x1006))):
        assert changed.interpretation_id != item.interpretation_id


def test_identical_names_in_different_units_and_adjacent_extents_are_distinct():
    first = binding()
    second = replace(first, unit="code/b.cc", origin=Origin(PIN, "custom:offset", 0x1003),
                     span=Span(0x1003, 0x1006))
    assert validate_literals([second, first]) == (second, first)
    assert validate_literals(()) == ()


@pytest.mark.parametrize("changes,reason", [
    ({"origin": Origin(PIN, "custom:offset", 0x1003), "span": Span(0x1003, 0x1006)}, "duplicate TU-private"),
    ({"unit": "code/b.cc"}, "same original start"),
    ({"unit": "code/b.cc", "origin": Origin(PIN, "custom:offset", 0x1002), "span": Span(0x1002, 0x1005)}, "overlap"),
    ({"origin": Origin("b" * 64, "custom:offset", 0x1000),
      "evidence": Origin("b" * 64, "custom:offset", 0x2000)}, "different originals"),
])
def test_ambiguous_binding_populations_refuse(changes, reason):
    first = binding()
    with pytest.raises(ValueError, match=reason):
        validate_literals((first, replace(first, **changes)))


def project(tmp_path):
    (tmp_path / "code/seeds").mkdir(parents=True)
    (tmp_path / "recovery").mkdir()
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "custom:offset",
              "roots": ["code"], "seed_roots": ["code/seeds"], "include_paths": ["code"]}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    (tmp_path / "code/a.cc").write_text(ANNOTATION)
    return tmp_path


def test_reader_honors_source_configuration_and_excludes_seed_obligations(tmp_path):
    root = project(tmp_path)
    (root / "code/seeds/old.cc").write_text("// RECOVERY_LITERAL: malformed historical seed")
    (root / "code/ignored.txt").write_text(ANNOTATION)
    (root / "outside.cc").write_text(ANNOTATION)
    assert read_literals(root, PIN) == (binding(),)


def test_reader_validates_across_all_product_units(tmp_path):
    root = project(tmp_path)
    other = root / "code/b.cc"
    other.write_text(ANNOTATION.replace("0x1000 0x1003", "0x1003 0x1006"))
    assert [item.unit for item in read_literals(root, PIN)] == ["code/a.cc", "code/b.cc"]
    other.write_text(ANNOTATION)
    with pytest.raises(ValueError, match="same original start"):
        read_literals(root, PIN)


def test_reader_does_not_silently_drop_malformed_product_annotation(tmp_path):
    root = project(tmp_path)
    (root / "code/b.cc").write_text("// RECOVERY_LITERAL: malformed")
    with pytest.raises(ValueError, match="code/b.cc:1"):
        read_literals(root, PIN)


@pytest.mark.parametrize("annotation", [
    "// RECOVERY_OBJECT: v1 0x1001 _g",
    "// RECOVERY_OBJECT: v2 0x0fff 0x1001 _g 0x2000",
    "// RECOVERY_GENERATED: v1 vtable 0x1002 ? ??_7Table@@6B@ 0x2000",
])
def test_literal_cannot_relabel_another_declared_object_or_table(tmp_path, annotation):
    root = project(tmp_path)
    (root / "code/b.cc").write_text(annotation + "\n")
    with pytest.raises(ValueError, match="overlaps another"):
        read_literals(root, PIN)


MERGED = "// RECOVERY_LITERAL: v2 merged 0x1000 0x1003 local_literal 0x2000"


def merged(**changes):
    item, = parse_literals(MERGED, "code/a.cc", PIN, address_space="custom:offset")
    return replace(item, **changes)


def test_merged_literal_parses_and_keeps_v1_identities_unchanged():
    item = merged()
    assert item.merged and not binding().merged
    assert item.interpretation == {**binding().interpretation, "linker_merged": True}
    assert item.interpretation_id != binding().interpretation_id
    assert "linker_merged" not in binding().interpretation


@pytest.mark.parametrize("annotation", [
    "v2 merge 0x1000 0x1003 local_literal 0x2000", "v2 merged 0x1000 local_literal 0x2000",
    "v1 merged 0x1000 0x1003 local_literal 0x2000",
    "v2 merged 0x1000 0x1003 local_literal 0x2000 extra",
])
def test_malformed_merged_annotations_refuse(annotation):
    with pytest.raises(ValueError):
        parse_literals("// RECOVERY_LITERAL: " + annotation, "code/a.cc", PIN, address_space="custom:offset")


def test_merged_literals_in_distinct_units_share_one_exact_extent():
    first = merged()
    second = merged(unit="code/b.cc", symbol="other_literal")
    third = merged(unit="code/c.cc")
    assert validate_literals([first, second, third]) == (first, second, third)


@pytest.mark.parametrize("changes,reason", [
    ({"unit": "code/b.cc", "merged": False}, "same original start"),  # one claimant is exclusive
    ({"unit": "code/b.cc", "span": Span(0x1000, 0x1004)}, "same original start"),  # extents differ
    ({"symbol": "other_literal"}, "same original start"),  # one unit, two names
    ({"unit": "code/b.cc", "origin": Origin(PIN, "custom:offset", 0x1002),
      "span": Span(0x1002, 0x1005)}, "overlap"),  # merged never admits partial overlap
])
def test_merged_literal_populations_stay_unambiguous(changes, reason):
    first = merged()
    with pytest.raises(ValueError, match=reason):
        validate_literals((first, replace(first, **changes)))


def test_exclusive_literal_still_refuses_a_merged_peer_at_its_start():
    with pytest.raises(ValueError, match="same original start"):
        validate_literals((binding(), merged(unit="code/b.cc")))


@pytest.mark.parametrize("annotation", [
    "// RECOVERY_OBJECT: v1 0x1000 _g",
    "// RECOVERY_OBJECT: v2 0x1000 0x1003 _g 0x2000",
])
def test_merged_literal_may_share_a_folded_constant_with_another_units_object(tmp_path, annotation):
    root = project(tmp_path)
    (root / "code/a.cc").write_text(MERGED)
    (root / "code/b.cc").write_text(annotation + "\n")
    assert read_literals(root, PIN) == (merged(),)


@pytest.mark.parametrize("unit,annotation", [
    ("code/b.cc", "// RECOVERY_OBJECT: v1 0x1001 _g"),  # interior start
    ("code/b.cc", "// RECOVERY_OBJECT: v2 0x1000 0x1004 _g 0x2000"),  # different extent
    ("code/b.cc", "// RECOVERY_OBJECT: v2 0x0fff 0x1001 _g 0x2000"),  # partial overlap
    ("code/b.cc", "// RECOVERY_GENERATED: v1 vtable 0x1000 ? ??_7Table@@6B@ 0x2000"),  # table
    ("code/a.cc", "// RECOVERY_OBJECT: v1 0x1000 _g"),  # same unit
])
def test_merged_literal_never_relabels_any_other_object_or_table(tmp_path, unit, annotation):
    root = project(tmp_path)
    (root / "code/a.cc").write_text(MERGED + "\n" + (annotation + "\n" if unit == "code/a.cc" else ""))
    if unit != "code/a.cc":
        (root / unit).write_text(annotation + "\n")
    with pytest.raises(ValueError, match="overlaps another"):
        read_literals(root, PIN)


def test_exclusive_literal_still_refuses_a_same_start_object(tmp_path):
    root = project(tmp_path)
    (root / "code/b.cc").write_text("// RECOVERY_OBJECT: v1 0x1000 _g\n")
    with pytest.raises(ValueError, match="overlaps another"):
        read_literals(root, PIN)


def test_only_same_address_merged_aliases_release_an_object_name():
    from match_recovery.literal_source import object_reserved_literal_symbols
    from match_recovery.object_source import parse_objects
    same, = parse_objects("// RECOVERY_OBJECT: v1 0x1000 local_literal", "code/b.cc", PIN,
                          address_space="custom:offset")
    moved, = parse_objects("// RECOVERY_OBJECT: v1 0x1003 local_literal", "code/b.cc", PIN,
                           address_space="custom:offset")
    assert object_reserved_literal_symbols([merged()], [same]) == frozenset()
    assert object_reserved_literal_symbols([binding()], [same]) == {"local_literal"}
    assert object_reserved_literal_symbols([merged()], [moved]) == {"local_literal"}
    assert object_reserved_literal_symbols([merged()], [same, moved]) == {"local_literal"}
    assert object_reserved_literal_symbols([merged()], []) == {"local_literal"}

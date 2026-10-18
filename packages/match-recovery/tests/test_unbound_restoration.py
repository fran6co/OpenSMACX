"""UNBOUND is pending identity, with a manual round trip back to a body.

The fixture uses a real Git history and the native source parser/writer and
guarded publisher. Claim coverage is checked separately from span identity;
check_claims only protects historical origin/dimensions, not image bounds.
The temporary source publication models a coordinator-reviewed migration,
not permission for an ordinary worker to change legacy_mode.
"""

from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.application_binary import select_bindings
from match_recovery.claim_integrity import check_claims
from match_recovery.ownership_source import project_view
from match_recovery.source import Mode, Recipe, State, read_file, remove, write
from match_recovery.source.publication import publish
from match_recovery.worker_bindings import capture, validate
from test_claim_integrity import commit, project as claim_history_project


BODY_PATH = "engine/first.cc"
PENDING_PATH = "engine/pending.hpp"
RESTORED_PATH = "engine/restored.cc"
BODY_TEXT = "int _first() { return 1; }\n"


def _paths(root: Path):
    return (root / BODY_PATH, root / PENDING_PATH, root / RESTORED_PATH)


def _publish(root: Path, expected: dict[Path, bytes], replacements: dict[Path, bytes]):
    return publish(expected, replacements, validate=lambda: None,
                   staging_directory=root / "build/recovery/publications")


def _add_lessons(path: Path):
    text = path.read_text(encoding="utf-8").replace("int first() {return 1;}", BODY_TEXT)
    row, = read_file(path)
    updated = replace(row, levers=(("fixture-fingerprint", "retained source rationale"),),
                      ruled_out=("previous spelling was rejected",),
                      deferred=("body ownership awaits reviewed migration",))
    path.write_text(write(text, [updated]), encoding="utf-8")
    return read_file(path)[0]


def _unbound_record(row, pending: Path):
    return replace(row, path=pending, line=1, mode=Mode.UNBOUND, state=State.UNBOUND,
                   recipe=Recipe.UNBOUND, region="", extract_error="", symbol="",
                   body="", context="")


def _bound_record(row, restored: Path):
    return replace(row, path=restored, line=1, mode=Mode.BODY, state=State.IMPLEMENTED,
                   recipe=Recipe.CENSUS, region=BODY_TEXT, extract_error="", symbol="",
                   body="", context="")


def _prepare_unbound(root: Path):
    body, pending, restored = _paths(root)
    before = read_file(body)[0]
    semantic = replace(before, semantic=True)
    body.write_text(write(body.read_text(encoding="utf-8"), [semantic]), encoding="utf-8")
    commit(root, "Add a second historical shape obligation for the fixture identity")
    pending.write_bytes(b"")
    restored.write_bytes(b"")
    original_row = _add_lessons(body)
    original_body = body.read_bytes()
    unbound = _unbound_record(original_row, pending)
    without_old_marker = remove(body.read_text(encoding="utf-8"), [original_row])
    pending_text = write("", [unbound])
    replacements = {
        body: without_old_marker.encode("utf-8"),
        pending: pending_text.encode("utf-8"),
    }
    expected = {body: body.read_bytes(), pending: b""}
    return body, pending, restored, original_body, original_row, unbound, expected, replacements


def test_manual_unbound_round_trip_preserves_claims_then_selects_real_body(
    claim_history_project,
):
    root = claim_history_project
    body, pending, restored, original_body, original_row, unbound, expected, replacements = (
        _prepare_unbound(root))
    assert check_claims(root)["result"] == "pass"
    baseline = check_claims(root)["historical_requirements"]
    assert baseline == 2  # historical byte and independently claimed shape dimensions
    assert original_row.byte_exact and original_row.image_spans

    # Worker source continuity still protects legacy_mode. Check the refusal
    # against the proposed bytes, then restore the fixture before explicit
    # guarded publication by the test's coordinator-side path.
    before = capture(root)
    for path, content in replacements.items():
        path.write_bytes(content)
    with pytest.raises(ValueError, match="removed or changed an original source binding"):
        validate(root, before, frozenset({BODY_PATH, PENDING_PATH, RESTORED_PATH}))
    for path, raw in expected.items():
        path.write_bytes(raw)
    assert body.read_bytes() == original_body
    assert pending.read_bytes() == b""

    changed = _publish(root, expected, replacements)
    assert set(changed) == {body, pending}
    parsed, = read_file(pending)
    assert (parsed.mode, parsed.state, parsed.recipe) == (Mode.UNBOUND, State.UNBOUND, Recipe.UNBOUND)
    assert parsed.region == parsed.symbol == parsed.body == parsed.context == ""
    assert parsed.address == original_row.address and parsed.name == original_row.name
    assert parsed.image_spans == original_row.image_spans
    assert parsed.byte_exact and parsed.semantic == original_row.semantic
    assert parsed.levers == original_row.levers
    assert parsed.ruled_out == original_row.ruled_out
    assert parsed.deferred == original_row.deferred
    assert check_claims(root)["result"] == "pass"
    assert check_claims(root)["historical_requirements"] == baseline

    # The old, now-unannotated C++ definition is incidental and cannot bind
    # the UNBOUND identity. Selection remains explicitly refused.
    view, _ = project_view(root)
    entity, = [item for group in view["groups"] for item in group["entities"]
               if item["origin"]["address"] == original_row.address]
    bindings, refused, _ = select_bindings(view, frozenset({entity["entity_id"]}))
    assert entity["candidates"][0]["unbound"] and entity["candidates"][0]["symbol"] == ""
    assert not bindings and len(refused) == 1

    # A later explicit owner reuses the same identity/claim/lessons. The
    # source-level selector now sees one real product mapping. This is not an
    # object/PE measurement or a claim that the historical span is matched.
    parsed_unbound, = read_file(pending)
    no_pending = remove(pending.read_text(encoding="utf-8"), [parsed_unbound])
    restored_text = write("", [_bound_record(parsed_unbound, restored)]) + "\n" + BODY_TEXT
    replacements = {
        body: b"", pending: no_pending.encode("utf-8"),
        restored: restored_text.encode("utf-8"),
    }
    expected = {body: body.read_bytes(), pending: pending.read_bytes(), restored: b""}
    before_unbound = capture(root)
    for path, content in replacements.items():
        path.write_bytes(content)
    with pytest.raises(ValueError, match="removed or changed an original source binding"):
        validate(root, before_unbound, frozenset({BODY_PATH, PENDING_PATH, RESTORED_PATH}))
    for path, raw in expected.items():
        path.write_bytes(raw)
    changed = _publish(root, expected, replacements)
    assert set(changed) == {body, pending, restored}
    restored_row, = read_file(restored)
    assert restored_row.mode is Mode.BODY and restored_row.state is State.IMPLEMENTED
    assert BODY_TEXT.strip() in restored_row.region
    assert restored_row.address == original_row.address
    assert restored_row.name == original_row.name
    assert restored_row.image_spans == original_row.image_spans
    assert restored_row.byte_exact and restored_row.semantic == original_row.semantic
    assert restored_row.levers == original_row.levers
    assert restored_row.ruled_out == original_row.ruled_out
    assert restored_row.deferred == original_row.deferred
    assert check_claims(root)["result"] == "pass"
    assert check_claims(root)["historical_requirements"] == baseline
    final_view, _ = project_view(root)
    final_entity, = [item for group in final_view["groups"] for item in group["entities"]
                     if item["origin"]["address"] == original_row.address]
    bindings, refused, _ = select_bindings(final_view, frozenset({final_entity["entity_id"]}))
    assert len(bindings) == 1 and not refused


@pytest.mark.parametrize("damage", ["remove-marker", "remove-claim"])
def test_claim_history_rejects_missing_unbound_marker_or_claim(claim_history_project, damage):
    root = claim_history_project
    body, pending, _restored, _raw, original_row, unbound, expected, replacements = _prepare_unbound(root)
    del unbound
    changed = _publish(root, expected, replacements)
    assert set(changed) == {body, pending}
    record, = read_file(pending)
    if damage == "remove-marker":
        damaged = remove(pending.read_text(encoding="utf-8"), [record])
    else:
        damaged = write(pending.read_text(encoding="utf-8"), [replace(record, byte_exact=False)])
    publish({pending: pending.read_bytes()}, {pending: damaged.encode("utf-8")},
            validate=lambda: None, staging_directory=root / "build/recovery/damage-publications")
    assert check_claims(root)["result"] == "fail"
    missing = check_claims(root)["missing"]
    expected_dimensions = ({"bytes", "instruction_shape"} if damage == "remove-marker"
                          else {"bytes"})
    assert {row["dimension"] for row in missing} == expected_dimensions
    assert {row["address"] for row in missing} == {f"0x{original_row.address:08X}"}


def test_claim_coverage_is_not_an_image_span_integrity_check(claim_history_project):
    root = claim_history_project
    body, pending, _restored, _raw, original_row, unbound, expected, replacements = _prepare_unbound(root)
    _publish(root, expected, replacements)
    record, = read_file(pending)
    changed_span = replace(record, image_spans=((record.address, record.image_spans[0][1] + 1),))
    damaged = write(pending.read_text(encoding="utf-8"), [changed_span])
    publish({pending: pending.read_bytes()}, {pending: damaged.encode("utf-8")},
            validate=lambda: None, staging_directory=root / "build/span-publications")
    reread, = read_file(pending)
    assert reread.address == original_row.address and reread.name == original_row.name
    assert reread.image_spans != original_row.image_spans
    # check_claims' historical rows contain origin/dimension, not span extent;
    # the separate typed identity assertion above is therefore essential.
    assert check_claims(root)["result"] == "pass"

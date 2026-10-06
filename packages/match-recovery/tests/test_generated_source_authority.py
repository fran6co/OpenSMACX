"""Source identities constrain intent; only live native evidence proves emission."""

from copy import deepcopy
from types import SimpleNamespace

import pytest

from match_recovery.generated_permissions import generated_sources
from match_recovery.model import Origin
from match_recovery.source_config import SourceConfiguration


PIN = "a" * 64
SPACE = "test:va"
ENTITY = Origin(PIN, SPACE, 0x10).entity_id
OTHER = Origin(PIN, SPACE, 0x20).entity_id
TASK = SimpleNamespace(author="coordinator")
SOURCE = SourceConfiguration("original/pin", SPACE, roots=("code", "hints"),
                             seed_roots=("hints",), include_paths=("code",))
SETTINGS = SimpleNamespace(source=SOURCE, unit_extensions=(".cc",))
FUNCTION = b"// ORIGINAL: 0x00000010 _owner 0x00000010-0x00000018\nvoid owner() {}\n"
OBJECT = b"// RECOVERY_OBJECT: v1 0x00000010 _object\nint object;\n"


def permission(**changes):
    row = {"entity_id": ENTITY, "unit": "code/owner.cc", "version": "current",
           "symbol": "_$E21", "offset": 0, "evidence": ["Scoped initializer intent"], **changes}
    return {"migration_id": "generated-lifecycle", "author": TASK.author,
            "evidence": ["Recover source-owned lifecycle"], "allowed": [], "generated_changes": [row]}


def observe(root, value=None, versions=None, *, scope=frozenset({ENTITY}),
            editable=frozenset({"code/owner.cc"}), settings=SETTINGS, pin=PIN):
    if versions is None:
        versions = {"baseline": {}, "current": {"code/owner.cc": FUNCTION}}
    return generated_sources(value or permission(), TASK, scope, editable, versions, settings, pin, root)


@pytest.mark.parametrize("raw", [FUNCTION, OBJECT,
    b"// RECOVERY_OBJECT: v2 0x00000010 0x00000014 _object 0x00000020\nint object;\n"])
@pytest.mark.parametrize("version", ["baseline", "current"])
def test_exact_function_or_object_anchor_is_supported_without_claiming_emission(tmp_path, raw, version):
    value = permission(version=version)
    versions = {"baseline": {}, "current": {}}
    versions[version]["code/owner.cc"] = raw
    assert observe(tmp_path, value, versions) == value["generated_changes"]
    assert list(tmp_path.iterdir()) == []  # Source versions, never ambient files.


def test_decision_result_is_defensive_and_inputs_unchanged(tmp_path):
    value = permission()
    before = deepcopy(value)
    rows = observe(tmp_path, value)
    rows[0]["evidence"].append("mutated output")
    rows[0]["unit"] = "elsewhere.cc"
    assert value == before


def test_absent_generated_intent_has_no_authority(tmp_path):
    value = permission()
    del value["generated_changes"]
    assert observe(tmp_path, value, {}) == []


@pytest.mark.parametrize("versions", [
    {"baseline": {"code/owner.cc": FUNCTION}, "current": {}},
    {"baseline": {}, "current": {"code/other.cc": FUNCTION}},
    {"baseline": {}, "current": {"code/owner.cc": b"void owner() {}\n"}},
    {"baseline": {}, "current": {"code/owner.cc": FUNCTION.replace(b"0x00000010", b"0x00000020")}},
])
def test_missing_wrong_version_unit_or_identity_refuses(tmp_path, versions):
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, versions=versions)


@pytest.mark.parametrize("editable", [frozenset(), frozenset({"code/other.cc"})])
def test_original_anchor_does_not_grant_editability(tmp_path, editable):
    with pytest.raises(ValueError, match="editable product translation unit"):
        observe(tmp_path, editable=editable)


def test_original_anchor_does_not_grant_subject_scope(tmp_path):
    with pytest.raises(ValueError, match="scoped original entity"):
        observe(tmp_path, scope=frozenset({OTHER}))


@pytest.mark.parametrize("unit", ["hints/owner.cc", "outside/owner.cc", "code/owner.hpp", "code/owner.txt"])
def test_seed_outside_header_and_non_tu_are_not_authorized_owners(tmp_path, unit):
    with pytest.raises(ValueError, match="editable product translation unit"):
        observe(tmp_path, permission(unit=unit), {"baseline": {}, "current": {unit: FUNCTION}},
                editable=frozenset({unit}))


@pytest.mark.parametrize("duplicate", ["code/other.cc", "code/owner.hpp"])
def test_duplicate_original_in_other_product_source_refuses(tmp_path, duplicate):
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, versions={"baseline": {}, "current": {
            "code/owner.cc": FUNCTION, duplicate: FUNCTION}})


@pytest.mark.parametrize("raw, diagnostic", [
    (FUNCTION + FUNCTION, "unique original source anchor"),
    (OBJECT + OBJECT, "duplicate original object symbol binding"),
    (FUNCTION + OBJECT, "unique original source anchor"),
])
def test_duplicate_anchors_within_tu_or_across_kinds_refuse(tmp_path, raw, diagnostic):
    with pytest.raises(ValueError, match=diagnostic):
        observe(tmp_path, versions={"baseline": {}, "current": {"code/owner.cc": raw}})


@pytest.mark.parametrize("ignored", ["hints/owner.cc", "outside/owner.cc", "code/notes.txt"])
def test_seed_external_and_non_source_duplicates_are_not_product_owners(tmp_path, ignored):
    assert observe(tmp_path, versions={"baseline": {}, "current": {
        "code/owner.cc": FUNCTION, ignored: FUNCTION}})


def test_current_uniqueness_does_not_borrow_baseline_uniqueness(tmp_path):
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, versions={"baseline": {"code/owner.cc": FUNCTION}, "current": {
            "code/owner.cc": FUNCTION, "code/duplicate.cc": FUNCTION}})


def test_decisions_for_both_versions_validate_both_source_trees(tmp_path):
    value = permission()
    value["generated_changes"].append({**value["generated_changes"][0], "version": "baseline"})
    versions = {"baseline": {"code/owner.cc": FUNCTION}, "current": {"code/owner.cc": OBJECT}}
    assert observe(tmp_path, value, versions) == value["generated_changes"]
    versions["baseline"] = {}
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, value, versions)


def test_original_pin_cannot_be_substituted(tmp_path):
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, pin="b" * 64)


def test_original_address_space_cannot_be_substituted(tmp_path):
    source = SourceConfiguration("original/pin", "other:va", roots=("code",), include_paths=("code",))
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, settings=SimpleNamespace(source=source, unit_extensions=(".cc",)))


@pytest.mark.parametrize("raw", [b"\xff", b"// RECOVERY_OBJECT: v1 broken\n"])
def test_unreadable_or_malformed_object_source_refuses(tmp_path, raw):
    with pytest.raises(ValueError):
        observe(tmp_path, versions={"baseline": {}, "current": {"code/owner.cc": raw}})


def test_missing_requested_source_version_refuses_as_missing_evidence(tmp_path):
    with pytest.raises(ValueError, match="source version"):
        observe(tmp_path, versions={"baseline": {}})


@pytest.mark.parametrize("versions", [[], "source", {"current": None}, {"current": []}])
def test_invalid_requested_source_version_refuses(tmp_path, versions):
    with pytest.raises(ValueError, match="source version"):
        observe(tmp_path, versions=versions)


@pytest.mark.parametrize("raw", [
    b"// RECOVERY_SHARED: v1 0x00000010 0x00000018 _shared 0x00000010\nvoid shared() {}\n",
    b"// RECOVERY_GENERATED: v1 vtable 0x00000020 0x00000028 _vtable 0x00000010\n",
    b"// RECOVERY_OBJECT: v2 0x00000020 0x00000024 _object 0x00000010\nint object;\n",
])
def test_shared_generated_or_borrowed_object_evidence_is_not_original_owner(tmp_path, raw):
    with pytest.raises(ValueError, match="unique original source anchor"):
        observe(tmp_path, versions={"baseline": {}, "current": {"code/owner.cc": raw}})

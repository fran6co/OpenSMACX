"""Physical native residual controls; no compiler, scope grant or acceptance."""

import struct

import pytest

from match_recovery.generated_code_residual import _compare_contexts
from match_recovery.linked_initializers import observe_build_initializers
from test_linked_initializers import update_artifact
from test_projected_initializer_order import selection, repeated_root
from test_slot_local_initializer_order import physical_context, replace_object, make_pair


CALL = b"\xe8\0\0\0\0\xc3"


def pair(monkeypatch, tmp_path, *, removed=True, damage=None, source_files=None):
    rich = physical_context(monkeypatch, tmp_path / "rich", codes=(b"\xc3", CALL),
                            references=(None, "_dep"), source_files=source_files)
    lean = physical_context(monkeypatch, tmp_path / "lean", codes=(b"\xc3",),
                            damage=damage)
    return (rich, lean) if removed else (lean, rich)


def assert_complete(*contexts):
    for context in contexts:
        observed = observe_build_initializers(context)
        assert observed["result"] == observed["code_dependencies"]["result"] == "pass", observed


def helper_damage(context, *, linked=True, source=True):
    if source:
        raw = bytearray(context.objects[-1].read_bytes())
        pointer, = struct.unpack_from("<I", raw, 40)
        assert raw[pointer] == 0xC3
        raw[pointer] = 0x90
        replace_object(context, raw, len(context.objects) - 1)
    if linked:
        image = bytearray((context.directory / "app.exe").read_bytes())
        assert image[512 + 0x1E0] == 0xC3
        image[512 + 0x1E0] = 0x90
        update_artifact(context, "app.exe", image)


@pytest.mark.parametrize("removed", [True, False])
def test_removed_or_added_initializer_keeps_its_otherwise_unreachable_helper(monkeypatch, tmp_path, removed):
    contexts = pair(monkeypatch, tmp_path, removed=removed)
    assert_complete(*contexts)
    version = "baseline" if removed else "current"
    report = _compare_contexts(*contexts, [selection(version=version)])
    assert report["result"] == "pass", report
    assert report["projected_order"]["result"] == "pass"
    assert report["preserved_producers"] == 2
    assert report["omitted_bodies"] == {"baseline": int(removed), "current": int(not removed)}
    for residual in report["residuals"].values():
        assert {row["identity"]["unit"] for row in residual["canonical"]["nodes"]} == {
            "src/unit0.cpp", "src/dependency.cpp"}
    assert report["remaining_linked_effects"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("removed", [True, False])
def test_coherent_source_and_linked_helper_damage_cannot_hide_outside_removed_root(monkeypatch, tmp_path, removed):
    contexts = pair(monkeypatch, tmp_path, removed=removed, damage=helper_damage)
    # Initially this helper is unreachable in the damaged lean image. Even
    # coherent native inputs must be compared after the union re-rooting.
    assert_complete(*contexts)
    report = _compare_contexts(*contexts, [selection(version="baseline" if removed else "current")])
    assert report["projected_order"]["result"] == "pass", report
    assert report["result"] == "fail", report
    assert report["code_residual"]["result"] == "fail"


@pytest.mark.parametrize("source", [False, True])
def test_reexpanded_helper_must_still_match_its_actual_linked_bytes(monkeypatch, tmp_path, source):
    contexts = pair(monkeypatch, tmp_path,
                    damage=lambda context: helper_damage(context, linked=not source, source=source))
    assert_complete(*contexts)
    report = _compare_contexts(*contexts, [selection(version="baseline")])
    assert report["result"] == "fail", report
    assert report["reason"] == "expanded dependency code is unverified"


def test_missing_current_helper_placement_is_not_a_discard_proof(monkeypatch, tmp_path):
    def absent(context):
        path = context.directory / "app.map"
        raw = "".join(line for line in path.read_text().splitlines(keepends=True) if " _dep " not in line)
        update_artifact(context, "app.map", raw.encode())

    contexts = pair(monkeypatch, tmp_path, damage=absent)
    assert_complete(*contexts)
    report = _compare_contexts(*contexts, [selection(version="baseline")])
    assert report["result"] == "unknown", report
    assert "placement" in report["reason"] or "producer" in report["reason"]
    assert not report.get("code_residual")


@pytest.mark.parametrize("reference", [None, "_dep"])
def test_native_slot_local_rename_preserves_residual_identity(monkeypatch, tmp_path, reference):
    contexts = make_pair(monkeypatch, tmp_path, codes=(CALL if reference else b"\xc3",),
                         references=(reference,))
    assert_complete(*contexts)
    report = _compare_contexts(*contexts, [])
    assert report["result"] == "pass", report
    assert report["projected_order"]["root_renamings"]


@pytest.mark.parametrize("annotation", [
    b"// ORIGINAL: 0x00401020 _$E1 0x00401020-0x00401030\nvoid original() {}\n",
    b"// RECOVERY_OBJECT: v1 0x00401020 _$E1\nint original;\n",
    b"// RECOVERY_OBJECT: v1 0x00402008 _$S1\nint slot;\n",
])
def test_source_protected_initializer_cannot_be_subtracted(monkeypatch, tmp_path, annotation):
    contexts = pair(monkeypatch, tmp_path, source_files={"src/unit1.cpp": annotation})
    assert_complete(*contexts)
    report = _compare_contexts(*contexts, [selection(version="baseline")])
    assert report["projected_order"]["result"] == "pass", report
    assert report["result"] == "unknown", report
    assert not report.get("code_residual")


def test_linked_alias_protects_selected_body_even_when_order_projection_passes(monkeypatch, tmp_path):
    def alias(context):
        path = context.directory / "app.map"
        line = " 0001:00000020 _escaped 00401020 f dependency.obj\n"
        update_artifact(context, "app.map", path.read_text().replace(" entry point", line + " entry point").encode())

    old = physical_context(monkeypatch, tmp_path / "old", codes=(b"\xc3", CALL),
                           references=(None, "_dep"), damage=alias)
    new = physical_context(monkeypatch, tmp_path / "new", codes=(b"\xc3",))
    assert_complete(old, new)
    report = _compare_contexts(old, new, [selection(version="baseline")])
    assert report["projected_order"]["result"] == "pass", report
    assert report["result"] == "unknown", report
    assert "alias" in report["reason"]


def test_shared_registration_cannot_be_selected_by_just_one_offset(monkeypatch, tmp_path):
    old = physical_context(monkeypatch, tmp_path / "old", damage=repeated_root)
    new = physical_context(monkeypatch, tmp_path / "new")
    assert_complete(old, new)
    report = _compare_contexts(old, new, [selection(version="baseline", unit="src/unit0.cpp")])
    assert report["result"] == "unknown", report


def test_native_context_expiration_during_comparison_refuses(monkeypatch, tmp_path):
    old, new = pair(monkeypatch, tmp_path)
    target = new.checks + 5

    def expire(checks):
        if checks == target:
            new.alive = False

    new.on_check = expire
    with pytest.raises(ValueError, match="expired"):
        _compare_contexts(old, new, [selection(version="baseline")])


@pytest.mark.parametrize("limits", [{"max_nodes": 1}, {"max_rounds": 0}, {"max_nodes": True}])
def test_closure_bounds_never_convert_missing_work_to_pass(monkeypatch, tmp_path, limits):
    contexts = pair(monkeypatch, tmp_path)
    if limits.get("max_nodes") == 1 and type(limits.get("max_nodes")) is int:
        report = _compare_contexts(*contexts, [selection(version="baseline")], **limits)
        assert report["result"] == "unknown", report
    else:
        with pytest.raises(ValueError, match="bounded"):
            _compare_contexts(*contexts, [selection(version="baseline")], **limits)

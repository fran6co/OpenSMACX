"""Physical section correspondence is not a byte or retention verdict."""

from copy import deepcopy
from hashlib import sha256
import json
from pathlib import Path
import struct

import pytest

from match_recovery import linked_data_ownership as native
from match_recovery import linked_input_epoch as epochs
from match_recovery.linked_context import linked_build
from match_recovery.pe_coff import Coff
from test_common_request_coverage import append_section
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_linked_modules import fixture as module_context, add_archive
from test_linked_storage import data_object
from test_startup_linked_consumption import context as startup_context


def fixture(root, *, raw=None, damage=None):
    root.mkdir(parents=True, exist_ok=True)
    raw = data_object() if raw is None else raw
    def install(context):
        path = context.objects[0]
        path.write_bytes(raw)
        evidence = json.loads((context.directory / "link.json").read_bytes())
        row = next(row for row in evidence["files"] if row["path"] == str(path))
        row["before"] = row["after"] = stamp(raw)
        update_artifact(context, "link.json", json.dumps(evidence).encode())
        if damage is not None:
            damage(context)
    return startup_context(root / "physical", damage=install)[0]


def descriptor():
    return {"kind": "compiled", "unit": "src/unit0.cpp", "section": ".data"}


def test_compiled_data_identity_survives_new_bytes_symbols_flags_and_section_ordinal(tmp_path):
    old = fixture(tmp_path / "old")
    raw = bytearray(data_object((("renamed", 0, 2),), flags=0x40300040))
    struct.pack_into("<I", raw, 4, 123)
    # The formerly sole section becomes .pad; a new .data receives ordinal 2.
    raw[20:28] = b".pad\0\0\0\0"
    changed = append_section(bytes(raw), data=b"different storage")
    new = fixture(tmp_path / "new", raw=changed)
    with linked_build(old) as a, linked_build(new) as b:
        identity = native.identify_section(old, a.inputs.objects, "unit0.obj", 1)
        assert identity == descriptor()
        assert native.identify_section(new, b.inputs.objects, "unit0.obj", 2) == identity
        assert native.resolve_section(new, b.inputs.objects, identity) == {"owner": "unit0.obj", "section": 2}
        assert sha256(old.objects[0].read_bytes()).digest() != sha256(new.objects[0].read_bytes()).digest()


def test_dependency_keeps_portable_full_member_hash_and_ordinal(tmp_path):
    identities = []
    for side in ("old", "new"):
        context = module_context(tmp_path / side)
        path, _ = add_archive(context, [("dir/storage.obj/", data_object())], ["storage.obj"])
        with linked_build(context) as live:
            identity = native.identify_section(context, live.inputs.objects, "RUNTIME:storage.obj", 1)
            assert identity == {"kind": "dependency", "input": {"kind": "build", "path": "runtime.lib"},
                "member": "dir/storage.obj", "occurrence": 0, "object_sha256": sha256(data_object()).hexdigest(),
                "section": ".data", "object_section": 1}
            assert native.resolve_section(context, live.inputs.objects, identity) == {"owner": "RUNTIME:storage.obj", "section": 1}
            assert path.is_file()
            identities.append(identity)
    assert identities[0] == identities[1]


@pytest.mark.parametrize("key,value", [("object_sha256", "a" * 64), ("member", "storage.obj"),
    ("occurrence", 1), ("occurrence", False), ("object_section", 2), ("object_section", True),
    ("section", ".bss")])
def test_dependency_cannot_fall_back_to_same_basename_bytes_or_adjacent_section(tmp_path, key, value):
    context = module_context(tmp_path)
    add_archive(context, [("dir/storage.obj/", data_object())], ["storage.obj"])
    with linked_build(context) as live:
        identity = native.identify_section(context, live.inputs.objects, "RUNTIME:storage.obj", 1)
        with pytest.raises(ValueError):
            native.resolve_section(context, live.inputs.objects, {**identity, key: value})


@pytest.mark.parametrize("raw", [data_object(flags=0xC0301040), data_object(flags=0x60300020),
    data_object(name=b".debug$S"), data_object(name=b".tls"), data_object(name=b".idata"),
    data_object(size=0), append_section(data_object())],
    ids=["comdat", "code", "debug", "tls", "imports", "empty", "duplicate"])
def test_nonordinary_and_duplicate_names_refuse_in_both_directions(tmp_path, raw):
    context = fixture(tmp_path, raw=raw)
    with linked_build(context) as live:
        with pytest.raises(ValueError):
            native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
        with pytest.raises(ValueError):
            native.resolve_section(context, live.inputs.objects, descriptor())


@pytest.mark.parametrize("key,value", [("object_section", None), ("object_section", 1),
    ("symbol", "_private"), ("object_sha256", "a" * 64), ("unit", "src/elsewhere/unit0.cpp"),
    ("section", ".missing"), ("unit", "../src/unit0.cpp")])
def test_compiled_descriptor_schema_is_exact_and_never_uses_basename_aliases(tmp_path, key, value):
    context = fixture(tmp_path)
    with linked_build(context) as live:
        with pytest.raises(ValueError):
            native.resolve_section(context, live.inputs.objects, {**descriptor(), key: value})


@pytest.mark.parametrize("phase", ["identify", "resolve"])
def test_native_source_and_receipt_or_context_damage_refuses(tmp_path, phase):
    context = fixture(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            (context.root / "src/unit0.cpp").write_bytes(b"changed original source")
            if phase == "identify":
                native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
            else:
                native.resolve_section(context, live.inputs.objects, descriptor())


def test_expired_native_context_and_other_build_objects_are_refused(tmp_path):
    context = fixture(tmp_path / "one")
    other = fixture(tmp_path / "two")
    with linked_build(context) as live, linked_build(other) as foreign:
        with pytest.raises(ValueError, match="epoch"):
            native.identify_section(context, foreign.inputs.objects, "unit0.obj", 1)
    # The public APIs consume an enclosing live native context. A caller-owned
    # lifecycle guard exposes expiry even when physical files remain present.
    context.check_current = live.check_current
    with pytest.raises(ValueError, match="expired"):
        native.resolve_section(context, live.inputs.objects, descriptor())


@pytest.mark.parametrize("phase", ["identify", "resolve"])
def test_physical_mutation_during_selected_lookup_is_caught_on_final_guard(tmp_path, monkeypatch, phase):
    context = fixture(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            original = live.inputs.objects.resolve
            def changed(owner):
                value = original(owner)
                Path(value["identity"]["path"]).write_bytes(b"changed after selected resolution")
                return value
            monkeypatch.setattr(live.inputs.objects, "resolve", changed)
            if phase == "identify":
                native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
            else:
                native.resolve_section(context, live.inputs.objects, descriptor())


def test_cached_owner_or_raw_substitution_cannot_replace_independent_selection(tmp_path):
    context = fixture(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            selected = live.inputs.objects.resolve("unit0.obj")
            altered = deepcopy(selected)
            altered["owner"] = "different.obj"
            live.inputs.objects._resolved["unit0.obj"] = altered
            native.identify_section(context, live.inputs.objects, "unit0.obj", 1)


@pytest.mark.parametrize("phase", ["identify", "resolve"])
@pytest.mark.parametrize("damage", ["missing", "duplicate"])
def test_final_owner_labels_are_required_and_cannot_be_duplicated(tmp_path, phase, damage):
    def change(context):
        log = (context.directory / "link.log").read_bytes()
        name = context.objects[0].name.encode() + b"\n"
        assert log.count(name) == 1
        log = log.replace(name, b"" if damage == "missing" else name * 2)
        evidence = json.loads((context.directory / "link.json").read_bytes())
        evidence["link_log_sha256"] = sha256(log).hexdigest()
        update_artifact(context, "link.log", log)
        update_artifact(context, "link.json", json.dumps(evidence).encode())
    context = fixture(tmp_path, damage=change)
    with linked_build(context) as live:
        with pytest.raises(ValueError):
            if phase == "identify":
                native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
            else:
                native.resolve_section(context, live.inputs.objects, descriptor())


@pytest.mark.parametrize("phase", ["identify", "resolve"])
def test_final_report_mutation_is_refused_even_when_report_still_claims_pass(tmp_path, monkeypatch, phase):
    context = fixture(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            original = native._selected
            def change(*args):
                value = original(*args)
                context.report["compile_link"]["compilations"][0]["compilation"]["exit_code"] = False
                return value
            monkeypatch.setattr(native, "_selected", change)
            if phase == "identify":
                native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
            else:
                native.resolve_section(context, live.inputs.objects, descriptor())


def test_dependency_section_hash_cannot_follow_changed_bytes_in_another_build(tmp_path):
    old = module_context(tmp_path / "old")
    new = module_context(tmp_path / "new")
    add_archive(old, [("dir/storage.obj/", data_object())], ["storage.obj"])
    raw = bytearray(data_object())
    raw[60] = 17
    add_archive(new, [("dir/storage.obj/", bytes(raw))], ["storage.obj"])
    with linked_build(old) as a, linked_build(new) as b:
        identity = native.identify_section(old, a.inputs.objects, "RUNTIME:storage.obj", 1)
        assert native.identify_section(new, b.inputs.objects, "RUNTIME:storage.obj", 1) != identity
        with pytest.raises(ValueError, match="contents differ"):
            native.resolve_section(new, b.inputs.objects, identity)


def test_returned_descriptor_is_defensive_and_is_not_a_content_verdict(tmp_path):
    context = fixture(tmp_path)
    with linked_build(context) as live:
        identity = native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
        identity["unit"] = "src/forged.cpp"
        assert native.identify_section(context, live.inputs.objects, "unit0.obj", 1) == descriptor()
        assert "result" not in identity and "address" not in identity


def test_ordinary_zero_fill_identity_uses_positive_declared_extent(tmp_path):
    raw = data_object(flags=0xC0300080, zero_fill=True, name=b".bss")
    context = fixture(tmp_path, raw=raw)
    with linked_build(context) as live:
        identity = native.identify_section(context, live.inputs.objects, "unit0.obj", 1)
        assert identity == {"kind": "compiled", "unit": "src/unit0.cpp", "section": ".bss"}
        assert native.resolve_section(context, live.inputs.objects, identity) == {"owner": "unit0.obj", "section": 1}


def test_compiled_producer_cannot_be_hidden_inside_dependency_namespace(tmp_path):
    context = fixture(tmp_path)
    path = context.objects[0]
    disguised = {"kind": "dependency", "input": {"kind": "build", "path": path.relative_to(context.directory).as_posix()},
                 "object_sha256": sha256(path.read_bytes()).hexdigest(), "section": ".data", "object_section": 1}
    with linked_build(context) as live:
        with pytest.raises(ValueError, match="reclassified"):
            native.resolve_section(context, live.inputs.objects, disguised)


def test_batch_reuses_one_native_inventory_but_keeps_each_call_guarded(tmp_path, monkeypatch):
    context = fixture(tmp_path)
    original = epochs._input_inventory
    calls = []
    def inventory(*args):
        calls.append(True)
        return original(*args)
    monkeypatch.setattr(epochs, "_input_inventory", inventory)
    with linked_build(context) as live:
        with native.section_producers(context, live.inputs.objects) as batch:
            for _ in range(5):
                assert batch.identify("unit0.obj", 1) == descriptor()
                assert batch.resolve(descriptor()) == {"owner": "unit0.obj", "section": 1}
                batch.check_current()
            assert len(calls) == 1
        with pytest.raises(ValueError, match="expired"):
            batch.identify("unit0.obj", 1)
        with pytest.raises(ValueError, match="expired"):
            batch.resolve(descriptor())
        with pytest.raises(ValueError, match="expired"):
            batch.check_current()


@pytest.mark.parametrize("damage", ["input", "source", "artifact", "report", "guard", "indices", "cache"])
@pytest.mark.parametrize("again", [False, True], ids=["close-only", "next-call"])
def test_batch_detects_deferred_damage_on_next_call_and_final_exit(tmp_path, damage, again):
    context = fixture(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            with native.section_producers(context, live.inputs.objects) as batch:
                assert batch.identify("unit0.obj", 1) == descriptor()
                if damage == "input":
                    context.objects[0].write_bytes(b"modified captured compiler output")
                elif damage == "source":
                    (context.root / "src/unit0.cpp").write_bytes(b"modified source")
                elif damage == "artifact":
                    (context.directory / "link.log").write_bytes(b"modified native log")
                elif damage == "report":
                    context.report["compile_link"]["compilations"][0]["compilation"]["exit_code"] = False
                elif damage == "guard":
                    context.check_current = lambda: None
                elif damage == "indices":
                    live.inputs.objects._owners.clear()
                else:
                    live.inputs.objects._resolved["unit0.obj"]["owner"] = "forged.obj"
                if again:
                    with pytest.raises(ValueError):
                        batch.resolve(descriptor())
    with pytest.raises(ValueError, match="expired"):
        batch.check_current()


def test_batch_expiry_survives_caller_exception_without_refreshing_context(tmp_path):
    context = fixture(tmp_path)
    with linked_build(context) as live:
        with pytest.raises(RuntimeError, match="caller stopped"):
            with native.section_producers(context, live.inputs.objects) as batch:
                assert batch.resolve(descriptor()) == {"owner": "unit0.obj", "section": 1}
                raise RuntimeError("caller stopped")
        with pytest.raises(ValueError, match="expired"):
            batch.identify("unit0.obj", 1)


def test_batch_refuses_other_context_objects_before_providing_a_handle(tmp_path):
    old = fixture(tmp_path / "old")
    new = fixture(tmp_path / "new")
    with linked_build(old) as a, linked_build(new) as b:
        with pytest.raises(ValueError, match="epoch"):
            with native.section_producers(old, b.inputs.objects):
                pytest.fail("foreign context yielded a section producer")
        with native.section_producers(old, a.inputs.objects) as batch:
            assert batch.identify("unit0.obj", 1) == descriptor()

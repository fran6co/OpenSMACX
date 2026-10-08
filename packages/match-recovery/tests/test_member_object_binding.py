"""Synthetic COFF/CodeView controls for member-backed object identities."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.binary import Relocation, RelocationKind, compare_bound_code
from match_recovery.codeview_data import member_data_layout
from match_recovery.model import Origin, Span
from match_recovery.application_binary import check_compiled
from match_recovery.bound_objects import FunctionBinding
from match_recovery.generated_source import GeneratedBinding
from match_recovery.import_source import ImportBinding
from match_recovery.object_data import check_initial_data, referenced_initial_object_units
from match_recovery.object_source import (ObjectBinding, object_targets, parse_objects, read_objects,
                                          referenced_object_units, emitted_object_symbols,
                                          require_disjoint_object_symbols)
from match_recovery.object_writer import replace_object_records, write_object_bytes
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.preservation import _outside_annotations
from match_recovery.shared_source import SharedFunctionBinding
from match_recovery.source_config import SourceConfiguration
from test_object_data import data_image
from test_codeview import record


PIN = "a" * 64
UNIT = "code/owner.cc"
PARENT = "_parent"


def udt_types(*, duplicate_field=False, omit_field=False, nested=False, member_type=0x74,
              inherited=False, truncated_base=False):
    if nested:
        inner_fields = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 0) + b"\x05value")
        inner = record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 4) + b"\x05Inner")
        outer_fields = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x1001, 4) + b"\x05inner")
        outer = record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1002, 0, 0, 16) + b"\x05Outer")
        return inner_fields + inner + outer_fields + outer, 0x1003, ("inner", "value")
    if inherited:
        base_fields = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 0) + b"\x0abase_child")
        base = record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 4) + b"\x04Base")
        members = struct.pack("<HHI", 0x1400, 0, 0x1001)
        if not truncated_base:
            members += struct.pack("<H", 0)
            if not omit_field:
                members += struct.pack("<HHIH", 0x1405, 0, member_type, 4) + b"\x05child"
                if duplicate_field:
                    members += struct.pack("<HHIH", 0x1405, 0, member_type, 8) + b"\x05child"
        fields = record(0x1203, members)
        count = 1 + (not omit_field) + duplicate_field
        parent = record(0x1005, struct.pack("<HHIIIH", count, 0, 0x1002, 0, 0, 16) + b"\x06Parent")
        return base_fields + base + fields + parent, 0x1003, ("child",)
    members = b""
    count = 0
    if not omit_field:
        members += struct.pack("<HHIH", 0x1405, 0, member_type, 4) + b"\x05child"
        count += 1
        if duplicate_field:
            members += struct.pack("<HHIH", 0x1405, 0, member_type, 8) + b"\x05child"
            count += 1
    parent = record(0x1005, struct.pack("<HHIIIH", count, 0, 0x1000 if count else 0, 0, 0, 16) + b"\x06Parent")
    return (record(0x1203, members) if count else b"") + parent, 0x1001 if count else 0x1000, ("child",)


def owner_object(value=None, *, damage=None, duplicate_symbol=False, nested=False,
                 field_size=4, member_type=0x74, data_relocations=(),
                 inherited=False, truncated_base=False):
    value = bytes(16) if value is None else value
    types, type_index, path = udt_types(duplicate_field=damage == "duplicate_field",
                                       omit_field=damage == "missing_field", nested=nested,
                                       inherited=inherited, truncated_base=truncated_base,
                                       member_type=member_type)
    if damage == "wrong_size":
        types = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x71, 4) + b"\x05child") + \
                record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 16) + b"\x06Parent")
        type_index = 0x1001
    elif damage in ("forward_only", "ambiguous_udt"):
        forward = record(0x1005, struct.pack("<HHIIIH", 0, 0x80, 0, 0, 0, 0) + b"\x06Parent")
        field_list = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 4) + b"\x05child")
        complete = record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1001, 0, 0, 16) + b"\x06Parent")
        if damage == "forward_only":
            types, type_index = forward, 0x1000
        else:
            types, type_index = forward + field_list + complete + field_list + complete, 0x1000
    elif damage == "lf_index":
        types = record(0x1203, struct.pack("<H", 0x1404) + bytes(8)) + \
                record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 16) + b"\x06Parent")
        type_index = 0x1001
    elif damage == "out_of_parent":
        types = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 15) + b"\x05child") + \
                record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 16) + b"\x06Parent")
        type_index = 0x1001
    elif damage == "cyclic":
        types = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x1001, 0) + b"\x05child") + \
                record(0x1005, struct.pack("<HHIIIH", 1, 0, 0x1000, 0, 0, 16) + b"\x06Parent")
        type_index = 0x1001
    obj = Coff(__import__("test_object_data").data_fixture(value, type_index=type_index, types=types,
                                                             relocations=data_relocations))
    obj.symbols[0] = replace(obj.symbols[0], name=PARENT)
    if damage == "missing_cv":
        obj.sections = tuple(section for section in obj.sections if section.name != ".debug$T")
    elif damage == "duplicate_parent":
        obj.symbols[2] = replace(obj.symbols[0], index=2)
    elif damage == "missing_parent":
        obj.symbols.pop(0)
    elif duplicate_symbol:
        obj.symbols[2] = replace(obj.symbols[0], index=2, name=PARENT)
    return obj, path


def image_for(value, *, absolute=None):
    raw = data_image(value, absolute=absolute)
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    return image, pin


def bound_object(start, *, size=4, path=("child",), symbol="old_child", parent=PARENT, pin=PIN):
    origin = Origin(pin, "pe:va", start)
    return ObjectBinding(origin, symbol, UNIT, 1, Span(start, start + size), origin,
                         parent, tuple(path))


def test_member_path_uses_fresh_codeview_offset_and_extent_and_ignores_adjacent_bytes():
    candidate = b"ABCD" + struct.pack("<I", 0x11223344) + b"EFGHIJKL"
    obj, path = owner_object(candidate)
    layout = member_data_layout(obj, PARENT, path)
    assert (layout["offset"], layout["size"], layout["member_offset"]) == (4, 4, 4)
    original = b"zzzz" + struct.pack("<I", 0x11223344) + b"yyyyyyyy"
    image, pin = image_for(original)
    item = bound_object(0x401004, pin=pin)
    row, = check_initial_data((item,), {UNIT: obj}, image, {PARENT: 0x401000})
    assert row["definition"] == "pass" and row["content"] == "pass"


def test_grouped_member_bindings_resolve_parent_base_and_addend_references():
    obj, _ = owner_object()
    first = bound_object(0x401004, symbol="old_first", path=("first",))
    second = bound_object(0x401008, symbol="old_second", path=("second",))
    # Second field is a valid integer member at offset eight.
    obj.sections = (replace(obj.sections[0], data=bytes(16)), *obj.sections[1:])
    # Use a parent type with both actual named fields.
    fields = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 4) + b"\x05first" +
                    struct.pack("<HHIH", 0x1405, 0, 0x74, 8) + b"\x06second")
    parent_type = record(0x1005, struct.pack("<HHIIIH", 2, 0, 0x1000, 0, 0, 16) + b"\x06Parent")
    obj, _ = owner_object()
    # Rewrite only debug type bytes to provide two independent named children.
    types = fields + parent_type
    from test_object_data import data_fixture
    obj = Coff(data_fixture(bytes(16), type_index=0x1001, types=types))
    obj.symbols[0] = replace(obj.symbols[0], name=PARENT)
    targets, reports = object_targets((first, second), {UNIT: obj})
    assert targets[PARENT] == 0x401000 and [r["definition"] for r in reports] == ["pass", "pass"]
    proof = compare_bound_code(Span(0x5000, 0x5004), struct.pack("<I", 0x401008),
                               struct.pack("<I", 8), (Relocation(0, RelocationKind.ABSOLUTE32, PARENT),),
                               targets, original_absolute_sites=frozenset({0}))
    assert proof["result"] == "pass"


def test_direct_parent_and_member_bindings_resolve_one_atomic_target():
    obj, _ = owner_object(inherited=True)
    parent = ObjectBinding(Origin(PIN, "pe:va", 0x401000), PARENT, UNIT, 1)
    child = bound_object(0x401004)
    targets, reports = object_targets((parent, child), {UNIT: obj})
    assert targets[PARENT] == 0x401000
    assert [row["definition"] for row in reports] == ["pass", "pass"]
    assert member_data_layout(obj, PARENT, ("child",))["member_offset"] == 4


def test_parent_address_mismatch_and_unavailable_layout_refuse_the_whole_group():
    obj, _ = owner_object()
    parent = ObjectBinding(Origin(PIN, "pe:va", 0x402000), PARENT, UNIT, 1)
    child = bound_object(0x401004)
    targets, reports = object_targets((parent, child), {UNIT: obj})
    assert PARENT not in targets and [row["definition"] for row in reports] == ["unknown", "unknown"]

    unavailable, _ = owner_object(damage="missing_cv")
    parent = ObjectBinding(Origin(PIN, "pe:va", 0x401000), PARENT, UNIT, 1)
    targets, reports = object_targets((parent, child), {UNIT: unavailable})
    assert PARENT not in targets and [row["definition"] for row in reports] == ["unknown", "unknown"]


def test_ambiguous_or_truncated_inherited_layout_stays_unknown_for_both_identities():
    parent = ObjectBinding(Origin(PIN, "pe:va", 0x401000), PARENT, UNIT, 1)
    child = bound_object(0x401004)
    for obj, _ in (owner_object(damage="duplicate_field", inherited=True),
                   owner_object(damage="truncated_base", inherited=True, truncated_base=True)):
        targets, reports = object_targets((parent, child), {UNIT: obj})
        assert PARENT not in targets
        assert [row["definition"] for row in reports] == ["unknown", "unknown"]


def test_direct_member_lookup_never_walks_base_names_or_out_of_bounds_extents():
    obj, _ = owner_object(inherited=True)
    assert member_data_layout(obj, PARENT, ("child",))["member_offset"] == 4
    with pytest.raises(ValueError, match="named CodeView member"):
        member_data_layout(obj, PARENT, ("base_child",))
    out_of_bounds, _ = owner_object(damage="out_of_parent")
    with pytest.raises(ValueError, match="outside its owning UDT"):
        member_data_layout(out_of_bounds, PARENT, ("child",))


def test_parent_binding_requires_same_unit_and_original_address_domain():
    child = bound_object(0x401004)
    other_unit = ObjectBinding(Origin(PIN, "pe:va", 0x401000), PARENT, "other/owner.cc", 1)
    with pytest.raises(ValueError, match="translation unit"):
        object_targets((child, other_unit), {})
    foreign = ObjectBinding(Origin("b" * 64, "pe:va", 0x401000), PARENT, UNIT, 1)
    with pytest.raises(ValueError, match="address domains"):
        object_targets((child, foreign), {})


def test_disagreeing_original_parent_bases_refuse_instead_of_last_binding_wins():
    fields = record(0x1203, struct.pack("<HHIH", 0x1405, 0, 0x74, 4) + b"\x05first" +
                    struct.pack("<HHIH", 0x1405, 0, 0x74, 8) + b"\x06second")
    parent = record(0x1005, struct.pack("<HHIIIH", 2, 0, 0x1000, 0, 0, 16) + b"\x06Parent")
    from test_object_data import data_fixture
    obj = Coff(data_fixture(bytes(16), type_index=0x1001, types=fields + parent))
    obj.symbols[0] = replace(obj.symbols[0], name=PARENT)
    rows = (bound_object(0x401004, symbol="old_first"),
            bound_object(0x401009, symbol="old_second", path=("second",)))
    targets, reports = object_targets(rows, {UNIT: obj})
    assert PARENT not in targets and [r["definition"] for r in reports] == ["unknown", "unknown"]


def test_wrong_v2_extent_is_a_measured_boundary_failure():
    obj, _ = owner_object(damage="wrong_size")
    image, pin = image_for(bytes(16))
    row, = check_initial_data((bound_object(0x401004, pin=pin),), {UNIT: obj}, image, {PARENT: 0x401000})
    assert row["definition"] == "pass" and row["content"] == "fail"
    assert row["proof"]["boundary"] == "fail"


def test_codeview_member_type_extent_is_not_inferred_from_source_or_neighbor():
    obj, path = owner_object(damage="wrong_size")
    assert member_data_layout(obj, PARENT, path)["size"] == 2


@pytest.mark.parametrize("damage", ["missing_field", "duplicate_field", "missing_cv", "duplicate_parent",
                                    "missing_parent", "forward_only", "ambiguous_udt", "lf_index",
                                    "cyclic", "out_of_parent"])
def test_missing_ambiguous_or_unsupported_codeview_evidence_stays_unknown(damage):
    obj, path = owner_object(damage=damage)
    with pytest.raises(ValueError):
        member_data_layout(obj, PARENT, path)


def test_nested_path_and_bad_path_are_resolved_or_refused_strictly():
    obj, path = owner_object(nested=True)
    layout = member_data_layout(obj, PARENT, path)
    assert layout["member_offset"] == 4 and layout["size"] == 4
    with pytest.raises(ValueError):
        member_data_layout(obj, PARENT, ("inner", "missing"))
    with pytest.raises(ValueError):
        member_data_layout(obj, PARENT, ("inner", "value", "extra"))


def test_relocation_intersecting_member_is_rebased_and_edge_crossing_is_rejected():
    # The byte comparator consumes a relocation only when the four-byte site
    # lies wholly inside the old member span; object_data applies this interval.
    target = {PARENT: 0x401000}
    good = compare_bound_code(Span(0x6000, 0x6004), struct.pack("<I", 0x401004),
                              struct.pack("<I", 4), (Relocation(0, RelocationKind.ABSOLUTE32, PARENT),), target,
                              original_absolute_sites=frozenset({0}))
    assert good["result"] == "pass"
    with pytest.raises(ValueError, match="out-of-bound"):
        compare_bound_code(Span(0x6000, 0x6004), bytes(4), bytes(4),
                           (Relocation(2, RelocationKind.ABSOLUTE32, PARENT),), target,
                           original_absolute_sites=frozenset({0}))


def test_member_slice_rebases_real_coff_relocation_without_neighbor_or_edge_borrowing():
    # Pointer child at parent+4 refers to parent+8. The COFF relocation names
    # the actual parent and carries the member offset as its addend.
    original = b"ABCD" + struct.pack("<I", 0x401008) + b"EFGHIJKL"
    image, pin = image_for(original, absolute=4)
    item = bound_object(0x401004, pin=pin)
    value = b"wxyz" + struct.pack("<I", 8) + b"12345678"
    obj, _ = owner_object(value, member_type=0x403, data_relocations=((4, 0, 6),))
    targets, reports = object_targets((item,), {UNIT: obj})
    assert targets[PARENT] == 0x401000 and reports[0]["definition"] == "pass"
    row, = check_initial_data((item,), {UNIT: obj}, image, targets)
    assert row["content"] == "pass" and row["proof"]["relocations"][0]["offset"] == 0

    # A relocation wholly in the next member is ignored for this old object.
    neighbor, _ = owner_object(value, member_type=0x403, data_relocations=((8, 0, 6),))
    row, = check_initial_data((item,), {UNIT: neighbor}, image, targets)
    assert row["content"] == "fail" and row["proof"]["boundary"] == "pass"

    crossing, _ = owner_object(value, member_type=0x403, data_relocations=((6, 0, 6),))
    row, = check_initial_data((item,), {UNIT: crossing}, image, targets)
    assert row["content"] == "unknown" and "cuts" in row["reason"]

    unsupported, _ = owner_object(value, member_type=0x403, data_relocations=((4, 0, 20),))
    row, = check_initial_data((item,), {UNIT: unsupported}, image, targets)
    assert row["content"] == "unknown" and "unsupported" in row["reason"]


def test_v3_writer_preserves_original_interpretation_and_parent_path_is_structural():
    text = b"int owner;\n"
    item = bound_object(0x401004, symbol="old_child")
    updated = write_object_bytes(text, expected=(), additions=(replace(item, line=1),),
                                 path=UNIT, pin=PIN, address_space="pe:va")
    parsed, = parse_objects(updated.decode(), UNIT, PIN, address_space="pe:va")
    assert parsed == replace(item, line=1)
    direct, = parse_objects("// RECOVERY_OBJECT: v2 0x401004 0x401008 old_child 0x401004\n",
                            UNIT, PIN, address_space="pe:va")
    assert direct.interpretation_id == parsed.interpretation_id
    assert direct.interpretation == parsed.interpretation
    assert (direct.parent_symbol, direct.member_path) != (parsed.parent_symbol, parsed.member_path)
    config = SourceConfiguration(PIN, "pe:va")
    first = b"// RECOVERY_OBJECT: v3 0x401004 0x401008 old_child 0x401004 member _parent child\n"
    moved = b"// RECOVERY_OBJECT: v3 0x401004 0x401008 old_child 0x401004 member _parent renamed\n"
    before = _outside_annotations({"src/owner.cc": first}, config, PIN, frozenset())
    after = _outside_annotations({"src/owner.cc": moved}, config, PIN, frozenset())
    assert before != after  # outside-scope source preservation sees owner/path changes
    unbounded, = parse_objects("// RECOVERY_OBJECT: v3 0x401008 old_tail member _parent tail\n",
                               UNIT, PIN, address_space="pe:va")
    assert unbounded.span is None and unbounded.origin.address == 0x401008
    direct_unbounded = ObjectBinding(Origin(PIN, "pe:va", 0x401008), "old_tail", UNIT, 1)
    with pytest.raises(ValueError, match="immutable tuple"):
        replace(direct_unbounded, member_path=[])
    with pytest.raises(ValueError):
        parse_objects("// RECOVERY_OBJECT: v3 0x401004 old_child member _parent x..y\n",
                      UNIT, PIN, address_space="pe:va")


def test_member_parent_references_schedule_owner_and_selected_bounded_initializers():
    obj, _ = owner_object()
    item = bound_object(0x401004)
    assert referenced_initial_object_units((item,), (item,), {UNIT: obj}) == frozenset()
    obj.sections = (replace(obj.sections[0], relocations=((4, 0, 6),)), *obj.sections[1:])
    assert referenced_initial_object_units((item,), (item,), {UNIT: obj}) == frozenset({UNIT})
    assert referenced_object_units((item,), {UNIT: obj}) == frozenset({UNIT})


def test_parent_and_historical_child_are_both_reserved_across_namespaces():
    image, pin = image_for(bytes(16))
    item = bound_object(0x401004, pin=pin)
    assert emitted_object_symbols((item,)) == frozenset({"old_child", PARENT})
    for names in ({PARENT}, {"old_child"}):
        with pytest.raises(ValueError, match="object symbol conflicts"):
            require_disjoint_object_symbols((item,), names)


def test_member_historical_identity_cannot_alias_another_emitted_member_owner(tmp_path):
    first = bound_object(0x401004, symbol=PARENT, parent="_state", path=("first",))
    second = bound_object(0x402004, symbol="old_second", parent=PARENT, path=("second",))
    rows = (first, second)
    with pytest.raises(ValueError, match="historical object identity conflicts"):
        require_disjoint_object_symbols(rows)
    with pytest.raises(ValueError, match="historical object identity conflicts"):
        object_targets(rows, {})

    # The source reader is the first project-wide boundary, before callers can
    # turn the colliding old identity into a relocation target.
    (tmp_path / "recovery").mkdir()
    (tmp_path / "src").mkdir()
    (tmp_path / "recovery" / "project.json").write_text(
        '{"schema_version":1,"source":{"image_pin":"recovery/pinned.exe",'
        '"address_space":"pe:va","roots":["src"]}}'
    )
    (tmp_path / "src" / "owner.cc").write_text(
        "// RECOVERY_OBJECT: v3 0x401004 _parent member _state first\n"
        "// RECOVERY_OBJECT: v3 0x402004 old_second member _parent second\n"
    )
    with pytest.raises(ValueError, match="historical object identity conflicts"):
        read_objects(tmp_path, PIN)


@pytest.mark.parametrize("namespace", ["function", "external", "generated", "import", "shared"])
def test_check_compiled_rejects_parent_collision_with_every_global_namespace(namespace):
    image, pin = image_for(bytes(16))
    item = bound_object(0x401004, pin=pin)
    origin = Origin(pin, "pe:va", 0x401000)
    function = FunctionBinding(origin, (Span(0x401000, 0x401004),), PARENT, UNIT)
    generated = GeneratedBinding(origin, Span(0x401000, 0x401004), PARENT, origin, UNIT, 1)
    imported = ImportBinding(PARENT, "module.dll", "name:target", None, UNIT, 1)
    shared = SharedFunctionBinding(origin, Span(0x401000, 0x401004), PARENT, origin, UNIT, 1)
    functions = (function,) if namespace == "function" else ()
    externals = {PARENT: 0x401000} if namespace == "external" else {}
    generated_rows = (generated,) if namespace == "generated" else ()
    imports = (imported,) if namespace == "import" else ()
    shared_rows = (shared,) if namespace == "shared" else ()
    with pytest.raises(ValueError, match="object symbol conflicts|conflicting function/object"):
        check_compiled(functions, (item,), generated_rows, {}, image, externals,
                       import_bindings=imports, shared_bindings=shared_rows)


def test_guarded_rehome_writer_requires_full_snapshot_and_preserves_all_identity_fields():
    source_a = ("src/a.cc", b"// RECOVERY_OBJECT: v2 0x401000 0x401004 old_a 0x401000\nint a;\n")
    source_b = ("src/b.cc", b"// RECOVERY_OBJECT: v2 0x401008 0x40100C old_b 0x401008\nint b;\n")
    source_c = ("src/c.cc", b"// RECOVERY_OBJECT: v1 0x401010 old_tail\nint c;\n")
    current = dict((source_a, source_b, source_c))
    expected = tuple(row for path, raw in current.items()
                     for row in parse_objects(raw.decode(), path, PIN, address_space="pe:va"))
    replacements = (replace(expected[0], unit="src/b.cc", line=1, parent_symbol="_state", member_path=("first",)),
                    replace(expected[1], unit="src/b.cc", line=1, parent_symbol="_state", member_path=("second",)),
                    replace(expected[2], unit="src/b.cc", line=1, parent_symbol="_state", member_path=("tail",)))
    result = replace_object_records(current, expected=expected, replacements=replacements,
                                    destination="src/b.cc", pin=PIN, address_space="pe:va")
    rows = tuple(row for path, raw in result.items()
                 for row in parse_objects(raw.decode(), path, PIN, address_space="pe:va"))
    interpretation_id = lambda row: row.interpretation_id if row.span is not None else None
    assert {(row.symbol, row.span, row.evidence, interpretation_id(row),
             row.parent_symbol, row.member_path) for row in rows} == {
        (old.symbol, old.span, old.evidence, interpretation_id(old),
         "_state", path)
         for old, path in zip(expected, (("first",), ("second",), ("tail",)))}
    assert all(interpretation_id(new) == interpretation_id(old)
               for old, new in zip(expected, replacements))
    assert result["src/a.cc"] == b"int a;\n"
    assert result["src/c.cc"] == b"int c;\n"
    assert result["src/b.cc"].count(b"RECOVERY_OBJECT: v3") == 3
    with pytest.raises(ValueError, match="complete current source snapshot"):
        replace_object_records(current, expected=expected[:1], replacements=replacements[:1],
                               destination="src/b.cc", pin=PIN, address_space="pe:va")
    altered = replace(replacements[0], symbol="swapped_identity")
    with pytest.raises(ValueError, match="changed historical identity"):
        replace_object_records(current, expected=expected, replacements=(altered, replacements[1]),
                               destination="src/b.cc", pin=PIN, address_space="pe:va")
    missing = replace(replacements[0], line=0)
    with pytest.raises(ValueError, match="source"):
        replace_object_records(current, expected=expected, replacements=(missing, replacements[1]),
                               destination="src/b.cc", pin=PIN, address_space="pe:va")


def test_guarded_writer_transports_full_25_identity_and_22_initializer_set():
    path = "src/aggregate.cc"
    raw = b"int aggregate;\n"
    originals = []
    for index in range(25):
        start = 0x500000 + index * 8
        origin = Origin(PIN, "pe:va", start)
        if index < 22:
            originals.append(ObjectBinding(origin, f"old_{index}", path, 1,
                                            Span(start, start + 4),
                                            Origin(PIN, "pe:va", 0x600000 + index * 4)))
        else:
            originals.append(ObjectBinding(origin, f"old_{index}", path, 1))
    raw = write_object_bytes(raw, expected=(), additions=tuple(originals),
                             path=path, pin=PIN, address_space="pe:va")
    expected = parse_objects(raw.decode(), path, PIN, address_space="pe:va")
    moved = tuple(replace(row, line=1, parent_symbol="_aggregate",
                          member_path=(f"field_{index}",))
                  for index, row in enumerate(expected))
    result = replace_object_records({path: raw}, expected=expected, replacements=moved,
                                    destination=path, pin=PIN, address_space="pe:va")
    actual = parse_objects(result[path].decode(), path, PIN, address_space="pe:va")
    assert len(actual) == 25 and sum(row.span is not None for row in actual) == 22
    assert [(row.origin, row.symbol, row.span, row.evidence) for row in actual] == [
        (row.origin, row.symbol, row.span, row.evidence) for row in expected]
    assert [row.interpretation_id for row in actual if row.span is not None] == [
        row.interpretation_id for row in expected if row.span is not None]
    assert all(row.parent_symbol == "_aggregate" for row in actual)
    assert [row.member_path for row in actual] == [(f"field_{i}",) for i in range(25)]


def test_guarded_rehome_writer_rejects_eof_anchor_inside_open_block_comment():
    old = b"// RECOVERY_OBJECT: v1 0x401000 old_tail\nint old;\n"
    expected = parse_objects(old.decode(), "src/old.cc", PIN, address_space="pe:va")
    replacement = replace(expected[0], unit="src/new.cc", line=2,
                          parent_symbol="_aggregate", member_path=("tail",))
    with pytest.raises(ValueError, match="inside a block comment"):
        replace_object_records(
            {"src/old.cc": old, "src/new.cc": b"/* open\n"},
            expected=expected, replacements=(replacement,), destination="src/new.cc",
            pin=PIN, address_space="pe:va")


def test_guarded_rehome_writer_preserves_direct_parent_and_child_history():
    path = "src/owner.cc"
    raw = (b"// RECOVERY_OBJECT: v1 0x401000 _state\nint state;\n"
           b"// RECOVERY_OBJECT: v2 0x401004 0x401008 old_child 0x401004\nint child;\n")
    expected = parse_objects(raw.decode(), path, PIN, address_space="pe:va")
    moved = replace(expected[1], line=5, parent_symbol="_state", member_path=("child",))
    result = replace_object_records({path: raw}, expected=expected, replacements=(moved,),
                                    destination=path, pin=PIN, address_space="pe:va")
    actual = parse_objects(result[path].decode(), path, PIN, address_space="pe:va")
    assert len(actual) == 2
    direct, child = actual
    assert direct == expected[0]
    old = expected[1]
    assert (child.origin, child.symbol, child.span, child.evidence, child.interpretation_id) == (
        old.origin, old.symbol, old.span, old.evidence, old.interpretation_id)
    assert (child.parent_symbol, child.member_path) == ("_state", ("child",))


def test_member_rehome_to_scalar_parent_does_not_publish_native_target():
    from test_object_data import data_fixture

    scalar_parent = Coff(data_fixture(bytes(4), type_index=0x74, definition=b"_state"))
    parent = ObjectBinding(Origin(PIN, "pe:va", 0x401000), "_state", UNIT, 1)
    child = replace(bound_object(0x401004), parent_symbol="_state")
    targets, reports = object_targets((parent, child), {UNIT: scalar_parent})
    assert "_state" not in targets
    assert [row["definition"] for row in reports] == ["unknown", "unknown"]


def test_guarded_rehome_writer_rejects_old_child_aliasing_sibling_parent():
    path = "src/owner.cc"
    raw = (b"// RECOVERY_OBJECT: v1 0x401000 _parent\nint first;\n"
           b"// RECOVERY_OBJECT: v1 0x402000 old_second\nint second;\n")
    expected = parse_objects(raw.decode(), path, PIN, address_space="pe:va")
    replacements = (
        replace(expected[0], parent_symbol="_state", member_path=("first",)),
        replace(expected[1], parent_symbol="_parent", member_path=("second",)),
    )
    with pytest.raises(ValueError, match="historical object identity conflicts"):
        replace_object_records({path: raw}, expected=expected, replacements=replacements,
                               destination=path, pin=PIN, address_space="pe:va")


def test_unavailable_member_layout_cannot_become_initializer_pass():
    obj, _ = owner_object(damage="missing_cv")
    image, pin = image_for(bytes(16))
    row, = check_initial_data((bound_object(0x401004, pin=pin),), {UNIT: obj}, image, {PARENT: 0x401000})
    assert row["content"] == "unknown" and row["definition"] == "unknown"


def test_initializer_refuses_a_parent_target_that_disagrees_with_member_identity():
    obj, _ = owner_object()
    image, pin = image_for(bytes(16))
    row, = check_initial_data((bound_object(0x401004, pin=pin),), {UNIT: obj}, image, {PARENT: 0x401001})
    assert row["content"] == "unknown" and row["definition"] == "unknown"

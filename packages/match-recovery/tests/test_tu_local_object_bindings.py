"""TU-qualified static-data references never turn into section/global aliases."""
from __future__ import annotations

import hashlib
import struct
from dataclasses import replace

import pytest

from match_recovery.application_binary import _validate_compiled_namespaces, check_compiled
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import (emitted_object_symbols, local_object_targets,
                                          object_targets, parse_objects,
                                          require_disjoint_object_symbols,
                                          validate_object_symbol_ownership)
from match_recovery.generated_source import GeneratedBinding
from match_recovery.import_source import ImportBinding
from match_recovery.literal_source import LiteralBinding
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.shared_source import SharedFunctionBinding
from test_binary import pe_fixture
from test_object_data import data_fixture, data_image


PINNED_TARGET = 0x401100
FUNCTION = 0x401000


def _section(name, data, relocations, flags):
    return (struct.pack("<8sIIIIIIHHI", name.encode().ljust(8, b"\0"), 0, 0,
                        len(data), 0, 0, 0, len(relocations), 0, flags),
            b"".join(struct.pack("<IIH", *row) for row in relocations))


def _auxiliary(length, relocations, number, selection, parent=None):
    return struct.pack("<IHHIHB", length, relocations, 0, 0,
                       number if parent is None else parent, selection) + bytes(3)


def data_object(*, caller="_caller", target="_hidden", target_value=0, target_section=2,
                target_type=0, target_storage=3, target_aux=0,
                extra_symbols=(), references=None, bss=False,
                data_comdat=False, data_associative=False):
    """One function COMDAT plus an ordinary data section and named COFF records."""
    code = b"\xa1\0\0\0\0\xc3" + b"\xcc" * 10
    data = b"" if bss else bytes(16)
    specs = [(caller, 0, 1, 0x20, 2, 0),
             (target, target_value, target_section, target_type, target_storage, target_aux),
             *extra_symbols]
    # Section symbols each consume one auxiliary record; ordinary symbols are
    # indexed after those records and their own optional auxiliary entries.
    indices = {}
    records = []
    index = 0
    data_selection = 5 if data_associative else 2 if data_comdat else 0
    data_parent = 1 if data_associative else None
    for section_name, section_no, section_data, relocs, selection, parent in (
            (".text$A", 1, code, references or (), 2, None),
            (".bss" if bss else ".data", 2, data, (), data_selection, data_parent)):
        indices[section_name] = index
        records.append((section_name, 0, section_no, 0, 3, 1,
                        _auxiliary(16 if section_no == 2 and bss else len(section_data),
                                   len(relocs), section_no, selection, parent)))
        index += 2
    for name, value, section, kind, storage, aux_count in specs:
        if name not in indices:
            indices[name] = index
        records.append((name, value, section, kind, storage, aux_count, bytes(18 * aux_count)))
        index += 1 + aux_count

    relocs = []
    for offset, symbol_name, kind in references or ():
        relocs.append((offset, indices[symbol_name], kind))
    text_table, text_reloc_bytes = _section(".text$A", code, relocs, 0x60501020)
    data_flags = 0xC0300080 if bss else 0xC0300040
    if data_comdat or data_associative:
        data_flags |= 0x1000
    data_table, data_reloc_bytes = _section(".bss" if bss else ".data", data, (), data_flags)
    header_size = 20 + 2 * 40
    text_at = header_size
    data_at = text_at + len(code)
    reloc_at = data_at + len(data)
    symbol_at = reloc_at + len(text_reloc_bytes)
    text_table = bytearray(text_table)
    data_table = bytearray(data_table)
    struct.pack_into("<I", text_table, 20, text_at)
    struct.pack_into("<I", text_table, 24, reloc_at if relocs else 0)
    if bss:
        struct.pack_into("<I", data_table, 16, 16)
        struct.pack_into("<I", data_table, 20, 0)
    else:
        struct.pack_into("<I", data_table, 20, data_at)
    struct.pack_into("<I", data_table, 24, 0)
    symbol_records = []
    for name, value, section, kind, storage, aux_count, aux in records:
        symbol_records.append(name.encode().ljust(8, b"\0")
                              + struct.pack("<IhHBB", value, section, kind, storage, aux_count)
                              + aux)
    symbol_bytes = b"".join(symbol_records)
    symbol_count = index
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_at, symbol_count, 0, 0)
    return (header + bytes(text_table) + bytes(data_table) + code + data
            + text_reloc_bytes + symbol_bytes + struct.pack("<I", 4))


def _image(target=PINNED_TARGET):
    raw = bytearray(pe_fixture(b"\xa1" + struct.pack("<I", target) + b"\xc3",
                               absolute_site=1))
    pin = hashlib.sha256(raw).hexdigest()
    return PeImage(bytes(raw), pin), pin


def _objects(pin, text="// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
             *, unit="src/owner.cpp", obj=None):
    bindings = parse_objects(text, unit, pin, address_space="pe:va")
    objects = {unit: Coff(obj if obj is not None else
                          data_object(references=((1, "_hidden", 6),)))}
    return bindings, objects


def _function(pin, *, unit="src/owner.cpp"):
    return FunctionBinding(Origin(pin, "pe:va", FUNCTION),
                          (Span(FUNCTION, FUNCTION + 6),), "_caller", unit)


def _target_rows(bindings, objects):
    targets, reports = local_object_targets(bindings, objects)
    return targets, reports


def _bounded_local_binding(pin, size=4, *, unit="src/owner.cpp", start=0x401000):
    text = (f"// RECOVERY_OBJECT: v2 0x{start:08X} 0x{start + size:08X} "
            "_hidden 0x00401100\n")
    return parse_objects(text, unit, pin, address_space="pe:va")


def test_same_translation_unit_static_data_resolves_without_section_placement():
    image, pin = _image()
    bindings, objects = _objects(pin)
    targets, reports = _target_rows(bindings, objects)
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    assert reports
    assert reports[0]["definition"] == "pass"
    assert reports[0]["linkage"] == "translation_unit_local"
    assert reports[0]["extent"] == "unbounded"
    rows = check_objects([_function(pin)], objects, image, local_data_targets=targets)
    assert rows[0]["result"] == Result.PASS


def test_bounded_static_root_needs_matching_same_tu_codeview_extent():
    raw_image = data_image()
    pin = hashlib.sha256(raw_image).hexdigest()
    bindings = _bounded_local_binding(pin)
    owner = Coff(data_fixture(bytes(4), definition=b"_hidden", storage=3))
    targets, reports = local_object_targets(bindings, {"src/owner.cpp": owner})
    assert targets == {("src/owner.cpp", "_hidden"): 0x401000}
    assert reports[0]["definition"] == Result.PASS
    assert reports[0]["extent"] == "bounded"
    assert reports[0]["compiler_storage"]["section"] == owner.symbols[0].section
    assert reports[0]["compiler_storage"]["offset"] == owner.symbols[0].value

    wrong_size = _bounded_local_binding(pin, 8)
    targets, reports = local_object_targets(wrong_size, {"src/owner.cpp": owner})
    assert targets == {}
    assert reports[0]["definition"] == Result.UNKNOWN
    assert "CodeView storage extent" in reports[0]["reason"]


def test_bounded_static_root_rejects_original_span_past_i386_address_domain():
    raw_image = data_image()
    pin = hashlib.sha256(raw_image).hexdigest()
    bindings = _bounded_local_binding(pin, start=0xFFFFFFFE)
    owner = Coff(data_fixture(bytes(4), definition=b"_hidden", storage=3))
    targets, reports = local_object_targets(bindings, {"src/owner.cpp": owner})
    assert targets == {}
    assert reports[0]["definition"] == Result.UNKNOWN
    assert "exceeds the i386 address domain" in reports[0]["reason"]


@pytest.mark.parametrize("damage", ["type", "section", "offset"])
def test_bounded_static_root_refuses_nonordinary_or_misaligned_codeview_storage(damage):
    raw_image = data_image()
    pin = hashlib.sha256(raw_image).hexdigest()
    bindings = _bounded_local_binding(pin)
    owner = Coff(data_fixture(bytes(4), definition=b"_hidden", storage=3))
    if damage == "type":
        symbol = owner.symbols[0]
        owner.symbols[0] = replace(symbol, type=0x20)
    elif damage == "section":
        owner.sections = (replace(owner.sections[0], flags=owner.sections[0].flags | 0x20),
                          *owner.sections[1:])
    else:
        debug = bytearray(owner.sections[2].data)
        debug[12] = 1
        owner.sections = (*owner.sections[:2], replace(owner.sections[2], data=bytes(debug)))
    targets, reports = local_object_targets(bindings, {"src/owner.cpp": owner})
    assert targets == {}
    assert reports[0]["definition"] == Result.UNKNOWN


def test_bounded_static_target_is_owner_qualified_and_does_not_prove_initializer():
    raw_image = data_image(b"\x01\0\0\0")
    pin = hashlib.sha256(raw_image).hexdigest()
    image = PeImage(raw_image, pin)
    bindings = _bounded_local_binding(pin)
    owner = Coff(data_fixture(bytes(4), definition=b"_hidden", storage=3))

    # A copied spelling in another TU cannot stand in for a missing owner.
    other = Coff(data_fixture(bytes(4), definition=b"_hidden", storage=3))
    targets, reports = local_object_targets(bindings, {"src/other.cpp": other})
    assert targets == {} and reports[0]["definition"] == Result.UNKNOWN

    targets, reports = local_object_targets(bindings, {"src/owner.cpp": owner})
    assert targets == {("src/owner.cpp", "_hidden"): 0x401000}
    # The local target resolves the owner identity, while original initializer
    # bytes are checked separately and still expose this mismatch.
    row, = check_initial_data(bindings, {"src/owner.cpp": owner}, image, targets)
    assert row["definition"] == Result.PASS
    assert row["content"] == Result.FAIL


def test_wrong_local_address_fails_and_neighbor_in_same_data_section_stays_unknown():
    image, pin = _image()
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    objects = {"src/owner.cpp": Coff(data_object(references=((1, "_hidden", 6),)))}
    wrong = {("src/owner.cpp", "_hidden"): PINNED_TARGET + 4}
    row, = check_objects([_function(pin)], objects, image, local_data_targets=wrong)
    assert row["result"] == Result.FAIL

    # The neighboring static symbol shares .data with the mapped object, but
    # neither that section nor the neighbor inherits the mapped identity.
    neighbor_obj = Coff(data_object(target="_hidden", extra_symbols=(
        ("_nearby", 8, 2, 0, 3, 0),), references=((1, "_nearby", 6),)))
    targets, _ = _target_rows(binding, {"src/owner.cpp": neighbor_obj})
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    row, = check_objects([_function(pin)], {"src/owner.cpp": neighbor_obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


def test_same_symbol_in_another_tu_does_not_leak_and_missing_units_are_unknown():
    image, pin = _image()
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    owner = Coff(data_object(target="_hidden"))
    caller = Coff(data_object(target="_hidden", target_section=0, target_storage=2,
                              references=((1, "_hidden", 6),)))
    objects = {"src/owner.cpp": owner, "src/caller.cpp": caller}
    targets, _ = _target_rows(binding, objects)
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    row, = check_objects([_function(pin, unit="src/caller.cpp")], objects, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN
    wrong_unit = {("src/missing.cpp", "_hidden"): PINNED_TARGET}
    owner_with_reference = Coff(data_object(references=((1, "_hidden", 6),)))
    row, = check_objects([_function(pin)], {"src/owner.cpp": owner_with_reference}, image,
                         local_data_targets=wrong_unit)
    assert row["result"] != Result.PASS
    row, = check_objects([_function(pin)], {}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


def test_v6_same_spelling_static_targets_are_tu_qualified_and_never_global():
    code = bytearray(b"\x90" * 48)
    for offset, target in ((0, 0x401100), (16, 0x401200), (32, 0x401100)):
        code[offset:offset + 6] = b"\xA1" + struct.pack("<I", target) + b"\xC3"
    raw = bytearray(pe_fixture(bytes(code), absolute_site=1))
    # Three independent absolute operand sites in the same original image.
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1100, 16)
    struct.pack_into("<IIHHHH", raw, 768, 0x1000, 16, 0x3001, 0x3011, 0x3021, 0)
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(bytes(raw), pin)
    units = ("src/owner_a.cpp", "src/owner_b.cpp")
    bindings = tuple(parse_objects(
        f"// RECOVERY_OBJECT: v6 0x{address:08X} {historical} target local _$S15\n",
        unit, pin, address_space="pe:va")[0]
        for unit, address, historical in zip(units, (0x401100, 0x401200),
                                             ("_old_a_guard", "_old_b_guard")))
    caller_unit = "src/external_caller.cpp"
    objects = {
        units[0]: Coff(data_object(caller="_call_a", target="_$S15", target_storage=3,
                                   references=((1, "_$S15", 6),))),
        units[1]: Coff(data_object(caller="_call_b", target="_$S15", target_storage=3,
                                   references=((1, "_$S15", 6),))),
        caller_unit: Coff(data_object(caller="_call_c", target="_$S15", target_section=0, target_storage=2,
                                      references=((1, "_$S15", 6),))),
    }
    validate_object_symbol_ownership(bindings)
    local, reports = local_object_targets(bindings, objects)
    assert local == {(units[0], "_$S15"): 0x401100,
                     (units[1], "_$S15"): 0x401200}
    assert [row["definition"] for row in reports] == [Result.PASS, Result.PASS]
    global_targets, global_rows = object_targets(bindings, objects)
    assert global_targets == {}
    assert len(global_rows) == 2 and all(row["definition"] == Result.UNKNOWN for row in global_rows)
    reserved = emitted_object_symbols(bindings)
    assert "_$S15" not in reserved
    assert {"_old_a_guard", "_old_b_guard"} <= reserved
    with pytest.raises(ValueError, match="another source namespace"):
        require_disjoint_object_symbols(bindings, {"_$S15"})

    functions = [
        FunctionBinding(Origin(pin, "pe:va", address), (Span(address, address + 6),),
                        symbol, unit)
        for address, symbol, unit in zip((0x401000, 0x401010, 0x401020),
                                         ("_call_a", "_call_b", "_call_c"),
                                         (*units, caller_unit))
    ]
    explicit_names = {(row.unit, row.emitted_symbol) for row in bindings}
    rows = check_objects(functions, objects, image, local_data_targets=local,
                         explicit_local_data_names=explicit_names)
    assert [row["result"] for row in rows] == [Result.PASS, Result.PASS, Result.UNKNOWN]

    # Actual local references cannot borrow a same-unit literal or function
    # binding with the same spelling.
    with pytest.raises(ValueError, match="also has a literal binding"):
        check_objects(functions[:1], objects, image, local_data_targets=local,
                      explicit_local_data_names=explicit_names,
                      literal_targets={(units[0], "_$S15"): None})
    conflicting_function = FunctionBinding(Origin(pin, "pe:va", 0x401300),
                                           (Span(0x401300, 0x401306),),
                                           "_$S15", units[0])
    with pytest.raises(ValueError, match="also has a function binding"):
        check_objects((conflicting_function,), objects, image, local_data_targets=local,
                      explicit_local_data_names=explicit_names)
    with pytest.raises(ValueError, match="global function/import binding"):
        check_objects(functions[:1], objects, image, local_data_targets=local,
                      explicit_local_data_names=explicit_names,
                      import_targets={"_$S15": 0x401100})
    with pytest.raises(ValueError, match="global function/import binding"):
        check_objects(functions[:1], objects, image, local_data_targets=local,
                      explicit_local_data_names=explicit_names,
                      external_targets={"_$S15": 0x401100})


@pytest.mark.parametrize("damage", ["external", "common", "code", "comdat", "duplicate"])
def test_v6_local_target_requires_one_ordinary_static_data_definition(damage):
    image, pin = _image()
    binding, = parse_objects(
        "// RECOVERY_OBJECT: v6 0x00401100 _old_guard target local _$S15\n",
        "src/owner.cpp", pin, address_space="pe:va")
    owner = Coff(data_object(target="_$S15", target_storage=3))
    target_index = next(index for index, symbol in owner.symbols.items()
                        if symbol.name == "_$S15")
    if damage == "external":
        owner.symbols[target_index] = replace(owner.symbols[target_index], storage=2)
    elif damage == "common":
        owner.symbols[target_index] = replace(owner.symbols[target_index], section=0, value=4, storage=2)
    elif damage == "code":
        owner.symbols[target_index] = replace(owner.symbols[target_index], type=0x20)
    elif damage == "comdat":
        number = owner.symbols[target_index].section - 1
        owner.sections = (*owner.sections[:number],
                          replace(owner.sections[number], flags=owner.sections[number].flags | 0x1000),
                          *owner.sections[number + 1:])
    else:
        duplicate_index = max(owner.symbols) + 1
        owner.symbols[duplicate_index] = replace(owner.symbols[target_index], index=duplicate_index)
    local, reports = local_object_targets((binding,), {binding.unit: owner})
    assert local == {}
    assert reports[0]["definition"] == Result.UNKNOWN


def test_v6_rejects_same_translation_unit_collision_but_legacy_v4_fallback_is_unchanged():
    image, pin = _image()
    first, = parse_objects("// RECOVERY_OBJECT: v6 0x00401100 _old_a target local _$S15\n",
                           "src/owner.cpp", pin, address_space="pe:va")
    second, = parse_objects("// RECOVERY_OBJECT: v4 0x00401200 _old_b target _$S15\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    with pytest.raises(ValueError, match="TU-local object target collides"):
        validate_object_symbol_ownership((first, second))

    legacy, = parse_objects("// RECOVERY_OBJECT: v4 0x00401100 _old_guard target _$S15\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    owner = Coff(data_object(target="_$S15", target_storage=3))
    local, reports = local_object_targets((legacy,), {legacy.unit: owner})
    assert local == {(legacy.unit, "_$S15"): legacy.origin.address}
    assert reports[0]["definition"] == Result.PASS


def test_v6_unbounded_data_check_proves_only_owner_start_and_rejects_peer_namespaces():
    raw = data_image()
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    binding, = parse_objects(
        "// RECOVERY_OBJECT: v6 0x00401100 _old_guard target local _actual\n",
        "src/owner.cpp", pin, address_space="pe:va")
    owner = Coff(data_fixture(definition=b"_actual", storage=3))
    local, _ = local_object_targets((binding,), {binding.unit: owner})
    row, = check_initial_data((binding,), {binding.unit: owner}, image, {},
                              local_data_targets=local)
    assert row["definition"] == Result.PASS
    assert row["content"] == Result.UNKNOWN
    assert row["lifetime"] == Result.UNKNOWN

    with pytest.raises(ValueError, match="global initial-data target"):
        check_initial_data((binding,), {binding.unit: owner}, image, {"_actual": 0x401100},
                           local_data_targets=local)
    with pytest.raises(ValueError, match="literal target"):
        check_initial_data((binding,), {binding.unit: owner}, image, {},
                           local_data_targets=local,
                           literal_targets={(binding.unit, "_actual"): None})


@pytest.mark.parametrize("peer", ["function", "literal", "generated", "import", "shared"])
def test_v6_emitted_name_is_refused_against_other_source_namespace(peer):
    name = "??_7Peer@@6B@"
    origin = Origin("a" * 64, "pe:va", 0x401000)
    local, = parse_objects(
        f"// RECOVERY_OBJECT: v6 0x00401000 _old_local target local {name}\n",
        "src/local.cpp", "a" * 64, address_space="pe:va")
    functions = ()
    literals = ()
    generated = ()
    imports = ()
    shared = ()
    if peer == "function":
        functions = (FunctionBinding(origin, (Span(0x401000, 0x401006),), name,
                                     "src/peer.cpp"),)
    elif peer == "literal":
        literals = (LiteralBinding(origin, Span(0x401000, 0x401004), name,
                                   "src/peer.cpp", 1, origin),)
    elif peer == "generated":
        generated = (GeneratedBinding(origin, None, name, origin, "src/peer.cpp", 1),)
    elif peer == "import":
        imports = (ImportBinding(name, "KERNEL32", "VirtualAlloc", None,
                                 "src/peer.cpp", 1),)
    else:
        shared = (SharedFunctionBinding(origin, Span(0x401000, 0x401006), name,
                                        origin, "src/peer.cpp", 1),)
    with pytest.raises(ValueError, match="another source namespace"):
        _validate_compiled_namespaces(functions, (local,), generated, imports, shared,
                                      literal_bindings=literals)


@pytest.mark.parametrize("damage", ["duplicate", "code", "section_symbol", "auxiliary",
                                     "common", "undefined", "global"])
def test_only_one_static_ordinary_data_definition_is_admitted(damage):
    image, pin = _image()
    if damage == "duplicate":
        raw = data_object(target="_hidden", extra_symbols=(
            ("_hidden", 4, 2, 0, 3, 0),))
    elif damage == "code":
        raw = data_object(target="_hidden", target_value=0, target_section=1,
                          target_type=0)
    elif damage == "section_symbol":
        raw = data_object()
        binding = parse_objects("// RECOVERY_OBJECT: v4 0x00401100 _legacy target .data\n",
                                "src/owner.cpp", pin, address_space="pe:va")
        targets, reports = _target_rows(binding, {"src/owner.cpp": Coff(raw)})
        assert targets == {}
        assert reports and reports[0]["definition"] == Result.UNKNOWN
        return
    elif damage == "auxiliary":
        raw = data_object(target="_hidden", target_aux=1)
    elif damage == "common":
        raw = data_object(target="_hidden", target_value=4, target_section=0,
                          target_storage=2)
    elif damage == "undefined":
        raw = data_object(target="_hidden", target_value=0, target_section=0,
                          target_storage=2)
    else:
        raw = data_object(target="_hidden", target_storage=2)
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    targets, reports = _target_rows(binding, {"src/owner.cpp": Coff(raw)})
    assert targets == {}
    assert reports and reports[0]["definition"] == Result.UNKNOWN
    if damage in ("duplicate", "code"):
        # An explicit local map does not make a duplicate or code-section
        # symbol valid. In particular, the code symbol would otherwise inherit
        # the function COMDAT's placement and spuriously resolve this operand.
        image_for_symbol = image
        if damage == "code":
            image_for_symbol, pin = _image(FUNCTION)
        obj = Coff(raw)
        row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image_for_symbol,
                             local_data_targets={("src/owner.cpp", "_hidden"): PINNED_TARGET})
        assert row["result"] != Result.PASS


def test_offset_static_symbol_preserves_original_relocation_addend():
    image, pin = _image(PINNED_TARGET + 8)
    binding = parse_objects("// RECOVERY_OBJECT: v4 0x00401100 _legacy target _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    raw = data_object(target="_hidden", target_value=4,
                      references=((1, "_hidden", 6),))
    targets, _ = _target_rows(binding, {"src/owner.cpp": Coff(raw)})
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    # The COFF static symbol's value (4) and relocation addend (8) remain
    # independent contributions; resolving to object start yields target+8.
    # Candidate immediate contains addend 8 for this focused control.
    obj = Coff(data_object(target="_hidden", target_value=4,
                           references=((1, "_hidden", 6),)))
    code = bytearray(obj.sections[0].data)
    code[1:5] = struct.pack("<I", 8)
    obj.sections = (replace(obj.sections[0], data=bytes(code)), *obj.sections[1:])
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.PASS
    code[1:5] = struct.pack("<I", 12)
    obj.sections = (replace(obj.sections[0], data=bytes(code)), *obj.sections[1:])
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.FAIL


def test_global_external_resolution_is_unchanged_and_not_promoted_from_local_map():
    image, pin = _image(PINNED_TARGET)
    binding = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                            "src/owner.cpp", pin, address_space="pe:va")
    # A separate global reference remains governed by explicit global targets.
    obj = Coff(data_object(target="_hidden", references=((1, "_global", 6),),
                           extra_symbols=(("_global", 0, 0, 0, 2, 0),)))
    local_targets, _ = _target_rows(binding, {"src/owner.cpp": obj})
    assert local_targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         external_targets={"_global": PINNED_TARGET},
                         local_data_targets=local_targets)
    assert row["result"] == Result.PASS
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=local_targets)
    assert row["result"] == Result.UNKNOWN


def test_bss_static_definition_resolves_from_declared_section_size_without_rawbytes():
    image, pin = _image()
    bindings = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                             "src/owner.cpp", pin, address_space="pe:va")
    obj = Coff(data_object(bss=True, references=((1, "_hidden", 6),)))
    assert obj.sections[1].data == b""
    assert obj.section_sizes[1] == 16
    targets, reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {("src/owner.cpp", "_hidden"): PINNED_TARGET}
    assert reports[0]["definition"] == "pass"
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets=targets)
    assert row["result"] == Result.PASS


@pytest.mark.parametrize("damage", ["function_type", "executable", "mixed_data_kinds",
                                     "past_section", "empty_section", "weak"])
def test_invalid_physical_local_target_cannot_borrow_section_or_global_fallback(damage):
    image, pin = _image()
    obj = Coff(data_object(references=((1, "_hidden", 6),)))
    symbol, = (s for s in obj.symbols.values() if s.name == "_hidden")
    if damage == "function_type":
        obj.symbols[symbol.index] = replace(symbol, type=0x20)
    elif damage == "executable":
        obj.sections = (obj.sections[0], replace(obj.sections[1], flags=0xE0300040))
    elif damage == "mixed_data_kinds":
        obj.sections = (obj.sections[0], replace(obj.sections[1], flags=0xC03000C0))
    elif damage == "past_section":
        obj.symbols[symbol.index] = replace(symbol, value=16)
    elif damage == "empty_section":
        obj.section_sizes = (obj.section_sizes[0], 0)
    else:
        obj.symbols[symbol.index] = replace(symbol, section=0, storage=105)
    bindings = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                             "src/owner.cpp", pin, address_space="pe:va")
    targets, reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {} and reports[0]["definition"] == Result.UNKNOWN
    row, = check_objects([_function(pin)], {"src/owner.cpp": obj}, image,
                         local_data_targets={("src/owner.cpp", "_hidden"): PINNED_TARGET},
                         external_targets={"_hidden": PINNED_TARGET},
                         section_bases={("src/owner.cpp", 2): PINNED_TARGET})
    assert row["result"] != Result.PASS


def test_unbound_static_in_another_tu_does_not_borrow_the_same_named_binding():
    image, pin = _image()
    bindings, objects = _objects(pin)
    objects["src/other.cpp"] = Coff(data_object(references=((1, "_hidden", 6),)))
    targets, _ = _target_rows(bindings, objects)
    row, = check_objects([_function(pin, unit="src/other.cpp")], objects, image,
                         local_data_targets=targets)
    assert row["result"] == Result.UNKNOWN


@pytest.mark.parametrize("associative", [False, True])
def test_comdat_and_associative_data_definitions_are_not_local_object_targets(associative):
    image, pin = _image()
    del image
    bindings = parse_objects("// RECOVERY_OBJECT: v1 0x00401100 _hidden\n",
                             "src/owner.cpp", pin, address_space="pe:va")
    obj = Coff(data_object(data_comdat=not associative,
                           data_associative=associative))
    targets, _reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {}


@pytest.mark.parametrize("text", [
    "// RECOVERY_OBJECT: v2 0x00401100 0x00401104 _hidden 0x00401100\n",
    "// RECOVERY_OBJECT: v3 0x00401100 _member member _parent field\n",
    ("// RECOVERY_OBJECT: v1 0x00401100 _parent\n"
     "// RECOVERY_OBJECT: v3 0x00401104 _member member _parent field\n"),
])
def test_bounded_member_and_member_group_bindings_are_not_local_object_targets(text):
    image, pin = _image()
    del image
    bindings = parse_objects(text, "src/owner.cpp", pin, address_space="pe:va")
    obj = Coff(data_object(target="_parent" if "_parent" in text else "_hidden"))
    targets, _reports = _target_rows(bindings, {"src/owner.cpp": obj})
    assert targets == {}


@pytest.mark.parametrize("mapping", [
    {"_hidden": PINNED_TARGET},
    {("src/owner.cpp", "_hidden"): True},
    {("src/owner.cpp", "_hidden"): -1},
    {("src/owner.cpp", "_hidden"): 0x100000000},
    {("src/owner.cpp", "_hidden", "extra"): PINNED_TARGET},
])
def test_local_data_map_rejects_malformed_keys_and_values(mapping):
    image, pin = _image()
    objects = {"src/owner.cpp": Coff(data_object(references=((1, "_hidden", 6),)))}
    with pytest.raises(ValueError, match="invalid TU-qualified local data target"):
        check_objects([_function(pin)], objects, image, local_data_targets=mapping)


def test_check_compiled_pipeline_uses_source_local_object_bindings():
    image, pin = _image()
    source_objects, objects = _objects(pin)
    function_rows, object_rows, *_ = check_compiled(
        [_function(pin)], source_objects, (), objects, image, {})
    assert function_rows[0]["result"] == Result.PASS
    assert object_rows and object_rows[0]["definition"] == "pass"
    assert object_rows[0]["linkage"] == "translation_unit_local"

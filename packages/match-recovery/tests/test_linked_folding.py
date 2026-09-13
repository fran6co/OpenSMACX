"""Physical original-owner ICF bindings, not equal-byte or any-frame inference."""

from dataclasses import FrozenInstanceError, replace
from hashlib import sha256
import struct

import pytest

from match_recovery.linked_folding import bind_folding
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import PeImage
from test_link_file_evidence import stamp
from test_link_input_coverage import inputs
from test_linked_fpo import object_bytes
from test_linked_initializers import build
from test_linked_objects import inventory
from test_pdb_build_binding import add_bound_pdb
from test_toolchain_components import archive


def fixture(root, *, group=None, raw_first=None, raw_second=None, map_owner="app.obj",
            second_address=0x401000, extra_groups=""):
    evidence, log = inputs(root)
    first = object_bytes() if raw_first is None else raw_first
    second = object_bytes().replace(b"_f\0\0\0\0\0\0", b"_g\0\0\0\0\0\0") if raw_second is None else raw_second
    (root / "app.obj").write_bytes(first)
    # Keep the fixture's selected import members and replace only its runtime owner.
    from test_coff_inventory import imported
    (root / "runtime.lib").write_bytes(archive([("runtime.obj/", second),
        ("DEMO.dll/", imported()), ("DEMO.dll/", imported("_Other", "Other"))]))
    if group is None:
        group = "Selected symbol:\n_f from app.obj\nReplaced symbol(s):\n_g from RUNTIME.lib(runtime.obj)\n"
    log = log.replace("Start Pass2", group + extra_groups + "ICF total savings: 16 bytes\nStart Pass2")
    evidence["link_log_sha256"] = sha256(log.encode()).hexdigest()
    for row in evidence["files"]:
        from pathlib import Path
        row["before"] = row["after"] = stamp(Path(row["path"]).read_bytes())
    objects = SelectedObjects(*inventory(evidence, log))
    context = build(root / "image")
    pdb = add_bound_pdb(context).read_bytes()
    raw = (context.directory / "app.exe").read_bytes()
    image = PeImage(raw, sha256(raw).hexdigest())
    fixups = bound_fixups(image, pdb)
    mapping = parse_link_map(("synthetic\n Preferred load address is 00400000\n"
        " Address         Publics by Value              Rva+Base     Lib:Object\n"
        " 0001:00000000 _f 00401000 f app.obj\n"
        f" 0001:{second_address - 0x401000:08x} _g {second_address:08x} f {map_owner}\n"
        " entry point at 0001:000001e0\n").encode())
    return objects, mapping, image, fixups, log.encode()


def test_folded_map_owner_does_not_replace_original_physical_owners(tmp_path):
    folding = bind_folding(*fixture(tmp_path))
    group = folding.group("RUNTIME:runtime.obj", "_g")
    assert folding.for_map("app.obj", "_g") == group
    assert group.address == 0x401000
    assert [member.owner for member in group.members] == ["app.obj", "RUNTIME:runtime.obj"]
    assert [member.symbol for member in group.members] == ["_f", "_g"]
    assert [member.fpo_sections for member in group.members] == [(2,), (2,)]
    assert group.members[1].input == (("member", "runtime.obj"), ("occurrence", 0),
                                      ("path", str(tmp_path / "runtime.lib")))
    assert all(member.object_sha256 == sha256(member.raw).hexdigest() for member in group.members)
    assert not hasattr(group, "result") and not hasattr(group, "accepted")
    with pytest.raises(FrozenInstanceError):
        group.members[1].owner = "app.obj"


@pytest.mark.parametrize("damage", ["duplicate", "missing", "multiple_groups"])
def test_partial_repeated_or_ambiguous_groups_never_return_a_prefix(tmp_path, damage):
    group = "Selected symbol:\n_f from app.obj\nReplaced symbol(s):\n_g from RUNTIME.lib(runtime.obj)\n"
    if damage == "duplicate":
        group += "_g from RUNTIME.lib(runtime.obj)\n"
    elif damage == "missing":
        group += "_absent from app.obj\n"
    values = fixture(tmp_path, group=group, extra_groups=group if damage == "multiple_groups" else "")
    with pytest.raises(ValueError, match="repeats|unique defined|unique complete"):
        bind_folding(*values).group("app.obj", "_f")


@pytest.mark.parametrize("options", [{"map_owner": "RUNTIME:runtime.obj"}, {"second_address": 0x401020}])
def test_map_owner_or_common_address_disagreement_refuses(tmp_path, options):
    with pytest.raises(ValueError, match="selected-owner|common linked address"):
        bind_folding(*fixture(tmp_path, **options)).group("app.obj", "_f")


def test_unrelated_same_image_log_cannot_borrow_input_selection(tmp_path):
    values = list(fixture(tmp_path))
    values[-1] = values[-1].replace(b"16 bytes", b"32 bytes")
    with pytest.raises(ValueError, match="selected-object input evidence"):
        bind_folding(*values)


def test_same_log_cannot_borrow_another_image_binding(tmp_path):
    values = list(fixture(tmp_path))
    raw = bytearray(values[2].data)
    raw[600] ^= 1
    values[2] = PeImage(bytes(raw), sha256(raw).hexdigest())
    with pytest.raises(ValueError, match="exact image"):
        bind_folding(*values)


def test_cached_original_inputs_are_rechecked_before_group_binding(tmp_path):
    folding = bind_folding(*fixture(tmp_path))
    folding.group("app.obj", "_f")
    (tmp_path / "runtime.lib").write_bytes(b"changed")
    with pytest.raises(ValueError, match="population changed"):
        folding.group("app.obj", "_f")


def test_mutation_during_group_binding_refuses(tmp_path, monkeypatch):
    values = fixture(tmp_path)
    folding = bind_folding(*values)
    original = values[0].resolve
    def resolve(owner):
        selected = original(owner)
        (tmp_path / "app.obj").write_bytes(b"changed after resolve")
        return selected
    monkeypatch.setattr(values[0], "resolve", resolve)
    with pytest.raises(ValueError, match="population changed"):
        folding.group("app.obj", "_f")


def test_section_name_requires_unique_exact_original_definition_and_root(tmp_path):
    group = 'Selected symbol:\n".text" from app.obj\nReplaced symbol(s):\n".text" from RUNTIME.lib(runtime.obj)\n'
    bound = bind_folding(*fixture(tmp_path, group=group)).group("app.obj", ".text")
    assert [member.reported_symbol for member in bound.members] == [".text", ".text"]
    assert [member.symbol for member in bound.members] == ["_f", "_g"]


def test_unsupported_unrelated_group_does_not_hide_or_break_requested_complete_group(tmp_path):
    other = "Selected symbol:\n_missing from app.obj\nReplaced symbol(s):\n_other from app.obj\n"
    folding = bind_folding(*fixture(tmp_path, extra_groups=other))
    assert len(folding.group("app.obj", "_f").members) == 2
    with pytest.raises(ValueError, match="unique defined"):
        folding.group("app.obj", "_missing")


def test_duplicate_map_symbol_is_not_an_original_owner_disambiguator(tmp_path):
    values = list(fixture(tmp_path))
    values[1] = replace(values[1], publics=(*values[1].publics, values[1].publics[-1]))
    with pytest.raises(ValueError, match="selected-owner"):
        bind_folding(*values).group("app.obj", "_f")


def test_every_replaced_member_needs_its_own_map_symbol(tmp_path):
    values = list(fixture(tmp_path))
    values[1] = replace(values[1], publics=values[1].publics[:1])
    with pytest.raises(ValueError, match="selected-owner"):
        bind_folding(*values).group("app.obj", "_f")


def test_section_name_ambiguity_never_selects_the_first_definition(tmp_path):
    raw = bytearray(object_bytes())
    at, = struct.unpack_from("<I", raw, 8)
    # Another actual section-definition name, not a synthetic parsed-object edit.
    raw[at + 3 * 18:at + 3 * 18 + 8] = b".text\0\0\0"
    group = 'Selected symbol:\n".text" from app.obj\nReplaced symbol(s):\n_g from RUNTIME.lib(runtime.obj)\n'
    with pytest.raises(ValueError, match="exact unique section definitions"):
        bind_folding(*fixture(tmp_path, raw_first=bytes(raw), group=group)).group("app.obj", ".text")


def test_observed_member_requires_selected_original_owner_not_map_winner_guess(tmp_path):
    group = "Selected symbol:\n_f from app.obj\nReplaced symbol(s):\n_g from missing.obj\n"
    with pytest.raises(ValueError, match="final-pass object owner"):
        bind_folding(*fixture(tmp_path, group=group)).group("app.obj", "_f")


def test_noncomdat_original_code_is_not_assigned_an_invented_function_extent(tmp_path):
    raw = bytearray(object_bytes())
    struct.pack_into("<I", raw, 20 + 36, 0x60500020)
    with pytest.raises(ValueError, match="COMDAT"):
        bind_folding(*fixture(tmp_path, raw_first=bytes(raw))).group("app.obj", "_f")


def two_code_sections(first, second, *, malformed=False):
    from test_multispan import object_fixture
    raw = bytearray(object_fixture(parent=0, selection=2))
    symbols, = struct.unpack_from("<I", raw, 8)
    raw[60:68] = b".text\0\0\0"
    raw[symbols + 3 * 18:symbols + 3 * 18 + 8] = b".text\0\0\0"
    for index, name in ((2, first), (5, second)):
        raw[symbols + index * 18:symbols + index * 18 + 8] = name.encode().ljust(8, b"\0")
        struct.pack_into("<HBB", raw, symbols + index * 18 + 14,
                         0 if malformed and index == 5 else 32, 2, 0)
    return bytes(raw)


def section_candidates(root, *, first_other=0x401040, second_other=0x401060, malformed=False):
    from match_recovery.msvc_link_map import Public
    group = 'Selected symbol:\n".text" from app.obj\nReplaced symbol(s):\n".text" from RUNTIME.lib(runtime.obj)\n'
    values = list(fixture(root, group=group,
        raw_first=two_code_sections("_f", "_h", malformed=malformed),
        raw_second=two_code_sections("_g", "_j")))
    values[1] = replace(values[1], publics=(*values[1].publics,
        Public("_h", 1, first_other - 0x401000, first_other, True, "app.obj"),
        Public("_j", 1, second_other - 0x401000, second_other, True, "app.obj")))
    return values


def test_section_groups_use_unique_cross_member_map_intersection_not_bytes(tmp_path):
    folding = bind_folding(*section_candidates(tmp_path))
    group = folding.group("app.obj", ".text")
    assert group.address == 0x401000
    assert [member.symbol for member in group.members] == ["_f", "_g"]
    assert folding.group("app.obj", "_f") == group
    assert folding.group("RUNTIME:runtime.obj", "_g") == group
    assert folding.for_map("app.obj", "_g") == group
    # The same physical owners have other .text roots, which are not this group.
    assert folding.group("app.obj", "_h", optional=True) is None
    assert folding.for_map("app.obj", "_h", optional=True) is None


@pytest.mark.parametrize("options,reason", [
    ({"second_other": 0x401040}, "one unambiguous common linked address"),
    ({"first_other": 0x401000}, "multiple original section candidates"),
    ({"malformed": True}, "exact defined code root"),
])
def test_unresolved_or_malformed_section_candidates_cannot_be_filtered(tmp_path, options, reason):
    with pytest.raises(ValueError, match=reason):
        bind_folding(*section_candidates(tmp_path, **options)).group("app.obj", ".text")


def test_missing_candidate_map_record_is_not_evidence_of_nonselection(tmp_path):
    values = section_candidates(tmp_path)
    values[1] = replace(values[1], publics=tuple(row for row in values[1].publics if row.symbol != "_h"))
    with pytest.raises(ValueError, match="consistent selected-owner map placement"):
        bind_folding(*values).group("app.obj", ".text")


def test_independently_placed_other_owner_candidate_is_not_the_folded_winner(tmp_path):
    values = section_candidates(tmp_path)
    values[1] = replace(values[1], publics=tuple(replace(row, owner="RUNTIME:runtime.obj")
        if row.symbol == "_j" else row for row in values[1].publics))
    group = bind_folding(*values).group("app.obj", ".text")
    assert [member.symbol for member in group.members] == ["_f", "_g"]


def test_unrelated_section_group_does_not_poison_optional_absent_symbol(tmp_path):
    folding = bind_folding(*section_candidates(tmp_path, malformed=True))
    assert folding.group("app.obj", "not_here", optional=True) is None
    assert folding.for_map("app.obj", "not_here", optional=True) is None


@pytest.mark.parametrize("lookup", ["group", "for_map"])
def test_optional_absence_still_requires_current_physical_inputs(tmp_path, lookup):
    folding = bind_folding(*fixture(tmp_path))
    (tmp_path / "app.obj").write_bytes(b"changed")
    with pytest.raises(ValueError, match="population changed"):
        getattr(folding, lookup)("app.obj", "not_here", optional=True)


def test_advisory_candidates_include_original_and_rewritten_section_names(tmp_path):
    folding = bind_folding(*section_candidates(tmp_path))
    keys = folding.candidate_keys()
    assert isinstance(keys, frozenset)
    assert {("app.obj", name) for name in (".text", "_f", "_h", "_g", "_j")} <= keys
    assert {("runtime:runtime.obj", name) for name in (".text", "_g", "_j")} <= keys
    # _h is only a lookup candidate; full binding still proves it is not folded.
    assert folding.group("app.obj", "_h", optional=True) is None


def test_advisory_candidates_recheck_physical_inputs(tmp_path):
    folding = bind_folding(*fixture(tmp_path))
    assert ("app.obj", "_g") in folding.candidate_keys()
    (tmp_path / "runtime.lib").write_bytes(b"changed")
    with pytest.raises(ValueError, match="population changed"):
        folding.candidate_keys()


def test_advisory_candidates_cannot_omit_unknown_original_owners(tmp_path):
    extra = "Selected symbol:\n_unknown from missing.obj\nReplaced symbol(s):\n_other from app.obj\n"
    folding = bind_folding(*fixture(tmp_path, extra_groups=extra))
    assert ("missing.obj", "_unknown") in folding.candidate_keys()
    with pytest.raises(ValueError, match="final-pass object owner"):
        folding.group("missing.obj", "_unknown")


def test_advisory_section_candidates_need_the_actual_original_owner(tmp_path):
    extra = 'Selected symbol:\n".text" from missing.obj\nReplaced symbol(s):\n_other from app.obj\n'
    folding = bind_folding(*fixture(tmp_path, extra_groups=extra))
    with pytest.raises(ValueError, match="final-pass object owner"):
        folding.candidate_keys()


def test_advisory_candidates_cannot_omit_unknown_section_observations(tmp_path):
    group = 'Selected symbol:\n".absent" from app.obj\nReplaced symbol(s):\n_g from RUNTIME.lib(runtime.obj)\n'
    folding = bind_folding(*fixture(tmp_path, group=group))
    with pytest.raises(ValueError, match="physical section candidates"):
        folding.candidate_keys()

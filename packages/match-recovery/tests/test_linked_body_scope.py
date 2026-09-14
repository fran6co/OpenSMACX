"""Physical native label/module/contribution qualification, not task authority."""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery.linked_body_scope import qualify_body_aliases
from match_recovery.linked_context import linked_build
from test_build_command_evidence import descriptor, STAGES
from test_linked_initializers import update_artifact
from test_linked_modules import fixture as module_fixture, set_link, set_modules
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution, module


def fixture(root, *, options=("/OPT:NOICF",), rows=None, damage=None):
    context = module_fixture(root)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].extend(options)
    set_link(context, evidence, (context.directory / "link.log").read_text())
    args = evidence["arguments"]
    context.report["compile_link"]["command_bindings"]["link"]["description"] = descriptor(
        args, STAGES["link"], [(i, 0, "input_file") for i, value in enumerate(args)
                              if value in evidence["explicit_inputs"]])
    set_contributions(context, contribution(offset=0, size=16, flags=0x60501020) if rows is None else rows)
    if damage:
        damage(context)
    return context


def observed(native):
    graph = native.code([{"owner": "unit0.obj", "symbol": "_$E1"}])
    assert graph["result"] == "pass", graph
    proof = graph["nodes"][0]["proof"]
    bodies = [{"node_id": 0, "entity_id": "task-owned-source", "address": proof["address"], "size": proof["size"]}]
    return graph, bodies


def check(context):
    with linked_build(context) as native:
        graph, bodies = observed(native)
        return qualify_body_aliases(native, graph, bodies)


def test_exact_native_module_and_full_code_contribution_qualify_only_the_body(tmp_path):
    report = check(fixture(tmp_path))
    assert report["result"] == "pass", report
    assert report["bodies"] == [{"entity_id": "task-owned-source", "address": 0x401000, "size": 16}]
    assert report["qualified"][0]["object_section"] == 1
    assert report["qualified"][0]["module"] == report["qualified"][0]["contribution"] == 0
    assert report["policy"] == "explicit_noicf_unmerged"
    assert report["metadata_aliases"] == report["outside_scope_verification"] == "unknown"


@pytest.mark.parametrize("options", [(), ("/DEBUG", "/OPT:NOREF"), ("/OPT:ICF",),
    ("/OPT:ICF", "/OPT:NOICF"), ("/OPT:NOICF", "/OPT:ICF"),
    ("/OPT:NOICF", "/MERGE:.data=.text"), ("/OPT:NOICF", "/FORCE")])
def test_unknown_default_folding_and_transform_policies_never_qualify(tmp_path, options):
    with pytest.raises(ValueError, match="NOICF|policy"):
        check(fixture(tmp_path, options=options))


@pytest.mark.parametrize("directive", [b"/OPT:ICF", b"/MERGE:.data=.text", b"/ALTERNATENAME:_a=_b"])
def test_candidate_archive_directives_cannot_be_ignored_as_unselected(tmp_path, directive):
    from test_linked_modules import add_archive
    from test_linked_constants import symbol_object

    def change(context):
        add_archive(context, [("unused.obj/", symbol_object([], directives=directive))], [])
        evidence = json.loads((context.directory / "link.json").read_bytes())
        args = evidence["arguments"]
        context.report["compile_link"]["command_bindings"]["link"]["description"] = descriptor(
            args, STAGES["link"], [(i, 0, "input_file") for i, value in enumerate(args)
                                  if value in evidence["explicit_inputs"]])
    with pytest.raises(ValueError, match="NOICF|policy"):
        check(fixture(tmp_path, damage=change))


@pytest.mark.parametrize("offset,size", [(0, 0), (0, 8), (0, 32), (1, 15), (16, 16)])
def test_only_exact_whole_root_contribution_can_bind(tmp_path, offset, size):
    with pytest.raises(ValueError, match="contribution"):
        check(fixture(tmp_path, rows=contribution(offset=offset, size=size, flags=0x60501020)))


@pytest.mark.parametrize("other", [contribution(offset=0, size=16, flags=0x60501020),
    contribution(offset=0, size=4, flags=0xC0300040), contribution(offset=8, size=20, flags=0x60501020)])
def test_duplicate_or_conflicting_sc_is_not_filtered_by_kind_or_success(tmp_path, other):
    rows = contribution(offset=0, size=16, flags=0x60501020) + other
    with pytest.raises(ValueError, match="competing PDB"):
        check(fixture(tmp_path, rows=rows))


def test_adjacent_and_zero_size_rows_remain_distinct_without_fabricating_aliases(tmp_path):
    rows = (contribution(offset=0, size=16, flags=0x60501020)
            + contribution(offset=16, size=4) + contribution(offset=0, size=0))
    report = check(fixture(tmp_path, rows=rows))
    assert report["result"] == "pass" and report["contributions_scanned"] == 3


def test_earlier_starting_contribution_cannot_overlap_an_independently_placed_body(tmp_path):
    def change(context):
        raw = bytearray((context.directory / "app.exe").read_bytes())
        raw[0x210:0x220] = raw[0x200:0x210]
        raw[0x200:0x210] = b"\xcc" * 16
        struct.pack_into("<I", raw, 0x404, 0x401010)
        update_artifact(context, "app.exe", bytes(raw))
        mapping = (context.directory / "app.map").read_text().replace(
            "0001:00000000 _$E1 00401000", "0001:00000010 _$E1 00401010")
        update_artifact(context, "app.map", mapping.encode())
        set_contributions(context, contribution(offset=16, size=16, flags=0x60501020)
                          + contribution(offset=0, size=32, flags=0xC0300040))
    with pytest.raises(ValueError, match="competing PDB"):
        check(fixture(tmp_path, damage=change))


@pytest.mark.parametrize("damage", ["wrong_module", "unbound_module", "duplicate_module", "flags", "first"])
def test_physical_module_and_all_contribution_characteristics_are_required(tmp_path, damage):
    def change(context):
        output = str(context.objects[0]).encode()
        if damage == "wrong_module":
            set_modules(context, module(output, output) + module(b"other.obj", b"other.obj"))
            set_contributions(context, contribution(offset=0, size=16, flags=0x60501020, module=1))
        elif damage == "unbound_module":
            set_modules(context, module(b"unit0.obj", b"unit0.obj"))  # Basename is not the observed absolute operand.
        elif damage == "duplicate_module":
            set_modules(context, module(output, output) * 2)
        elif damage == "flags":
            set_contributions(context, contribution(offset=0, size=16, flags=0x60501060))
        else:
            set_modules(context, module(output, output, first=contribution(offset=8, size=4, flags=0x60501020)))
    with pytest.raises(ValueError, match="module|flags|first"):
        check(fixture(tmp_path, damage=change))


@pytest.mark.parametrize("static", [False, True])
@pytest.mark.parametrize("offset", [0, 1, 15])
def test_all_public_and_static_map_labels_including_interior_data_aliases_refuse(tmp_path, static, offset):
    def change(context):
        text = (context.directory / "app.map").read_text()
        row = f" 0001:{offset:08x} _alias {0x401000 + offset:08x} unit0.obj\n"
        text = text + row if static else text.replace(" entry point", row + " entry point")
        update_artifact(context, "app.map", text.encode())
    with pytest.raises(ValueError, match="MAP"):
        check(fixture(tmp_path, damage=change))


def add_export(context, target):
    raw = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<II", raw, 0x98 + 96, 0x2080, 0x80)
    struct.pack_into("<IIHHIIIIIII", raw, 0x480, 0, 0, 0, 0, 0x20D0, 1, 1, 0, 0x20C0, 0, 0)
    struct.pack_into("<I", raw, 0x4C0, target)
    raw[0x4D0:0x4D8] = b"app.exe\0"
    update_artifact(context, "app.exe", bytes(raw))


@pytest.mark.parametrize("target", [0x1000, 0x1001, 0x100F])
def test_ordinal_only_export_aliases_cannot_escape_map_and_graph_checks(tmp_path, target):
    with pytest.raises(ValueError, match="exported"):
        check(fixture(tmp_path, damage=lambda context: add_export(context, target)))


def test_unrelated_export_is_not_a_body_alias_or_metadata_permission(tmp_path):
    report = check(fixture(tmp_path, damage=lambda context: add_export(context, 0x1010)))
    assert report["result"] == "pass" and report["metadata_aliases"] == "unknown"


def test_returned_ranges_do_not_mutate_native_inputs_or_later_observation(tmp_path):
    with linked_build(fixture(tmp_path)) as native:
        graph, bodies = observed(native)
        first = qualify_body_aliases(native, graph, bodies)
        expected = deepcopy(first)
        first["bodies"][0]["size"] = 1000
        first["qualified"][0]["input"].clear()
        assert qualify_body_aliases(native, graph, bodies) == expected


@pytest.mark.parametrize("damage", ["object", "pdb", "image", "graph", "scope"])
def test_final_native_or_scope_mutation_refuses_after_successful_module_lookup(tmp_path, monkeypatch, damage):
    from match_recovery.linked_context import LinkedBuild
    context = fixture(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(context) as native:
            graph, bodies = observed(native)
            original = LinkedBuild.module_bindings
            def change(self):
                report = original(self)
                if damage in ("object", "pdb", "image"):
                    path = context.objects[0] if damage == "object" else context.directory / ("app.pdb" if damage == "pdb" else "app.exe")
                    path.write_bytes(path.read_bytes() + b"late mutation")
                elif damage == "graph":
                    graph["unexpected"] = True
                else:
                    bodies[0]["entity_id"] = "other-source"
                return report
            monkeypatch.setattr(LinkedBuild, "module_bindings", change)
            qualify_body_aliases(native, graph, bodies)


def test_expired_native_scope_cannot_replay_previously_qualified_ranges(tmp_path):
    with linked_build(fixture(tmp_path)) as native:
        graph, bodies = observed(native)
        assert qualify_body_aliases(native, graph, bodies)["result"] == "pass"
    with pytest.raises(ValueError, match="expired"):
        qualify_body_aliases(native, graph, bodies)


def test_another_build_cannot_supply_the_source_node_even_when_bytes_match(tmp_path):
    with linked_build(fixture(tmp_path / "old")) as old:
        graph, bodies = observed(old)
        with linked_build(fixture(tmp_path / "new")) as new:
            with pytest.raises(ValueError, match="physical input"):
                qualify_body_aliases(new, graph, bodies)

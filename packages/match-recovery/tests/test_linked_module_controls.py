"""Independent physical module identity and live command-epoch controls."""

from copy import deepcopy
import json
from pathlib import Path
import struct

import pytest

from match_recovery import linked_context
from match_recovery.model import digest
from match_recovery.msf import SmallMsf
from test_build_command_evidence import descriptor, STAGES
from test_build_graph_commands import Context as CommandContext, stamp
from test_linked_initializers import update_artifact
from test_pdb_contribution_controls import native_context as physical_context, damage_live
from test_pdb_contributions import module, contribution
from test_msf import small_msf


def set_modules(context, names):
    path = context.directory / "app.pdb"
    msf = SmallMsf(path.read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    old = streams[3]
    old_modules, old_sc = struct.unpack_from("<II", old, 24)
    mods = b"".join(module(name.encode(), object_name.encode()) for name, object_name in names)
    records = b"".join(contribution(offset=16 + index * 8, module=index) for index in range(len(names)))
    header = bytearray(old[:64])
    struct.pack_into("<II", header, 24, len(mods), 4 + len(records))
    streams[3] = bytes(header) + mods + struct.pack("<I", 0xF12EBA2D) + records + old[64 + old_modules + old_sc:]
    update_artifact(context, "app.pdb", small_msf(streams))


def fixture(root, *, names=None, configure=None):
    context = physical_context(root)
    commands = CommandContext(root, ["src/unit0.cpp", "src/dependency.cpp"])
    proof = deepcopy(commands.proof)
    binding = proof["command_bindings"]
    for row, bound, output in zip(proof["compilations"], binding["compilations"], context.objects, strict=True):
        args = [row["source"], "/Fo" + str(output), "-O2", "-DKEEP"]
        row["output"] = str(output)
        row["compilation"]["arguments"] = args
        row["compilation"]["output"].update(path=str(output), after=stamp(output.read_bytes()))
        bound.update(output=str(output), arguments=args,
                     description=descriptor(args, STAGES["compile"], [(0, 0, "source"), (1, 3, "output_file")]))
    args = [str(path) for path in context.objects]
    binding["link"].update(arguments=args, cwd=str(context.directory),
        description=descriptor(args, STAGES["link"], [(index, 0, "input_file") for index in range(len(args))]))
    context.report["compile_link"] = proof
    context.proof, context.binding = proof, binding
    set_modules(context, [(name, name) for name in args] if names is None else names(context))
    if configure is not None:
        configure(context)
    expected_report = digest(context.report)
    pins = {path: stamp(path.read_bytes()) for path in (
        root / "compiler", root / "recovery/build.json", root / "recovery/project.json",
        *(Path(row["source"]) for row in proof["compilations"]))}
    original = context.check_current
    def fresh():
        original()
        if digest(context.report) != expected_report or any(
                not path.is_file() or stamp(path.read_bytes()) != expected for path, expected in pins.items()):
            raise ValueError("synthetic command/source epoch changed")
    context.check_current = fresh
    return context


def observe(context):
    with linked_context.linked_build(context) as live:
        return live.module_bindings()


def test_exact_full_object_operands_bind_without_granting_storage_coverage(tmp_path):
    context = fixture(tmp_path / "project")
    report = observe(context)
    assert len(report["modules"]) == 2
    for row, path in zip(report["modules"], context.objects, strict=True):
        assert row["result"] == "pass"
        assert row["input"] == {"path": str(path)}
        assert row["object_sha256"] == stamp(path.read_bytes())["sha256"]
        assert row["owner"] == path.name
    assert report["result"] == report["allocation_isolation"] == report["retained_storage_coverage"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("changed_label", [0, 1, 2])
@pytest.mark.parametrize("spelling", ["basename", "wrong_directory", "suffix_only"])
def test_both_standalone_labels_need_exact_full_operand_not_basename(tmp_path, changed_label, spelling):
    def names(context):
        path = context.objects[0]
        changed = {"basename": path.name, "wrong_directory": str(path.parent / "other" / path.name),
                   "suffix_only": str(path) + ".other"}[spelling]
        pair = [str(path), str(path)]
        for index in range(2):
            if changed_label == 2 or changed_label == index:
                pair[index] = changed
        return [tuple(pair)]
    report = observe(fixture(tmp_path / "project", names=names))
    row, = report["modules"]
    assert row["result"] == "unknown"
    assert "input" not in row and "object_sha256" not in row


def test_duplicate_module_rows_cannot_duplicate_one_physical_owner_proof(tmp_path):
    report = observe(fixture(tmp_path / "project", names=lambda context:
        [(str(context.objects[0]), str(context.objects[0]))] * 2))
    assert len(report["modules"]) == 2
    assert all(row["result"] == "unknown" for row in report["modules"])
    assert report["modules"][0]["index"] != report["modules"][1]["index"]


@pytest.mark.parametrize("name,object_name", [("* Linker *", ""), ("* Linker *", "unit0.obj"),
                                             ("unit0.obj", ""), ("missing.obj", "missing.obj")])
def test_synthetic_or_unobserved_labels_do_not_manufacture_input_identity(tmp_path, name, object_name):
    report = observe(fixture(tmp_path / "project", names=lambda context: [(name, object_name)]))
    row, = report["modules"]
    assert row["result"] == "unknown" and "input" not in row


@pytest.mark.parametrize("damage", ["source_membership", "missing_source", "missing_compilation", "missing_bound",
    "missing_link_operand", "duplicate_link_operand", "bad_role", "capture_arguments", "link_arguments", "link_cwd"])
def test_incomplete_or_conflicting_native_command_membership_refuses(tmp_path, damage):
    def configure(context):
        proof, binding = context.proof, context.binding
        if damage == "source_membership": proof["source_membership"] = "unknown"
        elif damage == "missing_source": proof["sources"].pop()
        elif damage == "missing_compilation": proof["compilations"].pop()
        elif damage == "missing_bound": binding["compilations"].pop()
        elif damage == "missing_link_operand": binding["link"]["description"]["operands"].pop()
        elif damage == "duplicate_link_operand":
            link = binding["link"]
            index = len(link["arguments"])
            link["arguments"].append(link["arguments"][0])
            link["description"]["operands"].append({**link["description"]["operands"][0], "argument": index})
        elif damage == "bad_role": binding["link"]["description"]["operands"][0]["kind"] = "output_file"
        elif damage == "capture_arguments": proof["compilations"][0]["compilation"]["arguments"] = ["different"]
        elif damage == "link_arguments": binding["link"]["arguments"].append("/FORCE")
        elif damage == "link_cwd": binding["link"]["cwd"] = str(context.root)
    context = fixture(tmp_path / "project", configure=configure)
    with pytest.raises(ValueError):
        observe(context)


def test_reports_are_defensive_and_expired_handles_cannot_rebind_modules(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        expected = live.module_bindings()
        changed = live.module_bindings()
        changed["modules"][0]["input"]["path"] = "forged"
        changed["modules"].pop()
        assert live.module_bindings() == expected
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        live.module_bindings()


@pytest.mark.parametrize("damage", ["opaque_report_field", "bool_for_int", "source_membership"])
def test_linked_epoch_itself_pins_command_report_even_if_outer_guard_does_not(tmp_path, damage):
    context = fixture(tmp_path / "project")
    # Model a live physical build callback without its own report snapshot.
    # LinkedBuild must guard the newly consumed command context independently.
    context.check_current = lambda: None
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert live.module_bindings()["modules"][0]["result"] == "pass"
            if damage == "opaque_report_field":
                context.report["new_field"] = "changed"
            elif damage == "bool_for_int":
                context.proof["compilations"][0]["compilation"]["exit_code"] = False
            else:
                context.proof["sources"].reverse()
            with pytest.raises(ValueError):
                live.module_bindings()


def mutate(context, live, damage):
    if damage == "command_report": context.binding["link"]["arguments"].append("/FORCE")
    elif damage == "source": Path(context.proof["sources"][0]).write_bytes(b"changed source")
    elif damage == "selected_hash":
        live.inputs.objects._resolved[context.objects[0].name.casefold()]["sha256"] = "0" * 64
    elif damage == "candidate_hash": live.inputs.objects._constant_records[0]["sha256"] = "0" * 64
    elif damage == "candidate_removed": live.inputs.objects._constant_records.pop()
    elif damage == "final_owner_removed": live.inputs.objects._owners.clear()
    else: damage_live(context, live, damage)


@pytest.mark.parametrize("damage", ["pdb", "parsed_pdb", "object", "image_file", "map_file", "receipt_file",
    "tool", "image_base", "records", "expired", "command_report", "source", "selected_hash", "candidate_hash",
    "candidate_removed", "final_owner_removed"])
@pytest.mark.parametrize("at_close", [False, True])
def test_module_binding_epoch_refuses_physical_parsed_and_command_damage(tmp_path, damage, at_close):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert live.module_bindings()["modules"][0]["result"] == "pass"
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError):
                    live.module_bindings()


@pytest.mark.parametrize("damage", ["pdb", "object", "candidate_hash", "command_report", "expired"])
def test_native_final_guard_rejects_damage_during_successful_observation(tmp_path, monkeypatch, damage):
    from match_recovery import linked_modules
    context = fixture(tmp_path / "project")
    original = linked_modules.observe_modules
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            def changed(*args, **kwargs):
                report = original(*args, **kwargs)
                mutate(context, live, damage)
                return report
            monkeypatch.setattr(linked_modules, "observe_modules", changed)
            with pytest.raises(ValueError):
                live.module_bindings()


def test_native_final_guard_also_runs_when_module_observation_raises(tmp_path, monkeypatch):
    from match_recovery import linked_modules
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with linked_context.linked_build(context) as live:
            def refused(*args, **kwargs):
                context.alive = False
                raise ValueError("unsupported module mapping")
            monkeypatch.setattr(linked_modules, "observe_modules", refused)
            with pytest.raises(ValueError, match="expired"):
                live.module_bindings()


def archive_context(root, *, duplicate=False, finals=("part.obj",)):
    from test_binary import coff_fixture
    from test_linked_modules import fixture as base, add_archive, set_modules as replace_modules
    context = base(root)
    members = [("dir/part.obj/", coff_fixture())] * (2 if duplicate else 1)
    path, spelling = add_archive(context, members, finals)
    replace_modules(context, module(b"dir/part.obj", spelling.encode()))
    return context, path, spelling


@pytest.mark.parametrize("duplicate", [False, True])
def test_exact_archive_module_cannot_choose_unselected_or_duplicate_full_members(tmp_path, duplicate):
    context, path, _ = archive_context(tmp_path / "project", duplicate=duplicate,
                                       finals=("part.obj",) if duplicate else ())
    row, = observe(context)["modules"]
    assert row["result"] == "unknown" and "input" not in row
    assert len(row["candidates"]) == (2 if duplicate else 1)
    assert all(item["input"]["path"] == str(path) for item in row["candidates"])
    assert {item["input"]["occurrence"] for item in row["candidates"]} == ({0, 1} if duplicate else {0})


@pytest.mark.parametrize("damage", ["missing", "different_directory", "no_physical_path"])
def test_pdb_library_label_requires_matching_observed_search_identity(tmp_path, damage):
    from test_linked_modules import set_link
    context, _, spelling = archive_context(tmp_path / "project", finals=())
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text()
    if damage == "missing":
        evidence["library_searches"].clear()
        log = log.replace(f" Searching {spelling}:\n", "")
    elif damage == "different_directory":
        changed = "Z:\\other\\runtime.lib"
        evidence["library_searches"][0]["spelling"] = changed
        log = log.replace(spelling, changed)
    else:
        evidence["library_searches"][0]["path"] = None
    set_link(context, evidence, log)
    if damage == "no_physical_path":
        with pytest.raises(ValueError):
            observe(context)
    else:
        row, = observe(context)["modules"]
        assert row["result"] == "unknown" and row["candidates"] == []
        assert "input" not in row


def test_one_library_spelling_cannot_choose_between_distinct_physical_archives(tmp_path):
    from test_linked_modules import set_link
    context, path, spelling = archive_context(tmp_path / "project", finals=())
    other = context.directory / "another-runtime.lib"
    other.write_bytes(path.read_bytes())
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["explicit_inputs"].append(str(other))
    evidence["arguments"].append(str(other))
    evidence["files"].append({"path": str(other), "before": stamp(other.read_bytes()), "after": stamp(other.read_bytes())})
    evidence["library_searches"].append({"spelling": spelling, "path": str(other)})
    log = (context.directory / "link.log").read_text().replace(
        f" Searching {spelling}:\n", f" Searching {spelling}:\n Searching {spelling}:\n")
    set_link(context, evidence, log)
    row, = observe(context)["modules"]
    assert row["result"] == "unknown" and "input" not in row
    assert len(row["candidates"]) == 2
    assert {item["input"]["path"] for item in row["candidates"]} == {str(path), str(other)}
    assert len({item["object_sha256"] for item in row["candidates"]}) == 1  # Equal bytes do not merge provenance.


@pytest.mark.parametrize("damage", ["existed_before", "missing_stamp", "wrong_hash", "missing_generation",
                                    "missing_capture", "duplicate_final"])
def test_generated_export_cannot_use_missing_or_nonfresh_native_generation(tmp_path, damage):
    from test_linked_modules import fixture as base, add_export, set_link, set_modules as replace_modules
    context = base(tmp_path / "project")
    add_export(context)
    replace_modules(context, module(b"app.exp", b"app.exp"))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text()
    generated = evidence["generated_exports"][0]
    if damage == "existed_before": generated["existed_before"] = True
    elif damage == "missing_stamp": generated["after"] = None
    elif damage == "wrong_hash": generated["after"]["sha256"] = "0" * 64
    elif damage == "missing_capture": evidence["generated_exports"].clear()
    elif damage == "duplicate_final": log = log.replace("app.exp\n", "app.exp\napp.exp\n")
    else:
        log = log.replace("Start Generating .exp file\nStart Pass1\nEnd Pass1\nEnd Generating .exp file\n", "")
    set_link(context, evidence, log)
    with pytest.raises(ValueError):
        observe(context)


def test_fresh_export_not_used_in_final_pass_has_no_selected_module_identity(tmp_path):
    from test_linked_modules import fixture as base, add_export, set_link, set_modules as replace_modules
    context = base(tmp_path / "project")
    add_export(context)
    replace_modules(context, module(b"app.exp", b"app.exp"))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text().replace("app.exp\n", "")
    set_link(context, evidence, log)
    row, = observe(context)["modules"]
    assert row["result"] == "unknown" and row["candidates"] == [] and "input" not in row


def test_direct_observer_final_guard_detects_bool_for_int_report_substitution(tmp_path, monkeypatch):
    from match_recovery import linked_modules
    from match_recovery.linked_initializers import _artifacts
    from test_linked_modules import fixture as base
    context = base(tmp_path / "project")
    inputs = linked_context.parse_inputs(_artifacts(context))
    original = linked_modules.bound_contributions
    def changed(*args, **kwargs):
        result = original(*args, **kwargs)
        context.report["compile_link"]["compilations"][0]["compilation"]["exit_code"] = False
        return result
    monkeypatch.setattr(linked_modules, "bound_contributions", changed)
    with pytest.raises(ValueError, match="command report changed"):
        linked_modules.observe_modules(context, inputs)

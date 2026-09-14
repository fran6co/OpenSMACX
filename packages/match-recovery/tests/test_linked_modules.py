"""Physical selected module identities through real command/file observations."""

from copy import deepcopy
import json
from pathlib import Path
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.common_allocation import _option
from match_recovery.msf import SmallMsf
from test_binary import coff_fixture
from test_build_graph_commands import Context as Commands
from test_build_command_evidence import descriptor, STAGES
from test_coff_inventory import imported
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_msf import small_msf
from test_pdb_contribution_controls import native_context
from test_pdb_contributions import module
from test_toolchain_components import archive


def set_modules(context, modules_bytes):
    msf = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    dbi = streams[3]
    count, = struct.unpack_from("<I", dbi, 24)
    header = bytearray(dbi[:64])
    struct.pack_into("<I", header, 24, len(modules_bytes))
    streams[3] = bytes(header) + modules_bytes + dbi[64 + count:]
    update_artifact(context, "app.pdb", small_msf(streams))


def set_link(context, evidence, log):
    evidence["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(context, "link.log", log.encode())
    update_artifact(context, "link.json", json.dumps(evidence).encode())
    arguments = evidence["arguments"]
    context.report["compile_link"]["command_bindings"]["link"] = {
        "arguments": list(arguments), "cwd": evidence["cwd"], "result": "pass",
        "description": descriptor(arguments, STAGES["link"], [(i, 0, "input_file")
            for i, word in enumerate(arguments) if word in evidence["explicit_inputs"] or not _option(word)])}


def fixture(root):
    graph = Commands(root, ["src/unit.cpp"])
    value = native_context(root / "native")
    value.root = root
    output = value.objects[0]
    proof = deepcopy(graph.report["compile_link"])
    compiled, = proof["compilations"]
    compiled["output"] = str(output)
    compiled["compilation"]["output"].update(path=str(output), after=stamp(output.read_bytes()))
    args = [compiled["source"], "/Fo" + str(output)]
    compiled["compilation"]["arguments"] = args
    bound, = proof["command_bindings"]["compilations"]
    bound.update(output=str(output), arguments=args,
                 description=descriptor(args, STAGES["compile"], [(0, 0, "source"), (1, 3, "output_file")]))
    value.report["compile_link"] = proof
    evidence = json.loads((value.directory / "link.json").read_bytes())
    set_link(value, evidence, (value.directory / "link.log").read_text())
    set_modules(value, module(str(output).encode(), str(output).encode()))
    return value


def add_archive(context, members, finals):
    path = context.directory / "runtime.lib"
    path.write_bytes(archive(members))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["explicit_inputs"].append(str(path))
    evidence["arguments"].append(str(path))
    evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    spelling = "Z:\\sdk\\runtime.lib"
    evidence["library_searches"].append({"spelling": spelling, "path": str(path)})
    log = (context.directory / "link.log").read_text().replace("Start Pass1\n", f"Start Pass1\n Searching {spelling}:\n", 1)
    log = log.replace("End Pass2", "".join(f"RUNTIME.lib({name})\n" for name in finals) + "End Pass2")
    set_link(context, evidence, log)
    return path, spelling


def add_export(context):
    path = context.directory / "app.exp"
    path.write_bytes(coff_fixture())
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["generated_exports"].append({"path": str(path), "existed_before": False, "after": stamp(path.read_bytes())})
    log = (context.directory / "link.log").read_text().replace("Start Pass1\n", "Start Pass1\nStart Generating .exp file\nStart Pass1\nEnd Pass1\nEnd Generating .exp file\n", 1)
    log = log.replace("End Pass2", "app.exp\nEnd Pass2")
    set_link(context, evidence, log)
    return path


def test_standalone_uses_exact_operand_and_source_output_membership(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as live:
        report = live.module_bindings()
        row, = report["modules"]
        assert row["result"] == "pass" and row["kind"] == "standalone_object"
        assert row["input"] == {"path": str(context.objects[0])}
        assert row["owner"] == "unit0.obj" and row["source"] == "src/unit.cpp"
        assert row["object_sha256"] == stamp(context.objects[0].read_bytes())["sha256"]
        assert report["result"] == report["allocation_isolation"] == report["retained_storage_coverage"] == "unknown"
        assert not report["ready_for_collection"] and not report["persisted"]


@pytest.mark.parametrize("separator", ["/", "\\"])
def test_relative_operand_uses_declared_codec_not_pdb_path_resolution(tmp_path, separator):
    context = fixture(tmp_path / "project")
    evidence = json.loads((context.directory / "link.json").read_bytes())
    names = [separator.join(Path(path).relative_to(context.root).parts)
             for path in evidence["explicit_inputs"]]
    evidence["arguments"] = names
    set_link(context, evidence, (context.directory / "link.log").read_text())
    description = context.report["compile_link"]["command_bindings"]["link"]["description"]
    description["codecs"] = [{"anchor": str(context.root), "prefix": "", "separator": separator}]
    for row, path in zip(description["operands"], evidence["explicit_inputs"], strict=True):
        row["path"] = path
    set_modules(context, module(names[0].encode(), names[0].encode()))
    with linked_build(context) as live:
        row, = live.module_bindings()["modules"]
        assert row["result"] == "pass" and row["source"] == "src/unit.cpp"
        assert row["input"] == {"path": str(context.objects[0])}


@pytest.mark.parametrize("name", [b"unit0.obj", b"different/unit0.obj"])
def test_basename_and_different_directory_do_not_replace_exact_operand(tmp_path, name):
    context = fixture(tmp_path / "project")
    set_modules(context, module(name, name))
    with linked_build(context) as live:
        assert live.module_bindings()["modules"][0]["result"] == "unknown"


@pytest.mark.parametrize("pdb_name", [b"dir/part.obj", b"dir\\part.obj", b"DIR\\PART.OBJ", b"dir\\\\part.obj"])
def test_observed_windows_library_and_full_member_labels_bind_without_host_translation(tmp_path, pdb_name):
    context = fixture(tmp_path / "project")
    path, spelling = add_archive(context, [("dir/part.obj/", coff_fixture())], ["part.obj"])
    set_modules(context, module(pdb_name, spelling.encode()))
    with linked_build(context) as live:
        row, = live.module_bindings()["modules"]
        assert row["result"] == "pass" and row["kind"] == "archive_object"
        assert row["input"] == {"path": str(path), "member": "dir/part.obj", "occurrence": 0}
        assert row["owner"] == "RUNTIME:part.obj"


def test_archive_basename_does_not_choose_a_different_full_member_path(tmp_path):
    context = fixture(tmp_path / "project")
    _, spelling = add_archive(context, [("dir/part.obj/", coff_fixture())], ["part.obj"])
    set_modules(context, module(b"part.obj", spelling.encode()))
    with linked_build(context) as live:
        assert live.module_bindings()["modules"][0]["result"] == "unknown"


def test_ambiguous_import_family_keeps_both_physical_occurrences(tmp_path):
    context = fixture(tmp_path / "project")
    path, spelling = add_archive(context, [("DEMO.dll/", imported()), ("DEMO.dll/", imported("_Other@8"))], ["DEMO.dll", "DEMO.dll"])
    set_modules(context, module(b"DEMO.dll", spelling.encode()))
    with linked_build(context) as live:
        row, = live.module_bindings()["modules"]
        assert row["result"] == "unknown" and len(row["candidates"]) == 2
        assert {item["input"]["occurrence"] for item in row["candidates"]} == {0, 1}
        assert all(item["input"]["path"] == str(path) for item in row["candidates"])


def test_generated_export_requires_existing_fresh_native_selection(tmp_path):
    context = fixture(tmp_path / "project")
    path = add_export(context)
    set_modules(context, module(b"app.exp", b"app.exp"))
    with linked_build(context) as live:
        row, = live.module_bindings()["modules"]
        assert row["result"] == "pass" and row["kind"] == "generated_export"
        assert row["input"] == {"path": str(path), "generated": "msvc_export"}


def test_linker_synthetic_owner_never_becomes_a_physical_object(tmp_path):
    context = fixture(tmp_path / "project")
    set_modules(context, module(b"* Linker *", b""))
    with linked_build(context) as live:
        row, = live.module_bindings()["modules"]
        assert row["kind"] == "linker_generated" and row["result"] == "unknown"
        assert row["candidates"] == [] and "input" not in row

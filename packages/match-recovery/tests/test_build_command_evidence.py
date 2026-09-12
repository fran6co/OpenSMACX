"""File/role bindings with real fixture files, not compiler or graph acceptance."""

from copy import deepcopy
from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.build_command_evidence import bind_build_commands
from match_recovery.command_paths import validate_command_paths
from test_compile_link_evidence import build as compile_link_fixture, verify as verify_files


STAGES = {"compile": "compiler-effective", "link": "linker-effective"}


class Describer:
    def __init__(self, expected, descriptions):
        self.expected, self.descriptions = expected, descriptions
        self.calls = []

    def describe_commands(self, commands):
        assert commands == self.expected
        self.calls.append(deepcopy(commands))
        return tuple(validate_command_paths(item["arguments"], item["cwd"], description, stage=item["stage"])
                     for item, description in zip(commands, self.descriptions, strict=True))


def descriptor(arguments, stage, roles):
    return {"schema_version": 1, "stage": stage,
            "codecs": [{"anchor": "/", "prefix": "/", "separator": "/"}],
            "operands": [{"argument": index, "start": start, "end": len(arguments[index]),
                          "path": arguments[index][start:], "kind": kind, "codec": 0}
                         for index, start, kind in roles], "unsupported": []}


@pytest.fixture
def observed(tmp_path, compile_link_fixture):
    compilations, link, _ = compile_link_fixture
    directory, root = tmp_path, tmp_path.parent
    (directory / "include").mkdir()
    (directory / "debug").mkdir()
    compile = compilations[0]
    compile["arguments"] = ["-DKEEP", compile["source"], "/Fo" + compile["output"]["path"],
                            "/FI" + str(directory / "header.h"), "/I" + str(directory / "include"),
                            "/Fd" + str(directory / "debug")]
    link["arguments"] = [*link["explicit_inputs"], "/OUT:" + str(directory / "application.exe")]
    descriptions = [descriptor(compile["arguments"], STAGES["compile"],
                               [(1, 0, "source"), (2, 3, "output_file"), (3, 3, "input_file"),
                                (4, 2, "input_directory"), (5, 3, "output_directory")]),
                    descriptor(link["arguments"], STAGES["link"],
                               [(index, 0, "input_file") for index in range(len(link["explicit_inputs"]))]
                               + [(len(link["explicit_inputs"]), 5, "output_file")])]
    assert verify_files(compile_link_fixture)["result"] == "pass"
    return root, directory, compilations, link, descriptions


def client(observed):
    _, _, compilations, link, descriptions = observed
    requests = [{"arguments": row["arguments"], "cwd": row["cwd"], "stage": STAGES["compile"]}
                for row in compilations]
    requests.append({"arguments": link["arguments"], "cwd": link["cwd"], "stage": STAGES["link"]})
    return Describer(deepcopy(requests), descriptions)


def bind(observed, *, compiler=None):
    root, directory, compilations, link, _ = observed
    return bind_build_commands(root, directory, compilations, link, STAGES,
                               compiler=client(observed) if compiler is None else compiler)


def test_one_exact_batch_binds_source_output_and_ordered_link_files_without_acceptance(observed):
    before = deepcopy(observed)
    compiler = client(observed)
    report = bind(observed, compiler=compiler)
    assert report["result"] == "pass" and len(compiler.calls) == 1
    assert observed == before
    compilation, = report["compilations"]
    assert compilation["source"] == observed[2][0]["source"]
    assert compilation["output"] == observed[2][0]["output"]["path"]
    assert compilation["arguments"] == observed[2][0]["arguments"]
    assert report["link"]["arguments"] == observed[3]["arguments"]
    assert report["input_coverage"] == report["build_graph_verification"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["missing_source", "missing_output", "swapped", "wrong_source", "wrong_output", "duplicate_source", "duplicate_output"])
def test_roles_must_name_exactly_the_observed_source_and_output(observed, damage):
    _, directory, compilations, _, descriptions = observed
    compile, description = compilations[0], descriptions[0]
    source, output = description["operands"][:2]
    if damage == "missing_source": description["operands"].remove(source)
    elif damage == "missing_output": description["operands"].remove(output)
    elif damage == "swapped": source["kind"], output["kind"] = output["kind"], source["kind"]
    elif damage.startswith("duplicate"):
        original = source if damage == "duplicate_source" else output
        argument = compile["arguments"][original["argument"]]
        description["operands"].append({**original, "argument": len(compile["arguments"])})
        compile["arguments"].append(argument)
    else:
        item = source if damage == "wrong_source" else output
        path = directory / ("other.cc" if damage == "wrong_source" else "other.obj")
        prefix = "" if damage == "wrong_source" else "/Fo"
        compile["arguments"][item["argument"]] = prefix + str(path)
        item.update(path=str(path), end=len(compile["arguments"][item["argument"]]))
    with pytest.raises(ValueError, match="source/output"):
        bind(observed)


def test_input_file_requires_actual_compilation_observation(observed):
    _, directory, compilations, _, descriptions = observed
    path = directory / "unobserved-header.h"
    path.write_bytes(b"exists but no pre/post evidence")
    compilations[0]["arguments"][3] = "/FI" + str(path)
    descriptions[0]["operands"][2].update(path=str(path), end=len(compilations[0]["arguments"][3]))
    with pytest.raises(ValueError, match="no compilation observation"):
        bind(observed)


@pytest.mark.parametrize("command_index", [0, 1])
def test_every_declared_output_must_stay_in_this_fresh_build(observed, command_index):
    root, directory, compilations, link, descriptions = observed
    command = compilations[0] if command_index == 0 else link
    path = root / (directory.name + "-outside.obj")
    argument = "/Fd" + str(path)
    descriptions[command_index]["operands"].append({"argument": len(command["arguments"]), "start": 3,
        "end": len(argument), "path": str(path), "kind": "output_file", "codec": 0})
    command["arguments"].append(argument)
    with pytest.raises(ValueError, match="outside the fresh build"):
        bind(observed)


@pytest.mark.parametrize("kind", ["input_directory", "output_directory"])
@pytest.mark.parametrize("damage", ["file", "missing", "symlink"])
def test_directory_roles_need_real_canonical_directories(observed, kind, damage):
    _, directory, compilations, _, descriptions = observed
    item = next(row for row in descriptions[0]["operands"] if row["kind"] == kind)
    path = directory / ("bad-" + kind)
    if damage == "file": path.write_bytes(b"not a directory")
    elif damage == "symlink": path.symlink_to(directory / "include", target_is_directory=True)
    argument = compilations[0]["arguments"][item["argument"]][:item["start"]] + str(path)
    compilations[0]["arguments"][item["argument"]] = argument
    item.update(path=str(path), end=len(argument))
    with pytest.raises(ValueError, match="canonical physical directory"):
        bind(observed)


def test_output_directory_may_be_the_fresh_build_directory_itself(observed):
    _, directory, compilations, _, descriptions = observed
    item = next(row for row in descriptions[0]["operands"] if row["kind"] == "output_directory")
    argument = "/Fd" + str(directory)
    compilations[0]["arguments"][item["argument"]] = argument
    item.update(path=str(directory), end=len(argument))
    report = bind(observed)
    assert report["result"] == "pass"
    operands = report["compilations"][0]["description"]["operands"]
    assert [row["path"] for row in operands if row["kind"] == "output_directory"] == [str(directory)]
    assert report["build_graph_verification"] == "unknown" and not report["ready_for_collection"]


@pytest.mark.parametrize("field", ["arguments", "cwd"])
@pytest.mark.parametrize("command_index", [0, 1])
def test_returned_typed_commands_must_match_requested_arguments_and_cwd(observed, field, command_index):
    ordinary = client(observed)
    class Mismatched:
        def describe_commands(self, requests):
            values = list(ordinary.describe_commands(requests))
            original = values[command_index]
            changed = (*original.arguments, "-DUNREQUESTED") if field == "arguments" else observed[0]
            values[command_index] = replace(original, **{field: changed})
            return values
    with pytest.raises(ValueError, match="does not match the requested arguments/cwd"):
        bind(observed, compiler=Mismatched())


@pytest.mark.parametrize("damage", ["repeated", "out_of_order", "missing", "source_role"])
def test_link_operand_sequence_cannot_hide_order_multiplicity_or_source_substitution(observed, damage):
    _, _, _, link, descriptions = observed
    description = descriptions[1]
    if damage == "repeated":
        item = description["operands"][0]
        description["operands"].append({**item, "argument": len(link["arguments"])})
        link["arguments"].append(link["arguments"][item["argument"]])
    elif damage == "out_of_order":
        link["arguments"][0], link["arguments"][1] = link["arguments"][1], link["arguments"][0]
        for item in description["operands"][:2]:
            item.update(path=link["arguments"][item["argument"]], end=len(link["arguments"][item["argument"]]))
    elif damage == "missing": description["operands"].pop(0)
    else: description["operands"][0]["kind"] = "source"
    with pytest.raises(ValueError, match="(ordered explicit inputs|substitute a source)"):
        bind(observed)


@pytest.mark.parametrize("damage", ["dropped", "extra", "reordered", "not_batch", "not_typed", "wrong_stage", "forged_slice", "forged_overlap"])
def test_invalid_or_forged_descriptor_batch_cannot_supply_bindings(observed, damage):
    ordinary = client(observed)
    class Broken:
        def describe_commands(self, requests):
            values = list(ordinary.describe_commands(requests))
            if damage == "dropped": return values[:-1]
            if damage == "extra": return [*values, values[0]]
            if damage == "reordered": return list(reversed(values))
            if damage == "not_batch": return {"result": "pass"}
            if damage == "not_typed": values[0] = {"result": "pass"}
            elif damage == "wrong_stage": values[0] = replace(values[0], stage="other-stage")
            elif damage == "forged_slice":
                item = replace(values[0].operands[0], start=0, end=1)
                values[0] = replace(values[0], operands=(item, *values[0].operands[1:]))
            elif damage == "forged_overlap":
                values[0] = replace(values[0], operands=(values[0].operands[0], *values[0].operands))
            return values
    with pytest.raises(ValueError):
        bind(observed, compiler=Broken())

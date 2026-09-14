"""Invocation-local COFF reuse retains per-node checks and final native guards."""

from copy import deepcopy

import pytest

from match_recovery import linked_code_ownership as ownership
from match_recovery.application_residual import _owned_bodies
from match_recovery.pe_coff import Coff
from test_build_command_evidence import descriptor, STAGES
from test_build_graph_commands import Context, stamp
from test_linked_code_graph import graph_build, direct_graph, function


def fixture(root):
    root = root / "project"
    names = ("_f", "_g", "_h")
    context = graph_build(root, [function(name, index * 16) for index, name in enumerate(names)], roots=names)
    graph = direct_graph(context)
    assert graph["result"] == "pass", graph
    commands = Context(root, ["src/main.cpp"])
    proof = deepcopy(commands.report["compile_link"])
    output = context.objects[0]
    row, = proof["compilations"]
    args = [row["source"], "/Fo" + str(output)]
    row["output"] = str(output)
    row["compilation"].update(arguments=args)
    row["compilation"]["output"].update(path=str(output), after=stamp(output.read_bytes()))
    bound, = proof["command_bindings"]["compilations"]
    bound.update(output=str(output), arguments=args,
                 description=descriptor(args, STAGES["compile"], [(0, 0, "source"), (1, 3, "output_file")]))
    args = [str(path) for path in context.objects]
    proof["command_bindings"]["link"].update(arguments=args, cwd=str(context.directory),
        description=descriptor(args, STAGES["link"], [(index, 0, "input_file") for index in range(len(args))]))
    context.report["compile_link"] = proof
    original = context.check_current
    def fresh():
        original()
        commands.check_current()
    context.check_current = fresh
    context.commands = commands
    return context, graph, {"src/main.cpp": {name: f"entity-{index}" for index, name in enumerate(names)}}


def count_parses(monkeypatch):
    original = Coff.__init__
    calls = []
    def counted(self, raw):
        calls.append(raw)
        original(self, raw)
    monkeypatch.setattr(Coff, "__init__", counted)
    return calls


def test_producer_index_parses_once_per_physical_input_per_invocation(tmp_path, monkeypatch):
    context, graph, _ = fixture(tmp_path)
    calls = count_parses(monkeypatch)
    before = ownership.identities(context, graph["nodes"])
    assert len(before) == 3 and len(calls) == 1
    assert ownership.identities(context, graph["nodes"]) == before
    assert len(calls) == 2  # No parsed object or verdict survives the invocation.


def test_owned_bodies_reuse_parsed_object_but_keep_each_function_binding(tmp_path, monkeypatch):
    context, graph, bindings = fixture(tmp_path)
    calls = count_parses(monkeypatch)
    before = _owned_bodies(context, graph, bindings)
    assert len(before) == 3
    assert {row[0]["symbol"] for row in before} == {"_f", "_g", "_h"}
    assert len(calls) == 2  # One producer-index parse and one private scope parse.
    assert _owned_bodies(context, graph, bindings) == before
    assert len(calls) == 4


@pytest.mark.parametrize("consumer", ["identities", "owned"])
@pytest.mark.parametrize("damage", ["sha", "symbol"])
def test_later_node_cannot_borrow_earlier_parse_to_skip_its_exact_checks(tmp_path, consumer, damage):
    context, graph, bindings = fixture(tmp_path)
    if damage == "sha":
        graph["nodes"][-1]["object_sha256"] = "0" * 64
    else:
        graph["nodes"][-1]["symbol"] = graph["nodes"][-1]["proof"]["symbol"] = "_absent"
    with pytest.raises(ValueError):
        if consumer == "identities":
            ownership.identities(context, graph["nodes"])
        else:
            _owned_bodies(context, graph, bindings)


@pytest.mark.parametrize("consumer", ["identities", "owned"])
@pytest.mark.parametrize("damage", ["physical", "context"])
def test_late_damage_after_warm_parse_refuses_final_guard(tmp_path, monkeypatch, consumer, damage):
    context, graph, bindings = fixture(tmp_path)
    original = ownership._contents
    calls = []
    def changed(*args):
        raw = original(*args)
        calls.append(True)
        if len(calls) == (3 if consumer == "identities" else 6):
            if damage == "physical":
                path = context.objects[0]
                path.write_bytes(path.read_bytes() + b"late corruption")
            else:
                context.commands.fail_at = context.commands.checks + 1
        return raw
    monkeypatch.setattr(ownership, "_contents", changed)
    with pytest.raises(ValueError):
        if consumer == "identities":
            ownership.identities(context, graph["nodes"])
        else:
            _owned_bodies(context, graph, bindings)

"""Command-description protocol controls using local Python clients, no compiler."""

from copy import deepcopy
import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.command_paths import CommandPaths, compare_commands
from match_recovery.compiler import Compiler
from test_compiler import compiler_project


def requests(root):
    return [{"arguments": ["/I" + str(root / "include"), "-DKEEP", "unit.cc"],
             "cwd": str(root / "code"), "stage": "compiler-effective"},
            {"arguments": ["-O2", "/I" + str(root / "other-include"), "other.cc"],
             "cwd": str(root / "code"), "stage": "compiler-effective"}]


def descriptions(root):
    result = []
    for index, request in enumerate(requests(root)):
        include_index = index
        include = root / ("include" if index == 0 else "other-include")
        result.append({"schema_version": 1, "stage": request["stage"],
                       "codecs": [{"anchor": "/", "prefix": "/", "separator": "/"},
                                  {"anchor": str(root / "code"), "prefix": "", "separator": "/"}],
                       "operands": [{"argument": include_index, "start": 2,
                                     "end": len(request["arguments"][include_index]), "path": str(include),
                                     "kind": "input_directory", "codec": 0},
                                    {"argument": 2, "start": 0, "end": len(request["arguments"][2]),
                                     "path": str(root / "code" / request["arguments"][2]), "kind": "source", "codec": 1}],
                       "unsupported": []})
    return result


def install_client(root, *, response=None, prefix="", exit_code=0, profile="custom-profile"):
    request = {"schema_version": 1, "operation": "command_paths", "profile": profile, "commands": requests(root)}
    response = response if response is not None else {"schema_version": 1, "descriptions": descriptions(root)}
    script = ('import json,sys\nfrom pathlib import Path\nvalue=json.load(sys.stdin)\n'
              + f'assert value == {request!r}, value\n' + prefix + '\n'
              + f'print({json.dumps(response)!r})\nraise SystemExit({exit_code})\n')
    (root / "recovery/compiler.py").write_text(script)
    return Compiler(root, profile=profile)


def test_exact_request_roundtrip_returns_validated_command_values_without_mutation(compiler_project):
    root = compiler_project
    compiler = install_client(root)
    commands = requests(root)
    original = deepcopy(commands)
    before = {path.relative_to(root): path.read_bytes() for path in root.rglob("*") if path.is_file()}
    result = compiler.describe_commands(commands)
    assert isinstance(result, tuple) and len(result) == 2
    assert all(isinstance(row, CommandPaths) for row in result)
    for expected, actual in zip(original, result, strict=True):
        assert actual.arguments == tuple(expected["arguments"])
        assert actual.cwd == Path(expected["cwd"]) and actual.stage == expected["stage"]
        assert compare_commands(actual, actual)["result"] == "pass"
    assert commands == original
    assert {path.relative_to(root): path.read_bytes() for path in root.rglob("*") if path.is_file()} == before
    commands[0]["arguments"][1] = "-DMUTATED"
    assert result[0].arguments[1] == "-DKEEP"


@pytest.mark.parametrize("damage", ["reorder", "dropped", "extra_description", "stage", "slice", "unsupported",
                                     "verdict", "description_verdict", "bool_version", "wrong_version",
                                     "not_array", "missing_descriptions", "object_response"])
def test_partial_reordered_or_verdict_responses_refuse_without_compilation(compiler_project, monkeypatch, damage):
    root = compiler_project
    response = {"schema_version": 1, "descriptions": descriptions(root)}
    if damage == "reorder": response["descriptions"].reverse()
    elif damage == "dropped": response["descriptions"].pop()
    elif damage == "extra_description": response["descriptions"].append(deepcopy(response["descriptions"][0]))
    elif damage == "stage": response["descriptions"][0]["stage"] = "link-wrapper"
    elif damage == "slice": response["descriptions"][0]["operands"][0]["start"] = 0
    elif damage == "unsupported": response["descriptions"][1]["unsupported"] = ["unexpanded response file"]
    elif damage == "verdict": response["result"] = "pass"
    elif damage == "description_verdict": response["descriptions"][0]["verified"] = True
    elif damage == "bool_version": response["schema_version"] = True
    elif damage == "wrong_version": response["schema_version"] = 2
    elif damage == "not_array": response["descriptions"] = {}
    elif damage == "missing_descriptions": del response["descriptions"]
    else: response = {"object_base64": "b2JqZWN0"}
    compiler = install_client(root, response=response)
    monkeypatch.setattr(compiler, "_compile", lambda *a, **k: pytest.fail("description errors cannot fall back to compilation"))
    with pytest.raises(ValueError):
        compiler.describe_commands(requests(root))


@pytest.mark.parametrize("code", [1, 3, 17])
def test_nonzero_client_exit_refuses_even_valid_descriptions(compiler_project, code):
    compiler = install_client(compiler_project, exit_code=code)
    with pytest.raises(ValueError):
        compiler.describe_commands(requests(compiler_project))


def test_timeout_is_unmeasurable_not_compilation_fallback(compiler_project, monkeypatch):
    root = compiler_project
    configuration = root / "recovery/project.json"
    value = json.loads(configuration.read_text())
    value["compiler"]["command"]["timeout"] = 1
    configuration.write_text(json.dumps(value))
    compiler = install_client(root, prefix="import time\ntime.sleep(30)\n")
    monkeypatch.setattr(compiler, "_compile", lambda *a, **k: pytest.fail("timeout cannot fall back to compilation"))
    with pytest.raises(ValueError, match="(complete|timeout|timed out)"):
        compiler.describe_commands(requests(root))


@pytest.mark.parametrize("failure", ["unavailable", "malformed_json"])
def test_unavailable_or_malformed_client_refuses_without_fallback(compiler_project, monkeypatch, failure):
    root = compiler_project
    if failure == "unavailable":
        path = root / "recovery/project.json"
        value = json.loads(path.read_text())
        value["compiler"]["command"]["argv"] = [str(root / "missing-client")]
        path.write_text(json.dumps(value))
    else:
        (root / "recovery/compiler.py").write_text("print('not a JSON description')\n")
    compiler = Compiler(root)
    monkeypatch.setattr(compiler, "_compile", lambda *a, **k: pytest.fail("client error cannot invoke compilation"))
    with pytest.raises(ValueError):
        compiler.describe_commands(requests(root))


@pytest.mark.parametrize("damage", ["helper", "configuration", "environment"])
def test_changed_compiler_inputs_refuse_before_client_execution(compiler_project, monkeypatch, damage):
    root = compiler_project
    compiler = install_client(root)
    if damage == "helper":
        path = root / "recovery/compiler.py"
        path.write_text(path.read_text() + "\n# changed helper\n")
    elif damage == "configuration":
        path = root / "recovery/project.json"
        value = json.loads(path.read_text())
        value["compiler"]["environment"].append("NEW_COMPILER_INPUT")
        path.write_text(json.dumps(value))
    else:
        monkeypatch.setenv("FIXTURE_COMPILER_OPTION", "changed-parent-environment")
    monkeypatch.setattr("match_recovery.compiler.invoke", lambda *a, **k: pytest.fail("stale compiler reached client"))
    with pytest.raises(ValueError, match="changed"):
        compiler.describe_commands(requests(root))


@pytest.mark.parametrize("damage", ["helper", "configuration", "declared_input"])
@pytest.mark.parametrize("client_failure", [False, True])
def test_client_mutations_refuse_on_success_and_failure_without_restoring(compiler_project, damage, client_failure):
    root = compiler_project
    if damage == "declared_input":
        path = root / "recovery/project.json"
        value = json.loads(path.read_text())
        value["compiler"]["inputs"].append("code/unit.cc")
        path.write_text(json.dumps(value))
    target = {"helper": "recovery/compiler.py", "configuration": "recovery/project.json",
              "declared_input": "code/unit.cc"}[damage]
    compiler = install_client(root, prefix=f'p=Path({target!r}); p.write_bytes(p.read_bytes()+b"\\n ")',
                              exit_code=3 if client_failure else 0)
    original = (root / target).read_bytes()
    with pytest.raises(ValueError, match="changed"):
        compiler.describe_commands(requests(root))
    assert (root / target).read_bytes() == original + b"\n "


@pytest.mark.parametrize("failure", [False, True])
def test_parent_environment_freshness_is_checked_after_return_and_exceptions(compiler_project, monkeypatch, failure):
    root = compiler_project
    compiler = install_client(root)
    def invoke(*args, **kwargs):
        monkeypatch.setenv("FIXTURE_COMPILER_OPTION", "changed-during-protocol")
        if failure:
            raise subprocess.TimeoutExpired(["local-client"], 1)
        return 0, {"schema_version": 1, "descriptions": descriptions(root)}
    monkeypatch.setattr("match_recovery.compiler.invoke", invoke)
    with pytest.raises(ValueError, match="changed"):
        compiler.describe_commands(requests(root))


@pytest.mark.parametrize("damage", ["empty", "not_list", "not_object", "extra", "missing", "arguments_string",
                                     "arguments_empty", "arguments_nul", "argument_type", "stage_empty", "stage_type",
                                     "cwd_relative", "cwd_outside", "cwd_dotdot", "cwd_noncanonical"])
def test_invalid_requests_cannot_launch_client(compiler_project, monkeypatch, damage):
    root = compiler_project
    compiler = install_client(root)
    commands = requests(root)
    if damage == "empty": commands = []
    elif damage == "not_list": commands = tuple(commands)
    elif damage == "not_object": commands[0] = []
    elif damage == "extra": commands[0]["trust"] = True
    elif damage == "missing": del commands[0]["stage"]
    elif damage == "arguments_string": commands[0]["arguments"] = "compiler"
    elif damage == "arguments_empty": commands[0]["arguments"] = []
    elif damage == "arguments_nul": commands[0]["arguments"] = ["bad\0argument"]
    elif damage == "argument_type": commands[0]["arguments"] = [3]
    elif damage == "stage_empty": commands[0]["stage"] = ""
    elif damage == "stage_type": commands[0]["stage"] = 5
    elif damage == "cwd_relative": commands[0]["cwd"] = "code"
    elif damage == "cwd_outside": commands[0]["cwd"] = str(root.parent)
    elif damage == "cwd_dotdot": commands[0]["cwd"] = str(root / "code") + "/../code"
    else:
        commands[0]["cwd"] = str(root / "code") + "//child"
    monkeypatch.setattr("match_recovery.compiler.invoke", lambda *a, **k: pytest.fail("invalid request reached client"))
    with pytest.raises(ValueError):
        compiler.describe_commands(commands)

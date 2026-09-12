"""Bind configured command syntax to freshly observed build file identities."""

from pathlib import Path

from .command_paths import CommandPaths, validate_command_paths
from .compiler import Compiler
from .model import Result


def bind_build_commands(root, directory, compilations, link, stages, *, compiler=None):
    """Describe one batch, retaining exact source/output/link bindings and argv.

    Called within the collector's immutable file epoch. Directory checks establish
    physical locations, not a complete include or library-search inventory. No
    project-provided normalized commands or comparison verdicts are consumed.
    """
    root, directory = Path(root), Path(directory)
    requests = [{"arguments": item["arguments"], "cwd": item["cwd"], "stage": stages["compile"]}
                for item in compilations]
    requests.append({"arguments": link["arguments"], "cwd": link["cwd"], "stage": stages["link"]})
    client = compiler if compiler is not None else Compiler(root)
    descriptions = client.describe_commands(requests)
    if not isinstance(descriptions, (list, tuple)) or len(descriptions) != len(requests):
        raise ValueError("command descriptions do not cover the exact build batch")
    commands = []
    for request, described in zip(requests, descriptions, strict=True):
        if not isinstance(described, CommandPaths):
            raise ValueError("build command description is not a validated syntax value")
        if described.arguments != tuple(request["arguments"]) or described.cwd != Path(request["cwd"]):
            raise ValueError("build command description does not match the requested arguments/cwd")
        command = validate_command_paths(request["arguments"], request["cwd"], described.to_description(),
                                         stage=request["stage"])
        for item in command.operands:
            if item.kind in ("input_directory", "output_directory"):
                if item.path.resolve() != item.path or not item.path.is_dir():
                    raise ValueError("command directory is not a canonical physical directory")
            if item.kind in ("output_file", "output_directory"):
                if (not item.path.is_relative_to(directory)
                        or item.path.resolve() != item.path or not item.path.parent.is_dir()
                        or item.kind == "output_file" and item.path.exists() and not item.path.is_file()):
                    raise ValueError("command output is outside the fresh build or has the wrong kind")
        commands.append(command)
    rows = []
    for value, command in zip(compilations, commands[:-1], strict=True):
        source, output = value["source"], value["output"]["path"]
        actual_sources = [str(item.path) for item in command.operands if item.kind == "source"]
        outputs = [str(item.path) for item in command.operands if item.kind == "output_file"]
        inputs = {item["path"] for item in value["inputs"]}
        if actual_sources != [source] or outputs.count(output) != 1:
            raise ValueError("command source/output roles do not bind the actual compilation")
        if any(str(item.path) not in inputs for item in command.operands if item.kind == "input_file"):
            raise ValueError("command input file has no compilation observation")
        rows.append({"source": source, "output": output, "arguments": list(command.arguments),
                     "cwd": str(command.cwd), "description": command.to_description(), "result": Result.PASS})
    linker = commands[-1]
    if any(item.kind == "source" for item in linker.operands):
        raise ValueError("link command cannot substitute a source for observed object inputs")
    if [str(item.path) for item in linker.operands if item.kind == "input_file"] != link["explicit_inputs"]:
        raise ValueError("link command operands do not bind the exact ordered explicit inputs")
    return {"check": "build_command_file_bindings", "result": Result.PASS,
            "compilations": rows, "link": {"arguments": list(linker.arguments), "cwd": str(linker.cwd),
                                            "description": linker.to_description(), "result": Result.PASS},
            "input_coverage": Result.UNKNOWN, "build_graph_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["fresh file binding does not authorize path pairs or compare two builds",
                            "project-owned path grammar; directories are physical locations, not full search closure",
                            "no linked section, initializer, common allocation or runtime proof"]}

"""Live application membership/command comparison, never linked-effect proof."""

from collections import Counter
from pathlib import Path

from .build_check import Build
from .command_paths import compare_commands, validate_command_paths
from .components import relative_path
from .link_file_evidence import _path, _stamp
from .model import Result


def _pass(value):
    if not isinstance(value, dict) or value.get("result") != Result.PASS:
        raise ValueError("build graph commands require fresh successful native evidence")
    return value


def _unique(rows, key):
    if not isinstance(rows, list):
        raise ValueError("build graph evidence requires row arrays")
    result = {}
    for row in rows:
        _pass(row)
        name = row.get(key)
        _path(name)
        if name in result:
            raise ValueError("duplicate build graph " + key)
        result[name] = row
    return result


def _command(row, stage):
    _pass(row)
    return validate_command_paths(row["arguments"], row["cwd"], row["description"], stage=stage)


def _context(context):
    root, directory = Path(context.root), Path(context.directory)
    for path in (root, directory):
        if not path.is_absolute() or path.resolve() != path or not path.is_dir():
            raise ValueError("build graph needs canonical live context directories")
    if directory == root or not directory.is_relative_to(root):
        raise ValueError("build graph directory must be inside its private project")
    policy = Build.read(root).compile_link
    if policy is None or "command_stages" not in policy:
        raise ValueError("build graph requires declared command observation stages")
    report = _pass(context.report)
    proof = _pass(report.get("compile_link"))
    binding = _pass(proof.get("command_bindings"))
    if proof.get("source_membership") != Result.PASS or proof.get("missing_sources") or proof.get("unexpected_sources"):
        raise ValueError("build graph needs exact successful source membership")
    sources = proof.get("sources")
    if not isinstance(sources, list) or not sources:
        raise ValueError("build graph needs unique nonempty source membership")
    for source in sources:
        _path(source)
    if len(set(sources)) != len(sources):
        raise ValueError("build graph needs unique nonempty source membership")
    compiled = _unique(proof.get("compilations"), "source")
    bound = _unique(binding.get("compilations"), "source")
    if set(sources) != compiled.keys() or compiled.keys() != bound.keys():
        raise ValueError("source and bound compilation membership disagree")
    outputs, rows = set(), {}
    for source in sources:
        source_path = Path(_path(source))
        if (not source_path.is_relative_to(root) or source_path.is_relative_to(directory)
                or source_path.resolve() != source_path or not source_path.is_file()):
            raise ValueError("application source is outside source context or generated in its build")
        row, declared = compiled[source], bound[source]
        compilation = _pass(row.get("compilation"))
        if compilation.get("output_provenance") == "postprocessed" or compilation.get("pipeline", {}).get("stages"):
            raise ValueError("postprocessed compiler output requires unsupported ordered pipeline command comparison")
        output = Path(_path(row.get("output")))
        if (output in outputs or not output.is_relative_to(directory) or output.resolve() != output
                or not output.is_file()):
            raise ValueError("application output must be a unique physical current-build file")
        outputs.add(output)
        captured_output = _pass(compilation.get("output"))
        if (declared.get("output") != str(output) or captured_output.get("path") != str(output)
                or compilation.get("source") != source or row.get("link_correspondence") != Result.PASS
                or captured_output.get("existed_before") is not False or compilation.get("exit_code") != 0):
            raise ValueError("source/output evidence identities disagree")
        command = _command(declared, policy["command_stages"]["compile"])
        if (list(command.arguments) != compilation.get("arguments")
                or str(command.cwd) != _pass(compilation.get("cwd")).get("path")):
            raise ValueError("bound compile command differs from actual observation")
        if ([str(item.path) for item in command.operands if item.kind == "source"] != [source]
                or [item.path for item in command.operands if item.kind == "output_file"].count(output) != 1):
            raise ValueError("compile source/output operand roles disagree")
        tool = _path(compilation.get("tool"))
        inputs = _unique(compilation.get("inputs"), "path")
        if tool not in inputs or source not in inputs:
            raise ValueError("actual source/compiler tool lacks file identity")
        tool_row = inputs[tool]
        for stamp in (tool_row.get("before"), tool_row.get("after")):
            _stamp(stamp)
        if tool_row.get("before") is None or tool_row["before"] != tool_row["after"]:
            raise ValueError("compiler tool identity is missing or changed")
        if any(str(item.path) not in inputs for item in command.operands if item.kind == "input_file"):
            raise ValueError("compile input operand has no observation")
        rows[source_path.relative_to(root).as_posix()] = {
            "command": command, "output": output, "tool": Path(tool), "tool_stamp": tool_row["after"]}
    link = _command(binding.get("link"), policy["command_stages"]["link"])
    _pass(proof.get("link_files"))
    if any(item.kind == "source" for item in link.operands):
        raise ValueError("link command contains source operands")
    linked = Counter(item.path for item in link.operands if item.kind == "input_file")
    if any(linked[output] != 1 for output in outputs):
        raise ValueError("application object must occur exactly once in link operands")
    return root, directory, rows, link


def _project_link(command, outputs):
    """Drop only independently bound whole added/removed object arguments."""
    removed, indices = [], set()
    for output in sorted(outputs):
        matches = [item for item in command.operands if item.path == output]
        if len(matches) != 1:
            raise ValueError("changed object operand is missing or ambiguous")
        item = matches[0]
        if (item.kind != "input_file" or item.start != 0 or item.end != len(command.arguments[item.argument])
                or sum(other.argument == item.argument for other in command.operands) != 1):
            raise ValueError("changed object must occupy one complete link argument")
        indices.add(item.argument)
        removed.append({"argument": item.argument, "text": command.arguments[item.argument], "output": str(output)})
    kept = [index for index in range(len(command.arguments)) if index not in indices]
    shifted = {index: number for number, index in enumerate(kept)}
    description = command.to_description()
    description["operands"] = [{**item, "argument": shifted[item["argument"]]}
                               for item in description["operands"] if item["argument"] not in indices]
    projected = validate_command_paths([command.arguments[index] for index in kept], command.cwd,
                                       description, stage=command.stage)
    return projected, sorted(removed, key=lambda item: item["argument"])


def compare_build_commands(baseline, current, *, editable):
    """Compare live actual commands; edit authority does not own emitted effects.

    The caller owns task/history/policy authorization and keeps both native build
    contexts alive. A PASS here cannot waive residual code, CRT initializers,
    data/common allocation, dependencies, references or linked selection.
    """
    def fresh():
        # Check both even when the first refuses; neither epoch can be refreshed.
        try:
            baseline.check_current()
        finally:
            current.check_current()

    try:
        fresh()
        if not isinstance(editable, (tuple, list, set, frozenset)):
            raise ValueError("editable source paths require an explicit collection")
        editable = {relative_path(name) for name in editable}
        old_root, old_build, old, old_link = _context(baseline)
        new_root, new_build, new, new_link = _context(current)
        pairs = ((old_root, new_root), (old_build, new_build))
        added, removed = sorted(new.keys() - old.keys()), sorted(old.keys() - new.keys())
        if (set(added) | set(removed)) - editable:
            raise ValueError("changed application source membership exceeds editable task scope")

        def identity(path, side):
            for index in (1, 0):
                if path.is_relative_to(pairs[index][side]):
                    return index, path.relative_to(pairs[index][side]).as_posix()
            return "literal", str(path)

        rows = []
        for source in sorted(old.keys() & new.keys()):
            left, right = old[source], new[source]
            command = compare_commands(left["command"], right["command"], path_pairs=pairs)
            tool = (identity(left["tool"], 0) == identity(right["tool"], 1)
                    and left["tool_stamp"] == right["tool_stamp"])
            rows.append({"source": source, "command": command, "compiler_identity": Result.PASS if tool else Result.FAIL,
                         "result": command["result"] if tool else Result.FAIL})
        old_link, deleted = _project_link(old_link, {old[name]["output"] for name in removed})
        new_link, inserted = _project_link(new_link, {new[name]["output"] for name in added})
        link = compare_commands(old_link, new_link, path_pairs=pairs)
        results = [link["result"], *(row["result"] for row in rows)]
        return {"check": "application_build_graph_commands", "result": Result.PASS if all(item == Result.PASS for item in results) else Result.FAIL,
                "sources": {"common": sorted(old.keys() & new.keys()), "added": added, "removed": removed},
                "compilations": rows, "link": link,
                "removed_operands": {"baseline": deleted, "current": inserted},
                "linked_effects": Result.UNKNOWN, "input_coverage": Result.UNKNOWN,
                "persisted": False, "ready_for_collection": False}
    except (KeyError, TypeError, AttributeError) as error:
        raise ValueError("incomplete or malformed native build command evidence") from error
    finally:
        fresh()

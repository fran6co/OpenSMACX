"""Lossless command comparison under explicitly paired host paths.

Compiler option parsing belongs to the configured environment. Its description
may identify path slices, never remove flags or supply a normalized verdict.
These syntax checks do not prove compiler execution, input closure or behavior.
"""

from dataclasses import dataclass
from pathlib import Path

from .model import Result


def _path(value):
    if (not isinstance(value, str) or not value or "\0" in value or "\\" in value
            or not Path(value).is_absolute() or str(Path(value)) != value or ".." in Path(value).parts):
        raise ValueError("command paths require normalized absolute host paths")
    return Path(value)


def _text(value, *, empty=False):
    if not isinstance(value, str) or not empty and not value or "\0" in value:
        raise ValueError("invalid command text")
    return value


@dataclass(frozen=True)
class PathEncoding:
    anchor: Path
    prefix: str
    separator: str

    def encode(self, path):
        if not path.is_relative_to(self.anchor):
            raise ValueError("command path is outside its encoding anchor")
        return self.prefix + self.separator.join(path.relative_to(self.anchor).parts)


@dataclass(frozen=True)
class PathOperand:
    argument: int
    start: int
    end: int
    path: Path
    kind: str
    encoding: PathEncoding


@dataclass(frozen=True)
class CommandPaths:
    arguments: tuple[str, ...]
    cwd: Path
    stage: str
    operands: tuple[PathOperand, ...]


def validate_command_paths(arguments, cwd, description, *, stage):
    """Validate exact argument slices and reversible configured path spellings.

    No filesystem reads or inferred option semantics: the caller must bind source,
    output and directory roles to its fresh build evidence and policy. Unknown
    grammar cannot contribute partial correspondence. Anchors encode paths only;
    they never authorize path correspondence between two builds.
    """
    if not isinstance(arguments, (tuple, list)) or not arguments:
        raise ValueError("command arguments must be a nonempty sequence")
    arguments = tuple(_text(word) for word in arguments)
    cwd = _path(str(cwd))
    _text(stage)
    fields = {"schema_version", "stage", "codecs", "operands", "unsupported"}
    if (not isinstance(description, dict) or set(description) != fields
            or type(description["schema_version"]) is not int or description["schema_version"] != 1
            or description["stage"] != stage):
        raise ValueError("invalid command path description or mismatched argument stage")
    if any(not isinstance(description[key], list) for key in ("codecs", "operands", "unsupported")):
        raise ValueError("command path description arrays are required")
    for problem in description["unsupported"]:
        _text(problem)
    if description["unsupported"]:
        raise ValueError("unsupported command path grammar: " + "; ".join(description["unsupported"]))
    codecs = []
    for value in description["codecs"]:
        if not isinstance(value, dict) or set(value) != {"anchor", "prefix", "separator"}:
            raise ValueError("invalid command path encoding")
        anchor = _path(value["anchor"])
        prefix = _text(value["prefix"], empty=True)
        if value["separator"] not in ("/", "\\"):
            raise ValueError("command path separator must be explicit")
        # A nonempty prefix represents a root spelling, never part of the
        # first filename component. This makes the encoding unambiguous.
        if prefix and not prefix.endswith(value["separator"]):
            raise ValueError("command path prefix must end at a directory boundary")
        codecs.append(PathEncoding(anchor, prefix, value["separator"]))
    operands = []
    for value in description["operands"]:
        if not isinstance(value, dict) or set(value) != {"argument", "start", "end", "path", "kind", "codec"}:
            raise ValueError("invalid command path operand")
        if any(type(value[key]) is not int for key in ("argument", "start", "end", "codec")):
            raise ValueError("command operand indices must be integers")
        index, start, end, codec = (value[key] for key in ("argument", "start", "end", "codec"))
        if (not 0 <= index < len(arguments) or not 0 <= start < end <= len(arguments[index])
                or not 0 <= codec < len(codecs)):
            raise ValueError("command path operand is outside its argument or codec")
        if value["kind"] not in ("source", "input_file", "input_directory", "output_file", "output_directory"):
            raise ValueError("unsupported command path role")
        path, encoding = _path(value["path"]), codecs[codec]
        if encoding.encode(path) != arguments[index][start:end]:
            raise ValueError("command path does not reproduce its exact argument slice")
        operands.append(PathOperand(index, start, end, path, value["kind"], encoding))
    operands.sort(key=lambda item: (item.argument, item.start, item.end))
    for left, right in zip(operands, operands[1:]):
        if left.argument == right.argument and left.end > right.start:
            raise ValueError("command path operands overlap")
    return CommandPaths(arguments, cwd, stage, tuple(operands))


def compare_commands(before, after, *, path_pairs=()):
    """Compare every token/character except declared, exactly encoded path slices.

    Each explicit pair identifies corresponding file or directory paths supplied
    by the enclosing verifier, not the project parser. Descendants preserve their
    full relative names; nested pairs use the most specific anchor. Codec syntax
    and path roles remain part of equality. No token can be dropped by this API.
    """
    if not isinstance(before, CommandPaths) or not isinstance(after, CommandPaths):
        raise ValueError("command comparison requires validated command paths")
    # Frozen dataclasses are convenient values, not proof of validation. A
    # caller can construct or replace them directly; check exact slices again
    # before any typed path segment can replace command text.
    def checked(command):
        if not isinstance(command.operands, (tuple, list)):
            raise ValueError("command operands must be a sequence")
        codecs, operands = [], []
        for item in command.operands:
            if not isinstance(item, PathOperand) or not isinstance(item.encoding, PathEncoding):
                raise ValueError("invalid typed command operand")
            operands.append({"argument": item.argument, "start": item.start, "end": item.end,
                             "path": str(item.path), "kind": item.kind, "codec": len(codecs)})
            codecs.append({"anchor": str(item.encoding.anchor), "prefix": item.encoding.prefix,
                           "separator": item.encoding.separator})
        return validate_command_paths(command.arguments, command.cwd,
            {"schema_version": 1, "stage": command.stage, "codecs": codecs,
             "operands": operands, "unsupported": []}, stage=command.stage)
    before, after = checked(before), checked(after)
    pairs = []
    for pair in path_pairs:
        if not isinstance(pair, (list, tuple)) or len(pair) != 2:
            raise ValueError("path correspondence requires explicit pairs")
        pairs.append(tuple(_path(str(path)) for path in pair))
    if any(len({pair[side] for pair in pairs}) != len(pairs) for side in (0, 1)):
        raise ValueError("ambiguous path correspondence anchors")

    def identity(path, side):
        matches = [(len(pair[side].parts), index, pair[side]) for index, pair in enumerate(pairs)
                   if path.is_relative_to(pair[side])]
        if not matches:
            return ("literal_path", str(path))
        _, index, anchor = max(matches)
        return ("paired_path", index, path.relative_to(anchor).as_posix())

    def represented(command, side):
        by_argument = {}
        for operand in command.operands:
            by_argument.setdefault(operand.argument, []).append(operand)
        result = []
        for index, word in enumerate(command.arguments):
            segments, start = [], 0
            for item in by_argument.get(index, ()):
                segments.append(("literal", word[start:item.start]))
                segments.append(("path", item.kind, identity(item.path, side),
                                 identity(item.encoding.anchor, side), item.encoding.prefix, item.encoding.separator))
                start = item.end
            segments.append(("literal", word[start:]))
            result.append(tuple(segments))
        return tuple(result)

    old, new = represented(before, 0), represented(after, 1)
    differences = []
    if before.stage != after.stage:
        differences.append({"field": "stage"})
    if identity(before.cwd, 0) != identity(after.cwd, 1):
        differences.append({"field": "cwd"})
    if len(old) != len(new):
        differences.append({"field": "argument_count", "before": len(old), "after": len(new)})
    differences += [{"field": "argument", "index": index} for index, (left, right) in
                    enumerate(zip(old, new)) if left != right]
    return {"check": "lossless_command_path_comparison", "result": Result.FAIL if differences else Result.PASS,
            "differences": differences, "argument_counts": {"before": len(old), "after": len(new)},
            "input_coverage": Result.UNKNOWN, "build_graph_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["trusted project-owned path grammar; no compiler execution or input-closure proof",
                            "path pairs require independent authorization and fresh source/object/build bindings",
                            "equal command syntax does not prove linked selection, lifecycle or behavior"]}

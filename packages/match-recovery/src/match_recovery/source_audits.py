"""Configured source-only ratchets; observations are recomputed, never banked.

Ceilings are project policy, not proof of semantic correctness. The versioned
pattern lens deliberately preserves line-wise matching (including its limited
comment handling); changing that lens requires a separately reviewed rebaseline.
"""

from collections import Counter
from hashlib import sha256
from pathlib import Path, PurePosixPath
import re

from .cpp_audit_rules import CPP_RECOVERY_V1
from .model import digest, require_text
from .records import parse_json
from .source_config import SourceConfiguration


def code_lines(text):
    """Historical line-wise lens: keep dereference statements, skip prose blocks.

    This is not a C++ lexer: inline block comments, literals and preprocessor
    branches are not interpreted. These counts cannot establish absence of defects.
    """
    in_block = False
    for line in text.splitlines():
        stripped = line.lstrip()
        if in_block:
            if "*/" in stripped:
                in_block = False
            continue
        if stripped.startswith("//"):
            continue
        if stripped.startswith("/*"):
            in_block = "*/" not in stripped
            continue
        yield line.split("//", 1)[0]


def _text(data):
    return data.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")


def audit_source(root: Path, name: str) -> dict:
    root = root.resolve()
    project_path = root / "recovery/project.json"
    if not project_path.resolve().is_relative_to(root):
        raise ValueError("project configuration resolves outside the worktree")
    project_bytes = project_path.read_bytes()
    project = parse_json(project_bytes)
    if type(project.get("schema_version")) is not int or project["schema_version"] != 1:
        raise ValueError("unsupported project configuration version")
    source = SourceConfiguration.from_dict(project.get("source"))
    policy_path = source.path(root, project.get("audits"))
    policy_bytes = policy_path.read_bytes()
    policy = parse_json(policy_bytes)
    if (set(policy) != {"schema_version", "audits"}
            or type(policy["schema_version"]) is not int or policy["schema_version"] != 1
            or not isinstance(policy["audits"], dict)):
        raise ValueError("invalid source audit policy")
    require_text(name, "audit name")
    if name not in policy["audits"]:
        raise ValueError(f"audit is not configured: {name}")
    config = policy["audits"][name]
    required = {"ruleset", "unit_extensions", "interface_extensions", "exclude_roots", "ceilings", "counters"}
    if not isinstance(config, dict) or set(config) != required or config["ruleset"] != "cpp-recovery-v1":
        raise ValueError("invalid or unsupported source audit ruleset")
    rules = CPP_RECOVERY_V1
    for field in ("unit_extensions", "interface_extensions", "exclude_roots"):
        values = config[field]
        if (not isinstance(values, list) or any(not isinstance(value, str) for value in values)
                or len(values) != len(set(values))):
            raise ValueError(f"audit {field} must be a unique list of strings")
    units, interfaces = config["unit_extensions"], config["interface_extensions"]
    if not units or set(units) & set(interfaces) or not set(units + interfaces) <= set(source.extensions):
        raise ValueError("audit extensions must be disjoint configured source extensions")

    def checked_path(relative):
        path = source.path(root, relative)
        if not any(PurePosixPath(relative).is_relative_to(parent) for parent in source.roots):
            raise ValueError(f"audit scope is outside configured source roots: {relative}")
        return path

    for relative in config["exclude_roots"]:
        checked_path(relative)
    counters = config["counters"]
    if not isinstance(counters, list):
        raise ValueError("audit counters must be a list")
    names = set(rules)
    patterns = {}
    for counter in counters:
        if not isinstance(counter, dict):
            raise ValueError("invalid audit counter")
        require_text(counter.get("name"), "counter name")
        if counter["name"] in names:
            raise ValueError("duplicate audit counter name")
        names.add(counter["name"])
        kind = counter.get("kind")
        extra = {"text": {"pattern"}, "files": {"extension", "recursive"}, "lines": set()}
        if not isinstance(kind, str) or kind not in extra or set(counter) != {"name", "kind", "path"} | extra[kind]:
            raise ValueError("invalid audit counter fields")
        checked_path(counter["path"])
        if kind == "files":
            if counter["extension"] not in source.extensions or type(counter["recursive"]) is not bool:
                raise ValueError("file counter needs a source extension and boolean recursive")
        elif kind == "text":
            require_text(counter["pattern"], "counter pattern")
            try:
                patterns[counter["name"]] = re.compile(counter["pattern"])
            except re.error as error:
                raise ValueError(f"invalid audit counter pattern: {error}") from error
    ceilings = config["ceilings"]
    if (not isinstance(ceilings, dict) or set(ceilings) != names
            or any(type(value) is not int or value < 0 for value in ceilings.values())):
        raise ValueError("audit ceilings must cover every rule/counter exactly with nonnegative integers")

    def snapshot():
        # Path.rglob does not descend through directory symlinks. Refuse that
        # unsupported scope instead of silently reporting zero for hidden code.
        for relative in source.roots:
            directory = source.path(root, relative)
            for path in (directory, *directory.rglob("*")):
                if path.is_symlink():
                    try:
                        source.path(root, path.relative_to(root).as_posix())
                    except RuntimeError as error:
                        raise ValueError("source symlink cannot be resolved") from error
                    if path.is_dir() or not path.exists():
                        raise ValueError(f"audit cannot enumerate a directory or dangling symlink: {path.relative_to(root)}")
        files = {path.relative_to(root).as_posix(): path.read_bytes() for path in source.files(root)}
        # Required counter scopes cannot silently vanish and become zero.
        for counter in counters:
            path = checked_path(counter["path"])
            if counter["kind"] == "files":
                if not path.is_dir():
                    raise ValueError(f"audit counter directory is missing: {counter['path']}")
            elif counter["path"] not in files:
                raise ValueError(f"audit counter file is missing or not a configured source file: {counter['path']}")
        return files

    implementation_paths = (Path(__file__), Path(__file__).with_name("cpp_audit_rules.py"))
    implementation = {path.name: path.read_bytes() for path in implementation_paths}
    before = snapshot()
    counts = Counter({key: 0 for key in names})
    per_file = {}
    for relative, data in before.items():
        if source.is_seed(relative) or any(PurePosixPath(relative).is_relative_to(parent)
                                          for parent in config["exclude_roots"]):
            continue
        suffix = PurePosixPath(relative).suffix
        if suffix not in units + interfaces:
            continue
        selected = {key: rx for key, (rx, headers, _) in rules.items() if suffix in units or headers}
        found = Counter()
        for line in code_lines(_text(data)):
            for key, rx in selected.items():
                n = len(rx.findall(line))
                if n:
                    found[key] += n
        if found:
            per_file[relative] = dict(found)
            counts.update(found)
    for counter in counters:
        relative, key = counter["path"], counter["name"]
        if counter["kind"] == "files":
            counts[key] = sum(PurePosixPath(path).suffix == counter["extension"]
                              and (PurePosixPath(path).is_relative_to(relative) if counter["recursive"]
                                   else PurePosixPath(path).parent == PurePosixPath(relative)) for path in before)
        else:
            text = _text(before[relative])
            counts[key] = (len(patterns[key].findall(text)) if counter["kind"] == "text" else
                           text.count("\n") + int(bool(text) and not text.endswith("\n")))
    # Detect changed inputs and added/deleted source files, not just mtimes.
    if (before != snapshot() or project_path.read_bytes() != project_bytes
            or policy_path.read_bytes() != policy_bytes
            or implementation != {path.name: path.read_bytes() for path in implementation_paths}):
        raise ValueError("source or audit policy changed during the census")
    rows = [{"name": key, "count": counts[key], "ceiling": ceilings[key],
             "status": "growth" if counts[key] > ceilings[key] else "slack" if counts[key] < ceilings[key] else "exact"}
            for key in sorted(names)]
    return {"audit": name, "result": "fail" if any(row["status"] != "exact" for row in rows) else "pass",
            "total": sum(counts.values()), "rules": rows, "per_file": per_file,
            "explanations": {key: why for key, (_, _, why) in rules.items()},
            "lowerings": {key: counts[key] for key in sorted(names) if counts[key] < ceilings[key]},
            "input_fingerprint": digest({"source": {path: sha256(data).hexdigest() for path, data in before.items()},
                                         "project": sha256(project_bytes).hexdigest(), "policy": sha256(policy_bytes).hexdigest(),
                                         "implementation": {path: sha256(data).hexdigest() for path, data in implementation.items()},
                                         "rules": {key: [rx.pattern, rx.flags, headers] for key, (rx, headers, _) in rules.items()}}),
            "persisted": False, "structure_verification": "unknown",
            "limitations": ["heuristic line-wise patterns, not a C++ parser or absence-of-defects proof",
                            "ceilings are explicit policy; slack and growth both fail",
                            "seed and excluded source roots skip patterns; explicit counters still observe them"]}

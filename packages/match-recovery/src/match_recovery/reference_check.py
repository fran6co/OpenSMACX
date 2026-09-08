"""Read-only live path references in source comments and recovery instructions.

This is a bounded prose lint, not a shell/C++ parser or proof that a command
works. Historical mentions are retained separately, never rewritten or deleted.
Project paths belong to configuration; no project script or compiler is needed.
"""

from hashlib import sha256
from pathlib import Path, PurePosixPath
import re

from .components import relative_path
from .model import digest
from .records import parse_json
from .source_config import SourceConfiguration


IMPERATIVE = re.compile(r"uv run|\brun\b|Do not edit|edit the generator|regenerate", re.I)
RETRACTED = re.compile(r"NO LONGER EXISTS|was retired|retired the bulk generators|is gone|"
                       r"(?:was|were|both) deleted", re.I)
GONE = re.compile(r"deleted|retired|promoted|merged from|moved here from|homed from|"
                  r"went into|no longer|used to|was replaced|scratch artifact|earlier", re.I)


def _policy(root):
    path = root / "recovery/project.json"
    if path.resolve() != path:
        raise ValueError("reference policy must be a canonical local file")
    raw = path.read_bytes()
    project = parse_json(raw)
    if type(project.get("schema_version")) is not int or project["schema_version"] != 1:
        raise ValueError("unsupported reference project version")
    source = SourceConfiguration.from_dict(project.get("source"))
    policy = project.get("references")
    if not isinstance(policy, dict) or set(policy) != {"documentation_roots", "command_roots", "exclude_roots"}:
        raise ValueError("references needs documentation_roots, command_roots and exclude_roots")
    for key, values in policy.items():
        if not isinstance(values, list) or any(not isinstance(v, str) for v in values) or len(set(values)) != len(values):
            raise ValueError(f"reference {key} must be unique relative paths")
        for value in values:
            relative_path(value)
    return raw, source, policy


def _pattern(roots, extensions):
    if not roots:
        return re.compile(r"(?!)")
    return re.compile(r"(?<![\w./:-])(?:" + "|".join(re.escape(r) for r in roots) +
                      r")/[A-Za-z0-9_./-]+(?:" + "|".join(re.escape(e) for e in extensions) +
                      r")(?![\w./-])")


def _files(root, source, policy):
    excluded = (*source.seed_roots, *policy["exclude_roots"])
    extensions = {*source.extensions, ".md", ".csv"}
    def skip(name):
        return any(PurePosixPath(name).is_relative_to(parent) for parent in excluded)
    files = {}
    def add(path):
        name = path.relative_to(root).as_posix()
        if skip(name):
            return
        if path.is_symlink():
            raise ValueError(f"reference scan cannot enumerate symlinks: {name}")
        if path.is_file() and path.suffix in extensions:
            files[name] = path.read_bytes().decode("utf-8")
    def walk(path):
        if skip(path.relative_to(root).as_posix()):
            return
        if path.is_symlink():
            if path.is_dir() or path.suffix in extensions:
                raise ValueError(f"reference scan cannot enumerate symlinks: {path.relative_to(root)}")
            return  # A non-input file (for example a compiler launcher) is not prose.
        if path.is_dir():
            for child in sorted(path.iterdir()):
                walk(child)
        else:
            add(path)
    for name in {*source.roots, *policy["documentation_roots"]}:
        directory = source.path(root, name)
        if not directory.is_dir():
            raise ValueError(f"missing reference scan root: {name}")
        walk(directory)
    for path in sorted(root.glob("*.md")):
        add(path)
    if not files:
        raise ValueError("refusing an empty reference scan")
    return dict(sorted(files.items()))


def reference_snapshot(root: Path) -> dict:
    """Track scan membership/content AND reference existence for gate freshness."""
    root = root.resolve()
    raw, source, policy = _policy(root)
    files = _files(root, source, policy)
    patterns = {"command": _pattern(policy["command_roots"], (".py", ".sh")),
                "source": _pattern(source.roots, source.extensions)}
    targets = {}
    for text in files.values():
        for pattern in patterns.values():
            for name in pattern.findall(text):
                try:
                    path = source.path(root, name)
                    # Aliases are not proof of a live local reference.
                    targets[name] = "file" if path.resolve() == path and path.is_file() else "missing_or_aliased"
                except ValueError:
                    targets[name] = "invalid_path"
    return {"project": sha256(raw).hexdigest(), "files": files, "targets": targets,
            "implementation": sha256(Path(__file__).read_bytes()).hexdigest()}


def check_references(root: Path) -> dict:
    root = root.resolve()
    before = reference_snapshot(root)
    _, source, policy = _policy(root)
    patterns = {"command": _pattern(policy["command_roots"], (".py", ".sh")),
                "source": _pattern(source.roots, source.extensions)}
    findings, history = [], []
    for path, text in before["files"].items():
        lines = text.splitlines()
        for index, line in enumerate(lines):
            window = "\n".join(lines[max(0, index - 6):index + 7])
            for kind, pattern in patterns.items():
                for target in sorted(set(pattern.findall(line))):
                    if before["targets"][target] == "file":
                        continue
                    live = (bool(IMPERATIVE.search(line)) and not RETRACTED.search(window)
                            if kind == "command" else not GONE.search(window))
                    row = {"path": path, "line": index + 1, "target": target, "kind": kind,
                           "reason": before["targets"][target]}
                    (findings if live else history).append(row)
    if before != reference_snapshot(root):
        raise ValueError("reference sources, target existence, policy or checker changed during verification")
    return {"schema_version": 1, "check": "live_references", "result": "fail" if findings else "pass",
            "files_scanned": len(before["files"]), "findings": findings, "history": history,
            "missing_paths": len({row["target"] for row in findings}),
            "historical_paths": len({row["target"] for row in history}),
            "input_fingerprint": digest(before), "persisted": False, "ready_for_collection": False,
            "limitations": ["literal configured-root file references only, not links, arbitrary CLI commands or shell parsing",
                            "English historical vocabulary in a bounded six-line context, not semantic interpretation",
                            "source seeds and explicitly configured excluded roots are not live instruction inputs"]}

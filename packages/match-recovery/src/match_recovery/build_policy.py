"""Classify historical build-policy differences without authorizing a change."""

import os
from pathlib import Path

from .build_check import Build
from .integration import preservation_base
from .records import read_json
from .source_history import _tree, git


def build_policy_changes(root, task, settings, build, *, integration=None):
    root = Path(root).resolve()
    revision = preservation_base(root, task, integration)
    project = read_json(root / "recovery/project.json")
    compiler_inputs = set(project.get("compiler", {}).get("inputs", []))
    compiler_inputs.update(project.get("toolchain", {}).get("inputs", []))

    def policy_file(name):
        return name in compiler_inputs or not any(Path(name).is_relative_to(base) for base in settings.source.roots)

    generators = [build]
    if "preparation" in project:
        generators.append(Build.read(root, key="preparation"))
    inputs, roots = set(compiler_inputs), set()
    for generator in generators:
        inputs.update(generator.snapshot(root)["inputs"])
        roots.update(generator.input_roots)
    names = {name for name in inputs if policy_file(name)} | {"recovery/project.json"}
    baseline = {name: entry for name, entry in _tree(root, revision, tuple(sorted(names | roots))).items()
                if policy_file(name)}
    changed, invalid = set(names ^ baseline.keys()), set(names ^ baseline.keys())
    graph = set(build.versioned_inputs)
    for name in sorted(names | baseline.keys()):
        entry = baseline.get(name)
        path = root / name
        if path.is_symlink():
            current = ("120000", os.fsencode(os.readlink(path)))
        elif path.is_file():
            current = ("100755" if path.stat().st_mode & 0o111 else "100644", path.read_bytes())
        else:
            current = None
        historical = (entry[0], git(root, "cat-file", "blob", entry[2])) if entry is not None and entry[1] == "blob" else None
        if current is None or historical is None or current != historical:
            changed.add(name)
        if name in graph and (current is None or historical is None
                or current[0] not in ("100644", "100755") or historical[0] != current[0]
                or name in compiler_inputs):
            invalid.add(name)
    return {"baseline_revision": revision, "unchanged": not changed,
            "graph_only": bool(changed) and changed <= graph and not invalid,
            "changed_paths": sorted(changed), "invalid_paths": sorted(invalid)}


def unchanged_build_policy(root, task, settings, build, *, integration=None):
    return build_policy_changes(root, task, settings, build, integration=integration)["unchanged"]

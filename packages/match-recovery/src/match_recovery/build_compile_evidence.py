"""Configured fresh-build collection for compiler/link file correspondence."""

from contextlib import contextmanager
from copy import deepcopy

import hashlib
import os
from pathlib import Path
import re

from .compilation_database import entries
from .compile_file_evidence import _identity, _observe, _physical, parse_compile_files
from .compile_link_evidence import verify_compile_link_inputs
from .components import relative_path
from .link_file_evidence import parse_link_files
from .model import digest
from .records import parse_json_value


def validate_configuration(value, outputs):
    if value is None:
        return None
    fields = {"database", "source_extensions", "directory", "suffix", "link_files", "link_log"}
    if not isinstance(value, dict) or not fields <= set(value) or set(value) - fields - {"command_stages"}:
        raise ValueError("compile_link requires database, source_extensions, directory, suffix, link_files and link_log")
    for name in ("database", "directory", "link_files", "link_log"):
        relative_path(value[name])
    extensions = value["source_extensions"]
    if (not isinstance(extensions, list) or not extensions
            or any(not isinstance(item, str) or not re.fullmatch(r"\.[A-Za-z0-9]+", item) for item in extensions)
            or len(set(extensions)) != len(extensions)):
        raise ValueError("compile_link source_extensions must be unique literal suffixes")
    if not isinstance(value["suffix"], str) or not re.fullmatch(r"\.[A-Za-z0-9][A-Za-z0-9_.-]*", value["suffix"]):
        raise ValueError("compile_link observation suffix must be literal")
    declared = dict(outputs)
    for key, kind in (("database", "compilation-database"), ("link_files", "link-input-files"), ("link_log", "msvc-link-log")):
        if declared.get(value[key]) != kind:
            raise ValueError("compile_link requires a declared " + kind + " output")
    stages = value.get("command_stages")
    if "command_stages" in value and (not isinstance(stages, dict) or set(stages) != {"compile", "link"}
            or any(not isinstance(stage, str) or not stage.strip() or "\0" in stage for stage in stages.values())):
        raise ValueError("command_stages requires explicit compile and link observation stages")
    return {**value, "source_extensions": list(extensions),
            **({"command_stages": dict(stages)} if stages is not None else {})}


def _observations(directory, suffix):
    if not directory.is_dir() or directory.resolve() != directory:
        raise ValueError("compile observation directory must be canonical and present")
    pending, selected = [directory], []
    while pending:
        with os.scandir(pending.pop()) as entries:
            for item in entries:
                if item.is_symlink():
                    raise ValueError("compile observation tree must not contain aliases")
                if item.is_dir(follow_symlinks=False):
                    pending.append(Path(item.path))
                elif not item.is_file(follow_symlinks=False):
                    raise ValueError("compile observation tree requires regular files")
                elif item.name.endswith(suffix):
                    selected.append(Path(item.path))
    return sorted(selected)


def _collect(root, directory, configuration, *, outputs, retain=False):
    """Consume evidence inside the fresh build's still-live isolated directory.

    The build driver owns source/policy/Git/toolchain freshness. This collector
    additionally guards the observed build files, including generated inputs,
    and refuses mixed artifact versions or changing observation membership.
    It establishes correspondence, not application graph/behavior acceptance.
    """
    root, directory = Path(root), Path(directory)
    if (not root.is_absolute() or root.resolve() != root or directory.resolve() != directory
            or directory == root or not directory.is_relative_to(root) or not directory.is_dir()):
        raise ValueError("compile evidence needs a canonical private build directory inside the project")
    configuration = validate_configuration(configuration, [(item["path"], item["format"]) for item in outputs])
    if configuration is None:
        raise ValueError("compile evidence needs an explicit configuration")
    declared = {item["path"]: item for item in outputs}
    artifacts = {}
    expected_artifacts = {}
    for item in outputs:
        path = directory / relative_path(item["path"])
        fingerprint = {"sha256": item.get("sha256"), "size": item.get("size")}
        if item.get("result") != "pass" or _observe(path) != fingerprint:
            raise ValueError("build artifact changed before compilation correspondence verification")
        expected_artifacts[path] = fingerprint
    observed_paths = set(expected_artifacts)
    for key in ("database", "link_files", "link_log"):
        name = configuration[key]
        path = directory / name
        fingerprint = _observe(path)
        if declared[name].get("result") != "pass" or fingerprint != {
                "sha256": declared[name].get("sha256"), "size": declared[name].get("size")}:
            raise ValueError("build artifact changed before compilation correspondence verification")
        raw = path.read_bytes()
        if hashlib.sha256(raw).hexdigest() != fingerprint["sha256"]:
            raise ValueError("build artifact changed while reading")
        artifacts[key] = raw
        observed_paths.add(path)
    database = artifacts["database"]
    expected = []
    recipe_paths = []
    for entry, raw in zip(entries(database, directory), parse_json_value(database), strict=True):
        origin = directory / raw["directory"]
        source = origin / raw["file"]
        if (origin != entry.directory or source != entry.file or source.resolve() != source
                or origin.resolve() != origin or not origin.is_dir() or not source.is_file()
                or not source.is_relative_to(root) or not origin.is_relative_to(root)):
            raise ValueError("application recipes require canonical existing project source and directory paths")
        recipe_paths.append((origin, source))
        if source.suffix in configuration["source_extensions"]:
            expected.append(source)
    tree = directory / configuration["directory"]
    selected = _observations(tree, configuration["suffix"])
    compilations = []
    for path in selected:
        _observe(path)  # Refuse hardlinks and nonphysical files before reading.
        data = path.read_bytes()
        value = parse_compile_files(data)
        output = Path(value["output"]["path"])
        if not output.is_relative_to(directory):
            raise ValueError("compiled output must belong to the current fresh build directory")
        observed_paths.update((path, output, *(Path(item["path"]) for item in value["inputs"])))
        compilations.append(data)
    link = parse_link_files(artifacts["link_files"])
    observed_paths.update(Path(item["path"]) for item in link["files"])
    before = {path: _observe(path) for path in sorted(observed_paths)}
    if any(before[path] != stamp for path, stamp in expected_artifacts.items()):
        raise ValueError("build artifact changed before compilation correspondence verification")
    # Pin the exact metadata bytes already parsed, not a later same-path report.
    metadata = {directory / configuration[key]: data for key, data in artifacts.items()}
    metadata.update(zip(selected, compilations, strict=True))
    if any(before[path] != {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}
           for path, raw in metadata.items()):
        raise ValueError("compiler/link observations changed before verification")
    report = verify_compile_link_inputs(compilations, artifacts["link_files"], artifacts["link_log"], sources=expected)
    if "command_stages" in configuration:
        report["command_bindings"] = {"result": "unknown", "verification": "not_measured",
                                      "reason": "fresh compile/link file correspondence is required"}
        if report["result"] == "pass":
            from .build_command_evidence import bind_build_commands
            report["command_bindings"] = bind_build_commands(
                root, directory, [parse_compile_files(raw) for raw in compilations], link, configuration["command_stages"])
            if report["command_bindings"].get("result") != "pass":
                raise ValueError("command file bindings did not complete")
    if (_observations(tree, configuration["suffix"]) != selected
            or {path: _observe(path) for path in sorted(observed_paths)} != before):
        raise ValueError("compiler/link files or observation membership changed during verification")
    report["evidence_fingerprint"] = digest({
        "files": {str(path): stamp for path, stamp in before.items()},
        "observations": [str(path) for path in selected]})
    if not retain:
        return report
    if report["result"] != "pass":
        raise ValueError("live compile/link evidence requires fresh complete native correspondence")
    # These are derived during this native verification, never supplied by a
    # caller's report. Capture exact content stamps and actual checked paths.
    directories = {origin for origin, _ in recipe_paths}
    directories.update(Path(parse_compile_files(raw)["cwd"]) for raw in compilations)
    output_roles = set()
    if "command_bindings" in report:
        binding = report["command_bindings"]
        for row in [*binding["compilations"], binding["link"]]:
            for operand in row["description"]["operands"]:
                path, kind = Path(operand["path"]), operand["kind"]
                if kind in ("input_directory", "output_directory"):
                    directories.add(path)
                if kind == "output_file":
                    directories.add(path.parent)
                    output_roles.add(path)
    return report, _epoch_check(before, tree, configuration["suffix"], selected,
                                tuple(recipe_paths), tuple(sorted(directories)), tuple(sorted(output_roles)))


def verify_build_compilations(root, directory, configuration, *, outputs):
    """Perform fresh semantic and physical verification without retaining state."""
    return _collect(root, directory, configuration, outputs=outputs)


def _epoch_check(stamps, tree, suffix, selected, recipes, directories, output_roles):
    # Immutable closure; no caller-owned mapping, mutable report, mtime cache or
    # persisted PASS can add paths, alter expected contents or refresh this epoch.
    stamps = tuple((path, stamp["sha256"], stamp["size"]) for path, stamp in sorted(stamps.items()))
    selected = tuple(selected)

    def paths_current():
        if tuple(_observations(tree, suffix)) != selected:
            raise ValueError("application compile/link evidence changed: observation membership")
        for origin, source in recipes:
            if (origin.resolve() != origin or source.resolve() != source
                    or not origin.is_dir() or not source.is_file()):
                raise ValueError("application compile/link evidence changed: recipe paths")
        for path in directories:
            _physical(path, directory=True)
        for path in output_roles:
            if path.resolve() != path or path.exists() and not path.is_file():
                raise ValueError("application compile/link evidence changed: command output path")

    def current():
        paths_current()
        # Each unique observed file is content-hashed once per freshness call.
        # Global pre/post physical identities also catch a file changing after
        # its hash, while a later member is being read (including mtime restore).
        before = tuple(_identity(_physical(path)) for path, _, _ in stamps)
        for path, fingerprint, size in stamps:
            if _observe(path) != {"sha256": fingerprint, "size": size}:
                raise ValueError("application compile/link evidence changed: observed file contents")
        paths_current()
        if tuple(_identity(_physical(path)) for path, _, _ in stamps) != before:
            raise ValueError("application compile/link evidence changed during freshness verification")

    return current


@contextmanager
def live_build_compilations(root, directory, configuration, *, outputs):
    """Seal one native verification for an unchanged enclosing live build.

    The enclosing application still checks source/policy/Git/compiler freshness
    before and after use. This closure rechecks only the collector's existing
    physical and membership obligations. It cannot be persisted or refreshed
    into another epoch and adds no completeness or acceptance authority.
    """
    report, check = _collect(root, directory, deepcopy(configuration),
                             outputs=deepcopy(outputs), retain=True)
    live = True

    def current():
        if not live:
            raise ValueError("compile/link evidence context has expired")
        check()

    try:
        current()
        yield deepcopy(report), current
    finally:
        live = False

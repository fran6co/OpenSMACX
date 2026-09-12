"""Configured fresh-build collection for compiler/link file correspondence."""

import hashlib
import os
from pathlib import Path
import re

from .compilation_database import entries
from .compile_file_evidence import _observe, parse_compile_files
from .compile_link_evidence import verify_compile_link_inputs
from .components import relative_path
from .link_file_evidence import parse_link_files
from .records import parse_json_value


def validate_configuration(value, outputs):
    if value is None:
        return None
    fields = {"database", "source_extensions", "directory", "suffix", "link_files", "link_log"}
    if not isinstance(value, dict) or set(value) != fields:
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
    return {**value, "source_extensions": list(extensions)}


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


def verify_build_compilations(root, directory, configuration, *, outputs):
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
    for entry, raw in zip(entries(database, directory), parse_json_value(database), strict=True):
        origin = directory / raw["directory"]
        source = origin / raw["file"]
        if (origin != entry.directory or source != entry.file or source.resolve() != source
                or origin.resolve() != origin or not origin.is_dir() or not source.is_file()
                or not source.is_relative_to(root) or not origin.is_relative_to(root)):
            raise ValueError("application recipes require canonical existing project source and directory paths")
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
    if (_observations(tree, configuration["suffix"]) != selected
            or {path: _observe(path) for path in sorted(observed_paths)} != before):
        raise ValueError("compiler/link files or observation membership changed during verification")
    return report

"""Fresh declared compile-file observations, never complete compilation proof."""

import hashlib
import os
from pathlib import Path
import stat

from .link_file_evidence import _path, _stamp
from .model import Result
from .records import parse_json


def parse_compile_files(data: bytes) -> dict:
    value = parse_json(data)
    if (set(value) != {"schema_version", "source", "tool", "cwd", "arguments", "inputs", "output", "exit_code"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1
            or type(value["exit_code"]) is not int):
        raise ValueError("invalid compile file evidence schema")
    for name in ("source", "tool", "cwd"):
        _path(value[name])
    if (not isinstance(value["arguments"], list) or not value["arguments"]
            or any(not isinstance(word, str) or not word or "\0" in word for word in value["arguments"])):
        raise ValueError("compile arguments must be a nonempty string array")
    if not isinstance(value["inputs"], list):
        raise ValueError("compile inputs must be an array")
    paths = set()
    for row in value["inputs"]:
        if not isinstance(row, dict) or set(row) != {"path", "before", "after"}:
            raise ValueError("invalid compile input observation")
        _path(row["path"])
        if row["path"] in paths:
            raise ValueError("duplicate compile input identity")
        paths.add(row["path"])
        _stamp(row["before"])
        _stamp(row["after"])
    if {value["source"], value["tool"]} - paths:
        raise ValueError("compile source or tool lacks file observations")
    output = value["output"]
    if (not isinstance(output, dict) or set(output) != {"path", "existed_before", "after"}
            or type(output["existed_before"]) is not bool):
        raise ValueError("invalid compile output observation")
    _path(output["path"])
    _stamp(output["after"])
    if output["path"] in paths:
        raise ValueError("compile output cannot also be a declared input")
    return value


def _physical(path, *, directory=False):
    if path.resolve() != path or any(part.is_symlink() for part in (path, *path.parents)):
        raise ValueError("compile evidence path is aliased")
    info = path.stat()
    if directory:
        if not stat.S_ISDIR(info.st_mode):
            raise ValueError("compile working directory is not a physical directory")
    elif not stat.S_ISREG(info.st_mode) or info.st_nlink != 1:
        raise ValueError("compile evidence needs an unaliased regular file")
    return info


def _identity(info):
    return (info.st_dev, info.st_ino, info.st_mode, info.st_nlink,
            info.st_size, info.st_mtime_ns, info.st_ctime_ns)


def _observe(path):
    before = _identity(_physical(path))
    # Do not block on a FIFO or follow a replaced leaf symlink between stat and
    # open. The descriptor and final physical path must still identify the file.
    descriptor = os.open(path, os.O_RDONLY | getattr(os, "O_NONBLOCK", 0) | getattr(os, "O_NOFOLLOW", 0))
    with os.fdopen(descriptor, "rb") as stream:
        if _identity(os.fstat(stream.fileno())) != before:
            raise ValueError("compile evidence file changed before reading")
        digest = hashlib.file_digest(stream, "sha256").hexdigest()
        if _identity(os.fstat(stream.fileno())) != before:
            raise ValueError("compile evidence file changed while reading")
    if _identity(_physical(path)) != before:
        raise ValueError("compile evidence file changed while reading")
    return {"sha256": digest, "size": before[4]}


def verify_compile_files(data: bytes, *, object_path: Path | None = None) -> dict:
    value = parse_compile_files(data)
    output = value["output"]
    if object_path is not None and str(object_path) != output["path"]:
        raise ValueError("compile output observation belongs to a different object path")
    directory = {"path": value["cwd"], "result": Result.UNKNOWN}
    try:
        _physical(Path(value["cwd"]), directory=True)
        directory.update(result=Result.PASS, reason="physical compiler working directory")
    except (OSError, ValueError) as error:
        directory["reason"] = str(error)
    inputs = []
    for row in value["inputs"]:
        result, reason = Result.UNKNOWN, "missing pre/post compile input observation"
        if row["before"] is not None and row["after"] is not None:
            if row["before"] != row["after"]:
                result, reason = Result.FAIL, "input changed across compilation"
            else:
                try:
                    current = _observe(Path(row["path"]))
                    result = Result.PASS if current == row["after"] else Result.FAIL
                    reason = "stable observed input contents" if result == Result.PASS else "input changed after compilation"
                except (OSError, ValueError) as error:
                    reason = str(error)
        inputs.append({**row, "result": result, "reason": reason})
    result, reason = Result.UNKNOWN, "missing compile output observation"
    if output["after"] is not None:
        try:
            current = _observe(Path(output["path"]))
            if current != output["after"]:
                result, reason = Result.FAIL, "output changed after compilation"
            elif output["existed_before"]:
                reason = "output existed before compilation"
            elif current["size"] == 0:
                reason = "compiler output is empty"
            else:
                result, reason = Result.PASS, "freshly observed nonempty compiler output"
        except (OSError, ValueError) as error:
            reason = str(error)
    observed_output = {**output, "result": result, "reason": reason}
    results = [directory["result"], result, *(row["result"] for row in inputs),
               Result.PASS if value["exit_code"] == 0 else Result.FAIL]
    result = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    return {"check": "compile_file_observations", "result": result,
            "sha256": hashlib.sha256(data).hexdigest(), "source": value["source"], "tool": value["tool"],
            "cwd": directory, "arguments": value["arguments"], "inputs": inputs,
            "output": observed_output, "exit_code": value["exit_code"],
            "input_coverage": Result.UNKNOWN, "linked_selection": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["trusted compiler-boundary pre/post capture, not filesystem or execution attestation",
                            "compiler arguments are opaque; full source/header/build membership is not established",
                            "fresh file observations do not prove object semantics, linked selection or recovery acceptance"]}

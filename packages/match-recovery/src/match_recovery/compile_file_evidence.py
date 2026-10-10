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
    fields = {"schema_version", "source", "tool", "cwd", "arguments", "inputs", "output", "exit_code"}
    version = value.get("schema_version")
    if version == 2:
        fields |= {"compiler_output", "postprocess"}
    if (set(value) != fields
            or type(version) is not int or version not in (1, 2)
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
    if version == 2:
        primary = value["compiler_output"]
        if (not isinstance(primary, dict) or set(primary) != {"path", "existed_before", "after"}
                or type(primary["existed_before"]) is not bool
                or primary["path"] != output["path"]
                or primary["existed_before"] != output["existed_before"]):
            raise ValueError("compiler pipeline requires the same fresh in-place output identity")
        _stamp(primary["after"])
        stages = value["postprocess"]
        if not isinstance(stages, list) or len(stages) > 32:
            raise ValueError("compile postprocess stages must be an explicit bounded array")
        for stage in stages:
            if (not isinstance(stage, dict) or set(stage) != {
                    "tool", "cwd", "argv", "inputs", "input_before", "output_after", "exit_code"}
                    or type(stage["exit_code"]) is not int):
                raise ValueError("invalid compile postprocess stage")
            _path(stage["tool"])
            _path(stage["cwd"])
            if stage["cwd"] != value["cwd"]:
                raise ValueError("in-place compiler stages require the observed compiler directory")
            argv = stage["argv"]
            if (not isinstance(argv, list) or not argv or argv[0] != stage["tool"]
                    or any(not isinstance(word, str) or not word or "\0" in word for word in argv)):
                raise ValueError("postprocess argv must bind its observed executable")
            declared = stage["inputs"]
            if (not isinstance(declared, list) or not declared
                    or any(not isinstance(path, str) for path in declared)
                    or len(set(declared)) != len(declared)
                    or set(declared) - paths or stage["tool"] not in declared):
                raise ValueError("postprocess file inputs require unique compile-input observations")
            _stamp(stage["input_before"])
            _stamp(stage["output_after"])
    return value


def _pipeline(value, inputs):
    """Check trusted in-place capture continuity, not transform semantics."""
    previous = value["compiler_output"]["after"]
    rows = []
    results = [Result.UNKNOWN if previous is None or previous["size"] == 0 else Result.PASS]
    observed = {row["path"]: row["result"] for row in inputs}
    for stage in value["postprocess"]:
        statuses = [observed[path] for path in stage["inputs"]]
        reason = "consistent trusted in-place postprocess observations"
        if stage["exit_code"] != 0:
            result, reason = Result.FAIL, "postprocess execution failed"
        elif previous is None or stage["input_before"] is None or stage["output_after"] is None:
            result, reason = Result.UNKNOWN, "missing postprocess content observations"
        elif previous != stage["input_before"]:
            result, reason = Result.FAIL, "postprocess input does not continue the output chain"
        elif Result.FAIL in statuses:
            result, reason = Result.FAIL, "postprocess input or executable changed"
        elif Result.UNKNOWN in statuses:
            result, reason = Result.UNKNOWN, "postprocess input or executable is unobserved"
        else:
            result = Result.PASS
        rows.append({**stage, "result": result, "reason": reason})
        results.append(result)
        previous = stage["output_after"]
    final = value["output"]["after"]
    if previous is None or final is None:
        results.append(Result.UNKNOWN)
    elif previous != final:
        results.append(Result.FAIL)
    else:
        results.append(Result.PASS)
    result = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    return {"result": result, "compiler_output": value["compiler_output"], "stages": rows,
            "output_provenance": "postprocessed" if rows else "direct",
            "transformation_semantics": Result.UNKNOWN,
            "reason": "trusted capture chain only; intermediate bytes are not independently retained"}


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
                label = "pipeline output" if value.get("postprocess") else "compiler output"
                result, reason = Result.PASS, "freshly observed nonempty " + label
        except (OSError, ValueError) as error:
            reason = str(error)
    observed_output = {**output, "result": result, "reason": reason}
    results = [directory["result"], result, *(row["result"] for row in inputs),
               Result.PASS if value["exit_code"] == 0 else Result.FAIL]
    pipeline = _pipeline(value, inputs) if value["schema_version"] == 2 else None
    if pipeline is not None:
        results.append(pipeline["result"])
    result = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    report = {"check": "compile_file_observations", "result": result,
            "sha256": hashlib.sha256(data).hexdigest(), "source": value["source"], "tool": value["tool"],
            "cwd": directory, "arguments": value["arguments"], "inputs": inputs,
            "output": observed_output, "exit_code": value["exit_code"],
            "input_coverage": Result.UNKNOWN, "linked_selection": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["trusted compiler-boundary pre/post capture, not filesystem or execution attestation",
                            "compiler arguments are opaque; full source/header/build membership is not established",
                            "fresh file observations do not prove object semantics, linked selection or recovery acceptance"]}
    if pipeline is not None:
        report["pipeline"] = pipeline
        report["output_provenance"] = pipeline["output_provenance"]
        report["limitations"].append("pipeline stages declare file inputs and opaque argv, not complete input coverage or execution attestation")
    return report

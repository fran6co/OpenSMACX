"""Fresh compiler-boundary file observations, not complete link-input closure.

The configured environment owns path resolution and pre/post capture. Native
verification binds its observations to the exact verbose log and rechecks file
contents. Labels alone or a replayed receipt cannot establish file identity.
"""

import hashlib
from pathlib import Path

from .model import Result, require_digest
from .msvc_link_log import parse_link_log
from .records import parse_json


def _path(value):
    if not isinstance(value, str) or not value or not Path(value).is_absolute() or "\0" in value:
        raise ValueError("link input evidence requires absolute host paths")
    if str(Path(value)) != value or ".." in Path(value).parts:
        raise ValueError("link input evidence requires normalized paths")
    return value


def _stamp(value):
    if value is None:
        return
    if not isinstance(value, dict) or set(value) != {"sha256", "size"}:
        raise ValueError("invalid link input content observation")
    require_digest(value["sha256"])
    if type(value["size"]) is not int or value["size"] < 0:
        raise ValueError("invalid link input size")


def parse_link_files(data: bytes) -> dict:
    value = parse_json(data)
    if (set(value) != {"schema_version", "link_log_sha256", "cwd", "arguments", "tool", "explicit_inputs",
                       "files", "library_searches", "generated_exports", "capture_gaps"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 2):
        raise ValueError("invalid link input evidence schema")
    require_digest(value["link_log_sha256"])
    _path(value["cwd"])
    _path(value["tool"])
    for field in ("arguments", "explicit_inputs", "capture_gaps"):
        if not isinstance(value[field], list) or any(not isinstance(item, str) or not item or "\0" in item for item in value[field]):
            raise ValueError("invalid link input evidence strings")
    for path in value["explicit_inputs"]:
        _path(path)
    if not isinstance(value["files"], list) or not value["files"] or not isinstance(value["library_searches"], list):
        raise ValueError("invalid link input evidence arrays")
    paths = set()
    for row in value["files"]:
        if not isinstance(row, dict) or set(row) != {"path", "before", "after"}:
            raise ValueError("invalid link input file observation")
        _path(row["path"])
        if row["path"] in paths:
            raise ValueError("duplicate link input file identity")
        paths.add(row["path"])
        _stamp(row["before"])
        _stamp(row["after"])
    for row in value["library_searches"]:
        if (not isinstance(row, dict) or set(row) != {"spelling", "path"}
                or not isinstance(row["spelling"], str) or not row["spelling"] or "\0" in row["spelling"]):
            raise ValueError("invalid link library search observation")
        if row["path"] is not None:
            _path(row["path"])
            if row["path"] not in paths:
                raise ValueError("searched library lacks file observations")
    if value["tool"] not in paths or set(value["explicit_inputs"]) - paths:
        raise ValueError("tool or explicit input lacks file observations")
    if not isinstance(value["generated_exports"], list):
        raise ValueError("generated export observations must be an array")
    generated = set()
    for row in value["generated_exports"]:
        if (not isinstance(row, dict) or set(row) != {"path", "existed_before", "after"}
                or type(row["existed_before"]) is not bool):
            raise ValueError("invalid generated export observation")
        _path(row["path"])
        if row["path"] in generated or row["path"] in value["explicit_inputs"]:
            raise ValueError("duplicate or explicitly supplied generated export")
        generated.add(row["path"])
        _stamp(row["after"])
    return value


def verify_link_files(data: bytes, log_data: bytes) -> dict:
    value, log = parse_link_files(data), parse_link_log(log_data)
    if value["link_log_sha256"] != log["sha256"]:
        raise ValueError("link input observations belong to different diagnostics")
    if [row["spelling"] for row in value["library_searches"]] != [row["path"] for row in log["searched_libraries"]]:
        raise ValueError("link library search observations do not cover the exact diagnostic sequence")
    files = {}
    for row in value["files"]:
        result, reason = Result.UNKNOWN, "missing pre/post input observation"
        if row["before"] is not None and row["after"] is not None:
            if row["before"] != row["after"]:
                result, reason = Result.FAIL, "file contents changed across linking"
            else:
                try:
                    path = Path(row["path"])
                    if not path.is_file() or any(item.is_symlink() for item in (path, *path.parents)):
                        raise ValueError("input is no longer a regular physical file")
                    raw = path.read_bytes()
                    current = {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}
                    result = Result.PASS if current == row["after"] else Result.FAIL
                    reason = "stable observed file contents" if result == Result.PASS else "input changed after linking"
                except (OSError, ValueError) as error:
                    reason = str(error)
        files[row["path"]] = {**row, "result": result, "reason": reason}
    searches = [{**row, "phase": log["searched_libraries"][index]["phase"],
                 "result": files[row["path"]]["result"] if row["path"] else Result.UNKNOWN}
                for index, row in enumerate(value["library_searches"])]
    required = {value["tool"], *value["explicit_inputs"], *(row["path"] for row in searches if row["path"])}
    results = [files[path]["result"] for path in required] + [row["result"] for row in searches]
    if value["capture_gaps"]:
        results.append(Result.UNKNOWN)
    result = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    return {"check": "link_input_file_observations", "result": result,
            "sha256": hashlib.sha256(data).hexdigest(), "link_log_sha256": log["sha256"],
            "files": list(files.values()), "library_searches": searches,
            "capture_gaps": value["capture_gaps"], "input_coverage": Result.UNKNOWN,
            "allocation_verification": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["trusted compiler-boundary path resolution and pre/post capture, not filesystem attestation",
                            "stable observed inputs do not prove complete input coverage or archive member selection",
                            "generated inputs and common allocation extents require separate verification"]}

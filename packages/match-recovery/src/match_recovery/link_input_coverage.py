"""Conservative COFF input inventory bound to fresh linker observations.

All explicit objects and every member of each searched archive are inventoried,
not just members with public symbols or retained sections. This deliberate
superset prevents unselected/ambiguous records from becoming absence evidence.
It does not choose common allocations or claim exact archive-member selection.
"""

from collections import Counter
import hashlib
from pathlib import Path, PurePosixPath, PureWindowsPath

from .coff_archive import CoffArchive
from .coff_inventory import inventory_object
from .link_file_evidence import parse_link_files, verify_link_files
from .model import Result
from .msvc_link_log import parse_link_log


def input_coverage(data: bytes, log_data: bytes) -> dict:
    return _input_inventory(data, log_data)[0]


def _input_inventory(data: bytes, log_data: bytes) -> tuple[dict, list[dict]]:
    """One checked read epoch; record detail is consumed in memory, not a ledger."""
    evidence = parse_link_files(data)
    files = verify_link_files(data, log_data)
    log = parse_link_log(log_data)
    file_rows = {row["path"]: row for row in files["files"]}
    records, gaps, generated = [], [], {}

    def read(path, expected):
        path = Path(path)
        if not path.is_file() or any(item.is_symlink() for item in (path, *path.parents)):
            raise ValueError("input is no longer a regular file")
        raw = path.read_bytes()
        if expected != {"sha256": hashlib.sha256(raw).hexdigest(), "size": len(raw)}:
            raise ValueError("input contents changed before inventory")
        return raw

    def record(path, raw, **identity):
        inventory = inventory_object(raw)
        row = {"path": path, **identity, **inventory}
        records.append(row)
        return row

    library_paths = {row["path"] for row in files["library_searches"] if row["path"]}
    for path in sorted({*evidence["explicit_inputs"], *library_paths}):
        try:
            row = file_rows[path]
            if row["result"] != Result.PASS:
                raise ValueError("input lacks stable file observations")
            raw = read(path, row["after"])
            if path in library_paths or raw.startswith(b"!<arch>\n"):
                archive = CoffArchive(raw)
                if not archive.members:
                    raise ValueError("empty input archive")
                for name, bodies in archive.members.items():
                    for occurrence, body in enumerate(bodies):
                        record(path, body, member=name, occurrence=occurrence)
            else:
                record(path, raw)
        except (OSError, ValueError) as error:
            gaps.append({"path": path, "reason": str(error)})
    final_exports = [row for row in log["inputs"] if row["library"] is None and row["member"].lower().endswith(".exp")]
    for row in evidence["generated_exports"]:
        # Unconsumed expected outputs are not link inputs. Existing explicit
        # .exp files instead go through ordinary pre/post input observations.
        labels = [item for item in final_exports if PureWindowsPath(item["member"]).name.casefold()
                  == Path(row["path"]).name.casefold()]
        if not labels:
            continue
        try:
            if row["existed_before"] or row["after"] is None or log["export_generations"] != 1 or len(labels) != 1:
                raise ValueError("missing unique fresh export-generation evidence")
            raw = read(row["path"], row["after"])
            inventory = record(row["path"], raw, generated="msvc_export")
            if inventory["format"] != "coff" or inventory["result"] != Result.PASS:
                raise ValueError("generated export object has unsupported records")
            generated[row["path"]] = inventory
        except (OSError, ValueError) as error:
            gaps.append({"path": row["path"], "reason": str(error)})
    observations = []
    explicit_objects = [row for row in records if "member" not in row and "generated" not in row]
    for item in log["inputs"]:
        paths, candidates = set(), []
        if item["library"] is None:
            candidates = [row for row in [*explicit_objects, *generated.values()]
                          if Path(row["path"]).name.casefold() == PureWindowsPath(item["member"]).name.casefold()]
            paths = {row["path"] for row in candidates}
            result = Result.PASS if len(candidates) == 1 and candidates[0]["result"] == Result.PASS else Result.UNKNOWN
        else:
            paths = {row["path"] for row in files["library_searches"] if row["phase"] == "executable"
                     and PureWindowsPath(row["spelling"]).name.casefold() == PureWindowsPath(item["library"]).name.casefold()}
            if len(paths) == 1:
                path = next(iter(paths))
                candidates = [row for row in records if row["path"] == path and "member" in row
                              and PurePosixPath(row["member"]).name.casefold() == PureWindowsPath(item["member"]).name.casefold()]
            result = Result.PASS if candidates and all(row["result"] == Result.PASS for row in candidates) else Result.UNKNOWN
        observations.append({**item, "paths": sorted(path for path in paths if path),
                             "candidate_count": len(candidates), "candidate_coverage": result,
                             "selection": Result.PASS if result == Result.PASS and len(candidates) == 1 else Result.UNKNOWN})
    complete = (files["result"] == Result.PASS and not gaps and bool(records)
                and all(row["result"] == Result.PASS for row in records)
                and all(row["candidate_coverage"] == Result.PASS for row in observations))
    if verify_link_files(data, log_data) != files:
        raise ValueError("input file observations changed during inventory")
    for row in evidence["generated_exports"]:
        if row["path"] in generated:
            read(row["path"], row["after"])
    common = [{**{key: row[key] for key in ("path", "member", "occurrence", "generated") if key in row},
               "symbol": symbol["name"], "size": symbol["value"]}
              for row in records if row["result"] == Result.PASS
              for symbol in row["symbols"] if symbol["declaration"] == "common"]
    report = {"check": "link_input_candidate_coverage", "result": Result.PASS if complete else Result.UNKNOWN,
            "inputs": observations, "input_count": len(observations), "record_count": len(records),
            "format_counts": dict(Counter(row["format"] for row in records)), "common_requests": common,
            "generated_exports": [{"path": path, "sha256": row["sha256"]} for path, row in generated.items()],
            "unsupported_records": [{key: row[key] for key in ("path", "member", "occurrence", "reason") if key in row}
                                    for row in records if row["result"] != Result.PASS], "gaps": gaps,
            "allocation_verification": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["conservative inventory of boundary-reported inputs and every searched archive member",
                            "candidate coverage is not exact selected-member identity, retained sections or common allocation proof",
                            "trusted compiler capture and final-pass diagnostics, not filesystem attestation"]}
    return report, records

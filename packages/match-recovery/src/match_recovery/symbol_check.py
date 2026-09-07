"""Fresh source-symbol coverage, without nm, stale objects or a missing-name floor.

This checks emission under configured compiler trials, not the final linker
selection or original bytes. Missing/undefined/weak names remain unknown; a
data definition cannot satisfy a function mapping. No symbol is guessed or banked.
"""

from collections import Counter, defaultdict
from pathlib import Path

from .compiler import Compiler
from .mapping_check import build_units
from .measurement import Measurement
from .model import Result, digest
from .pe_coff import Coff
from .records import read_json
from .source_inventory import source_snapshot, assert_snapshot_current


def function_definition(obj: Coff, name: str) -> dict:
    matches = [s for s in obj.symbols.values() if s.name == name]
    definitions = [s for s in matches if s.section > 0 and s.storage in (2, 3)]
    if len(definitions) != 1:
        head = name.split("@@")[0]
        alternatives = sorted({s.name for s in obj.symbols.values()
                               if head and s.name != name and s.name.split("@@")[0] == head
                               and s.section > 0 and s.storage in (2, 3) and s.type & 0x20
                               and obj.sections[s.section - 1].flags & 0x20})
        return {"result": Result.UNKNOWN,
                "reason": "ambiguous definitions" if definitions else "no defined function with the mapped name",
                "undefined_or_weak": bool(matches) and not definitions,
                "alternatives": alternatives}
    symbol, = definitions
    section = obj.sections[symbol.section - 1]
    if not symbol.type & 0x20 or not section.flags & 0x20 or not section.flags & 0x20000000:
        return {"result": Result.FAIL, "reason": "mapped name defines non-function or non-executable storage"}
    if not 0 <= symbol.value < len(section.data):
        return {"result": Result.FAIL, "reason": "function start is outside emitted code"}
    return {"result": Result.PASS, "reason": "defined executable function",
            "section": symbol.section, "offset": symbol.value}


def check_symbols(root: Path, *, files=None, compiler=None) -> dict:
    root = root.resolve()
    settings = Measurement.read(root)
    if files is not None and (not isinstance(files, (tuple, list)) or not files
            or any(not isinstance(name, str) or not name for name in files) or len(set(files)) != len(files)):
        raise ValueError("symbol selection requires unique nonempty source paths")
    for name in files or ():
        settings.source.path(root, name)
    metadata = {"files": files}
    before = settings.snapshot(root, metadata)
    project = read_json(root / "recovery/project.json")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("symbol coverage requires a configured compilation database")
    database = settings.source.path(root, mappings["compile_commands"])
    database_bytes = database.read_bytes()
    units, external = build_units(root, database_bytes)
    snapshot = source_snapshot(root)
    selected = [r for r in snapshot["records"] if r["candidate_kind"] == "source"
                and not r["legacy_exclusion"] and (files is None or r["path"] in files)]
    if not selected or files is not None and {r["path"] for r in selected} != set(files):
        raise ValueError("symbol coverage selection has missing or empty product mappings")
    groups = defaultdict(list)
    for row in selected:
        groups[row["path"]].append(row)
    rows, trials, attempts = [], [], 0

    def unchanged():
        if settings.snapshot(root, metadata) != before or database.read_bytes() != database_bytes:
            raise ValueError("source, compiler policy or compilation database changed during symbol coverage")

    for path, records in sorted(groups.items()):
        observed = [[] for _ in records]
        if path not in units or Path(path).suffix not in settings.unit_extensions:
            observed = [[{"result": Result.UNKNOWN, "reason": "mapping host is not a configured build translation unit"}]
                        for _ in records]
        else:
            for flags in settings.flag_sets:
                # Emission trials are alternatives. One observed definition
                # suffices for presence, but never use another TU's definition.
                pending = [i for i, candidates in enumerate(observed)
                           if not any(r["result"] == Result.PASS for r in candidates)]
                if not pending:
                    break
                unchanged()
                attempts += 1
                try:
                    if compiler is None:
                        compiler = Compiler(root, profile=settings.compiler_profile)
                    data = compiler(root / path, (), flags)
                    if not isinstance(data, bytes) or not data:
                        raise ValueError("compiler returned no object bytes")
                    obj = Coff(data)
                    outcomes = [function_definition(obj, records[i]["emitted_symbol_hint"]) for i in pending]
                    trials.append({"path": path, "flags": list(flags), "result": Result.PASS})
                except (OSError, ValueError) as error:
                    outcomes = [{"result": Result.UNKNOWN, "reason": str(error)} for _ in pending]
                    trials.append({"path": path, "flags": list(flags), "result": Result.UNKNOWN, "reason": str(error)})
                unchanged()
                for i, outcome in zip(pending, outcomes):
                    observed[i].append({**outcome, "flags": list(flags)})
        for record, variants in zip(records, observed):
            results = {row["result"] for row in variants}
            result = Result.PASS if Result.PASS in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.FAIL
            rows.append({"entity_id": record["entity_id"], "address": f"0x{record['origin']['address']:08X}",
                         "path": path, "line": record["line"], "symbol": record["emitted_symbol_hint"],
                         "result": result, "variants": variants})
    unchanged()
    assert_snapshot_current(root, snapshot)
    counts = Counter(row["result"] for row in rows)
    result = Result.FAIL if counts[Result.FAIL] else Result.UNKNOWN if counts[Result.UNKNOWN] else Result.PASS
    return {"schema_version": 1, "check": "source_symbol_coverage", "result": result,
            "annotations": len(rows), "translation_units": len(groups), "compile_attempts": attempts,
            "counts": dict(sorted(counts.items())), "functions": rows, "trials": trials,
            "external_build_inputs": external, "selection": files,
            "input_fingerprint": digest({"inputs": before, "database": database_bytes.hex()}),
            "persisted": False, "ready_for_collection": False,
            "limitations": ["configured compiler emission, not actual final-link selection or byte/reference agreement",
                            "source catalogue names without emitted bindings may remain unresolved",
                            "seeds and declared exclusions are not product-emission requirements",
                            "file selection scopes this diagnostic, not whole-project acceptance"]}

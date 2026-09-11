"""Source-first mapped-function inspection; evidence inputs, never recovery claims."""

import base64
from collections import defaultdict
from dataclasses import asdict
from hashlib import sha256
from pathlib import Path

from .model import Span, digest
from .pe_coff import PeImage
from .source_config import SourceConfiguration
from .source_inventory import source_snapshot, assert_snapshot_current
from .source_selection import select_records, snapshot_mappings, selected_source_records


def inspect_source(root: Path, targets, *, in_file=None, image=None, callers=False, max_bytes=4 * 1024 * 1024):
    """Inspect exactly selected mappings and optionally their pinned original spans.

    Linear decoding follows declared spans, not inferred function or TU boundaries.
    Repeated calls, seed alternatives, shared tails and unresolved indirect edges
    stay visible. A caller search covers mapped spans only, never the whole binary.
    """
    root = root.resolve()
    if type(callers) is not bool or type(max_bytes) is not int or not 1 <= max_bytes <= 64 * 1024 * 1024:
        raise ValueError("inspection needs a boolean callers option and a byte budget from 1 to 67108864")
    if callers and image is None:
        raise ValueError("caller inspection requires the pinned original image")
    image_path = (image if image.is_absolute() else root / image).resolve() if image is not None else None
    snapshot = source_snapshot(root, image_path)
    implementation = {name: sha256(Path(__file__).with_name(name).read_bytes()).hexdigest()
                      for name in ("inspection.py", "source_selection.py")}
    source = SourceConfiguration.from_dict(snapshot["source_configuration"])
    # Inspection has always required lossless UTF-8 throughout its source
    # inventory. Validate that contract without re-extracting every body.
    for item in snapshot["files"].values():
        base64.b64decode(item["content_base64"]).decode("utf-8")
    records = snapshot_mappings(root, snapshot)
    host = source.path(root, in_file) if in_file is not None else None
    selected = selected_source_records(root, snapshot, select_records(records, targets, host, source.symbol_format))
    facts = {(row["path"], row["line"]): row for row in snapshot["records"]}
    by_address, owners = defaultdict(list), defaultdict(list)
    for record in records:
        fact = facts[record.path.relative_to(root).as_posix(), record.line]
        by_address[record.address].append(fact)
        for low, high in record.image_spans:
            if fact not in owners[low, high]:
                owners[low, high].append(fact)

    def identity(row):
        return {key: row[key] for key in ("entity_id", "path", "line", "catalogued_symbol",
                                         "emitted_symbol_hint", "candidate_kind", "kind_hint")}

    pe, imports, raw = None, {}, None
    if image_path is not None:
        if source.address_space != "pe:va":
            raise ValueError("image inspection currently supports PE i386 virtual-address mappings")
        raw = image_path.read_bytes()
        pe = PeImage(raw, snapshot["image_sha256"])
        imports = {item.address: asdict(item) for item in pe.imports()}
    decoded, consumed = {}, 0
    def decode(pair):
        nonlocal consumed
        if pair in decoded:
            return decoded[pair]
        from .source.asm import Listing
        from .source.calls import call_sites
        low, high = pair
        result = {"span": [low, high], "disassembly": [], "calls": [], "result": "unknown"}
        try:
            span = Span(low, high)
            if high - low > max_bytes - consumed:
                raise ValueError("inspection byte budget exhausted; span not decoded")
            data = pe.read(span)
            consumed += len(data)
            listing = Listing(data, low)
            result["disassembly"] = listing.lines
            result["decoded_bytes"] = sum(len(instruction.bytes) for instruction in listing.instructions)
            result["calls"] = [asdict(site) for site in call_sites(listing)]
            result["result"] = "pass" if result["decoded_bytes"] == len(data) else "unknown"
            if result["result"] == "unknown":
                result["reason"] = "declared span was not completely decoded; it may contain data or invalid instructions"
        except ValueError as error:
            result["reason"] = str(error)
        decoded[pair] = result
        return result

    def edges(record):
        output = []
        for pair in record.image_spans:
            for site in decode(pair)["calls"]:
                if site["form"] == "tail" and any(low <= site["target"] < high for low, high in record.image_spans):
                    continue  # A jump into this function's other recorded fragment.
                output.append({**site, "mapped_targets": [identity(row) for row in by_address.get(site["target"], [])],
                               "import": imports.get(site["slot"])})
        return sorted(output, key=lambda row: row["at"])

    subjects = []
    for record in selected:
        fact = facts[record.path.relative_to(root).as_posix(), record.line]
        row = {"source": fact, "source_region": record.region,
               "alternatives": [identity(item) for item in by_address[record.address] if item is not fact],
               "original": None}
        if pe is not None:
            spans = [{**decode(pair), "mapped_owners": [identity(item) for item in owners[pair]]}
                     for pair in record.image_spans]
            row["original"] = {"spans": spans, "calls": edges(record),
                               "result": "pass" if spans and all(item["result"] == "pass" for item in spans) else "unknown"}
        subjects.append(row)
    incoming, caller_errors = [], []
    if callers:
        wanted = {record.address for record in selected}
        for record in records:
            if not record.image_spans:
                caller_errors.append({"path": record.path.relative_to(root).as_posix(), "line": record.line,
                                      "reason": "mapping has no original spans"})
            for edge in edges(record):
                if edge["target"] in wanted:
                    incoming.append({"caller": identity(facts[record.path.relative_to(root).as_posix(), record.line]), **edge})
        caller_errors.extend({"span": list(pair), "reason": item["reason"]}
                             for pair, item in decoded.items() if item["result"] != "pass")
    assert_snapshot_current(root, snapshot)
    if image_path is not None and image_path.read_bytes() != raw:
        raise ValueError("original image changed during inspection")
    if implementation != {name: sha256(Path(__file__).with_name(name).read_bytes()).hexdigest() for name in implementation}:
        raise ValueError("inspection implementation changed during inspection")
    unknown = any(row["original"] and row["original"]["result"] != "pass" for row in subjects) or bool(caller_errors)
    return {"schema_version": 1, "operation": "inspect", "result": "unknown" if unknown else "pass",
            "subjects": subjects, "image_verified": pe is not None,
            "callers": {"requested": callers, "edges": incoming, "errors": caller_errors,
                        "coverage": "mapped_spans_only" if callers else "not_requested"},
            "decoded_bytes": consumed, "byte_budget": max_bytes, "persisted": False,
            "verification": "not_measured", "ready_for_collection": False,
            "input_fingerprint": digest({"source": {name: row["sha256"] for name, row in snapshot["files"].items()},
                                         "image": snapshot["image_sha256"], "targets": list(targets), "host": in_file,
                                         "image_requested": image is not None, "configuration": snapshot["source_configuration"],
                                         "revision": snapshot["revision"], "implementation": implementation,
                                         "callers": callers, "max_bytes": max_bytes}),
            "limitations": ["source annotations and name/kind labels are hints or historical obligations, not current measurements",
                            "linear disassembly inside declared spans, not executable coverage, function extent or historical TU proof",
                            "caller search covers mapped spans only; no dynamic dispatch resolution or inference from nearby names",
                            "imports and duplicate source/seed target interpretations are retained, not silently filtered"]}

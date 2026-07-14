#!/usr/bin/env python3

import argparse
from collections import defaultdict
import csv
import json
from pathlib import Path
import re


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DEFINITION = REPO_ROOT / "src" / "OpenSMACX.def"
DEFAULT_INVENTORY = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_EXTERNAL_DIR = REPO_ROOT / ".opensmacx" / "external-analysis"
DEFAULT_OUTPUT = DEFAULT_EXTERNAL_DIR / "export-recovery-queue.csv"
EXPORT_RE = re.compile(r'^\s*"(?P<export>[^"]+)"\s*=\s*(?P<implementation>\S+)\s*$')
STATE_TIERS = {
    "original_dependency": "P0",
    "source_in_progress": "P1",
    "redirect_name_match": "P2",
    "unrecovered": "P3",
    "source_complete": "P4",
}


def symbol_stem(name):
    return name.split("@@", 1)[0] if name.startswith("?") else name


def load_exports(path):
    exports = []
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        match = EXPORT_RE.match(line)
        if match:
            exports.append((match.group("export"), match.group("implementation")))
    return exports


def load_inventory(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        return list(csv.DictReader(file))


def map_exports(exports, functions):
    exact = {row["name"]: row for row in functions}
    stems = defaultdict(list)
    for row in functions:
        stems[symbol_stem(row["name"])].append(row)
    mapped = []
    for export_name, implementation in exports:
        row = exact.get(export_name)
        relation = "exact" if row else ""
        if row is None:
            candidates = stems[symbol_stem(export_name)]
            if len(candidates) == 1:
                row = candidates[0]
                relation = "unique_stem"
            elif candidates:
                relation = "ambiguous_stem"
            else:
                relation = "missing"
        mapped.append((export_name, implementation, relation, row))
    return mapped


def load_external_counts(directory, catalog_path):
    counts = defaultdict(int)
    sources = defaultdict(set)
    if not directory.is_dir():
        return counts, sources
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    expected = {
        artifact["path"]: artifact["sha256"]
        for artifact in catalog["artifacts"]
        if artifact["kind"] in {
            "human_function_notes", "generated_disassembly_with_game_text"}}
    for source_path, source_sha256 in sorted(expected.items()):
        path = directory / (
            re.sub(r"[^a-z0-9]+", "-", source_path.lower()).strip("-")
            + "-summary.csv")
        if not path.is_file():
            continue
        with path.open(newline="", encoding="utf-8") as file:
            rows = list(csv.DictReader(file))
        if not rows:
            continue
        if (rows[0].get("source_path") != source_path
                or rows[0].get("source_sha256") != source_sha256):
            raise RuntimeError(f"external summary provenance mismatch: {path}")
        if any(row.get("source_path") != source_path
               or row.get("source_sha256") != source_sha256 for row in rows):
            raise RuntimeError(f"mixed external summary provenance: {path}")
        source_name = re.sub(r"[^a-z0-9]+", "-", source_path.lower()).strip("-")
        for row in rows:
            counts[row["canonical_address"]] += int(row["lead_count"])
            sources[row["canonical_address"]].add(source_name)
    return counts, sources


def queue_rows(mapped, external_counts, external_sources):
    rows = []
    for export_name, implementation, relation, function in mapped:
        state = function["recovery_state"] if function else "unmapped"
        tier = STATE_TIERS.get(state, "P5")
        address = function["address"] if function else ""
        rows.append({
            "tier": tier,
            "export_name": export_name,
            "implementation_symbol": implementation,
            "mapping": relation,
            "canonical_address": address,
            "canonical_name": function["name"] if function else "",
            "recovery_state": state,
            "binary_kind": function["binary_kind"] if function else "",
            "size": function["size"] if function else "",
            "caller_count": function["caller_count"] if function else "",
            "call_target_count": function["call_target_count"] if function else "",
            "external_lead_count": external_counts.get(address, 0),
            "external_sources": ";".join(sorted(external_sources.get(address, set()))),
        })
    rows.sort(key=lambda row: (
        row["tier"],
        -int(row["external_lead_count"]),
        -int(row["caller_count"] or 0),
        int(row["size"] or 0),
        row["export_name"],
    ))
    for rank, row in enumerate(rows, 1):
        row["rank"] = rank
    return rows


def main():
    parser = argparse.ArgumentParser(
        description="Build an exported-first source recovery queue")
    parser.add_argument("--definition", type=Path, default=DEFAULT_DEFINITION)
    parser.add_argument("--inventory", type=Path, default=DEFAULT_INVENTORY)
    parser.add_argument("--external-dir", type=Path, default=DEFAULT_EXTERNAL_DIR)
    parser.add_argument("--catalog", type=Path,
                        default=REPO_ROOT / "docs" / "recovery" /
                        "external-analysis-sources.json")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()

    for path in (args.definition, args.inventory):
        if not path.is_file():
            parser.error(f"input not found: {path}")
    external_counts, external_sources = load_external_counts(
        args.external_dir, args.catalog)
    rows = queue_rows(
        map_exports(load_exports(args.definition), load_inventory(args.inventory)),
        external_counts, external_sources)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fieldnames = [
        "rank", "tier", "export_name", "implementation_symbol", "mapping",
        "canonical_address", "canonical_name", "recovery_state", "binary_kind",
        "size", "caller_count", "call_target_count", "external_lead_count",
        "external_sources",
    ]
    with args.output.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)
    counts = defaultdict(int)
    for row in rows:
        counts[row["tier"]] += 1
    print(f"Wrote {len(rows)} exports to {args.output}")
    print("Tiers: " + ", ".join(f"{tier}={counts[tier]}" for tier in sorted(counts)))


if __name__ == "__main__":
    main()

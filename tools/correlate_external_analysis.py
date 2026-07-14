#!/usr/bin/env python3

import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import json
from pathlib import Path
import re

from fetch_external_analysis import validated_relative_path


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CATALOG = REPO_ROOT / "docs" / "recovery" / "external-analysis-sources.json"
DEFAULT_INVENTORY = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_SOURCE_DIR = REPO_ROOT / ".opensmacx" / "external-analysis" / "sources"
DEFAULT_OUTPUT_DIR = REPO_ROOT / ".opensmacx" / "external-analysis"
FUNCTION_NOTE_RE = re.compile(
    r"^\s*(?P<addresses>[0-9A-Fa-f]{6,8}(?:/[0-9A-Fa-f]{1,8})*):")
DISASSEMBLY_RE = re.compile(r"^\s*(?P<address>[0-9A-Fa-f]{8})\s+")


def expand_addresses(value):
    parts = value.split("/")
    first = parts[0]
    addresses = [int(first, 16)]
    for suffix in parts[1:]:
        expanded = suffix if len(suffix) >= len(first) else first[:-len(suffix)] + suffix
        addresses.append(int(expanded, 16))
    return addresses


def parse_function_notes(text):
    leads = []
    for line_number, line in enumerate(text.splitlines(), 1):
        match = FUNCTION_NOTE_RE.match(line)
        if match:
            leads.extend(
                (line_number, address)
                for address in expand_addresses(match.group("addresses")))
    return leads


def parse_disassembly(text):
    leads = []
    for line_number, line in enumerate(text.splitlines(), 1):
        match = DISASSEMBLY_RE.match(line)
        if match:
            leads.append((line_number, int(match.group("address"), 16)))
    return leads


def parse_ranges(value):
    ranges = []
    for item in value.split(";"):
        if item:
            start, end = item.split("-", 1)
            ranges.append((int(start, 0), int(end, 0)))
    return ranges


def load_inventory(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        rows = list(csv.DictReader(file))
    entries = {}
    ranges = []
    for row in rows:
        start = int(row["address"], 0)
        entries[start] = row
        ranges.extend(
            (range_start, range_end, row)
            for range_start, range_end in parse_ranges(row["body_ranges"]))
    return entries, sorted(ranges, key=lambda item: (item[0], item[1]))


def correlate_leads(leads, entries, ranges):
    correlated = []
    for line_number, address in leads:
        row = entries.get(address)
        relation = "entry" if row else ""
        if row is None:
            matches = {}
            for range_start, range_end, candidate in ranges:
                if range_start <= address < range_end:
                    matches[candidate["address"]] = candidate
            if len(matches) == 1:
                row = next(iter(matches.values()))
                relation = "interior"
            elif matches:
                relation = "ambiguous"
        correlated.append((line_number, address, relation, row))
    return correlated


def summarize_correlations(rows):
    grouped = defaultdict(list)
    for row in rows:
        if row[3]:
            grouped[row[3]["address"]].append(row)
    summary = []
    for address, items in grouped.items():
        relations = Counter(item[2] for item in items)
        function = items[0][3]
        summary.append({
            "canonical_address": address,
            "canonical_name": function["name"],
            "recovery_state": function["recovery_state"],
            "lead_count": len(items),
            "entry_count": relations["entry"],
            "interior_count": relations["interior"],
        })
    return sorted(summary, key=lambda row: (-row["lead_count"], row["canonical_address"]))


def load_artifact(catalog_path, source_path):
    catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
    for artifact in catalog["artifacts"]:
        if artifact["path"] == source_path:
            return artifact
    raise RuntimeError(f"source path is absent from catalog: {source_path}")


def output_stem(source_path):
    return re.sub(r"[^a-z0-9]+", "-", source_path.lower()).strip("-")


def write_correlations(path, source_path, source_sha256, rows):
    fieldnames = [
        "source_path", "source_sha256", "line", "address", "relation",
        "canonical_address", "canonical_name", "recovery_state",
    ]
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        for line_number, address, relation, row in rows:
            writer.writerow({
                "source_path": source_path,
                "source_sha256": source_sha256,
                "line": line_number,
                "address": f"0x{address:08X}",
                "relation": relation or "missing",
                "canonical_address": row["address"] if row else "",
                "canonical_name": row["name"] if row else "",
                "recovery_state": row["recovery_state"] if row else "",
            })


def write_summary(path, source_path, source_sha256, rows):
    fieldnames = [
        "source_path", "source_sha256", "canonical_address", "canonical_name",
        "recovery_state", "lead_count", "entry_count", "interior_count",
    ]
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        for row in summarize_correlations(rows):
            writer.writerow({
                "source_path": source_path,
                "source_sha256": source_sha256,
                **row,
            })


def main():
    parser = argparse.ArgumentParser(
        description="Correlate hash-pinned external analysis with canonical functions")
    parser.add_argument("--source-path", required=True)
    parser.add_argument("--source-dir", type=Path, default=DEFAULT_SOURCE_DIR)
    parser.add_argument("--catalog", type=Path, default=DEFAULT_CATALOG)
    parser.add_argument("--inventory", type=Path, default=DEFAULT_INVENTORY)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    args = parser.parse_args()

    artifact = load_artifact(args.catalog, args.source_path)
    source = args.source_dir / validated_relative_path(args.source_path)
    if not source.is_file() or not args.inventory.is_file():
        parser.error("source artifact or canonical inventory not found")
    data = source.read_bytes()
    actual_sha256 = hashlib.sha256(data).hexdigest()
    if actual_sha256 != artifact["sha256"]:
        parser.error(
            f"source SHA-256 mismatch: expected {artifact['sha256']}, got {actual_sha256}")
    text = data.decode(artifact["encoding"])
    if artifact["kind"] == "human_function_notes":
        leads = parse_function_notes(text)
    elif artifact["kind"] == "generated_disassembly_with_game_text":
        leads = parse_disassembly(text)
    else:
        parser.error(f"artifact kind does not contain function addresses: {artifact['kind']}")
    entries, ranges = load_inventory(args.inventory)
    rows = correlate_leads(leads, entries, ranges)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = output_stem(args.source_path)
    correlations = args.output_dir / f"{stem}-correlation.csv"
    summary = args.output_dir / f"{stem}-summary.csv"
    write_correlations(correlations, args.source_path, actual_sha256, rows)
    write_summary(summary, args.source_path, actual_sha256, rows)
    print(f"Correlated {len(rows)} address leads to {correlations}")
    print(f"Wrote per-function summary to {summary}")


if __name__ == "__main__":
    main()

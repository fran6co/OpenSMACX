#!/usr/bin/env python3

import argparse
from bisect import bisect_right
from collections import Counter
import csv
import hashlib
import json
from pathlib import Path
import re


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CANONICAL = REPO_ROOT / "docs" / "recovery" / "functions.csv"
DEFAULT_CANONICAL_SUMMARY = REPO_ROOT / "docs" / "recovery" / "summary.json"
DEFAULT_BINDING_CLASSIFICATIONS = (
    REPO_ROOT / "docs" / "recovery-binding-classifications.csv")
DEFAULT_IDA = REPO_ROOT / "docs" / "recovery" / "ida9-functions.csv"
DEFAULT_GHIDRA = REPO_ROOT / "docs" / "recovery" / "ghidra-functions.csv"
DEFAULT_GHIDRA_REFERENCES = (
    REPO_ROOT / "docs" / "recovery" / "ghidra-interior-references.csv")
DEFAULT_OUTPUT_DIR = REPO_ROOT / "docs" / "recovery"
ADD_FUNCTION_RE = re.compile(
    r"add_func\(0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)\);")
FUNCTION_FLAGS_RE = re.compile(
    r"set_func_flags\(0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+)\);")
NAME_RE = re.compile(
    r'set_name\(0x([0-9A-Fa-f]+),\s*"((?:\\.|[^"\\])*)"\);')
TYPE_RE = re.compile(
    r'apply_type\(0x([0-9A-Fa-f]+),\s*"((?:\\.|[^"\\])*)"\);')
FUNCTION_TAIL_RE = re.compile(
    r"append_func_tail\(0x([0-9A-Fa-f]+),\s*0x([0-9A-Fa-f]+),\s*"
    r"0x([0-9A-Fa-f]+)\);")


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def decode_idc_string(value):
    return value.replace(r"\"", '"').replace(r"\\", "\\")


def merge_ranges(ranges):
    result = []
    for start, end in sorted(ranges):
        if result and start <= result[-1][1]:
            result[-1] = (result[-1][0], max(result[-1][1], end))
        else:
            result.append((start, end))
    return result


def parse_ranges(value):
    ranges = []
    for item in value.split(";"):
        if not item:
            continue
        start, end = item.split("-", 1)
        ranges.append((int(start, 0), int(end, 0)))
    return merge_ranges(ranges)


def format_ranges(ranges):
    return ";".join(f"0x{start:08X}-0x{end:08X}" for start, end in ranges)


def load_canonical(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        rows = list(csv.DictReader(file))
    functions = []
    for row in rows:
        functions.append({
            **row,
            "start": int(row["address"], 0),
            "end": int(row["end_address"], 0),
            "body_ranges_value": parse_ranges(row["body_ranges"]),
            "size_value": int(row["size"]),
            "caller_count_value": int(row["caller_count"]),
            "call_target_count_value": int(row["call_target_count"]),
        })
    return functions


def load_binding_classifications(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)
        required = {"symbol", "category", "strategy", "notes"}
        if not reader.fieldnames or not required.issubset(reader.fieldnames):
            raise RuntimeError(
                f"{path} must contain columns: {', '.join(sorted(required))}")
        rows = list(reader)
    classifications = {}
    for row in rows:
        symbol = row["symbol"].strip()
        if not symbol:
            raise RuntimeError("binding classification has an empty symbol")
        if symbol in classifications:
            raise RuntimeError(f"duplicate binding classification: {symbol}")
        for field in ("category", "strategy", "notes"):
            row[field] = row[field].strip()
            if not row[field]:
                raise RuntimeError(
                    f"binding classification {symbol} has an empty {field}")
        classifications[symbol] = row
    return classifications


def load_idc(path):
    text = path.read_text(encoding="utf-8-sig", errors="replace")
    flags = {int(address, 16): int(value, 16)
             for address, value in FUNCTION_FLAGS_RE.findall(text)}
    names = {int(address, 16): decode_idc_string(value)
             for address, value in NAME_RE.findall(text)}
    types = {int(address, 16): decode_idc_string(value)
             for address, value in TYPE_RE.findall(text)}
    tails = {}
    for owner, start, end in FUNCTION_TAIL_RE.findall(text):
        tails.setdefault(int(owner, 16), []).append((int(start, 16), int(end, 16)))
    functions = []
    for start_text, end_text in ADD_FUNCTION_RE.findall(text):
        start = int(start_text, 16)
        end = int(end_text, 16)
        body_ranges = merge_ranges([(start, end), *tails.get(start, [])])
        functions.append({
            "start": start,
            "end": end,
            "size": sum(range_end - range_start
                        for range_start, range_end in body_ranges),
            "body_ranges_value": body_ranges,
            "name": names.get(start, f"sub_{start:X}"),
            "prototype": types.get(start, ""),
            "flags": flags.get(start, 0),
        })
    return functions


def load_ida(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        rows = list(csv.DictReader(file))
    functions = [{
        "start": int(row["address"], 0),
        "end": int(row["end_address"], 0),
        "size": int(row["size"]),
        "body_ranges_value": parse_ranges(row["body_ranges"]),
        "name": row["name"],
        "prototype": row["prototype"],
        "flags": int(row["flags"], 0),
    } for row in rows]
    metadata = {
        "source_idc_sha256": rows[0]["source_idc_sha256"] if rows else "",
    }
    return functions, metadata


def load_ghidra(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        rows = list(csv.DictReader(file))
    functions = [{
        "start": int(row["address"], 0),
        "end": int(row["end_address"], 0),
        "size": int(row["size"]),
        "body_ranges_value": parse_ranges(row["body_ranges"]),
        "name": row["name"],
        "prototype": row["signature"],
        "is_thunk": row["is_thunk"].casefold() == "true",
    } for row in rows]
    metadata = {
        "program_sha256": rows[0]["program_sha256"] if rows else "",
        "ghidra_version": rows[0]["ghidra_version"] if rows else "",
        "language": rows[0]["language"] if rows else "",
    }
    return functions, metadata


class AnalysisIndex:
    def __init__(self, functions):
        self.functions = sorted(functions, key=lambda item: (item["start"], item["end"]))
        self.starts = [function["start"] for function in self.functions]
        self.exact = {function["start"]: function for function in self.functions}

    def correlate(self, canonical):
        same_start = self.exact.get(canonical["start"])
        if same_start:
            if same_start["body_ranges_value"] == canonical["body_ranges_value"]:
                relation = "exact"
            elif same_start["end"] == canonical["end"]:
                relation = "entry_range"
            else:
                relation = "start_only"
            return relation, same_start

        index = bisect_right(self.starts, canonical["start"]) - 1
        if index >= 0 and canonical["start"] < self.functions[index]["end"]:
            return "containing", self.functions[index]
        next_index = index + 1
        if (next_index < len(self.functions) and
                self.functions[next_index]["start"] < canonical["end"]):
            return "split", self.functions[next_index]
        return "missing", None


def write_csv(path, fieldnames, rows):
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)


def analyzer_columns(prefix, relation, match):
    return {
        f"{prefix}_relation": relation,
        f"{prefix}_address": f"0x{match['start']:08X}" if match else "",
        f"{prefix}_end_address": f"0x{match['end']:08X}" if match else "",
        f"{prefix}_size": match["size"] if match else "",
        f"{prefix}_body_ranges": (
            format_ranges(match["body_ranges_value"]) if match else ""),
        f"{prefix}_name": match["name"] if match else "",
    }


def main():
    parser = argparse.ArgumentParser(
        description="Correlate independent analyses and rank recovery work")
    parser.add_argument("--canonical", type=Path, default=DEFAULT_CANONICAL)
    parser.add_argument("--canonical-summary", type=Path,
                        default=DEFAULT_CANONICAL_SUMMARY)
    parser.add_argument("--binding-classifications", type=Path,
                        default=DEFAULT_BINDING_CLASSIFICATIONS)
    parser.add_argument("--ida", type=Path, default=DEFAULT_IDA)
    parser.add_argument("--ghidra", type=Path, default=DEFAULT_GHIDRA)
    parser.add_argument("--ghidra-references", type=Path,
                        default=DEFAULT_GHIDRA_REFERENCES)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    args = parser.parse_args()
    for path in (args.canonical, args.canonical_summary,
                 args.binding_classifications, args.ida, args.ghidra,
                 args.ghidra_references):
        if not path.is_file():
            parser.error(f"input not found: {path}")

    canonical = load_canonical(args.canonical)
    canonical_summary = json.loads(args.canonical_summary.read_text(encoding="utf-8"))
    binding_classifications = load_binding_classifications(
        args.binding_classifications)
    current_binding_symbols = {
        symbol.strip()
        for function in canonical
        for symbol in function["original_dependencies"].split("|")
        if symbol.strip()
    }
    missing_classifications = sorted(
        current_binding_symbols - set(binding_classifications))
    stale_classifications = sorted(
        set(binding_classifications) - current_binding_symbols)
    if missing_classifications:
        raise RuntimeError(
            "unclassified original function bindings: " +
            ", ".join(missing_classifications))
    if stale_classifications:
        raise RuntimeError(
            "classifications without live function bindings: " +
            ", ".join(stale_classifications))
    ida_functions, ida_metadata = load_ida(args.ida)
    ghidra_functions, ghidra_metadata = load_ghidra(args.ghidra)
    analyses = {
        "ida9": AnalysisIndex(ida_functions),
        "ghidra": AnalysisIndex(ghidra_functions),
    }
    relation_counts = {name: Counter() for name in analyses}
    correlated = []
    priorities = []
    for function in canonical:
        row = {
            "address": function["address"],
            "end_address": function["end_address"],
            "size": function["size"],
            "body_ranges": format_ranges(function["body_ranges_value"]),
            "name": function["name"],
            "binary_kind": function["binary_kind"],
            "recovery_state": function["recovery_state"],
        }
        entry_agreement = 0
        for name, index in analyses.items():
            relation, match = index.correlate(function)
            relation_counts[name][relation] += 1
            row.update(analyzer_columns(name, relation, match))
            if relation in {"exact", "entry_range", "start_only"}:
                entry_agreement += 1
        correlated.append(row)

        dependency_symbols = [
            symbol.strip() for symbol in function["original_dependencies"].split("|")
            if symbol.strip()
        ]
        dependency = bool(dependency_symbols)
        if not dependency and not (
                function["binary_kind"] == "game" and
                function["recovery_state"] == "unrecovered"):
            continue
        score = (1_000_000 if dependency else 0)
        score += function["caller_count_value"] * 1_000
        score += function["call_target_count_value"] * 100
        score += entry_agreement * 10
        score += bool(function["prototype"])
        if dependency:
            tier = "P0"
        elif function["caller_count_value"] >= 20:
            tier = "P1"
        elif function["caller_count_value"] >= 5:
            tier = "P2"
        else:
            tier = "P3"
        priorities.append({
            "priority": tier,
            "score": score,
            "address": function["address"],
            "name": function["name"],
            "size": function["size"],
            "caller_count": function["caller_count"],
            "call_target_count": function["call_target_count"],
            "recovery_state": function["recovery_state"],
            "binding_category": ";".join(sorted({
                binding_classifications[symbol]["category"]
                for symbol in dependency_symbols
            })),
            "binding_strategy": ";".join(sorted({
                binding_classifications[symbol]["strategy"]
                for symbol in dependency_symbols
            })),
            "analysis_entry_agreement": entry_agreement,
            "ida9_relation": row["ida9_relation"],
            "ghidra_relation": row["ghidra_relation"],
            "prototype": function["prototype"],
        })

    priorities.sort(key=lambda row: (
        0 if row["priority"] == "P0" else 1,
        -row["score"],
        int(row["address"], 0),
    ))
    for rank, row in enumerate(priorities, start=1):
        row["rank"] = rank

    args.output_dir.mkdir(parents=True, exist_ok=True)
    correlation_fields = list(correlated[0])
    write_csv(args.output_dir / "analysis-correlation.csv", correlation_fields, correlated)
    priority_fields = [
        "rank", "priority", "score", "address", "name", "size", "caller_count",
        "call_target_count", "recovery_state", "binding_category", "binding_strategy",
        "analysis_entry_agreement", "ida9_relation", "ghidra_relation", "prototype",
    ]
    write_csv(args.output_dir / "priorities.csv", priority_fields, priorities)

    canonical_starts = {function["start"] for function in canonical}
    ida_starts = {function["start"] for function in ida_functions}
    ghidra_starts = {function["start"] for function in ghidra_functions}
    ida_kinds = Counter(
        "thunk" if function["flags"] & 0x80 else
        "library" if function["flags"] & 0x4 else "game"
        for function in ida_functions
    )
    canonical_program_hash = canonical_summary["inputs"]["original_input_sha256"]
    ghidra_program_hash = ghidra_metadata["program_sha256"]
    summary = {
        "format_version": 1,
        "canonical_functions": len(canonical),
        "inputs": {
            "binding_classifications_sha256": sha256(
                args.binding_classifications),
            "canonical_inventory_sha256": sha256(args.canonical),
            "canonical_summary_sha256": sha256(args.canonical_summary),
        },
        "binary_identity": {
            "canonical_program_sha256": canonical_program_hash,
            "ghidra_program_sha256": ghidra_program_hash,
            "ghidra_matches_canonical": ghidra_program_hash == canonical_program_hash,
            "ida9_program_sha256": "unknown",
            "comparison_mode": (
                "same_binary" if ghidra_program_hash == canonical_program_hash
                else "cross_build"),
        },
        "analyses": {
            "ida9": {
                "functions": len(ida_functions),
                "input_sha256": sha256(args.ida),
                **ida_metadata,
                "by_binary_kind": dict(sorted(ida_kinds.items())),
                "shared_entry_points": len(canonical_starts & ida_starts),
                "extra_entry_points": len(ida_starts - canonical_starts),
                "relations": dict(sorted(relation_counts["ida9"].items())),
            },
            "ghidra": {
                "functions": len(ghidra_functions),
                "input_sha256": sha256(args.ghidra),
                "interior_references_input_sha256": sha256(
                    args.ghidra_references),
                "shared_entry_points": len(canonical_starts & ghidra_starts),
                "extra_entry_points": len(ghidra_starts - canonical_starts),
                "thunks": sum(function["is_thunk"] for function in ghidra_functions),
                "relations": dict(sorted(relation_counts["ghidra"].items())),
                **ghidra_metadata,
            },
        },
        "outputs": {
            "analysis_correlation_sha256": sha256(
                args.output_dir / "analysis-correlation.csv"),
            "priorities_sha256": sha256(args.output_dir / "priorities.csv"),
        },
        "priorities": {
            "total": len(priorities),
            "by_tier": dict(sorted(Counter(row["priority"] for row in priorities).items())),
            "original_dependencies": sum(
                row["priority"] == "P0" for row in priorities),
            "bindings_by_category": dict(sorted(Counter(
                row["binding_category"] for row in priorities
                if row["priority"] == "P0").items())),
        },
    }
    (args.output_dir / "analysis-summary.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Correlated {len(canonical)} canonical functions")
    print(f"Ranked {len(priorities)} recovery candidates")


if __name__ == "__main__":
    main()

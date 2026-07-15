#!/usr/bin/env python3

import argparse
from bisect import bisect_right
from collections import Counter, defaultdict
import csv
import hashlib
import json
import logging
from pathlib import Path
import re

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM
import idb
import idb.analysis
from idb.idapython import IDAPython


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_IDB = (
    REPO_ROOT / ".opensmacx" / "analysis" /
    "terranx_ORIG_200_v3_7.5.SP3.idb")
DEFAULT_SOURCE_DIR = REPO_ROOT / "src"
DEFAULT_DEFINITION = DEFAULT_SOURCE_DIR / "OpenSMACX.def"
DEFAULT_OVERRIDES = REPO_ROOT / "docs" / "recovery-overrides.csv"
DEFAULT_OUTPUT_DIR = REPO_ROOT / "docs" / "recovery"
SUPPORTED_IDB_HASHES = {
    "6ffdcf2d6644f2c1b19c218d3b1b293b4e442d56b8cf1f537b0403608ff866fa",
}

OFFSET_RE = re.compile(r"Original Offset:\s*(?:0x)?([0-9A-Fa-f]{6,8})\b")
STATUS_RE = re.compile(r"Status:\s*(.+)")
DEFINITION_RE = re.compile(r'^\s*"([^"]+)"\s*=')
BINDING_RE = re.compile(
    r"^(?P<declaration>.+?)\s*=\s*\((?P<cast>[^)]+)\)\s*"
    r"(?P<address>0x[0-9A-Fa-f]+)\s*;",
    re.MULTILINE,
)
FUNCTION_CAST_RE = re.compile(r"func\w*\s*\*\s*$")
RECOVERY_STATES = {
    "external_library",
    "original_dependency",
    "redirect_name_match",
    "source_complete",
    "source_in_progress",
    "thunk",
    "unrecovered",
}


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def repo_path(path):
    try:
        return path.resolve().relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return str(path.resolve())


def strip_cpp_comments(text):
    result = []
    index = 0
    in_block = False
    in_line = False
    in_string = None
    while index < len(text):
        character = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""

        if in_line:
            if character == "\n":
                in_line = False
                result.append(character)
            else:
                result.append(" ")
        elif in_block:
            if character == "*" and following == "/":
                result.extend("  ")
                index += 1
                in_block = False
            else:
                result.append("\n" if character == "\n" else " ")
        elif in_string:
            result.append(character)
            if character == "\\" and following:
                result.append(following)
                index += 1
            elif character == in_string:
                in_string = None
        elif character == "/" and following == "/":
            result.extend("  ")
            index += 1
            in_line = True
        elif character == "/" and following == "*":
            result.extend("  ")
            index += 1
            in_block = True
        else:
            result.append(character)
            if character in ('"', "'"):
                in_string = character
        index += 1
    return "".join(result)


def load_source_annotations(source_dir):
    annotations = defaultdict(list)
    for path in sorted(source_dir.glob("*.cpp")):
        lines = path.read_text(encoding="utf-8-sig", errors="replace").splitlines()
        for index, line in enumerate(lines):
            match = OFFSET_RE.search(line)
            if not match:
                continue
            address = int(match.group(1), 16)
            status = ""
            for candidate in lines[index + 1:index + 12]:
                status_match = STATUS_RE.search(candidate)
                if status_match:
                    status = status_match.group(1).strip(" */\t\r\n")
                    break
            annotations[address].append({
                "location": f"{repo_path(path)}:{index + 1}",
                "status": status,
            })
    return annotations


def load_source_bindings(source_dir):
    bindings = []
    for path in sorted(source_dir.glob("*.cpp")):
        text = path.read_text(encoding="utf-8-sig", errors="replace")
        uncommented = strip_cpp_comments(text)
        for match in BINDING_RE.finditer(uncommented):
            declaration = " ".join(match.group("declaration").split())
            names = re.findall(r"[A-Za-z_]\w*", declaration)
            if not names:
                continue
            cast = " ".join(match.group("cast").split())
            bindings.append({
                "symbol": names[-1],
                "address": int(match.group("address"), 16),
                "kind": "function" if FUNCTION_CAST_RE.search(cast) else "data",
                "type": cast,
                "location": f"{repo_path(path)}:{uncommented.count(chr(10), 0, match.start()) + 1}",
            })
    return bindings


def load_redirects(path):
    redirects = []
    for line_number, line in enumerate(
            path.read_text(encoding="utf-8-sig").splitlines(), start=1):
        match = DEFINITION_RE.match(line)
        if match:
            redirects.append({
                "name": match.group(1),
                "location": f"{repo_path(path)}:{line_number}",
            })
    return redirects


def load_overrides(path):
    if not path.is_file():
        return {}
    overrides = {}
    with path.open(newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)
        required = {"address", "recovery_state", "priority", "notes"}
        if not reader.fieldnames or not required.issubset(reader.fieldnames):
            raise RuntimeError(
                f"{path} must contain columns: {', '.join(sorted(required))}")
        for row in reader:
            if not row["address"].strip():
                continue
            address = int(row["address"], 0)
            if address in overrides:
                raise RuntimeError(f"duplicate override for 0x{address:08X}")
            recovery_state = row["recovery_state"].strip()
            if recovery_state and recovery_state not in RECOVERY_STATES:
                raise RuntimeError(
                    f"unknown recovery state for 0x{address:08X}: {recovery_state}")
            overrides[address] = {
                "recovery_state": recovery_state,
                "priority": row["priority"].strip(),
                "notes": row["notes"].strip(),
            }
    return overrides


def flag_names(flags, idaapi):
    names = []
    for name in (
            "FUNC_NORET", "FUNC_FAR", "FUNC_LIB", "FUNC_STATICDEF",
            "FUNC_FRAME", "FUNC_USERFAR", "FUNC_HIDDEN", "FUNC_THUNK",
            "FUNC_BOTTOMBP", "FUNC_NORET_PENDING", "FUNC_SP_READY",
            "FUNC_PURGED_OK"):
        value = getattr(idaapi, name, 0)
        if value and flags & value:
            names.append(name.removeprefix("FUNC_").lower())
    return names


def flatten(values):
    return " | ".join(value.replace("\r", "").replace("\n", "\\n")
                      for value in values if value)


def assign_comments(functions, comments):
    starts = [function["start"] for function in functions]
    for address, text in comments.items():
        index = bisect_right(starts, address) - 1
        if index >= 0 and address < functions[index]["end"]:
            functions[index]["comments"].append(
                f"0x{address:08X}: {text}")


def derive_recovery_state(function):
    if function["source_annotations"]:
        statuses = [item["status"].casefold()
                    for item in function["source_annotations"] if item["status"]]
        if statuses and all(status.startswith("complete") for status in statuses):
            return "source_complete"
        return "source_in_progress"
    if function["redirects"]:
        return "redirect_name_match"
    if function["dependencies"]:
        return "original_dependency"
    if function["binary_kind"] == "library":
        return "external_library"
    if function["binary_kind"] == "thunk":
        return "thunk"
    return "unrecovered"


def export_inventory(args):
    source_annotations = load_source_annotations(args.source_dir)
    source_bindings = load_source_bindings(args.source_dir)
    redirects = load_redirects(args.definition)
    overrides = load_overrides(args.overrides)
    redirects_by_name = defaultdict(list)
    for redirect in redirects:
        redirects_by_name[redirect["name"]].append(redirect)
    function_bindings = defaultdict(list)
    for binding in source_bindings:
        if binding["kind"] == "function":
            function_bindings[binding["address"]].append(binding)

    logging.getLogger("idb").setLevel(logging.ERROR)
    with idb.from_file(str(args.idb)) as database:
        api = IDAPython(database)
        idc = api.idc
        idautils = api.idautils
        idaapi = api.idaapi
        ida_bytes = api.ida_bytes
        root = idb.analysis.Root(database)
        function_analysis = idb.analysis.Functions(database)

        functions = []
        functions_by_address = {}
        for start in idautils.Functions():
            chunks = list(idautils.Chunks(start))
            end = idc.GetFunctionAttr(start, idc.FUNCATTR_END)
            if not chunks:
                chunks = [(start, end)]
            flags = idc.GetFunctionFlags(start)
            kind = "game"
            if flags & idaapi.FUNC_THUNK:
                kind = "thunk"
            elif flags & idaapi.FUNC_LIB:
                kind = "library"
            function = {
                "start": start,
                "end": end,
                "size": sum(chunk_end - chunk_start
                            for chunk_start, chunk_end in chunks),
                "chunks": chunks,
                "segment": idc.SegName(start),
                "name": idc.GetFunctionName(start),
                "prototype": idc.GetType(start) or "",
                "flags": flags,
                "flag_names": flag_names(flags, idaapi),
                "binary_kind": kind,
                "source_annotations": source_annotations.get(start, []),
                "redirects": redirects_by_name.get(idc.GetFunctionName(start), []),
                "dependencies": function_bindings.get(start, []),
                "comments": [],
                "call_targets": set(),
                "called_by": set(),
                "priority": "",
                "notes": "",
            }
            function["recovery_state"] = derive_recovery_state(function)
            if start in overrides:
                override = overrides[start]
                function.update({key: value for key, value in override.items() if value})
            functions.append(function)
            functions_by_address[start] = function

        owned_chunks = [
            (chunk_start, chunk_end, function)
            for function in functions
            for chunk_start, chunk_end in function["chunks"]
        ]
        owned_chunks.sort(key=lambda chunk: (chunk[0], chunk[1]))
        owned_chunk_starts = [chunk[0] for chunk in owned_chunks]

        def find_function(address):
            function = functions_by_address.get(address)
            if function:
                return function
            index = bisect_right(owned_chunk_starts, address) - 1
            if index >= 0 and address < owned_chunks[index][1]:
                return owned_chunks[index][2]
            return None

        assign_comments(functions, function_analysis.comments)
        assign_comments(functions, function_analysis.repeatable_comments)

        segment_bytes = {}
        for function in functions:
            for chunk_start, _chunk_end in function["chunks"]:
                segment_start = idc.SegStart(chunk_start)
                if segment_start in segment_bytes:
                    continue
                segment_end = idc.SegEnd(chunk_start)
                segment_bytes[segment_start] = (
                    segment_end,
                    idc.GetManyBytes(segment_start, segment_end - segment_start),
                )

        disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
        disassembler.detail = True
        call_sites = defaultdict(set)
        interior_call_targets = Counter()
        unresolved_call_targets = Counter()
        indirect_call_sites = []
        print(f"Loaded {len(functions)} functions; decoding direct calls...")
        for function in functions:
            for chunk_start, chunk_end in function["chunks"]:
                segment_start = idc.SegStart(chunk_start)
                _segment_end, image = segment_bytes[segment_start]
                offset = chunk_start - segment_start
                code = image[offset:offset + chunk_end - chunk_start]
                if not code:
                    continue
                item_heads = set(idautils.Heads(chunk_start, chunk_end))
                for instruction in disassembler.disasm(code, chunk_start):
                    if not instruction.group(CS_GRP_CALL):
                        continue
                    if (instruction.address not in item_heads or
                            not ida_bytes.is_code(idc.GetFlags(instruction.address))):
                        continue
                    if not instruction.operands or instruction.operands[0].type != X86_OP_IMM:
                        indirect_call_sites.append(instruction.address)
                        continue
                    target_address = instruction.operands[0].imm & 0xFFFFFFFF
                    target = find_function(target_address)
                    if not target:
                        unresolved_call_targets[target_address] += 1
                        continue
                    if target_address != target["start"]:
                        interior_call_targets[target_address] += 1
                    function["call_targets"].add(target["start"])
                    target["called_by"].add(function["start"])
                    call_sites[(function["start"], target["start"])].add(
                        instruction.address)

        known_starts = set(functions_by_address)
        unresolved_annotations = sorted(set(source_annotations) - known_starts)
        function_names = {function["name"] for function in functions}
        unmatched_redirects = sorted(
            (redirect for redirect in redirects
             if redirect["name"] not in function_names),
            key=lambda item: (item["name"], item["location"]),
        )
        unmatched_overrides = sorted(set(overrides) - known_starts)
        if unmatched_overrides:
            formatted = ", ".join(f"0x{address:08X}" for address in unmatched_overrides)
            raise RuntimeError(f"overrides do not match IDB function starts: {formatted}")

        for binding in source_bindings:
            target = functions_by_address.get(binding["address"])
            binding["function"] = target["name"] if target else ""
            binding["function_address"] = target["start"] if target else None

        args.output_dir.mkdir(parents=True, exist_ok=True)
        inventory_path = args.output_dir / "functions.csv"
        with inventory_path.open("w", newline="", encoding="utf-8") as file:
            fieldnames = [
                "address", "end_address", "size", "segment", "name", "prototype",
                "body_ranges", "binary_kind", "flags", "source_locations", "source_statuses",
                "redirect_exports", "original_dependencies", "recovery_state",
                "priority", "notes", "comments", "call_target_count", "caller_count",
            ]
            writer = csv.DictWriter(file, fieldnames=fieldnames, lineterminator="\n")
            writer.writeheader()
            for function in functions:
                writer.writerow({
                    "address": f"0x{function['start']:08X}",
                    "end_address": f"0x{function['end']:08X}",
                    "size": function["size"],
                    "segment": function["segment"],
                    "name": function["name"],
                    "prototype": function["prototype"],
                    "body_ranges": ";".join(
                        f"0x{start:08X}-0x{end:08X}"
                        for start, end in function["chunks"]),
                    "binary_kind": function["binary_kind"],
                    "flags": ";".join(function["flag_names"]),
                    "source_locations": flatten(
                        item["location"] for item in function["source_annotations"]),
                    "source_statuses": flatten(
                        item["status"] for item in function["source_annotations"]),
                    "redirect_exports": flatten(
                        item["name"] for item in function["redirects"]),
                    "original_dependencies": flatten(
                        item["symbol"] for item in function["dependencies"]),
                    "recovery_state": function["recovery_state"],
                    "priority": function["priority"],
                    "notes": function["notes"],
                    "comments": flatten(function["comments"]),
                    "call_target_count": len(function["call_targets"]),
                    "caller_count": len(function["called_by"]),
                })

        graph = {
            "format_version": 1,
            "idb_sha256": sha256(args.idb),
            "function_count": len(functions),
            "edges": [
                {
                    "source": f"0x{source:08X}",
                    "target": f"0x{target:08X}",
                    "call_sites": [f"0x{site:08X}" for site in sorted(sites)],
                }
                for (source, target), sites in sorted(call_sites.items())
            ],
            "unresolved_call_targets": [
                {"address": f"0x{address:08X}", "reference_count": count}
                for address, count in sorted(unresolved_call_targets.items())
            ],
            "indirect_call_sites": [
                f"0x{address:08X}" for address in sorted(indirect_call_sites)
            ],
            "interior_call_targets": [
                {
                    "address": f"0x{address:08X}",
                    "function": f"0x{find_function(address)['start']:08X}",
                    "reference_count": count,
                }
                for address, count in sorted(interior_call_targets.items())
            ],
        }
        graph_path = args.output_dir / "callgraph.json"
        graph_path.write_text(
            json.dumps(graph, indent=2, sort_keys=True) + "\n", encoding="utf-8")

        state_counts = Counter(function["recovery_state"] for function in functions)
        kind_counts = Counter(function["binary_kind"] for function in functions)
        binding_counts = Counter(binding["kind"] for binding in source_bindings)
        matched_redirect_names = {
            function["name"] for function in functions if function["redirects"]
        }
        summary = {
            "format_version": 1,
            "inputs": {
                "idb": repo_path(args.idb),
                "idb_sha256": sha256(args.idb),
                "original_input_path": root.input_file_path,
                "original_input_sha256": root.sha256,
                "definition": repo_path(args.definition),
                "source_directory": repo_path(args.source_dir),
                "overrides": repo_path(args.overrides),
            },
            "functions": {
                "total": len(functions),
                "by_binary_kind": dict(sorted(kind_counts.items())),
                "by_recovery_state": dict(sorted(state_counts.items())),
                "with_prototypes": sum(bool(function["prototype"])
                                       for function in functions),
                "with_comments": sum(bool(function["comments"])
                                     for function in functions),
            },
            "source_annotations": {
                "annotations": sum(len(items) for items in source_annotations.values()),
                "unique_addresses": len(source_annotations),
                "matched_function_starts": len(set(source_annotations) & known_starts),
                "unmatched_addresses": [
                    f"0x{address:08X}" for address in unresolved_annotations
                ],
            },
            "redirects": {
                "definitions": len(redirects),
                "exact_name_matches": len(matched_redirect_names),
                "unmatched": unmatched_redirects,
            },
            "source_bindings": {
                "total": len(source_bindings),
                "by_kind": dict(sorted(binding_counts.items())),
                "matched_function_starts": sum(
                    binding["kind"] == "function" and
                    binding["function_address"] is not None
                    for binding in source_bindings),
                "items": [
                    {
                        **{key: value for key, value in binding.items()
                           if key not in {"address", "function_address"}},
                        "address": f"0x{binding['address']:08X}",
                        "function_address": (
                            f"0x{binding['function_address']:08X}"
                            if binding["function_address"] is not None else ""),
                    }
                    for binding in source_bindings
                ],
            },
            "call_graph": {
                "edges": len(call_sites),
                "indirect_call_sites": len(indirect_call_sites),
                "interior_target_addresses": len(interior_call_targets),
                "unresolved_target_addresses": len(unresolved_call_targets),
                "unresolved_references": sum(unresolved_call_targets.values()),
            },
        }
        summary_path = args.output_dir / "summary.json"
        summary_path.write_text(
            json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    print(f"Exported {len(functions)} functions to {inventory_path}")
    print(f"Exported {len(call_sites)} call-graph edges to {graph_path}")
    print(f"Wrote recovery summary to {summary_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Export the annotated IDA database as a recovery inventory")
    parser.add_argument("--idb", type=Path, default=DEFAULT_IDB)
    parser.add_argument("--source-dir", type=Path, default=DEFAULT_SOURCE_DIR)
    parser.add_argument("--definition", type=Path, default=DEFAULT_DEFINITION)
    parser.add_argument("--overrides", type=Path, default=DEFAULT_OVERRIDES)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    args = parser.parse_args()

    for path in (args.idb, args.source_dir, args.definition):
        if not path.exists():
            parser.error(f"input not found: {path}")
    idb_hash = sha256(args.idb)
    if idb_hash not in SUPPORTED_IDB_HASHES:
        parser.error(f"unsupported IDB SHA-256: {idb_hash}")
    export_inventory(args)


if __name__ == "__main__":
    main()

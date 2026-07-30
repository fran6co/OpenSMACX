#!/usr/bin/env python3
"""Verify recovery metadata determinism with staged content-hash caches.

Regenerating the canonical inventory parses the multi-hundred-megabyte IDB and
re-disassembles the image, which costs minutes per invocation. Every input of
that regeneration is hash-pinned, so when the complete input closure and every
committed output are byte-identical to a previous fully verified run, skipping
regeneration cannot weaken the determinism guarantee. The stamp key covers the
committed outputs too, so hand-editing a generated catalog always invalidates
the cache and fails the byte comparison on the regeneration that follows.

The expensive IDB export has its own narrower checkpoint under the ignored
verification directory. A downstream correlation/classification failure, or
promoting newly generated catalogs into docs/recovery, can therefore resume
without parsing and disassembling the IDB again. The checkpoint hashes the
binary-analysis input closure and binary-only projections of the canonical
outputs. Source annotations, bindings, redirects, and overrides are cheaply
reapplied before correlation, so ordinary recovery promotions do not reopen
the IDB. The checkpoint is never sufficient to skip correlation or the final
committed-output comparison.
"""

import argparse
from collections import Counter, defaultdict
import csv
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

from local_artifact import require_local_artifact_path
import recovery_metrics


REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_CACHE_DIR = REPO_ROOT / "build" / "recovery-metadata-cache"
DEFAULT_IDB = (
    REPO_ROOT / ".opensmacx" / "analysis" /
    "terranx_ORIG_200_v3_7.5.SP3.idb")
CACHE_KEEP_COUNT = 5
EXPORT_CHECKPOINT = ".canonical-export-checkpoint.json"

EXPORT_TOOL = REPO_ROOT / "tools" / "export_recovery_inventory.py"
CORRELATE_TOOL = REPO_ROOT / "tools" / "correlate_recovery_analyses.py"
WRAPPER_TOOL = Path(__file__).resolve()

COMPARED_OUTPUTS = (
    "functions.csv",
    "callgraph.json",
    "summary.json",
    "analysis-correlation.csv",
    "priorities.csv",
    "analysis-summary.json",
)

EXPORT_OUTPUTS = (
    "functions.csv",
    "callgraph.json",
    "summary.json",
)

EXPORT_STATIC_INPUTS = (
    EXPORT_TOOL,
    WRAPPER_TOOL,
)

EXPORT_METADATA_COLUMNS = {
    "source_locations",
    "source_statuses",
    "redirect_exports",
    "original_dependencies",
    "recovery_state",
    "priority",
    "notes",
}

STATIC_INPUTS = (
    EXPORT_TOOL,
    CORRELATE_TOOL,
    WRAPPER_TOOL,
    REPO_ROOT / "src" / "OpenSMACX.def",
    REPO_ROOT / "docs" / "recovery-overrides.csv",
    REPO_ROOT / "docs" / "recovery-binding-classifications.csv",
    REPO_ROOT / "docs" / "recovery" / "ida9-functions.csv",
    REPO_ROOT / "docs" / "recovery" / "ghidra-functions.csv",
    REPO_ROOT / "docs" / "recovery" / "ghidra-interior-references.csv",
)


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for chunk in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def source_metadata_sha256(source_dir):
    """Hash exactly the source-derived inputs consumed by the exporter.

    Recovery catalogs depend on Original Offset/Status annotations and
    fixed-address bindings, including their reported source locations. They do
    not depend on ordinary implementation bytes. Hashing this deterministic
    projection keeps the cache complete while allowing tests, runtime oracles,
    and behavior-only source edits to reuse a verified IDB regeneration.

    The exporter itself remains a raw-hashed STATIC_INPUT, so any change to
    the projection rules invalidates this digest and forces regeneration.
    """
    from export_recovery_inventory import (  # Imported lazily for fast test import.
        load_source_annotations,
        load_source_bindings,
    )

    annotations = [
        {"address": address, "items": items}
        for address, items in sorted(load_source_annotations(source_dir).items())
    ]
    payload = {
        "annotations": annotations,
        "bindings": load_source_bindings(source_dir),
    }
    serialized = json.dumps(
        payload, ensure_ascii=True, separators=(",", ":"), sort_keys=True)
    return hashlib.sha256(serialized.encode("utf-8")).hexdigest()


def path_manifest_line(path):
    if not path.is_file():
        raise RuntimeError(f"required metadata input missing: {path}")
    try:
        label = path.resolve().relative_to(REPO_ROOT).as_posix()
    except ValueError:
        label = str(path.resolve())
    return f"{label}:{sha256(path)}"


def export_manifest_lines(idb_path):
    """Return the input closure of the expensive binary-only IDB export.

    Source annotations, bindings, DEF exports, and recovery overrides are
    deliberately excluded. They are reapplied from the checkpointed canonical
    function rows without reopening the IDB.
    """
    lines = [f"python:{sys.version_info[0]}.{sys.version_info[1]}"]
    for path in (idb_path, *EXPORT_STATIC_INPUTS):
        lines.append(path_manifest_line(path))
    return lines


def manifest_lines(idb_path):
    input_paths = [idb_path, *STATIC_INPUTS]
    output_paths = [
        REPO_ROOT / "docs" / "recovery" / name for name in COMPARED_OUTPUTS
    ]
    lines = [f"python:{sys.version_info[0]}.{sys.version_info[1]}"]
    for path in input_paths:
        lines.append(path_manifest_line(path))
    lines.append(
        f"source_metadata:{source_metadata_sha256(REPO_ROOT / 'src')}")
    for path in output_paths:
        if not path.is_file():
            raise RuntimeError(f"required metadata output missing: {path}")
        label = path.resolve().relative_to(REPO_ROOT).as_posix()
        lines.append(f"{label}:{sha256(path)}")
    return lines


def json_sha256(value):
    serialized = json.dumps(
        value, ensure_ascii=True, separators=(",", ":"), sort_keys=True)
    return hashlib.sha256(serialized.encode("utf-8")).hexdigest()


def binary_functions_payload(path):
    with path.open(newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)
        if not reader.fieldnames or not EXPORT_METADATA_COLUMNS.issubset(
                reader.fieldnames):
            raise RuntimeError(f"invalid canonical function inventory: {path}")
        binary_fields = [
            field for field in reader.fieldnames
            if field not in EXPORT_METADATA_COLUMNS
        ]
        rows = [
            {field: row[field] for field in binary_fields}
            for row in reader
        ]
    return {"fieldnames": binary_fields, "rows": rows}


def binary_summary_payload(path):
    summary = json.loads(path.read_text(encoding="utf-8"))
    inputs = summary["inputs"]
    functions = summary["functions"]
    return {
        "format_version": summary["format_version"],
        "inputs": {
            key: inputs[key] for key in (
                "idb", "idb_sha256", "original_input_path",
                "original_input_sha256")
        },
        "functions": {
            key: functions[key] for key in (
                "total", "by_binary_kind", "with_prototypes", "with_comments")
        },
        "call_graph": summary["call_graph"],
    }


def export_checkpoint_payload(idb_path, verify_dir):
    manifest_text = "\n".join(export_manifest_lines(idb_path)) + "\n"
    for name in EXPORT_OUTPUTS:
        path = verify_dir / name
        if not path.is_file():
            raise RuntimeError(f"canonical export did not produce {path}")
    outputs = {
        "functions_binary": json_sha256(binary_functions_payload(
            verify_dir / "functions.csv")),
        "callgraph.json": sha256(verify_dir / "callgraph.json"),
        "summary_binary": json_sha256(binary_summary_payload(
            verify_dir / "summary.json")),
    }
    return {
        "format_version": 2,
        "key": hashlib.sha256(manifest_text.encode("utf-8")).hexdigest(),
        "manifest": manifest_text,
        "outputs": outputs,
    }


def write_export_checkpoint(idb_path, verify_dir):
    payload = export_checkpoint_payload(idb_path, verify_dir)
    with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=verify_dir, delete=False) as handle:
        json.dump(payload, handle, indent=1, sort_keys=True)
        handle.write("\n")
        temporary = Path(handle.name)
    temporary.replace(verify_dir / EXPORT_CHECKPOINT)


def export_checkpoint_matches(idb_path, verify_dir):
    checkpoint = verify_dir / EXPORT_CHECKPOINT
    if not checkpoint.is_file():
        return False
    try:
        payload = json.loads(checkpoint.read_text(encoding="utf-8"))
        expected = export_checkpoint_payload(idb_path, verify_dir)
    except (json.JSONDecodeError, OSError, RuntimeError):
        return False
    return payload == expected


def clear_generated_outputs(verify_dir):
    for name in (*COMPARED_OUTPUTS, EXPORT_CHECKPOINT):
        path = verify_dir / name
        if path.is_file() or path.is_symlink():
            path.unlink()


def refresh_export_metadata(verify_dir):
    """Reapply cheap source metadata to a checkpointed binary inventory."""
    from export_recovery_inventory import (
        DEFAULT_DEFINITION,
        DEFAULT_OVERRIDES,
        DEFAULT_SOURCE_DIR,
        derive_recovery_state,
        flatten,
        load_overrides,
        load_redirects,
        load_source_annotations,
        load_source_bindings,
        repo_path,
    )

    functions_path = verify_dir / "functions.csv"
    summary_path = verify_dir / "summary.json"
    with functions_path.open(newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)
        if not reader.fieldnames or not EXPORT_METADATA_COLUMNS.issubset(
                reader.fieldnames):
            raise RuntimeError(
                f"invalid canonical function inventory: {functions_path}")
        fieldnames = reader.fieldnames
        rows = list(reader)

    source_annotations = load_source_annotations(DEFAULT_SOURCE_DIR)
    source_bindings = load_source_bindings(DEFAULT_SOURCE_DIR)
    redirects = load_redirects(DEFAULT_DEFINITION)
    overrides = load_overrides(DEFAULT_OVERRIDES)
    redirects_by_name = defaultdict(list)
    for redirect in redirects:
        redirects_by_name[redirect["name"]].append(redirect)
    function_bindings = defaultdict(list)
    for binding in source_bindings:
        if binding["kind"] == "function":
            function_bindings[binding["address"]].append(binding)

    rows_by_address = {int(row["address"], 0): row for row in rows}
    known_starts = set(rows_by_address)
    unmatched_overrides = sorted(set(overrides) - known_starts)
    if unmatched_overrides:
        formatted = ", ".join(
            f"0x{address:08X}" for address in unmatched_overrides)
        raise RuntimeError(
            f"overrides do not match IDB function starts: {formatted}")

    for address, row in rows_by_address.items():
        annotations = source_annotations.get(address, [])
        function_redirects = redirects_by_name.get(row["name"], [])
        dependencies = function_bindings.get(address, [])
        function = {
            "binary_kind": row["binary_kind"],
            "source_annotations": annotations,
            "redirects": function_redirects,
            "dependencies": dependencies,
        }
        row["source_locations"] = flatten(
            item["location"] for item in annotations)
        row["source_statuses"] = flatten(
            item["status"] for item in annotations)
        row["redirect_exports"] = flatten(
            item["name"] for item in function_redirects)
        row["original_dependencies"] = flatten(
            item["symbol"] for item in dependencies)
        row["recovery_state"] = derive_recovery_state(function)
        row["priority"] = ""
        row["notes"] = ""
        if address in overrides:
            for key, value in overrides[address].items():
                if value:
                    row[key] = value

    function_names = {row["name"] for row in rows}
    unmatched_redirects = sorted(
        (redirect for redirect in redirects
         if redirect["name"] not in function_names),
        key=lambda item: (item["name"], item["location"]),
    )
    for binding in source_bindings:
        target = rows_by_address.get(binding["address"])
        binding["function"] = target["name"] if target else ""
        binding["function_address"] = (
            int(target["address"], 0) if target else None)

    with tempfile.NamedTemporaryFile(
            "w", newline="", encoding="utf-8", dir=verify_dir,
            delete=False) as handle:
        writer = csv.DictWriter(
            handle, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
        temporary_functions = Path(handle.name)
    temporary_functions.replace(functions_path)

    summary = json.loads(summary_path.read_text(encoding="utf-8"))
    summary["inputs"].update({
        "definition": repo_path(DEFAULT_DEFINITION),
        "source_directory": repo_path(DEFAULT_SOURCE_DIR),
        "overrides": repo_path(DEFAULT_OVERRIDES),
    })
    summary["functions"]["by_recovery_state"] = dict(sorted(
        Counter(row["recovery_state"] for row in rows).items()))
    # The byte-weighted block is recomputed here for the same reason the count
    # above is: `recovery_state` has just changed, and every figure in the
    # block is derived from it - the lift-scope denominator, the exclusion, and
    # the machine-carried debt. Leaving it stale would publish a byte total
    # that disagreed with the counts printed beside it, and the reused-export
    # path is the path most runs take.
    summary["functions"]["bytes"] = recovery_metrics.bytes_block(rows)
    unresolved_annotations = sorted(set(source_annotations) - known_starts)
    summary["source_annotations"] = {
        "annotations": sum(len(items) for items in source_annotations.values()),
        "unique_addresses": len(source_annotations),
        "matched_function_starts": len(set(source_annotations) & known_starts),
        "unmatched_addresses": [
            f"0x{address:08X}" for address in unresolved_annotations
        ],
    }
    matched_redirect_names = {
        row["name"] for row in rows if row["redirect_exports"]
    }
    summary["redirects"] = {
        "definitions": len(redirects),
        "exact_name_matches": len(matched_redirect_names),
        "unmatched": unmatched_redirects,
    }
    binding_counts = Counter(binding["kind"] for binding in source_bindings)
    summary["source_bindings"] = {
        "total": len(source_bindings),
        "by_kind": dict(sorted(binding_counts.items())),
        "matched_function_starts": sum(
            binding["kind"] == "function"
            and binding["function_address"] is not None
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
    }
    with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=verify_dir,
            delete=False) as handle:
        json.dump(summary, handle, indent=2, sort_keys=True)
        handle.write("\n")
        temporary_summary = Path(handle.name)
    temporary_summary.replace(summary_path)


def run_regeneration(idb_path, verify_dir, force=False):
    verify_dir.mkdir(parents=True, exist_ok=True)
    if not force and export_checkpoint_matches(idb_path, verify_dir):
        print("verify-recovery-metadata: reused canonical IDB export")
        refresh_export_metadata(verify_dir)
    else:
        clear_generated_outputs(verify_dir)
        subprocess.run(
            [sys.executable, str(EXPORT_TOOL),
             "--idb", str(idb_path),
             "--output-dir", str(verify_dir)],
            check=True)
        write_export_checkpoint(idb_path, verify_dir)
    subprocess.run(
        [sys.executable, str(CORRELATE_TOOL),
         "--canonical", str(verify_dir / "functions.csv"),
         "--canonical-summary", str(verify_dir / "summary.json"),
         "--output-dir", str(verify_dir)],
        check=True)


# Figures in summary.json that this tool CANNOT regenerate, and must therefore
# not compare. `proven_recovered` and `unproven_recovered` are derived from the
# oracle markers in src/*_oracle.cpp and the island addresses in CMakeLists.txt
# by tools/export_proven_functions.py - nothing about them comes from the IDB.
# Regeneration here always yields 0 proven, so a byte comparison of the whole
# file fails for ANY tree that has committed a proof: 54 of them, since 7a2c554.
# That made the gate unpassable rather than strict, which is worse than absent -
# it stopped `stage-hybrid-game`, the target that runs the oracles that would
# earn the next proof.
#
# `analysis-summary.json` is the same problem one layer up: it records
# `inputs/canonical_summary_sha256`, a hash OF summary.json, so the
# unregenerable proof block poisons the hash too. Dropping the hash loses no
# coverage that matters - summary.json itself is still compared field by field
# just above, so drift in it fails there; the hash only additionally covered the
# proof block, which is deliberately not compared.
NOT_REGENERABLE = {
    "summary.json": (("functions", "bytes", "proven_recovered"),
                     ("functions", "bytes", "unproven_recovered")),
    "analysis-summary.json": (("inputs", "canonical_summary_sha256"),),
}


def without_proof_figures(payload, name="summary.json"):
    """A copy of `name` with the blocks this tool cannot regenerate removed."""
    trimmed = json.loads(json.dumps(payload))
    for path in NOT_REGENERABLE.get(name, ()):
        node = trimmed
        for key in path[:-1]:
            node = node.get(key) if isinstance(node, dict) else None
            if node is None:
                break
        if isinstance(node, dict):
            node.pop(path[-1], None)
    return trimmed


def compare_outputs(verify_dir):
    for name in COMPARED_OUTPUTS:
        generated = verify_dir / name
        committed = REPO_ROOT / "docs" / "recovery" / name
        if not generated.is_file():
            raise RuntimeError(f"regeneration did not produce {generated}")
        if generated.read_bytes() == committed.read_bytes():
            continue
        if name in NOT_REGENERABLE:
            # Compare everything the IDB does determine, skipping only the
            # proof-derived blocks. A drift anywhere else still fails.
            if (without_proof_figures(json.loads(generated.read_text()), name)
                    == without_proof_figures(json.loads(committed.read_text()),
                                             name)):
                continue
        raise RuntimeError(
            f"regenerated {name} differs from committed docs/recovery/{name}")


def promote_outputs(verify_dir):
    """Copy freshly regenerated metadata over the committed copies.

    Every recovery changes the source bindings the exporter reads, so the
    committed metadata is stale by construction the moment a body lands. Run
    unpromoted, the gate therefore always fails its first pass, gets a manual
    copy, and is run again -- a guaranteed second full gate cycle per recovery
    that proves nothing the first one did not.

    Promotion is only a way to *reach* that comparison in one pass, never a way
    to skip it. The caller regenerates, promotes, and then compares against
    what it just wrote; the check still runs and still fails on a
    nondeterministic exporter, because a second regeneration into the verify
    directory would differ from the promoted bytes. What it removes is the
    round trip, not the evidence.
    """
    promoted = []
    for name in COMPARED_OUTPUTS:
        generated = verify_dir / name
        committed = REPO_ROOT / "docs" / "recovery" / name
        if not generated.is_file():
            raise RuntimeError(f"regeneration did not produce {generated}")
        if not committed.is_file() or (
                generated.read_bytes() != committed.read_bytes()):
            shutil.copyfile(generated, committed)
            promoted.append(name)
    if promoted:
        print("verify-recovery-metadata: promoted " + ", ".join(promoted))
    else:
        print("verify-recovery-metadata: committed metadata already current")
    return promoted


def write_stamp(cache_dir, key, manifest_text):
    cache_dir.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=cache_dir, delete=False) as handle:
        handle.write(manifest_text)
        temporary = Path(handle.name)
    temporary.replace(cache_dir / f"{key}.stamp")
    stamps = sorted(
        cache_dir.glob("*.stamp"),
        key=lambda path: path.stat().st_mtime,
        reverse=True)
    for stale in stamps[CACHE_KEEP_COUNT:]:
        stale.unlink()


def main():
    parser = argparse.ArgumentParser(
        description="Verify deterministic recovery metadata with stamp caching")
    parser.add_argument("--idb", type=Path, default=DEFAULT_IDB)
    parser.add_argument("--verify-dir", type=Path, required=True)
    parser.add_argument("--cache-dir", type=Path, default=DEFAULT_CACHE_DIR)
    parser.add_argument("--force", action="store_true",
                        help="Regenerate even when a matching stamp exists")
    parser.add_argument("--promote", action="store_true",
                        help="Copy regenerated metadata over docs/recovery/ "
                             "before comparing, so a recovery reaches a green "
                             "gate in one pass instead of two")
    args = parser.parse_args()

    verify_dir = require_local_artifact_path(
        args.verify_dir, "metadata verify directory")
    cache_dir = require_local_artifact_path(
        args.cache_dir, "metadata stamp cache directory")
    if not args.idb.is_file():
        raise SystemExit(f"IDB not found: {args.idb}")

    manifest_text = "\n".join(manifest_lines(args.idb)) + "\n"
    key = hashlib.sha256(manifest_text.encode("utf-8")).hexdigest()
    stamp = cache_dir / f"{key}.stamp"
    if not args.force and stamp.is_file():
        if stamp.read_text(encoding="utf-8") == manifest_text:
            print(f"verify-recovery-metadata: cached ({key[:16]})")
            return 0
        stamp.unlink()

    verify_dir.mkdir(parents=True, exist_ok=True)

    run_regeneration(args.idb, verify_dir, force=args.force)
    if args.promote and promote_outputs(verify_dir):
        # The committed outputs are part of the hashed input closure, so
        # promoting them invalidates the key computed above. Recompute it from
        # the promoted tree, or the stamp would certify a state that no longer
        # exists and the next run would miss.
        manifest_text = "\n".join(manifest_lines(args.idb)) + "\n"
        key = hashlib.sha256(manifest_text.encode("utf-8")).hexdigest()
    compare_outputs(verify_dir)
    write_stamp(cache_dir, key, manifest_text)
    print(f"verify-recovery-metadata: verified and stamped ({key[:16]})")
    return 0


if __name__ == "__main__":
    sys.exit(main())

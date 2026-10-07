"""Read-only selected-body measurement through configured compiler interaction.

Diagnostic trials reuse the same per-TU comparer as recording, but never invoke
publication, replace missing recipes, or require historical obligations to be
currently satisfied before investigating a structural regression.
"""

from collections import Counter, defaultdict
from concurrent.futures import ProcessPoolExecutor
from dataclasses import asdict
from hashlib import file_digest
from pathlib import Path

from .compiler import Compiler
from .measurement import Measurement
from .model import digest
from .source import State
from .source.asm import (AsmComparison, compare_unit, resolve_fragment_pairs,
                         shared_spans)
from .source.model import Tier
from .source_history import git
from .source_inventory import image_pin, source_snapshot
from .source_selection import (select_records, snapshot_mappings,
                               selected_source_records)
from .component_original_routing import (component_claim_result, component_function_observations,
                                         component_record_identity_candidates,
                                         exact_component_record_bindings)
from .components import SystemImportManifest
from .model import Origin


def _unit(job):
    root, profile, path, records, image, flags, shared, fragment_pairs = job
    return compare_unit(records, image, path, (), flags, shared,
                        compiler=Compiler(root, profile=profile),
                        fragment_pairs=fragment_pairs)


def measure_source(root: Path, targets, *, image: Path, in_file=None, jobs=1):
    root = root.resolve()
    if not isinstance(targets, (list, tuple)) or not targets:
        raise ValueError("measurement requires explicit nonempty targets")
    if type(jobs) is not int or jobs < 1:
        raise ValueError("measurement jobs must be a positive integer")
    settings = Measurement.read(root)
    image = (image if image.is_absolute() else root / image).resolve()

    def inputs():
        with image.open("rb") as stream:
            actual = file_digest(stream, "sha256").hexdigest()
        if actual != image_pin(root, settings.source):
            raise ValueError("measurement requires the configured pinned original image")
        snapshot = settings.snapshot(root, {
            "head": git(root, "rev-parse", "HEAD").decode().strip(),
            "image": {"path": str(image), "sha256": actual},
            "targets": list(targets), "host": in_file})
        if any(value is None for value in snapshot["inputs"].values()):
            raise ValueError("declared measurement input is missing")
        for name in (*settings.source.roots, *settings.input_roots):
            directory = settings.source.path(root, name)
            if not directory.is_dir():
                raise ValueError(f"measurement input root is missing: {name}")
            if any(path.is_symlink() for path in (directory, *directory.rglob("*"))):
                raise ValueError("measurement cannot enumerate source/input-root symlinks")
        return snapshot

    before = inputs()
    # Reuse the content-keyed inventory's global mapping facts. Only selected
    # hosts need complete body/lesson parsing; shared spans still use EVERY
    # mapping, including unselected seed alternatives.
    snapshot = source_snapshot(root, allow_empty=True)
    mappings = snapshot_mappings(root, snapshot)
    host = settings.source.path(root, in_file) if in_file is not None else None
    selected_mappings = select_records(mappings, targets, host, settings.source.symbol_format)
    selected_paths = {row.path for row in selected_mappings}
    typed_hosts = selected_source_records(
        root, snapshot, [row for row in mappings if row.path in selected_paths])
    typed_by_key = {(row.path.resolve(), row.address, row.line): row for row in typed_hosts}
    selected = [typed_by_key[(row.path.resolve(), row.address, row.line)]
                for row in selected_mappings]
    component_owners = exact_component_record_bindings(snapshot)
    component_candidates = component_record_identity_candidates(snapshot)
    component_keys = set()
    for record in mappings:
        relative = record.path.relative_to(root).as_posix()
        entity = Origin(image_pin(root, settings.source), settings.source.address_space, record.address).entity_id
        if (relative, entity) in component_candidates:
            component_keys.add((record.path.resolve(), record.address, record.line))
    fragment_pairs, fragment_refused = resolve_fragment_pairs(
        mappings, selected_mappings, component_keys=frozenset(component_keys),
        is_seed=lambda row: settings.source.is_seed(row.path.relative_to(root).as_posix()),
        bind=lambda row: typed_by_key[(row.path.resolve(), row.address, row.line)])
    pair_by_record = {}
    for pair in fragment_pairs:
        child, parent = pair
        pair_by_record[(child.path.resolve(), child.address, child.line)] = pair
        pair_by_record[(parent.path.resolve(), parent.address, parent.line)] = pair
    groups, unavailable, component_selected, pairs_by_path = defaultdict(list), [], {}, defaultdict(list)
    for record in selected:
        relative = record.path.relative_to(root).as_posix()
        entity = Origin(image_pin(root, settings.source), settings.source.address_space, record.address).entity_id
        owner = component_owners.get((relative, entity))
        key = (record.path.resolve(), record.address, record.line)
        if key in fragment_refused:
            unavailable.append((record, "fragment parent/child ownership is ambiguous or conflicts with a component"))
        elif record.function_fragment is not None and key not in pair_by_record:
            unavailable.append((record, "fragment has no unambiguous same-TU parent support"))
        elif owner is not None:
            component_selected[(relative, record.line)] = (record, owner)
        elif record.path.suffix not in settings.unit_extensions:
            unavailable.append((record, "mapping host is not a configured translation unit"))
        elif record.state is State.PLACEHOLDER:
            unavailable.append((record, "unfinished source placeholder; matching an empty body is not recovery"))
        else:
            groups[record.path].append(record)
            pair = pair_by_record.get(key)
            if pair is not None and pair not in pairs_by_path[record.path]:
                pairs_by_path[record.path].append(pair)
    shared = shared_spans(mappings)
    work = [(root, settings.compiler_profile, path, rows, image, settings.flag_sets, shared,
             tuple(pairs_by_path[path]))
            for path, rows in sorted(groups.items())]
    if inputs() != before:
        raise ValueError("measurement inputs changed before compilation")
    if jobs == 1 or len(work) < 2:
        units = [_unit(job) for job in work]
    else:
        from .framework_runtime import export_binding, initialize_worker
        with ProcessPoolExecutor(max_workers=min(jobs, len(work)), initializer=initialize_worker,
                                 initargs=(export_binding(),)) as pool:
            units = list(pool.map(_unit, work))
    if inputs() != before:
        raise ValueError("source, original, history or compiler inputs changed during measurement")
    results = list(unavailable)
    for job, unit in zip(work, units, strict=True):
        expected = job[3]
        if len(unit.results) != len(expected) or [row for row, _ in unit.results] != expected:
            raise ValueError("unit comparison did not cover exactly the selected source mappings")
        results.extend(unit.results)
    component_functions, component_checks = component_function_observations(
        root, image, {key: value[1] for key, value in component_selected.items()}) if component_selected else ({}, [])
    if inputs() != before:
        raise ValueError("source, original, history or component inputs changed during measurement")
    rows = []
    for record, value in results:
        row = {"address": record.address_hex, "path": record.path.relative_to(root).as_posix(),
               "line": record.line, "name": record.name, "symbol": record.symbol or record.name,
               "historical_byte_obligation": record.byte_exact, "historical_shape_obligation": record.semantic,
               "source_state": record.state.value}
        if isinstance(value, str):
            row.update(result="unknown", verdict="UNRESOLVED", diagnostic=value)
        elif isinstance(value, AsmComparison):
            row.update(asdict(value))
            row["result"] = ("pass" if value.verdict == Tier.BYTE_EXACT else
                             "unknown" if value.verdict in (Tier.NO_COMPILE, Tier.REFUSED, Tier.SHARED_TAIL) else "fail")
        else:
            raise ValueError("invalid selected-body comparison observation")
        rows.append(row)
    for (relative, _), (record, owner) in component_selected.items():
        component_id, _, binding = owner
        observed = component_functions.get(binding.origin.entity_id)
        result, verdict = component_claim_result(observed)
        backend = observed.get("measurement_backend", "component_binary") if observed else (
            "system_import_original_thunk" if isinstance(owner[1], SystemImportManifest)
            else "component_binary")
        row = {"address": record.address_hex, "path": relative, "line": record.line,
               "name": record.name, "symbol": record.symbol or record.name,
               "historical_byte_obligation": record.byte_exact,
               "historical_shape_obligation": record.semantic,
               "source_state": record.state.value, "result": result, "verdict": verdict,
               "measurement_backend": backend,
               "component": component_id, "component_unit": binding.unit,
               "provenance": ("verified_system_import_original_thunk" if backend == "system_import_original_thunk"
                              else "verified_component_mapping")}
        if observed is None:
            row["diagnostic"] = next((item["reason"] for item in component_checks
                                      if item["component"] == component_id),
                                     "component verifier returned no row for the exact original identity")
        else:
            row["component_dimensions"] = {name: observed.get(name)
                                            for name in ("bytes", "boundary", "references")}
            if observed.get("dependency_verification"):
                row["dependency_verification"] = observed["dependency_verification"]
            if observed.get("reason"):
                row["diagnostic"] = observed["reason"]
        rows.append(row)
    counts = Counter(row["result"] for row in rows)
    result = "fail" if counts["fail"] else "unknown" if counts["unknown"] else "pass"
    return {"schema_version": 1, "operation": "measure", "result": result,
            "verification": ("source_and_system_import_original_thunks"
                             if any(row.get("measurement_backend") == "system_import_original_thunk" for row in rows)
                             else "source_masked_bytes_and_component_binary" if component_selected
                             else "historical_masked_bytes_only"),
            "ready_for_collection": False, "persisted": False,
            "selected": len(selected), "counts": dict(counts),
            "measurements": sorted(rows, key=lambda row: (row["path"], row["line"])),
            "translation_units": len(units), "compile_attempts": sum(unit.compile_attempts for unit in units),
            "component_checks": component_checks,
            "reused_attempts": sum(unit.reused_attempts for unit in units), "input_fingerprint": digest(before),
            "limitations": ["selected current spellings only; no source claims are banked, demoted or reverted",
                            "masked-byte comparison is not strict reference, structure, preservation, build/link or runtime proof",
                            "missing recipes remain unavailable; compiler interaction owns any explicitly configured seed policy",
                            "use task variants for private candidate decks with scope and damage controls"]}

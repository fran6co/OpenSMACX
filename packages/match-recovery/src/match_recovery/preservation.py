"""Fresh baseline/current compiler evidence outside task-owned functions.

The task's Git baseline is fixed. Current compiler policy is used on both source
versions; this is not a recreation of historical binaries or a migration gate.
All reports are disposable. No claims, baselines or task decisions are rewritten.
"""

from collections import Counter, defaultdict
from dataclasses import asdict
from pathlib import Path
import re

from .coff_preservation import compare_outside_functions
from .coff_replica import object_signature
from .compiler import Compiler
from .component_source import parse_components
from .driver import changes, working_state
from .measurement import Measurement
from .generated_source import parse_generated
from .layout_source import parse_members, parse_bases
from .model import Origin, Result, digest
from .pe_coff import Coff
from .object_source import parse_objects
from .import_source import parse_imports
from .records import parse_json
from .source_config import SourceConfiguration
from .source_history import HistoryConfiguration, _tree, git, revision_files
from .source_inventory import image_pin
from .source_replica import replica_roots, replica_snapshot
from .tasks import TaskStore


def _bindings(root, files, source, pin):
    from .source.reader import read_text

    rows = defaultdict(list)
    for name, raw in sorted(files.items()):
        if (source.is_seed(name) or Path(name).suffix not in source.extensions
                or not any(Path(name).is_relative_to(directory) for directory in source.roots)):
            continue
        for record in read_text(raw.decode("utf-8", errors="replace"), root / name):
            entity = Origin(pin, source.address_space, record.address).entity_id
            rows[entity].append({"unit": name, "symbol": record.symbol or record.name,
                                 "spans": record.image_spans, "body": record.body,
                                 "byte_obligation": record.byte_exact, "shape_obligation": record.semantic})
    return dict(rows)


def _unit_scope(rows, selected, unit):
    allowed, outside = {}, set()
    for entity, candidates in rows.items():
        for row in candidates:
            if row["unit"] != unit:
                continue
            symbol = row["symbol"]
            if entity not in selected:
                outside.add(symbol)
            elif len(candidates) != 1 or symbol in allowed:
                raise ValueError("ambiguous scoped source binding: " + symbol)
            else:
                allowed[symbol] = entity
    if allowed.keys() & outside:
        raise ValueError("source symbol belongs to both task and outside scope")
    return allowed, outside


def _outside_annotations(files, source, pin, selected):
    """Load-bearing non-function records do not disappear in an object-byte check."""
    records, occurrences = {}, Counter()
    def add(value):
        key = digest(value)
        occurrences[key] += 1
        records[digest({"annotation": key, "occurrence": occurrences[key]})] = value
    for name, raw in sorted(files.items()):
        if (source.is_seed(name) or Path(name).suffix not in source.extensions
                or not any(Path(name).is_relative_to(directory) for directory in source.roots)):
            continue
        text = raw.decode("utf-8", errors="replace")
        for item in parse_imports(text, name):
            # System identities are not task-owned function bodies. A changed
            # DLL/name/ordinal must not disappear behind identical object bytes.
            add({"kind": "import", **{key: value for key, value in asdict(item).items() if key != "line"}})
        for kind, parser in (("object", parse_objects), ("member", parse_members),
                             ("base", parse_bases), ("generated", parse_generated)):
            for item in parser(text, name, pin, address_space=source.address_space):
                anchor = item.origin if kind == "object" else item.evidence
                if anchor.entity_id not in selected:
                    value = {"kind": kind, **{key: value for key, value in asdict(item).items() if key != "line"}}
                    add(value)
        if name.endswith(".recovery.h"):
            for manifest in parse_components({name: text}):
                value = {"kind": "component", "path": name, "manifest": asdict(manifest)}
                add(value)
    return records


def _symbol_sources(bindings, annotations, selected):
    """Index declared source identities, not inferred compiler ownership."""
    index = defaultdict(list)
    for entity, rows in bindings.items():
        for row in rows:
            value = {"kind": "function", "entity_id": entity, "source": row["unit"],
                     "task_scope": entity in selected}
            index[row["symbol"]].append(value)
            if entity in selected:
                index["scoped-entry:" + entity].append(value)
    for row in annotations.values():
        if row["kind"] in ("object", "generated"):
            entity = Origin(**row["origin"]).entity_id
            value = {"kind": row["kind"], "entity_id": entity,
                     "source": row["unit"] if row["kind"] == "object" else row["path"],
                     "task_scope": entity in selected}
            if row["kind"] == "generated":
                value["evidence_entity_id"] = Origin(**row["evidence"]).entity_id
            index[row["symbol"]].append(value)
        elif row["kind"] == "component":
            for binding in row["manifest"]["bindings"]:
                entity = Origin(**binding["origin"]).entity_id
                index[binding["symbol"]].append({"kind": "component_function", "entity_id": entity,
                                                "source": row["manifest"]["source_root"] + "/" + binding["unit"],
                                                "annotation_source": row["path"], "task_scope": entity in selected})
    return index


def _explain_sources(comparison, old_sources, new_sources):
    affected, unbound = {}, {}
    def attach(symbol, sources, version):
        # Retain every candidate for duplicate/ambiguous annotations. A matching
        # spelling is a navigation hint, not evidence that the compiler emitted
        # this definition from the annotated source home.
        symbol["source_bindings"] = sources.get(symbol["symbol"], [])
        if not symbol["source_bindings"] and symbol["storage"] in (2, 105):
            value = {"version": version, **{key: symbol[key] for key in
                                            ("symbol", "offset", "section_identity", "type", "storage")}}
            unbound[digest(value)] = value
        for binding in symbol["source_bindings"]:
            value = {"version": version, "symbol": symbol["symbol"], **binding}
            affected[digest(value)] = value
    for section in comparison["changed_sections"]:
        for version, sources in (("baseline", old_sources), ("current", new_sources)):
            for symbol in section[version + "_symbols"]:
                attach(symbol, sources, version)
        for field, sources, version in (("references_removed", old_sources, "baseline"),
                                        ("references_added", new_sources, "current")):
            for reference in section[field]:
                attach(reference["target"], sources, version)
    for symbol in comparison["symbol_changes"]:
        version = "baseline" if symbol["change"] == "removed" else "current"
        attach(symbol, old_sources if version == "baseline" else new_sources, version)
    comparison["affected_source_bindings"] = sorted(affected.values(), key=lambda row: (row["source"], row["symbol"], row["version"]))
    comparison["unbound_linker_symbols"] = sorted(unbound.values(), key=lambda row: (row["symbol"], row["version"], row["section_identity"]))
    comparison["changed_section_count"] = len(comparison["changed_sections"])
    comparison["symbol_change_count"] = len(comparison["symbol_changes"])


def check_preservation(root: Path, task_name: str, *, units: tuple[str, ...] | None = None,
                       diagnostic: bool = False, compiler=None):
    if type(diagnostic) is not bool:
        raise ValueError("diagnostic mode must be boolean")
    root = root.resolve()
    settings, store = Measurement.read(root), TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    selected, _ = store._scope(task, decisions)
    head = git(root, "rev-parse", "HEAD").decode().strip()
    git(root, "merge-base", "--is-ancestor", task.base_revision, head)
    pin = image_pin(root, settings.source)
    if any(subject.image_sha256 != pin or subject.address_space != settings.source.address_space for subject in task.subjects):
        raise ValueError("preservation task refers to another original image/address space")
    metadata = {"task": task, "decisions": decisions, "head": head}
    before, state = settings.snapshot(root, metadata), working_state(root)
    roots = replica_roots((*settings.source.roots, *settings.input_roots))
    current = {name: data for name, data in replica_snapshot(root, roots).items() if data is not None}
    baseline = revision_files(root, task.base_revision, roots)
    configuration_path = "recovery/project.json"
    tree = _tree(root, task.base_revision, (configuration_path,))
    if configuration_path in tree:
        if tree[configuration_path][:2] not in (("100644", "blob"), ("100755", "blob")):
            raise ValueError("historical project configuration is not a regular file")
        config = parse_json(git(root, "cat-file", "blob", tree[configuration_path][2]))
        if not isinstance(config, dict) or type(config.get("schema_version")) is not int or config["schema_version"] != 1:
            raise ValueError("unsupported historical project configuration")
        historical_source = (SourceConfiguration.from_dict(config["source"]) if "source" in config else
                             HistoryConfiguration.read(root).before_configuration)
    else:
        historical_source = HistoryConfiguration.read(root).before_configuration
    if historical_source != settings.source:
        raise ValueError("preservation requires the same baseline/current source interpretation; history cannot be guessed")
    pin_tree = _tree(root, task.base_revision, (settings.source.image_pin,))
    entry = pin_tree.get(settings.source.image_pin)
    if entry is None or entry[:2] not in (("100644", "blob"), ("100755", "blob")):
        raise ValueError("preservation baseline original pin is missing or not regular")
    if re.findall(rb"^sha256\s+([0-9a-f]{64})\s*$", git(root, "cat-file", "blob", entry[2]), re.MULTILINE) != [pin.encode()]:
        raise ValueError("preservation baseline original pin is missing or different")
    old_bindings, new_bindings = _bindings(root, baseline, settings.source, pin), _bindings(root, current, settings.source, pin)
    binding_changes = [{"entity_id": key, "baseline": old_bindings.get(key, []), "current": new_bindings.get(key, [])}
                       for key in sorted((old_bindings.keys() | new_bindings.keys()) - selected)
                       if old_bindings.get(key) != new_bindings.get(key)]
    old_annotations = _outside_annotations(baseline, settings.source, pin, selected)
    new_annotations = _outside_annotations(current, settings.source, pin, selected)
    old_sources = _symbol_sources(old_bindings, _outside_annotations(baseline, settings.source, pin, frozenset()), selected)
    new_sources = _symbol_sources(new_bindings, _outside_annotations(current, settings.source, pin, frozenset()), selected)
    annotation_changes = [{"change": "removed", "annotation": old_annotations[key]}
                          for key in sorted(old_annotations.keys() - new_annotations.keys())]
    annotation_changes += [{"change": "added", "annotation": new_annotations[key]}
                           for key in sorted(new_annotations.keys() - old_annotations.keys())]
    all_units = {name for name in baseline.keys() | current.keys() if Path(name).suffix in settings.unit_extensions
                 and not settings.source.is_seed(name)
                 and any(Path(name).is_relative_to(directory) for directory in settings.source.roots)}
    if not all_units:
        raise ValueError("refusing empty preservation coverage")
    if units is not None and (not units or len(set(units)) != len(units) or not set(units) <= all_units):
        raise ValueError("preservation units must be unique configured source translation units")
    measured_units = sorted(all_units if units is None else units)
    changed_files = sorted(name for name in baseline.keys() | current.keys() if baseline.get(name) != current.get(name))
    file_set_changes = sorted(baseline.keys() ^ current.keys())
    tree_mode = bool(file_set_changes)
    # Full-tree protocol v5 handles historical header/seed file sets. A new or
    # removed product TU still needs explicit build coverage and recipe mapping;
    # do not infer that mapping from filenames or similar emitted symbols.
    unavailable = (file_set_changes if compiler is not None and tree_mode
                   and not callable(getattr(compiler, "materialize", None)) else
                   sorted(set(file_set_changes) & all_units))
    reports, controls, attempts = [], [], 0

    def fresh():
        if (settings.snapshot(root, metadata) != before or store.read(task_name) != task
                or store.decisions(task_name) != decisions or git(root, "rev-parse", "HEAD").decode().strip() != head
                or changes(state, working_state(root))):
            raise ValueError("source, compiler inputs, task or Git state changed during preservation checking")

    def compile(unit, flags, replacements=None):
        nonlocal compiler, attempts
        attempts += 1
        fresh()
        if compiler is None:
            compiler = Compiler(root, profile=settings.compiler_profile)
        value = (compiler(root / unit, (), flags) if replacements is None else
                 compiler.materialize(root / unit, roots, replacements, (), flags) if tree_mode else
                 compiler.overlay(root / unit, replacements, (), flags))
        fresh()
        return Coff(value)

    try:
        if not unavailable:
            old_overlay = baseline if tree_mode else {name: baseline[name] for name in changed_files}
            identity_overlay = current if tree_mode else {name: current[name] for name in changed_files}
            for flags in settings.flag_sets:
                control = {"flags": list(flags), "result": Result.UNKNOWN if diagnostic else Result.PASS, "files": [],
                           "mode": "omitted_for_diagnostics" if diagnostic else "per_file_damage",
                           "omitted_file_controls": len(changed_files) if diagnostic else 0}
                for name in (() if diagnostic else changed_files):
                    marker = "MATCH_RECOVERY_PRESERVATION_" + digest({"inputs": before, "file": name, "flags": flags})
                    # Deleted files must be observable in their historical tree;
                    # newly added files in the current one. Never resurrect them
                    # in the other version just to manufacture a witness.
                    control_inputs = identity_overlay if name in current else baseline
                    contents = current[name] if name in current else baseline[name]
                    damage = {**control_inputs, name: b"#error " + marker.encode() + b"\n" + contents}
                    witnessed = None
                    # Ordering is only a latency hint. The actual compiler must
                    # observe this file's unique diagnostic; path adjacency or a
                    # generic failed compile never satisfies the control.
                    preferred = sorted(measured_units, key=lambda unit: (Path(unit).with_suffix("") != Path(name).with_suffix(""),
                                                                        not any(row["unit"] == unit for key in selected
                                                                                for row in new_bindings.get(key, [])), unit))
                    for unit in preferred:
                        try:
                            compile(unit, flags, damage)
                        except (OSError, ValueError) as error:
                            if marker in str(error):
                                witnessed = unit
                                break
                    fresh()
                    control["files"].append({"path": name, "unit": witnessed,
                                             "tree": "current" if name in current else "baseline",
                                             "result": Result.PASS if witnessed else Result.UNKNOWN})
                    if witnessed is None:
                        control["result"] = Result.UNKNOWN
                controls.append(control)
                for unit in measured_units:
                    row = {"unit": unit, "flags": list(flags), "result": Result.UNKNOWN}
                    try:
                        original = compile(unit, flags)
                        # Always exercise the overlay path, including no-op
                        # tasks. Each TU gets a positive current replica control.
                        same = compile(unit, flags, identity_overlay or {unit: current[unit]})
                        if object_signature(original) != object_signature(same):
                            raise ValueError("current replica does not agree with direct compilation")
                        prior = compile(unit, flags, old_overlay) if old_overlay else same
                        old_allowed, old_outside = _unit_scope(old_bindings, selected, unit)
                        new_allowed, new_outside = _unit_scope(new_bindings, selected, unit)
                        comparison = compare_outside_functions(prior, original, old_allowed, new_allowed,
                                                               baseline_outside=old_outside, current_outside=new_outside)
                        _explain_sources(comparison, old_sources, new_sources)
                        row.update(comparison, replica=Result.PASS)
                        row["compiled_comparison"] = comparison["result"]
                        if control["result"] != Result.PASS:
                            row.update(result=Result.UNKNOWN, reason="per-file controls deliberately omitted for diagnostics" if diagnostic else
                                       "baseline substitutions lack per-file compiler controls")
                    except (OSError, ValueError) as error:
                        row.update(result=Result.UNKNOWN, reason=str(error))
                    reports.append(row)
    finally:
        fresh()
    unmeasured = sorted(all_units - set(measured_units))
    counts = Counter(row["result"] for row in reports)
    result = (Result.UNKNOWN if diagnostic or unavailable or unmeasured or counts[Result.UNKNOWN] or not reports else
              Result.FAIL if binding_changes or annotation_changes or counts[Result.FAIL] else Result.PASS)
    return {"task_id": task_name, "check": "baseline_current_compiled_residuals", "result": result,
            "base_revision": task.base_revision, "head": head, "changed_files": changed_files,
            "outside_binding_changes": binding_changes, "file_set_changes": file_set_changes,
            "unsupported_file_set_changes": unavailable, "materialization": "complete-tree-v5" if tree_mode else "existing-file-v4",
            "outside_annotation_changes": annotation_changes,
            "translation_units": measured_units, "unmeasured_units": unmeasured,
            "controls": controls, "units": reports, "counts": dict(counts), "compile_attempts": attempts,
            "diagnostic_only": diagnostic,
            "input_fingerprint": digest(before), "persisted": False, "ready_for_collection": False,
            "scope_complete": False, "outside_scope_verification": Result.UNKNOWN,
            "limitations": ["compiled residuals only; no original-byte, full-link, runtime or lifecycle proof",
                            "same current compiler policy on both source versions, not historical artifact recreation",
                            "only source-mapped function COMDAT ownership may vary; independent data/tables stay protected",
                            "source/configuration roots must agree; added/deleted product TUs require explicit historical recipe/coverage mapping",
                            "changed non-TU file sets require compiler protocol v5; unobserved seed/header substitutions remain unknown",
                            "symbol-to-source matches are annotation navigation hints, not ownership proof or scope authorization",
                            "diagnostic mode omits per-file damage controls and can never establish preservation",
                            "filtered units and missing compiler controls cannot yield an overall pass",
                            "all configured non-seed source units, including dependencies and unmapped units; build coverage still requires verification"]}

"""Fresh baseline/current compiler evidence outside task-owned functions.

The task's Git baseline is fixed. Current compiler interaction and measurement
policy are used on both source versions. Explicit versioned build inputs may
change generated recipes; this is not historical binary recreation or a migration gate.
All reports are disposable. No claims, baselines or task decisions are rewritten.
"""

from collections import Counter, defaultdict
from contextlib import ExitStack
from copy import deepcopy
from dataclasses import asdict
from pathlib import Path
import re
from weakref import WeakKeyDictionary

from .coff_preservation import compare_outside_functions, compare_unit_presence
from .coff_replica import object_signature
from .compiler import Compiler
from .component_source import parse_components
from .driver import changes, working_state
from .measurement import Measurement
from .generated_source import parse_generated
from .generated_permissions import generated_sources
from .layout_source import parse_members, parse_bases
from .model import Origin, Result, digest
from .pe_coff import Coff
from .preservation_objects import ObjectTrial, application_objects
from .object_source import parse_objects
from .shared_source import parse_shared
from .import_source import parse_imports
from .records import parse_json, read_json
from .source_config import SourceConfiguration
from .source_history import HistoryConfiguration, _tree, git, revision_files
from .source_inventory import image_pin
from .source_replica import replica_roots, replica_snapshot, snapshot_digest
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
        for item in parse_shared(raw.decode("utf-8", errors="replace"), name, pin, address_space=source.address_space):
            # Shared methods have distinct preservation identities, while the
            # evidence anchor determines editing scope. Do not merge siblings
            # under the single canonical original-function selection key.
            rows[item.interpretation_id].append({"unit": name, "symbol": item.symbol,
                "spans": ((item.span.start, item.span.end),), "body": None,
                "byte_obligation": False, "shape_obligation": False, "scope_entity": item.evidence.entity_id})
    return dict(rows)


def _unit_scope(rows, selected, unit):
    allowed, outside = {}, set()
    for entity, candidates in rows.items():
        for row in candidates:
            if row["unit"] != unit:
                continue
            symbol = row["symbol"]
            if row.get("scope_entity", entity) not in selected:
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
                             ("base", parse_bases), ("generated", parse_generated), ("shared", parse_shared)):
            for item in parser(text, name, pin, address_space=source.address_space):
                # Bounded objects belong to their explicit evidence anchor,
                # as in task binary checks and historical obligations. Legacy
                # unbounded identities have no anchor and retain origin scope.
                anchor = (item.evidence or item.origin) if kind == "object" else item.evidence
                if anchor.entity_id not in selected:
                    value = {"kind": kind, **{key: value for key, value in asdict(item).items() if key not in ("line", "body_error")}}
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
                     "task_scope": row.get("scope_entity", entity) in selected}
            index[row["symbol"]].append(value)
            if row.get("scope_entity", entity) in selected:
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
                                                "source": (row["manifest"]["source_root"] + "/" + binding["unit"]
                                                           if "source_root" in row["manifest"] else row["path"]),
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
                       diagnostic: bool = False, compiler=None, integration: dict | None = None,
                       application=None, permission=None):
    if type(diagnostic) is not bool:
        raise ValueError("diagnostic mode must be boolean")
    root = root.resolve()
    settings, store = Measurement.read(root), TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    from .integration import preservation_base
    baseline_revision = preservation_base(root, task, integration)
    selected, editable = store._scope(task, decisions)
    permission = deepcopy(permission)
    if permission is not None and (application is None or diagnostic):
        raise ValueError("generated preservation requires a live application graph and complete controls")
    head = git(root, "rev-parse", "HEAD").decode().strip()
    git(root, "merge-base", "--is-ancestor", task.base_revision, head)
    pin = image_pin(root, settings.source)
    if any(subject.image_sha256 != pin or subject.address_space != settings.source.address_space for subject in task.subjects):
        raise ValueError("preservation task refers to another original image/address space")
    metadata = {"task": task, "decisions": decisions, "head": head, "integration": integration}
    if permission is not None:
        metadata["permission"] = permission
    before, state = settings.snapshot(root, metadata), working_state(root)
    regenerated = "preparation" in read_json(root / "recovery/project.json")
    if regenerated and compiler is not None:
        raise ValueError("prepared preservation requires the configured compiler in independently generated projects")
    if regenerated:
        from .prepared_context import source_roots
        roots = source_roots(root, settings)
    else:
        roots = replica_roots((*settings.source.roots, *settings.input_roots))
    current = {name: data for name, data in replica_snapshot(root, roots).items() if data is not None}
    baseline = revision_files(root, baseline_revision, roots)
    authority = {"task": task.to_dict(), "decisions": decisions, "integration": integration}
    generated = []
    if permission is not None:
        authority["permission"] = permission
        generated = generated_sources(permission, task, selected, editable,
                                      {"baseline": baseline, "current": current}, settings, pin, root)
        if not generated:
            raise ValueError("generated preservation requires explicit generated changes")
    authorization = digest(authority)
    source_digests = {}
    if application is not None:
        application.check_current()
        if (application.root != root or application.task_name != task_name
                or application.baseline_revision != baseline_revision
                or digest(application.authorization) != authorization
                or application.roots != replica_roots(settings.source.roots)):
            raise ValueError("live application graph differs from preservation task or authorization")
        sources = application.source_versions
        for version, files in (("baseline", baseline), ("current", current)):
            source_files = {name: raw for name, raw in files.items()
                            if any(Path(name).is_relative_to(directory) for directory in application.roots)}
            if source_files != sources[version]:
                raise ValueError("live application graph differs from preservation source versions")
            source_digests[version] = snapshot_digest(source_files)
    # Build graph inputs are distinct from compiler implementation/policy. Only
    # explicitly declared files travel with their historical source version.
    versioned_inputs, build_inputs = (), {"baseline": {}, "current": {}}
    if regenerated:
        from .build_check import Build
        versioned_inputs = Build.read(root, key="preparation").versioned_inputs
        for name in versioned_inputs:
            path = root / name
            if not path.is_file() or path.resolve() != path:
                raise ValueError("versioned build inputs must be canonical regular files")
            build_inputs["current"][name] = path.read_bytes()
        if versioned_inputs:
            entries = _tree(root, baseline_revision, versioned_inputs)
            for name in versioned_inputs:
                entry = entries.get(name)
                if entry is not None:
                    if entry[:2] not in (("100644", "blob"), ("100755", "blob")):
                        raise ValueError("historical versioned build input is not a regular file")
                    build_inputs["baseline"][name] = git(root, "cat-file", "blob", entry[2])
    configuration_path = "recovery/project.json"
    tree = _tree(root, baseline_revision, (configuration_path,))
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
    pin_tree = _tree(root, baseline_revision, (settings.source.image_pin,))
    entry = pin_tree.get(settings.source.image_pin)
    if entry is None or entry[:2] not in (("100644", "blob"), ("100755", "blob")):
        raise ValueError("preservation baseline original pin is missing or not regular")
    if re.findall(rb"^sha256\s+([0-9a-f]{64})\s*$", git(root, "cat-file", "blob", entry[2]), re.MULTILINE) != [pin.encode()]:
        raise ValueError("preservation baseline original pin is missing or different")
    old_bindings, new_bindings = _bindings(root, baseline, settings.source, pin), _bindings(root, current, settings.source, pin)
    binding_changes = [{"entity_id": key, "baseline": old_bindings.get(key, []), "current": new_bindings.get(key, [])}
                       for key in sorted((old_bindings.keys() | new_bindings.keys()) - selected)
                       if old_bindings.get(key) != new_bindings.get(key)
                       and any(row.get("scope_entity", key) not in selected
                               for row in (*old_bindings.get(key, []), *new_bindings.get(key, [])))]
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
    changed_units = sorted(set(file_set_changes) & all_units)
    measured_added_units = set(measured_units) & (current.keys() - baseline.keys())
    # Complete materialization alone cannot establish a missing-version recipe.
    # Regenerated projects can supply explicit per-version source/recipe sets.
    unavailable = (file_set_changes if compiler is not None and tree_mode
                   and not callable(getattr(compiler, "materialize", None)) else
                   changed_units if not regenerated else [])
    reports, controls, attempts = [], [], 0
    contexts, replicas = {}, ExitStack()
    membership, preparation_error = {}, None
    # Only live Coff instances retain compiler bytes. Damage/replica objects
    # discarded by the existing loop do not accumulate a second object cache.
    object_trials = WeakKeyDictionary()
    compare_application_object = None

    def fresh():
        if (settings.snapshot(root, metadata) != before or store.read(task_name) != task
                or store.decisions(task_name) != decisions or git(root, "rev-parse", "HEAD").decode().strip() != head
                or changes(state, working_state(root))):
            raise ValueError("source, compiler inputs, task or Git state changed during preservation checking")

    def environment(files, version):
        from .prepared_context import prepared_compiler
        key = digest({"source": snapshot_digest(files), "build": snapshot_digest(build_inputs[version])})
        if key not in contexts:
            contexts[key] = replicas.enter_context(prepared_compiler(
                root, roots, files, settings, versioned_files=build_inputs[version]))
        return contexts[key]

    def compile(unit, flags, replacements=None, *, version="current"):
        nonlocal compiler, attempts
        attempts += 1
        fresh()
        if compiler is None:
            compiler = Compiler(root, profile=settings.compiler_profile)
        if replacements is not None and regenerated:
            files = replacements if tree_mode else {**current, **replacements}
            prepared = environment(files, version)
            value = prepared(prepared.root / unit, (), flags)
        else:
            value = (compiler(root / unit, (), flags) if replacements is None else
                     compiler.materialize(root / unit, roots, replacements, (), flags) if tree_mode else
                     compiler.overlay(root / unit, replacements, (), flags))
        fresh()
        raw = bytes(value)
        obj = Coff(raw)
        if application is not None:
            files = (current if replacements is None else replacements if tree_mode else
                     {**current, **replacements})
            canonical = baseline if version == "baseline" else current
            # A successful damage compile is not canonical trial evidence.
            if files == canonical:
                object_trials[obj] = ObjectTrial(
                    raw, root, task_name, baseline_revision, authorization, source_digests[version],
                    unit, version, tuple(flags), digest(before))
        return obj

    def linked_objects(unit, flags, prior, original):
        evidence = []
        for version, obj in (("baseline", prior), ("current", original)):
            if obj is None:
                continue  # Absence never has a fabricated object or receipt.
            try:
                report = compare_application_object(
                    object_trials.get(obj), obj, unit=unit, version=version,
                    flags=flags, input_fingerprint=digest(before))
            except (OSError, ValueError) as error:
                report = {"unit": unit, "version": version, "flags": list(flags),
                          "result": Result.UNKNOWN, "reason": str(error), "linked_effects": Result.UNKNOWN}
            evidence.append(report)
        return evidence

    def startup_coverage(unit, flags, version, obj):
        rows = [row for row in generated if row["unit"] == unit and row["version"] == version]
        if not rows:
            return None
        if obj is None:
            raise ValueError("authorized generated occurrence lacks its actual preservation object")
        return compare_application_object.cover_startup(
            object_trials.get(obj), obj, unit=unit, version=version,
            flags=flags, input_fingerprint=digest(before), changes=rows)

    try:
        if application is not None:
            # Index native membership once, bracket the entire compiler epoch
            # with live build checks, and bind each read to its captured receipt.
            compare_application_object = replicas.enter_context(application_objects(application))
        if regenerated:
            try:
                for version, files in (("baseline", baseline), ("current", current)):
                    prepared = environment(files, version)
                    expected = all_units & files.keys()
                    # Do not intersect away generated/extra recipes: an added
                    # build-only TU is still unmeasured program input.
                    observed = prepared.units
                    if observed != expected:
                        raise ValueError(version + " prepared recipe membership differs from source TU coverage")
                    membership[version] = sorted(observed)
                fresh()
            except (OSError, ValueError) as error:
                unavailable = changed_units
                preparation_error = str(error)
        if not unavailable and preparation_error is None:
            old_overlay = baseline if tree_mode else {name: baseline[name] for name in changed_files}
            identity_overlay = current if tree_mode else {name: current[name] for name in changed_files}
            for flags in settings.flag_sets:
                # Objects/witnesses are shared only within this immutable flag
                # trial. Historical and current source versions never share a
                # cached definition or damage result.
                verified_objects, damaged_objects = {}, {}

                def version_compile(version, unit, replacements=None):
                    overlay = old_overlay if version == "baseline" else identity_overlay
                    return compile(unit, flags, {**overlay, **(replacements or {})} or
                                   {unit: (baseline if version == "baseline" else current)[unit]}, version=version)

                def verified(version, unit):
                    key = version, unit
                    if key not in verified_objects:
                        first = version_compile(version, unit)
                        replica = version_compile(version, unit)
                        if object_signature(first) != object_signature(replica):
                            raise ValueError("inline canonical replica does not agree")
                        verified_objects[key] = first
                    return verified_objects[key]

                def damaged(version, unit, header, contents):
                    key = version, unit, header, digest(contents.hex())
                    if key not in damaged_objects:
                        damaged_objects[key] = version_compile(version, unit, {header: contents})
                    return damaged_objects[key]

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
                    control_version = "current" if name in current else "baseline"
                    present_files = current if name in current else baseline
                    preferred = sorted((unit for unit in measured_units if unit in present_files), key=lambda unit: (Path(unit).with_suffix("") != Path(name).with_suffix(""),
                                                                        not any(row["unit"] == unit for key in selected
                                                                                for row in new_bindings.get(key, [])), unit))
                    for unit in preferred:
                        try:
                            compile(unit, flags, damage, version=control_version)
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
                protected, protection_error = set(), None
                if measured_added_units and not diagnostic:
                    # A new TU cannot hide a formerly unbound/outside definition
                    # by annotating it at a different path. Observe the complete
                    # baseline's emitted names, not just source annotations.
                    try:
                        for unit in sorted(all_units & baseline.keys()):
                            obj = verified("baseline", unit)
                            allowed, _ = _unit_scope(old_bindings, selected, unit)
                            protected.update(s.name for s in obj.symbols.values()
                                             if s.section > 0 and s.name not in allowed)
                    except (OSError, ValueError) as error:
                        protection_error = str(error)
                for unit in measured_units:
                    row = {"unit": unit, "flags": list(flags), "result": Result.UNKNOWN}
                    try:
                        if unit in measured_added_units and protection_error:
                            raise ValueError("complete baseline definition coverage unavailable: " + protection_error)
                        original = compile(unit, flags) if unit in current else None
                        # Always exercise the overlay path, including no-op
                        # tasks. Each TU gets a positive current replica control.
                        if original is not None:
                            same = compile(unit, flags, identity_overlay or {unit: current[unit]})
                            if object_signature(original) != object_signature(same):
                                raise ValueError("current replica does not agree with direct compilation")
                            verified_objects["current", unit] = original
                        if diagnostic and unit in measured_added_units:
                            # Without complete baseline definition coverage,
                            # even a source-scoped name might be old unbound
                            # code. Show raw present-object evidence instead:
                            # subtract nothing and claim no compiled pass.
                            comparison = compare_unit_presence(original, {}, present="current")
                            _explain_sources(comparison, old_sources, new_sources)
                            row.update(comparison, replica=Result.PASS, result=Result.UNKNOWN,
                                       diagnostic_basis="raw_present_object",
                                       baseline_definition_coverage="omitted_for_diagnostics",
                                       scope_subtraction="omitted_for_diagnostics",
                                       reason="raw added-TU diagnostic; baseline definition protection and scope subtraction omitted")
                            if application is not None:
                                row["application_objects"] = linked_objects(unit, flags, None, original)
                            reports.append(row)
                            continue
                        prior = None
                        if unit in baseline:
                            if changed_units:
                                prior = verified("baseline", unit)
                            elif old_overlay or build_inputs["baseline"] != build_inputs["current"] or application is not None:
                                prior = compile(unit, flags, old_overlay, version="baseline")
                            else:
                                prior = same
                        old_allowed, old_outside = _unit_scope(old_bindings, selected, unit)
                        new_allowed, new_outside = _unit_scope(new_bindings, selected, unit)
                        from .inline_ownership import observe_copies
                        inline_reports = []
                        for version, obj, bindings, files, allowed in (
                                ("baseline", prior, old_bindings, baseline, old_allowed),
                                ("current", original, new_bindings, current, new_allowed)):
                            if obj is None:
                                continue
                            copies, evidence = observe_copies(
                                obj, unit, bindings, selected, editable, files, settings,
                                lambda owner, version=version: verified(version, owner),
                                lambda owner, header, content, version=version:
                                    damaged(version, owner, header, content))
                            allowed.update(copies)
                            inline_reports.extend({"version": version, **item} for item in evidence)
                        if inline_reports:
                            row["inline_ownership"] = inline_reports
                        old_startup = startup_coverage(unit, flags, "baseline", prior)
                        new_startup = startup_coverage(unit, flags, "current", original)
                        if prior is None or original is None:
                            comparison = compare_unit_presence(
                                prior if original is None else original,
                                old_allowed if original is None else new_allowed,
                                present="baseline" if original is None else "current",
                                outside=old_outside if original is None else new_outside,
                                protected=protected if prior is None else frozenset(),
                                startup=old_startup if original is None else new_startup)
                        else:
                            comparison = compare_outside_functions(prior, original, old_allowed, new_allowed,
                                                                   baseline_outside=old_outside, current_outside=new_outside,
                                                                   baseline_startup=old_startup, current_startup=new_startup)
                        if old_startup is not None or new_startup is not None:
                            row["generated_changes"] = [item for item in generated if item["unit"] == unit]
                        _explain_sources(comparison, old_sources, new_sources)
                        row.update(comparison, replica=Result.PASS)
                        row["compiled_comparison"] = comparison["result"]
                        if application is not None:
                            row["application_objects"] = linked_objects(unit, flags, prior, original)
                        if control["result"] != Result.PASS:
                            row.update(result=Result.UNKNOWN, reason="per-file controls deliberately omitted for diagnostics" if diagnostic else
                                       "baseline substitutions lack per-file compiler controls")
                        elif any(item["result"] != Result.PASS for item in inline_reports):
                            row.update(result=Result.UNKNOWN, reason="header-defined copies lack compiler ownership witnesses")
                    except (OSError, ValueError) as error:
                        row.update(result=Result.UNKNOWN, reason=str(error))
                    reports.append(row)
    finally:
        try:
            replicas.close()
        finally:
            try:
                fresh()
            finally:
                if application is not None:
                    application.check_current()
    unmeasured = sorted(all_units - set(measured_units))
    counts = Counter(row["result"] for row in reports)
    result = (Result.UNKNOWN if diagnostic or unavailable or preparation_error or unmeasured or counts[Result.UNKNOWN] or not reports else
              Result.FAIL if binding_changes or annotation_changes or counts[Result.FAIL] else Result.PASS)
    return {"task_id": task_name, "check": "baseline_current_compiled_residuals", "result": result,
            "base_revision": baseline_revision,
            **({"task_base_revision": task.base_revision, "integration": integration} if integration is not None else {}),
            "head": head, "changed_files": changed_files,
            "outside_binding_changes": binding_changes, "file_set_changes": file_set_changes,
            "unsupported_file_set_changes": unavailable,
            "preparation_error": preparation_error,
            "recipe_membership": membership,
            "versioned_build_inputs": list(versioned_inputs),
            "changed_build_inputs": sorted(name for name in versioned_inputs
                                           if build_inputs["baseline"].get(name) != build_inputs["current"].get(name)),
            "materialization": "fresh-prepared-project" if regenerated else "complete-tree-v5" if tree_mode else "existing-file-v4",
            "prepared_contexts": len(contexts),
            "outside_annotation_changes": annotation_changes,
            "translation_units": measured_units, "unmeasured_units": unmeasured,
            "controls": controls, "units": reports, "counts": dict(counts), "compile_attempts": attempts,
            "diagnostic_only": diagnostic,
            "input_fingerprint": digest(before), "persisted": False, "ready_for_collection": False,
            "scope_complete": False, "outside_scope_verification": Result.UNKNOWN,
            "limitations": ["compiled residuals only; no original-byte, full-link, runtime or lifecycle proof",
                            "current compiler interaction and measurement policy on both versions; explicit versioned build inputs may change recipes, not historical artifact recreation",
                            "configured preparation regenerates each private source context; current replicas must agree with direct output",
                            "live application object agreement alone grants no residual scope; explicit generated decisions additionally require exact live initializer coverage",
                            "source-mapped functions and explicitly covered initializer slots may vary; unowned roots, helpers, storage, references and metadata remain strict",
                            "header copies require an authorized explicit body path, canonical replica and per-version/flag body-damage witnesses; exports and outside callers remain protected",
                            "source interpretation must agree; added/deleted TUs require fresh per-version source/recipe coverage and strict present-object residual checks",
                            "only explicitly declared versioned build inputs are substituted; compiler policy stays current, and build inputs are not compiler #error controls",
                            "absence has no fabricated compiler object; retained types, directives, empty section metadata and unowned symbols remain conservative differences",
                            "changed non-TU file sets require compiler protocol v5; unobserved seed/header substitutions remain unknown",
                            "symbol-to-source matches are annotation navigation hints, not ownership proof or scope authorization",
                            "diagnostic mode omits per-file damage controls and can never establish preservation",
                            "added-TU diagnostics show replica-checked raw residuals without baseline definition coverage or scope subtraction",
                            "filtered units and missing compiler controls cannot yield an overall pass",
                            "all configured non-seed source units, including dependencies and unmapped units; build coverage still requires verification"]}

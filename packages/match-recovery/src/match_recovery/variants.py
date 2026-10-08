"""Task-scoped candidate decks: one map read, temporary compiles, no banking."""

from concurrent.futures import ThreadPoolExecutor
from contextvars import copy_context
import hashlib
from pathlib import Path
from threading import Lock

from .application_binary import check_compiled, select_bindings
from .coff_replica import function_signature, object_signature
from .compiler import Compiler
from .components import local_path, relative_path
from .component_source import parse_components
from .generated_source import read_generated
from .measurement import Measurement
from .model import Origin, Result, Span, digest, require_digest, require_text
from .object_source import read_objects, referenced_object_units, require_disjoint_object_symbols
from .literal_source import read_literals
from .import_source import read_imports, parse_imports
from .ownership_source import class_hint, project_view
from .partition import Candidate, partition
from .pe_coff import Coff, PeImage
from .source.reader import read_text
from .tasks import TaskStore
from .shared_source import read_shared, parse_shared, referenced_shared_units, validate_shared


def _rank_trials(baselines, trials):
    """Explain partial binary progress, without granting recovery acceptance.

    Compare stable original identities, not proposed symbols or list positions.
    All trials of a flag set use one measurement universe: losing a discovered
    dependency is missing evidence, not a smaller denominator or a zero distance.
    """
    def measurements(trial):
        rows = {("function", row["unit"], row["entity_id"]): row
                for row in (trial["target"], *trial["neighbors"])}
        rows.update({("object", row["unit"], row["symbol"], row["address"]): row
                     for row in trial["object_definitions"] if "content" in row})
        rows.update({("shared", row["unit"], row["symbol"], row["address"]): row
                     for row in trial.get("shared_functions", [])})
        rows.update({("literal", row["unit"], row["symbol"], row["address"]): row
                     for row in trial.get("literal_definitions", [])})
        return rows

    def result(key, row):
        return row.get("content" if key[0] in ("object", "literal") else "result", Result.UNKNOWN)

    rankings = []
    for baseline in baselines:
        group = [trial for trial in trials if trial["flags"] == baseline["flags"]]
        if not group:
            continue
        before = measurements(baseline)
        observed = [(trial, measurements(trial)) for trial in group]
        scoped = {key for rows in (before, *(rows for _, rows in observed)) for key, row in rows.items()
                  if key[0] in ("object", "shared", "literal") or row.get("in_task") or key[2] == baseline["target"]["entity_id"]}
        passing = {key for key, row in before.items() if result(key, row) == Result.PASS}
        details = []
        for trial, rows in observed:
            losses = []
            for key in sorted(passing):
                after = result(key, rows.get(key, {}))
                if after != Result.PASS:
                    identity = ({"entity_id": key[2]} if key[0] == "function" else
                                {"symbol": key[2], "address": key[3]})
                    losses.append({"kind": key[0], "unit": key[1], **identity, "after": after})
            counts = {state.value: 0 for state in Result}
            proofs = []
            for key in sorted(scoped):
                row = rows.get(key, {})
                counts[result(key, row)] += 1
                proofs.extend([row.get("proof", {})] if key[0] == "object" else row.get("spans") or [row])
            details.append({"name": trial["name"], "compile_errors": len(trial["compile_errors"]),
                            "lost_passing_measurements": losses, "scored_results": counts,
                            "unmeasured_distances": sum("differing_offsets" not in proof for proof in proofs),
                            "known_differing_bytes": sum(len(proof.get("differing_offsets", ())) for proof in proofs)})

        def key(detail):
            return (detail["compile_errors"], len(detail["lost_passing_measurements"]),
                    detail["scored_results"]["unknown"], detail["scored_results"]["fail"],
                    detail["unmeasured_distances"], detail["known_differing_bytes"], detail["name"])

        details.sort(key=key)
        rankings.append({"flags": baseline["flags"], "names": [row["name"] for row in details], "details": details})
    return rankings


def _texts(value):
    """One exact edit, or an ordered list of edits to the same original file."""
    if isinstance(value, str):
        return [value]
    if not isinstance(value, list) or not value or any(not isinstance(item, str) for item in value):
        raise ValueError("candidate edit text must be a string or nonempty string array")
    return value


def _anchor_ranges(original, value):
    ranges = []
    for text in _texts(value):
        require_text(text, "candidate replacement anchor")
        anchor = text.encode("utf-8")
        start = original.find(anchor)
        # bytes.count misses overlapping occurrences such as 'aaa' in 'aaaa'.
        if start < 0 or original.find(anchor, start + 1) >= 0:
            raise ValueError("candidate replacement anchor must occur exactly once")
        ranges.append((start, start + len(anchor)))
    ordered = sorted(ranges)
    if any(left[1] > right[0] for left, right in zip(ordered, ordered[1:])):
        raise ValueError("candidate replacement anchors must not overlap")
    return ranges


def _splice(original, ranges, value):
    texts = _texts(value)
    if len(texts) != len(ranges):
        raise ValueError("candidate replacement count must equal the file's anchor count")
    pieces, cursor = [], 0
    # All ranges refer to original bytes; replacement text is never searched
    # again or consumed by a later edit. Preserve untouched bytes verbatim.
    for (start, end), text in sorted(zip(ranges, texts)):
        pieces.extend((original[cursor:start], text.encode("utf-8")))
        cursor = end
    pieces.append(original[cursor:])
    return b"".join(pieces)


def _deck(value, root, editable, selected, settings):
    fields = {"schema_version", "source", "entity_id", "files", "candidates"}
    if set(value) != fields or type(value["schema_version"]) is not int or value["schema_version"] != 2:
        raise ValueError("invalid candidate deck schema")
    relative_path(value["source"])
    require_text(value["entity_id"], "candidate entity")
    if not isinstance(value["files"], dict) or not value["files"]:
        raise ValueError("candidate deck requires file anchors and per-file controls")
    originals, anchors, controls = {}, {}, {}
    for name, row in value["files"].items():
        relative_path(name)
        if (name not in editable or settings.source.is_seed(name) or Path(name).suffix not in settings.source.extensions
                or not any(Path(name).is_relative_to(directory) for directory in settings.source.roots)
                or name == settings.source.image_pin):
            raise ValueError("candidate replacements require authorized product source files")
        path = local_path(root, name)
        if path.resolve() != path or not path.is_file():
            raise ValueError("candidate replacements require existing canonical files")
        if not isinstance(row, dict) or set(row) != {"source_sha256", "before", "control", "entity_id"}:
            raise ValueError("candidate file requires digest, anchor, control and witness entity")
        if row["entity_id"] not in selected:
            raise ValueError("candidate damage witnesses must be task subjects")
        original = originals[name] = path.read_bytes()
        require_digest(row["source_sha256"])
        if hashlib.sha256(original).hexdigest() != row["source_sha256"]:
            raise ValueError("candidate deck source digest is stale")
        anchors[name] = _anchor_ranges(original, row["before"])
        controls[name] = (row["entity_id"], _splice(original, anchors[name], row["control"]))
    candidates = value["candidates"]
    if not isinstance(candidates, dict) or not candidates:
        raise ValueError("candidate deck requires named replacements")
    replacements = {}
    for name, after in candidates.items():
        require_text(name, "candidate name")
        if not isinstance(after, dict) or set(after) != set(originals):
            raise ValueError("each candidate must provide replacement text for every declared file")
        replacements[name] = {path: _splice(original, anchors[path], after[path])
                              for path, original in originals.items()}
    for path, original in originals.items():
        def identities(contents):
            return [(item.symbol, item.module, item.name, item.ordinal)
                    for item in parse_imports(contents.decode("utf-8"), path)]
        expected = identities(original)
        if any(identities(contents) != expected for contents in
               [controls[path][1], *(candidate[path] for candidate in replacements.values())]):
            raise ValueError("candidate deck cannot change import identities; checkpoint binding decisions first")
    return originals, controls, replacements


def _trial_bindings(view, originals, contents, selected, settings, pin, *, control=False):
    """Rebind proposed function symbols without borrowing baseline correlations.

    Original identities/obligations and non-function interpretations belong to
    checkpoint decisions, not spelling search. A deck may propose an emitted
    function symbol for a task subject; it never publishes that decision.
    Only edited files are reparsed, and the ordinary selector handles ambiguity.
    """
    protected = ("address", "name", "image_spans", "mode", "body", "kind",
                 "exclusion", "byte_exact", "semantic")
    updates, changes = {}, []
    for path, original in originals.items():
        baseline_text, proposed_text = original.decode("utf-8"), contents[path].decode("utf-8")
        # These line-oriented interpretations are not candidate function names.
        # Keep their evidence intact rather than scoring changed C++ against an
        # obsolete object/import/table/layout binding. Component blocks also
        # contain multi-line metadata and need their complete parser below.
        def interpretations(text):
            return [line.strip() for line in text.splitlines()
                    if line.lstrip().startswith("// RECOVERY_")]
        if (interpretations(baseline_text) != interpretations(proposed_text)
                or parse_components({path: baseline_text}) != parse_components({path: proposed_text})):
            raise ValueError("candidate deck cannot change recovery interpretations; checkpoint binding decisions first")
        baseline, proposed = read_text(baseline_text, path), read_text(proposed_text, path)
        def identities(records):
            return [tuple(getattr(row, field) for field in protected) for row in records]
        if identities(baseline) != identities(proposed):
            raise ValueError("candidate deck cannot change original identities, spans or historical claims")
        for old, new in zip(baseline, proposed):
            old_symbol, new_symbol = old.symbol or old.name, new.symbol or new.name
            origin = Origin(pin, settings.source.address_space, old.address)
            if old_symbol != new_symbol:
                if control or origin.entity_id not in selected:
                    raise ValueError("candidate symbol changes require task authorization and cannot be damage controls")
                changes.append({"entity_id": origin.entity_id, "unit": path,
                                "before": old_symbol, "after": new_symbol})
            updates[path, old.line] = (new_symbol, new.line)
    candidates = []
    for group in view["groups"]:
        for entity in group["entities"]:
            for row in entity["candidates"]:
                symbol, line = updates.get((row["path"], row["line"]), (row["symbol"], row["line"]))
                owner, basis = row["owner_hint"], row["owner_hint_basis"]
                if symbol != row["symbol"] and settings.source.symbol_format == "msvc":
                    emitted = class_hint(symbol)
                    owner = emitted or row["catalogued_owner_hint"]
                    basis = "source_symbol" if emitted else "catalogued_symbol"
                candidates.append(Candidate(Origin(**row["origin"]), tuple(Span(**span) for span in row["spans"]),
                                            row["path"], line, row["seed"], symbol, owner,
                                            row["component"], basis, row["catalogued_owner_hint"]))
    selected_all = frozenset(entity["entity_id"] for group in view["groups"] for entity in group["entities"])
    bindings, refused, externals = select_bindings(partition(candidates), selected_all)
    return bindings, refused, externals, changes


def measure_variants(root: Path, task_name: str, image_path: Path, deck: dict, *, jobs: int = 1, compiler=None,
                     compiled_diagnostics: bool = False):
    """Score atomic header/body experiments within authorized ownership work.

    The exact-text anchor deliberately replaces brace guessing. Only compiler
    invocations are parallel; project/source/binding inputs are prepared once.
    No candidate is selected, applied, recorded or admitted by this operation.
    """
    if type(compiled_diagnostics) is not bool:
        raise ValueError("compiled_diagnostics must be a boolean")
    if type(jobs) is not int or jobs < 1:
        raise ValueError("candidate jobs must be a positive integer")
    root = root.resolve()
    settings, store = Measurement.read(root), TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    selected, files = store._scope(task, decisions)
    if not isinstance(deck, dict) or not isinstance(deck.get("source"), str):
        raise ValueError("candidate deck requires a source path")
    source = local_path(root, deck["source"])
    if deck["source"] not in files or deck.get("entity_id") not in selected:
        raise ValueError("candidate source and entity must be authorized by the task")
    if source.suffix not in settings.unit_extensions or settings.source.is_seed(deck["source"]):
        raise ValueError("candidate source must be a product translation unit")
    metadata = {"task": task, "decisions": decisions, "deck": deck}
    before = settings.snapshot(root, metadata)
    originals, damage_controls, candidates = _deck(deck, root, files, selected, settings)
    view, snapshot = project_view(root)
    pin = snapshot["image_sha256"]
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va" for subject in task.subjects):
        raise ValueError("candidate task refers to another original image")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, pin)
    all_entities = frozenset(entity["entity_id"] for group in view["groups"] for entity in group["entities"])
    bindings, all_refused, externals = select_bindings(view, all_entities)
    refused = [row for row in all_refused if row["entity_id"] in selected]
    target = next((item for item in bindings if item.origin.entity_id == deck["entity_id"] and item.unit == deck["source"]), None)
    if target is None:
        raise ValueError("candidate target requires one unambiguous mapped product definition with image spans")
    data_bindings, generated = read_objects(root, pin), read_generated(root, pin)
    literal_bindings = read_literals(root, pin)
    shared = read_shared(root, pin)
    validate_shared(shared, view)
    # Compile every mapped task TU together, not just the primary target. This
    # catches coupled header/body changes and includes out-of-scope neighbors in
    # those TUs. Uncompiled callers remain explicit, never an outside-scope pass.
    units = sorted({item.unit for item in bindings if item.origin.entity_id in selected
                    and Path(item.unit).suffix in settings.unit_extensions}
                   | {name for name in originals if Path(name).suffix in settings.unit_extensions}
                   | {item.unit for item in data_bindings if item.evidence is not None and item.evidence.entity_id in selected}
                   | {item.unit for item in literal_bindings if item.evidence.entity_id in selected}
                   | {item.unit for item in shared if item.evidence.entity_id in selected})
    if any(Path(unit).suffix not in settings.unit_extensions for unit in units):
        raise ValueError("required data owner is not a configured translation unit")
    unit_bindings = [item for item in bindings if item.unit in units]
    witnesses = {}
    for name, (entity, _) in damage_controls.items():
        witness = next((item for item in unit_bindings if item.origin.entity_id == entity), None)
        if witness is None:
            raise ValueError("each edited file requires a mapped compiled task witness")
        witnesses[name] = witness
    imports = read_imports(root)
    object_symbols = set(require_disjoint_object_symbols(
        data_bindings, set(externals) | {item.symbol for item in bindings},
        {item.symbol for item in generated},
        {item.symbol for item in imports}, {item.symbol for item in shared}))
    validate_shared(shared, view, reserved=[*object_symbols, *[item.symbol for item in (*generated, *imports)]])
    if (object_symbols & set(externals)
            or {item.symbol for item in generated} & (set(externals) | object_symbols)):
        raise ValueError("conflicting function/object/generated symbol bindings")
    candidate_bindings = {}
    for path, (_, contents) in damage_controls.items():
        _trial_bindings(view, originals, {**originals, path: contents}, selected, settings, pin, control=True)
    for name, contents in candidates.items():
        resolved = _trial_bindings(view, originals, contents, selected, settings, pin)
        proposed, _, targets, _ = resolved
        if {item.origin.entity_id for item in proposed} != {item.origin.entity_id for item in bindings}:
            raise ValueError("candidate symbol changes create or resolve ambiguous bindings; checkpoint decisions first")
        reserved = {*object_symbols, *[item.symbol for item in (*generated, *imports, *shared)]}
        if set(targets) & reserved:
            raise ValueError("conflicting function/object/generated/import symbol bindings")
        candidate_bindings[name] = resolved
    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    flags_list = tuple(flags + settings.layout_flags for flags in settings.flag_sets) if any(item.span is not None for item in data_bindings) else settings.flag_sets
    if compiled_diagnostics:
        from .compiled_diagnostics import compiled_diagnostic, diagnostic_report, trial_budget
        diagnostic_budget = trial_budget(len(flags_list) * (1 + len(candidates)))
    controls, baselines, trials = [], [], []
    attempts = 0
    # Only successful unchanged-source controls are reusable, within this
    # frozen deck and exact flag set. Keep bytes, not mutable parsed objects.
    # Candidate overlays and per-file damage controls always compile afresh.
    support_replicas, support_locks = {}, {}
    support_index_lock = Lock()
    overlay_cache, overlay_locks = {}, {}

    def overlay_compile(unit, contents, flags):
        key = (unit, flags, digest({name: hashlib.sha256(raw).hexdigest()
                                   for name, raw in sorted(contents.items())}))
        with support_index_lock:
            lock = overlay_locks.setdefault(key, Lock())
        with lock:
            if key in overlay_cache:
                raw = overlay_cache[key]
                return Coff(raw), raw, False
            raw = bytes(compiler.overlay(root / unit, contents, (), flags))
            overlay_cache[key] = raw
            return Coff(raw), raw, True
    baseline_cache = {}

    def effective(unit, requested):
        suffix = settings.layout_flags if settings.layout_flags and requested[-len(settings.layout_flags):] == settings.layout_flags else ()
        base = requested[:-len(suffix)] if suffix else requested
        return settings.effective_flags(unit, base) + suffix

    def fresh():
        if (settings.snapshot(root, metadata) != before
                or image_path.read_bytes() != image_data or store.read(task_name) != task or store.decisions(task_name) != decisions):
            raise ValueError("candidate source, image, compiler inputs or task changed; discard the deck")

    def score(objects, flags, name, errors=None, contents=None):
        errors = dict(errors or {})
        trial_bindings, _, trial_externals, changes = (
            (bindings, all_refused, externals, []) if contents is None else candidate_bindings[name])
        support_controls, support_attempts = [], 0
        trial_shared = shared
        if contents is not None:
            trial_shared = tuple(item for item in shared if item.unit not in contents) + tuple(
                item for unit, raw in contents.items()
                for item in parse_shared(raw.decode("utf-8"), unit, pin, address_space=settings.source.address_space))
        while pending := (referenced_object_units(data_bindings, objects)
                          | referenced_shared_units(trial_shared, objects)) - objects.keys() - errors.keys():
            for unit in sorted(pending):
                control = {"unit": unit, "flags": list(effective(unit, flags)), "graph_flags": list(flags),
                           "result": Result.UNKNOWN}
                try:
                    if Path(unit).suffix not in settings.unit_extensions:
                        raise ValueError("object owner is not a configured translation unit")
                    unit_flags = effective(unit, flags)
                    key = (unit, unit_flags)
                    with support_index_lock:
                        lock = support_locks.setdefault(key, Lock())
                    # A dependency first discovered by concurrent candidates
                    # needs one control, not one racing compilation per worker.
                    with lock:
                        replica_data = support_replicas.get(key)
                        control["replica_reused"] = replica_data is not None
                        if replica_data is None:
                            support_attempts += 1
                            original = Coff(compiler(root / unit, (), unit_flags))
                            replica_obj, replica_data, fresh_overlay = overlay_compile(unit, originals, unit_flags)
                            support_attempts += int(fresh_overlay)
                            if object_signature(original) != object_signature(replica_obj):
                                raise ValueError("unchanged-source replica differs from the original compilation: " + unit)
                            support_replicas[key] = replica_data
                    control["result"] = Result.PASS
                    if contents is None:
                        objects[unit] = Coff(replica_data)
                    else:
                        objects[unit], _, fresh_overlay = overlay_compile(unit, contents, unit_flags)
                        support_attempts += int(fresh_overlay)
                except (OSError, ValueError) as error:
                    errors[unit] = str(error)
                    control["reason"] = str(error)
                support_controls.append(control)
        measured = [item for item in trial_bindings if item.unit in objects or item.unit in units]
        active_shared = [item for item in trial_shared if item.unit in objects or item.unit in errors]
        rows, data_rows, generated_rows, import_rows, shared_rows, literal_rows = check_compiled(
            measured, data_bindings, generated, objects, image, trial_externals, errors=errors,
            import_bindings=imports, shared_bindings=active_shared, literal_bindings=literal_bindings)
        selected_row = next(row for row in rows if row["entity_id"] == target.origin.entity_id)
        neighbors = [row for row in rows if row is not selected_row]
        selected_row = {**selected_row, "flags": list(effective(target.unit, flags)), "graph_flags": list(flags)}

        def annotate(items):
            return [{**row, **({"flags": list(effective(row["unit"], flags)), "graph_flags": list(flags)}
                               if row.get("unit") else {})} for row in items]

        result = {"name": name, "flags": list(flags), "target": selected_row,
                "neighbors": [{**row, "in_task": row["entity_id"] in selected,
                               "flags": list(effective(row["unit"], flags)), "graph_flags": list(flags)}
                              for row in neighbors],
                "compile_errors": errors or {},
                "binding_changes": changes,
                "object_dependencies": support_controls, "support_compile_attempts": support_attempts,
                "object_definitions": annotate(data_rows), "generated_tables": annotate(generated_rows),
                "import_bindings": annotate(import_rows), "shared_functions": annotate(shared_rows),
                "literal_definitions": annotate(literal_rows),
                "unit_flag_sets": {unit: [list(effective(unit, flags))]
                                   for unit in sorted(set(objects) | set(errors))}}
        if compiled_diagnostics:
            binding = next(item for item in trial_bindings if item.origin.entity_id == target.origin.entity_id)
            result["_compiled_diagnostic"] = compiled_diagnostic(
                objects.get(binding.unit), binding, name=name, flags=effective(binding.unit, flags), budget=diagnostic_budget)
        return result

    # Replica and damage controls are per flag trial, not a one-time capability
    # claim. A matching score alone cannot show that the intended code changed.
    valid = []
    for flags in flags_list:
        row = {"flags": list(flags), "result": Result.UNKNOWN, "replicas": [], "files": []}
        try:
            fresh()
            baseline, replicas = {}, {}
            for unit in units:
                unit_flags = effective(unit, flags)
                cache_key = (unit, unit_flags)
                if cache_key not in baseline_cache:
                    attempts += 1
                    original = Coff(compiler(root / unit, (), unit_flags))
                    attempts += 1
                    replica = Coff(compiler.overlay(root / unit, originals, (), unit_flags))
                    baseline_cache[cache_key] = original, replica
                baseline[unit], replicas[unit] = baseline_cache[cache_key]
                if object_signature(baseline[unit]) != object_signature(replicas[unit]):
                    raise ValueError("unchanged-source replica differs from the original compilation: " + unit)
                row["replicas"].append({"unit": unit, "flags": list(unit_flags),
                                         "graph_flags": list(flags), "result": Result.PASS})
            baselines.append(score(baseline, flags, "baseline"))
            for name, (_, control) in damage_controls.items():
                witness = witnesses[name]
                damaged, _, compiled = overlay_compile(witness.unit, {**originals, name: control},
                                                       effective(witness.unit, flags))
                attempts += int(compiled)
                if function_signature(replicas[witness.unit], witness.symbol) == function_signature(damaged, witness.symbol):
                    raise ValueError("damage control did not change the selected compiled function for " + name)
                row["files"].append({"path": name, "entity_id": witness.origin.entity_id,
                                     "unit": witness.unit, "flags": list(effective(witness.unit, flags)),
                                     "graph_flags": list(flags), "result": Result.PASS})
            row.update(result=Result.PASS, reason="all TU replicas agree; each file independently changes its compiled witness")
            valid.append(flags)
        except (OSError, ValueError) as error:
            row["reason"] = str(error)
        controls.append(row)
    fresh()

    def compile_trial(item):
        nonlocal attempts
        name, contents, flags = item
        objects, errors = {}, {}
        for unit in units:
            try:
                unit_flags = effective(unit, flags)
                objects[unit], _, compiled = overlay_compile(unit, contents, unit_flags)
                attempts += int(compiled)
            except (OSError, ValueError) as error:
                errors[unit] = str(error)
        return score(objects, flags, name, errors, contents)

    work = [(name, contents, flags) for name, contents in candidates.items() for flags in valid]
    if jobs == 1:
        # Keep bounded-command signal cleanup on the main thread in the default
        # workflow. Wider pools require the coordinator's external job containment.
        trials = [compile_trial(item) for item in work]
    else:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            # ContextVars are not inherited by executor threads. Each task gets
            # its own copied context: sharing one context across simultaneous
            # trials would fail and could lose the explicit runtime binding.
            futures = [pool.submit(copy_context().run, compile_trial, item) for item in work]
            trials = [future.result() for future in futures]
    fresh()
    attempts += sum(row["support_compile_attempts"] for row in (*baselines, *trials))
    diagnostics = None
    if compiled_diagnostics:
        ordered = [*baselines, *trials]
        diagnostics = diagnostic_report([row.pop("_compiled_diagnostic") for row in ordered], len(ordered))
    report = {"task_id": task_name, "operation": "candidate_deck", "entity_id": target.origin.entity_id,
            "source": target.unit, "files": sorted(originals), "translation_units": units,
            "controls": controls, "baseline": baselines, "candidates": trials,
            "rankings": _rank_trials(baselines, trials),
            "status": "scored" if len(valid) == len(flags_list) and all(not item["compile_errors"] for item in trials) else "incomplete",
            "compile_attempts": attempts, "jobs": jobs, "input_fingerprint": digest(before),
            "unmeasured_task_entities": sorted(selected - {item.origin.entity_id for item in unit_bindings}),
            "unmeasured_product_units": sorted({item.unit for item in bindings if Path(item.unit).suffix in settings.unit_extensions} - set(units)),
            "refused_bindings": refused,
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "structure_verification": Result.UNKNOWN,
            "limitations": ["diagnostic source trials only; no winner applied and no claims stamped",
                            "task function symbol proposals are scored against candidate bindings; original identities, claims and other recovery interpretations require checkpoint decisions",
                            "advisory ranking per flag trial: compile errors, lost baseline passes (including measured neighbors), unknown and failed scored task/data results, missing distances, known differing bytes, then name; trials are never combined",
                            "ranking details cover the union of observed task code, shared bodies and bounded initial data in that flag trial; absent observations remain unknown, and generated tables/import selection, structure and uncompiled code are not ranked",
                            "all mapped task TUs compiled; neighboring rows also show unambiguous outside-task functions in those TUs, not all affected callers",
                            "layout, lifecycle, full build, protected obligations and review still require task checks",
                            "compiler commands are trusted; requested parallelism must fit isolated compiler capacity"]}
    if compiled_diagnostics:
        report["compiled_diagnostics"] = diagnostics
    return report

"""Task-scoped candidate decks: one map read, temporary compiles, no banking."""

from concurrent.futures import ThreadPoolExecutor
import hashlib
from pathlib import Path

from .application_binary import check_compiled, select_bindings
from .coff_replica import function_signature, object_signature
from .compiler import Compiler
from .components import local_path, relative_path
from .generated_source import read_generated
from .measurement import Measurement
from .model import Result, digest, require_digest, require_text
from .object_source import read_objects, referenced_object_units
from .ownership_source import project_view
from .pe_coff import Coff, PeImage
from .tasks import TaskStore


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
        require_text(row["before"], "candidate replacement anchor")
        before = anchors[name] = row["before"].encode("utf-8")
        if original.count(before) != 1:
            raise ValueError("candidate replacement anchor must occur exactly once")
        if not isinstance(row["control"], str):
            raise ValueError("candidate control must be replacement text")
        controls[name] = (row["entity_id"], original.replace(before, row["control"].encode("utf-8"), 1))
    candidates = value["candidates"]
    if not isinstance(candidates, dict) or not candidates:
        raise ValueError("candidate deck requires named replacements")
    replacements = {}
    for name, after in candidates.items():
        require_text(name, "candidate name")
        if not isinstance(after, dict) or set(after) != set(originals) or any(not isinstance(text, str) for text in after.values()):
            raise ValueError("each candidate must provide replacement text for every declared file")
        replacements[name] = {path: original.replace(anchors[path], after[path].encode("utf-8"), 1)
                              for path, original in originals.items()}
    return originals, controls, replacements


def measure_variants(root: Path, task_name: str, image_path: Path, deck: dict, *, jobs: int = 1, compiler=None):
    """Score atomic header/body experiments within authorized ownership work.

    The exact-text anchor deliberately replaces brace guessing. Only compiler
    invocations are parallel; project/source/binding inputs are prepared once.
    No candidate is selected, applied, recorded or admitted by this operation.
    """
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
    # Compile every mapped task TU together, not just the primary target. This
    # catches coupled header/body changes and includes out-of-scope neighbors in
    # those TUs. Uncompiled callers remain explicit, never an outside-scope pass.
    units = sorted({item.unit for item in bindings if item.origin.entity_id in selected
                    and Path(item.unit).suffix in settings.unit_extensions}
                   | {name for name in originals if Path(name).suffix in settings.unit_extensions})
    unit_bindings = [item for item in bindings if item.unit in units]
    witnesses = {}
    for name, (entity, _) in damage_controls.items():
        witness = next((item for item in unit_bindings if item.origin.entity_id == entity), None)
        if witness is None:
            raise ValueError("each edited file requires a mapped compiled task witness")
        witnesses[name] = witness
    data_bindings, generated = read_objects(root, pin), read_generated(root, pin)
    if ({item.symbol for item in data_bindings} & set(externals)
            or {item.symbol for item in generated} & (set(externals) | {item.symbol for item in data_bindings})):
        raise ValueError("conflicting function/object/generated symbol bindings")
    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    flags_list = settings.flag_sets
    controls, baselines, trials = [], [], []
    attempts = 0

    def fresh():
        if (settings.snapshot(root, metadata) != before
                or image_path.read_bytes() != image_data or store.read(task_name) != task or store.decisions(task_name) != decisions):
            raise ValueError("candidate source, image, compiler inputs or task changed; discard the deck")

    def score(objects, flags, name, errors=None, contents=None):
        errors = dict(errors or {})
        support_controls, support_attempts = [], 0
        while pending := referenced_object_units(data_bindings, objects) - objects.keys() - errors.keys():
            for unit in sorted(pending):
                control = {"unit": unit, "result": Result.UNKNOWN}
                try:
                    if Path(unit).suffix not in settings.unit_extensions:
                        raise ValueError("object owner is not a configured translation unit")
                    support_attempts += 1
                    original = Coff(compiler(root / unit, (), flags))
                    support_attempts += 1
                    replica = Coff(compiler.overlay(root / unit, originals, (), flags))
                    if object_signature(original) != object_signature(replica):
                        raise ValueError("unchanged-source replica differs from the original compilation: " + unit)
                    control["result"] = Result.PASS
                    if contents is None:
                        objects[unit] = replica
                    else:
                        support_attempts += 1
                        objects[unit] = Coff(compiler.overlay(root / unit, contents, (), flags))
                except (OSError, ValueError) as error:
                    errors[unit] = str(error)
                    control["reason"] = str(error)
                support_controls.append(control)
        measured = [item for item in bindings if item.unit in objects or item.unit in units]
        rows, data_rows, generated_rows = check_compiled(measured, data_bindings, generated, objects, image, externals, errors=errors)
        selected_row = next(row for row in rows if row["entity_id"] == target.origin.entity_id)
        return {"name": name, "flags": list(flags), "target": selected_row,
                "neighbors": [{**row, "in_task": row["entity_id"] in selected} for row in rows if row is not selected_row],
                "compile_errors": errors or {},
                "object_dependencies": support_controls, "support_compile_attempts": support_attempts,
                "object_definitions": data_rows, "generated_tables": generated_rows}

    # Replica and damage controls are per flag trial, not a one-time capability
    # claim. A matching score alone cannot show that the intended code changed.
    valid = []
    for flags in flags_list:
        row = {"flags": list(flags), "result": Result.UNKNOWN, "replicas": [], "files": []}
        try:
            fresh()
            baseline, replicas = {}, {}
            for unit in units:
                attempts += 1
                baseline[unit] = Coff(compiler(root / unit, (), flags))
                attempts += 1
                replicas[unit] = Coff(compiler.overlay(root / unit, originals, (), flags))
                if object_signature(baseline[unit]) != object_signature(replicas[unit]):
                    raise ValueError("unchanged-source replica differs from the original compilation: " + unit)
                row["replicas"].append({"unit": unit, "result": Result.PASS})
            baselines.append(score(baseline, flags, "baseline"))
            for name, (_, control) in damage_controls.items():
                witness = witnesses[name]
                attempts += 1
                damaged = Coff(compiler.overlay(root / witness.unit, {**originals, name: control}, (), flags))
                if function_signature(replicas[witness.unit], witness.symbol) == function_signature(damaged, witness.symbol):
                    raise ValueError("damage control did not change the selected compiled function for " + name)
                row["files"].append({"path": name, "entity_id": witness.origin.entity_id, "unit": witness.unit, "result": Result.PASS})
            row.update(result=Result.PASS, reason="all TU replicas agree; each file independently changes its compiled witness")
            valid.append(flags)
        except (OSError, ValueError) as error:
            row["reason"] = str(error)
        controls.append(row)
    fresh()

    def compile_trial(item):
        name, contents, flags = item
        objects, errors = {}, {}
        for unit in units:
            try:
                objects[unit] = Coff(compiler.overlay(root / unit, contents, (), flags))
            except (OSError, ValueError) as error:
                errors[unit] = str(error)
        return score(objects, flags, name, errors, contents)

    work = [(name, contents, flags) for name, contents in candidates.items() for flags in valid]
    attempts += len(work) * len(units)
    if jobs == 1:
        # Keep bounded-command signal cleanup on the main thread in the default
        # workflow. Wider pools require the coordinator's external job containment.
        trials = [compile_trial(item) for item in work]
    else:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            trials = list(pool.map(compile_trial, work))
    fresh()
    attempts += sum(row["support_compile_attempts"] for row in (*baselines, *trials))
    # Keep byte/boundary/reference dimensions intact, rather than collapse a
    # relocation failure into a deceptively good masked similarity percentage.
    def ranking_key(trial):
        rows = [trial["target"], *(row for row in trial["neighbors"] if row["in_task"])]
        result = (Result.UNKNOWN if any(row["result"] == Result.UNKNOWN for row in rows) else
                  Result.FAIL if any(row["result"] == Result.FAIL for row in rows) else Result.PASS)
        proofs = [span for row in rows for span in row.get("spans", [row])]
        differing = sum(len(proof["differing_offsets"]) for proof in proofs) if all("differing_offsets" in proof for proof in proofs) else float("inf")
        return (bool(trial["compile_errors"]), {Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 2}[result], differing, trial["name"])

    return {"task_id": task_name, "operation": "candidate_deck", "entity_id": target.origin.entity_id,
            "source": target.unit, "files": sorted(originals), "translation_units": units,
            "controls": controls, "baseline": baselines, "candidates": trials,
            "rankings": [{"flags": list(flags), "names": [trial["name"] for trial in sorted(
                (item for item in trials if item["flags"] == list(flags)), key=ranking_key)]} for flags in valid],
            "status": "scored" if len(valid) == len(flags_list) and all(not item["compile_errors"] for item in trials) else "incomplete",
            "compile_attempts": attempts, "jobs": jobs, "input_fingerprint": digest(before),
            "unmeasured_task_entities": sorted(selected - {item.origin.entity_id for item in unit_bindings}),
            "unmeasured_product_units": sorted({item.unit for item in bindings if Path(item.unit).suffix in settings.unit_extensions} - set(units)),
            "refused_bindings": refused,
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "structure_verification": Result.UNKNOWN,
            "limitations": ["diagnostic source trials only; no winner applied and no claims stamped",
                            "ranked per flag trial by compilation, measured task-wide strict result and differing bytes; missing distances sort last, trials are never combined",
                            "all mapped task TUs compiled; neighboring rows also show unambiguous outside-task functions in those TUs, not all affected callers",
                            "layout, lifecycle, full build, protected obligations and review still require task checks",
                            "compiler commands are trusted; requested parallelism must fit isolated compiler capacity"]}

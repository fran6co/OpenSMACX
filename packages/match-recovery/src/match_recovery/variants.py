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
from .object_source import read_objects
from .ownership_source import project_view
from .pe_coff import Coff, PeImage
from .tasks import TaskStore


def _deck(value, original):
    fields = {"schema_version", "source", "entity_id", "source_sha256", "before", "control", "candidates"}
    if not isinstance(value, dict) or set(value) != fields or type(value["schema_version"]) is not int or value["schema_version"] != 1:
        raise ValueError("invalid candidate deck schema")
    relative_path(value["source"])
    require_text(value["entity_id"], "candidate entity")
    require_digest(value["source_sha256"])
    if hashlib.sha256(original).hexdigest() != value["source_sha256"]:
        raise ValueError("candidate deck source digest is stale")
    require_text(value["before"], "candidate replacement anchor")
    before = value["before"].encode("utf-8")
    if original.count(before) != 1:
        raise ValueError("candidate replacement anchor must occur exactly once")
    if not isinstance(value["control"], str):
        raise ValueError("candidate control must be replacement text")
    candidates = value["candidates"]
    if not isinstance(candidates, dict) or not candidates:
        raise ValueError("candidate deck requires named replacements")
    for name, after in candidates.items():
        require_text(name, "candidate name")
        if not isinstance(after, str):
            raise ValueError("candidate replacements must be text")
    replace = lambda after: original.replace(before, after.encode("utf-8"), 1)
    return replace(value["control"]), {name: replace(after) for name, after in candidates.items()}


def measure_variants(root: Path, task_name: str, image_path: Path, deck: dict, *, jobs: int = 1, compiler=None):
    """Score whole-TU experiments within authorized ownership work.

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
    original = source.read_bytes()
    control, candidates = _deck(deck, original)
    view, snapshot = project_view(root)
    pin = snapshot["image_sha256"]
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va" for subject in task.subjects):
        raise ValueError("candidate task refers to another original image")
    image_data = image_path.read_bytes()
    image = PeImage(image_data, pin)
    bindings, refused, externals = select_bindings(view, selected)
    target = next((item for item in bindings if item.origin.entity_id == deck["entity_id"] and item.unit == deck["source"]), None)
    if target is None:
        raise ValueError("candidate target requires one unambiguous mapped product definition with image spans")
    unit_bindings = [item for item in bindings if item.unit == target.unit]
    data_bindings, generated = read_objects(root, pin), read_generated(root, pin)
    if ({item.symbol for item in data_bindings} & set(externals)
            or {item.symbol for item in generated} & (set(externals) | {item.symbol for item in data_bindings})):
        raise ValueError("conflicting function/object/generated symbol bindings")
    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    flags_list = settings.flag_sets
    controls, baselines, trials = [], [], []
    attempts = 0

    def fresh():
        if (settings.snapshot(root, metadata) != before or source.read_bytes() != original
                or image_path.read_bytes() != image_data or store.read(task_name) != task or store.decisions(task_name) != decisions):
            raise ValueError("candidate source, image, compiler inputs or task changed; discard the deck")

    def score(obj, flags, name):
        objects = {target.unit: obj}
        rows, data_rows, generated_rows = check_compiled(unit_bindings, data_bindings, generated, objects, image, externals)
        selected_row = next(row for row in rows if row["entity_id"] == target.origin.entity_id)
        return {"name": name, "flags": list(flags), "target": selected_row,
                "neighbors": [row for row in rows if row is not selected_row],
                "object_definitions": data_rows, "generated_tables": generated_rows}

    # Replica and damage controls are per flag trial, not a one-time capability
    # claim. A matching score alone cannot show that the intended code changed.
    valid = []
    for flags in flags_list:
        row = {"flags": list(flags), "result": Result.UNKNOWN}
        try:
            fresh()
            attempts += 1
            baseline = Coff(compiler(source, (), flags))
            baselines.append(score(baseline, flags, "baseline"))
            attempts += 1
            replica = Coff(compiler.variant(source, original, (), flags))
            if object_signature(baseline) != object_signature(replica):
                raise ValueError("unchanged-source replica differs from the original compilation")
            attempts += 1
            damaged = Coff(compiler.variant(source, control, (), flags))
            if function_signature(replica, target.symbol) == function_signature(damaged, target.symbol):
                raise ValueError("damage control did not change the selected compiled function")
            row.update(result=Result.PASS, reason="unchanged replica agrees; damage changes the selected function")
            valid.append(flags)
        except (OSError, ValueError) as error:
            row["reason"] = str(error)
        controls.append(row)
    fresh()

    def compile_trial(item):
        name, contents, flags = item
        try:
            obj = Coff(compiler.variant(source, contents, (), flags))
            return score(obj, flags, name)
        except (OSError, ValueError) as error:
            return {"name": name, "flags": list(flags), "target": {
                "entity_id": target.origin.entity_id, "result": Result.UNKNOWN, "reason": str(error)}}

    work = [(name, contents, flags) for name, contents in candidates.items() for flags in valid]
    attempts += len(work)
    if jobs == 1:
        # Keep bounded-command signal cleanup on the main thread in the default
        # workflow. Wider pools require the coordinator's external job containment.
        trials = [compile_trial(item) for item in work]
    else:
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            trials = list(pool.map(compile_trial, work))
    fresh()
    # Keep byte/boundary/reference dimensions intact, rather than collapse a
    # relocation failure into a deceptively good masked similarity percentage.
    def ranking_key(trial):
        row = trial["target"]
        proofs = row.get("spans", [row])
        differing = sum(len(proof["differing_offsets"]) for proof in proofs) if all("differing_offsets" in proof for proof in proofs) else float("inf")
        return ({Result.PASS: 0, Result.FAIL: 1, Result.UNKNOWN: 2}[row["result"]], differing, trial["name"])

    return {"task_id": task_name, "operation": "candidate_deck", "entity_id": target.origin.entity_id,
            "source": target.unit, "controls": controls, "baseline": baselines, "candidates": trials,
            "rankings": [{"flags": list(flags), "names": [trial["name"] for trial in sorted(
                (item for item in trials if item["flags"] == list(flags)), key=ranking_key)]} for flags in valid],
            "status": "scored" if len(valid) == len(flags_list) and all("neighbors" in item for item in trials) else "incomplete",
            "compile_attempts": attempts, "jobs": jobs, "input_fingerprint": digest(before),
            "unmeasured_task_entities": sorted(selected - {item.origin.entity_id for item in unit_bindings}),
            "refused_bindings": refused,
            "scope_complete": False, "ready_for_collection": False, "persisted": False,
            "structure_verification": Result.UNKNOWN,
            "limitations": ["diagnostic source trials only; no winner applied and no claims stamped",
                            "ranked per flag trial by strict result then known differing bytes; missing byte distances sort last, trials are never combined",
                            "only this translation unit compiled; neighboring rows cover mapped task subjects here, not all affected callers",
                            "layout, lifecycle, full build, protected obligations and review still require task checks",
                            "compiler commands are trusted; requested parallelism must fit isolated compiler capacity"]}

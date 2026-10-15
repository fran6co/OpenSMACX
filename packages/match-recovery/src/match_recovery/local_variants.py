"""Bounded, body-only one-TU spelling screens.

This is an exploratory masked-byte comparison. It does not publish source,
claims, symbols, references, or acceptance evidence.
"""

from dataclasses import asdict
import hashlib
from pathlib import Path
import re
import time

from .application_binary import select_bindings
from .coff_replica import function_signature, object_signature
from .compiler import Compiler
from .components import local_path
from .model import Result, digest
from .ownership_source import project_view
from .pe_coff import Coff, PeImage
from .source.asm import AsmComparison, compare_unit, shared_spans
from .source.mangled import qualified_name
from .source.model import Mode
from .source.reader import function_line, read_text
from .source_selection import snapshot_mappings, selected_source_records
from .source_inventory import image_pin
from .measurement import Measurement
from .tasks import TaskStore
from .variants import _deck, _trial_bindings


class _LeaseChanged(ValueError):
    """A measured input changed; abort without further compiler calls."""


_LINE_SPLICE = re.compile(r"\\[ \t\v\f]*(?:\r\n|\n|\r)")


def _reject_line_splicing(text: str) -> None:
    # Phase-two C/C++ splicing changes which physical lines are comments and
    # definitions. The source lexer does not implement that preprocessor step.
    if _LINE_SPLICE.search(text):
        raise ValueError("local body screening does not support backslash-newline splicing")


def _function_body_bytes(text: str, name: str, *, after_line: int | None = None,
                         before_line: int | None = None) -> tuple[int, int]:
    """Return exact byte bounds of one C++ definition's braces.

    The lexical walk ignores comments, quoted literals, and raw strings so a
    brace in prose or data cannot enlarge the authorized edit window.
    """
    line = function_line(text, name, after_line=after_line, before_line=before_line)
    line_start = sum(len(part) for part in text.splitlines(keepends=True)[:line - 1])
    definition_line = text.splitlines(keepends=True)[line - 1]
    if definition_line.count(name) != 1 or "{" in definition_line.split(name, 1)[0]:
        raise ValueError("target definition has an ambiguous same-line body boundary")
    region_end = (len(text) if before_line is None else
                  sum(len(part) for part in text.splitlines(keepends=True)[:before_line - 1]))
    state = "code"
    quote = ""
    raw_end = ""
    depth = 0
    declarator_parens = 0
    opening = None
    index = 0
    while index < region_end:
        char = text[index]
        following = text[index:index + 2]
        if state == "line":
            if char == "\n":
                state = "code"
            index += 1
            continue
        if state == "block":
            if following == "*/":
                state = "code"
                index += 2
            else:
                index += 1
            continue
        if state == "raw":
            if text.startswith(raw_end, index):
                index += len(raw_end)
                state, raw_end = "code", ""
            else:
                index += 1
            continue
        if state == "quoted":
            if char == "\\":
                index += 2
            elif char == quote:
                index += 1
                state, quote = "code", ""
            else:
                index += 1
            continue
        if following == "//":
            state, index = "line", index + 2
            continue
        if following == "/*":
            state, index = "block", index + 2
            continue
        if char in "\"'":
            state, quote, index = "quoted", char, index + 1
            continue
        if char == "R" and index + 1 < len(text) and text[index + 1] == '"':
            paren = text.find("(", index + 2)
            if paren >= 0 and paren - (index + 2) <= 16:
                delimiter = text[index + 2:paren]
                if not any(c.isspace() or c in "\\()" for c in delimiter):
                    raw_end = ")" + delimiter + '"'
                    state, index = "raw", paren + 1
                    continue
        if index >= line_start:
            if opening is None and char == "(":
                declarator_parens += 1
            elif opening is None and char == ")":
                declarator_parens -= 1
                if declarator_parens < 0:
                    raise ValueError("target definition has an ambiguous declarator")
            if char == "{":
                if opening is None:
                    if declarator_parens:
                        raise ValueError("target definition has a brace inside its signature")
                    opening, depth = index, 1
                else:
                    depth += 1
            elif char == "}" and opening is not None:
                depth -= 1
                if depth == 0:
                    return (len(text[:opening].encode("utf-8")),
                            len(text[:index + 1].encode("utf-8")))
        index += 1
    raise ValueError("target definition has no balanced body")


def _marker_identity(record) -> tuple:
    return (record.address, record.name, record.symbol, record.image_spans,
            record.mode, record.body)


def _bound_function_body_bytes(text: str, name: str, source: Path,
                               target_record, expected_markers: tuple) -> tuple[int, int]:
    """Find one real definition inside its unchanged source mapping interval."""
    _reject_line_splicing(text)
    markers = read_text(text, source)
    if tuple(_marker_identity(row) for row in markers) != expected_markers:
        raise ValueError("local candidate changed source mapping identities")
    selected = [index for index, row in enumerate(markers)
                if row.address == target_record.address]
    if len(selected) != 1:
        raise ValueError("target source mapping is absent or ambiguous")
    index, = selected
    marker = markers[index]
    if (marker.line != target_record.line or marker.mode not in (Mode.FILE, Mode.BODY)
            or marker.body or _marker_identity(marker) != _marker_identity(target_record)):
        raise ValueError("target definition is not in its exact product source mapping")
    before_line = (markers[index + 1].line if index + 1 < len(markers)
                   else len(text.splitlines()) + 1)
    return _function_body_bytes(text, name, after_line=marker.line,
                                before_line=before_line)


def _check_body_only(original_text: str, original_body: tuple[int, int], name: str,
                     replacement_sources, *, source: Path, target_record,
                     expected_markers: tuple) -> None:
    original = original_text.encode("utf-8")
    low, high = original_body
    prefix, suffix = original[:low], original[high:]
    for contents in replacement_sources:
        try:
            text = contents.decode("utf-8")
        except UnicodeDecodeError as error:
            raise ValueError("local candidates must preserve the selected function definition") from error
        _reject_line_splicing(text)
        try:
            body = _bound_function_body_bytes(text, name, source, target_record,
                                              expected_markers)
        except ValueError as error:
            if str(error) == "local candidate changed source mapping identities":
                raise ValueError("local candidates may change only the selected function body") from error
            raise ValueError("local candidates must preserve the selected function definition") from error
        changed = contents[:body[0]] + contents[body[1]:]
        if changed != prefix + suffix:
            raise ValueError("local candidates may change only the selected function body")


def _check_replacement_text(replacement_sets) -> None:
    for replacements in replacement_sets:
        items = (replacements,) if isinstance(replacements, str) else replacements
        if any(re.search(r"(?m)^\s*#", item) for item in items):
            raise ValueError("local candidates cannot add translation-unit preprocessor directives")


def _comparison_rows(result):
    rows = []
    for record, value in result.results:
        row = {"address": f"0x{record.address:08X}", "name": record.name,
               "symbol": record.symbol or record.name,
               "historical_byte_obligation": record.byte_exact,
               "historical_shape_obligation": record.semantic}
        if isinstance(value, str):
            row.update(result="unknown", verdict="UNRESOLVED", diagnostic=value)
        elif isinstance(value, AsmComparison):
            row.update(asdict(value))
            row["verdict"] = value.verdict.value
            row["result"] = ("pass" if value.verdict.value == "BYTE_EXACT" else
                             "unknown" if value.verdict.value in ("NO_COMPILE", "REFUSED", "SHARED_TAIL")
                             else "fail")
        else:
            raise ValueError("invalid local comparison observation")
        rows.append(row)
    return rows


def measure_local_variants(root: Path, task_name: str, image_path: Path, deck: dict,
                           *, compiler=None):
    """Screen body-only replacements in one authorized product TU.

    The deck uses the existing schema-v2 exact-byte anchors and per-file
    damage witness. It must name one source file and one task function; all
    selected task functions in that TU reuse each compiled candidate object.
    """
    started = time.monotonic()
    root = Path(root).resolve()
    settings, store = Measurement.read(root), TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    selected, editable = store._scope(task, decisions)
    if not isinstance(deck, dict) or not isinstance(deck.get("source"), str):
        raise ValueError("local candidate deck requires a source path")
    source_name = deck["source"]
    source = local_path(root, source_name)
    if (source_name not in editable or deck.get("entity_id") not in selected
            or source.suffix not in settings.unit_extensions
            or source.suffix in settings.source.interface_extensions
            or settings.source.is_seed(source_name)):
        raise ValueError("local screening requires an authorized product translation unit and task subject")
    if not isinstance(deck.get("files"), dict) or set(deck["files"]) != {source_name}:
        raise ValueError("local screening is limited to one edited translation unit")
    witness = deck["files"][source_name].get("entity_id") if isinstance(deck["files"][source_name], dict) else None
    if witness != deck["entity_id"]:
        raise ValueError("local damage control must witness the selected target")

    metadata = {"operation": "local_variant_screen", "task": task,
                "decisions": decisions, "deck": deck}
    before = settings.snapshot(root, metadata)
    originals, damage_controls, candidates = _deck(deck, root, editable, selected, settings)
    original = originals[source_name]
    try:
        original_text = original.decode("utf-8")
    except UnicodeDecodeError as error:
        raise ValueError("local screening requires UTF-8 product source") from error
    _reject_line_splicing(original_text)

    view, view_snapshot = project_view(root)
    pin = view_snapshot["image_sha256"]
    if image_pin(root, settings.source) != pin:
        raise ValueError("local task binding uses another pinned image")
    if any(subject.image_sha256 != pin or subject.address_space != settings.source.address_space
           for subject in task.subjects):
        raise ValueError("local task refers to another original image")
    image_path = (image_path if image_path.is_absolute() else root / image_path).resolve()
    image_data = image_path.read_bytes()
    image = PeImage(image_data, pin)

    all_entities = frozenset(entity["entity_id"] for group in view["groups"]
                             for entity in group["entities"])
    bindings, refused, _externals = select_bindings(view, all_entities)
    bindings_by_id = {binding.origin.entity_id: binding for binding in bindings}
    target = bindings_by_id.get(deck["entity_id"])
    if target is None or target.unit != source_name or not target.symbol:
        reason = next((row.get("reason") for row in refused
                       if row.get("entity_id") == deck["entity_id"]), "no exact native function binding")
        raise ValueError("local target needs one exact native product COFF symbol binding: " + reason)

    mapped = snapshot_mappings(root, view_snapshot)
    source_mappings = [row for row in mapped if row.path == source]
    all_source_records = selected_source_records(root, view_snapshot, source_mappings)
    records_by_key = {(record.address, record.line): record for record in all_source_records}
    task_bindings = [bindings_by_id[key] for key in sorted(selected & bindings_by_id.keys())
                     if bindings_by_id[key].unit == source_name]
    if not any(binding.origin.entity_id == target.origin.entity_id for binding in task_bindings):
        raise ValueError("selected local target was not retained in task translation-unit bindings")
    records = []
    for binding in task_bindings:
        record = records_by_key.get((binding.origin.address,
                                     next((row.line for row in source_mappings
                                           if row.address == binding.origin.address), -1)))
        if record is None:
            matches = [item for item in all_source_records if item.address == binding.origin.address]
            if len(matches) != 1:
                raise ValueError("task mapping has no unique current source record")
            record = matches[0]
        if record.function_fragment is not None:
            raise ValueError("local screening does not independently score fragment mappings")
        if (record.symbol or record.name) != binding.symbol:
            raise ValueError("source mapping and native function binding disagree")
        records.append(record)
    target_record = next(record for record in records
                         if record.address == target.origin.address)
    expected_markers = tuple(_marker_identity(row) for row in read_text(original_text, source))
    function_names = [qualified_name(target.symbol), target_record.name]
    body = None
    body_error = None
    body_name = None
    for name in dict.fromkeys(name for name in function_names if name):
        try:
            body = _bound_function_body_bytes(original_text, name, source,
                                              target_record, expected_markers)
            body_name = name
            break
        except ValueError as error:
            body_error = error
    if body is None:
        raise ValueError("cannot locate the uniquely bound target definition") from body_error

    replacement_sets = [deck["files"][source_name]["control"]]
    replacement_sets.extend(row[source_name] for row in deck["candidates"].values())
    for replacements in replacement_sets:
        items = [replacements] if isinstance(replacements, str) else replacements
        if not isinstance(items, list) or not items or any(not isinstance(item, str) for item in items):
            raise ValueError("local replacement must match its exact-text anchor list")
    _check_replacement_text(replacement_sets)
    replacement_sources = [damage_controls[source_name][1],
                            *(row[source_name] for row in candidates.values())]
    _check_body_only(original_text, body, body_name, replacement_sources,
                     source=source, target_record=target_record,
                     expected_markers=expected_markers)

    for path, (_entity, control) in damage_controls.items():
        rebound, _refused, _targets, changes = _trial_bindings(
            view, originals, {path: control}, selected, settings, pin, control=True)
        if changes:
            raise ValueError("local damage control cannot change emitted symbols")
    resolved_candidates = {}
    for name, contents in candidates.items():
        _rebound, _refused, _targets, changes = _trial_bindings(
            view, originals, contents, selected, settings, pin)
        if changes:
            raise ValueError("local body candidates cannot change emitted symbols")
        resolved_candidates[name] = contents[source_name]

    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    compiler_initial = compiler.snapshot() if hasattr(compiler, "snapshot") else None

    def fresh():
        if (settings.snapshot(root, metadata) != before or image_path.read_bytes() != image_data
                or store.read(task_name) != task or store.decisions(task_name) != decisions
                or (compiler_initial is not None and compiler.snapshot() != compiler_initial)):
            raise _LeaseChanged("local screening inputs changed during the compiler lease")

    attempts = 0
    flag_rows = []
    flags_grid = settings.flags_for(source_name)
    shared = shared_spans(mapped)
    for flags in flags_grid:
        flags = tuple(flags)
        row = {"flags": list(flags), "controls": {}, "baseline": [], "candidates": {}}
        baseline_raw = replica_raw = None
        baseline_error = replica_error = ""
        attempts += 1
        try:
            baseline_raw = bytes(compiler(source, (), flags))
            if not baseline_raw:
                raise ValueError("compiler returned no baseline object")
            baseline_obj = Coff(baseline_raw)
            baseline_obj.function(target.symbol)
            fresh()
        except (OSError, ValueError) as error:
            if isinstance(error, _LeaseChanged):
                raise
            baseline_error = str(error)
        if baseline_raw is not None:
            attempts += 1
            try:
                replica_raw = bytes(compiler.overlay(source, {source_name: original}, (), flags))
                fresh()
                if not replica_raw:
                    raise ValueError("overlay returned no unchanged replica")
                if object_signature(Coff(baseline_raw)) != object_signature(Coff(replica_raw)):
                    raise ValueError("physical unchanged-source replica differs from direct compile")
                Coff(replica_raw).function(target.symbol)
                row["controls"]["unchanged_replica"] = {"result": Result.PASS.value}
            except (OSError, ValueError) as error:
                if isinstance(error, _LeaseChanged):
                    raise
                replica_error = str(error)
                row["controls"]["unchanged_replica"] = {"result": Result.FAIL.value,
                                                            "reason": replica_error}
        else:
            row["controls"]["unchanged_replica"] = {"result": Result.UNKNOWN.value,
                                                        "reason": baseline_error}

        control_pass = False
        if replica_raw is not None and not replica_error:
            witness_binding = target
            attempts += 1
            try:
                _entity, damage_bytes = damage_controls[source_name]
                damaged_raw = bytes(compiler.overlay(source, {source_name: damage_bytes}, (), flags))
                fresh()
                if not damaged_raw:
                    raise ValueError("damage overlay returned no object")
                old_sig = function_signature(Coff(replica_raw), witness_binding.symbol)
                new_sig = function_signature(Coff(damaged_raw), witness_binding.symbol)
                if old_sig == new_sig:
                    raise ValueError("damage control did not physically change the selected COFF function")
                Coff(damaged_raw).function(witness_binding.symbol)
                control_pass = True
                row["controls"]["damage"] = {"result": Result.PASS.value,
                                                "entity_id": witness_binding.origin.entity_id,
                                                "symbol": witness_binding.symbol}
            except (OSError, ValueError) as error:
                if isinstance(error, _LeaseChanged):
                    raise
                row["controls"]["damage"] = {"result": Result.FAIL.value,
                                               "reason": str(error)}
        else:
            row["controls"]["damage"] = {"result": Result.UNKNOWN.value,
                                           "reason": "unchanged replica control unavailable"}

        def measure_raw(raw, error=""):
            if error:
                return {"results": [{"address": f"0x{record.address:08X}",
                                      "result": "unknown", "verdict": "NO_COMPILE",
                                      "diagnostic": error} for record in records],
                        "compile_attempts": 0}
            try:
                Coff(raw).function(target.symbol)
                measured = compare_unit(records, image_path, source, (), (flags,), shared,
                                        compiler=lambda *_args: raw)
                return {"results": _comparison_rows(measured),
                        "compile_attempts": 0, "reused_compiled_object": True}
            except (OSError, ValueError) as failure:
                return {"results": [{"address": f"0x{record.address:08X}",
                                      "result": "unknown", "verdict": "REFUSED",
                                      "diagnostic": str(failure)} for record in records],
                        "compile_attempts": 0}

        if not control_pass:
            for name in resolved_candidates:
                row["candidates"][name] = {"result": Result.UNKNOWN.value,
                                            "diagnostic": "per-flag physical controls did not pass",
                                            "measurements": []}
        else:
            for name, contents in resolved_candidates.items():
                attempts += 1
                try:
                    raw = bytes(compiler.overlay(source, {source_name: contents}, (), flags))
                    fresh()
                    measured = measure_raw(raw)
                    status = ("scored" if all(result["result"] != "unknown"
                                              for result in measured["results"])
                              else "incomplete")
                    row["candidates"][name] = {"result": status,
                                                "measurements": measured["results"],
                                                "reused_compiled_object": True}
                except (OSError, ValueError) as error:
                    if isinstance(error, _LeaseChanged):
                        raise
                    row["candidates"][name] = {"result": Result.UNKNOWN.value,
                                                "diagnostic": str(error), "measurements": []}
        row["baseline"] = measure_raw(baseline_raw, baseline_error)["results"]
        flag_rows.append(row)
        fresh()

    all_scored = bool(flag_rows) and all(
        row["controls"]["unchanged_replica"]["result"] == Result.PASS.value
        and row["controls"]["damage"]["result"] == Result.PASS.value
        and all(candidate["result"] == "scored" for candidate in row["candidates"].values())
        for row in flag_rows)
    measured_ids = {record.address for record in records}
    return {"schema_version": 1, "operation": "local_variant_screen",
            "task_id": task_name, "entity_id": target.origin.entity_id,
            "source": source_name, "symbol": target.symbol,
            "translation_units": [source_name],
            "flag_trials": flag_rows,
            "compile_attempts": attempts,
            "elapsed_seconds": round(time.monotonic() - started, 6),
            "status": "scored" if all_scored else "incomplete",
            "strict_reference_result": Result.UNKNOWN.value,
            "strict_references": "unknown",
            "unmeasured_task_entities": sorted(selected - {
                bindings_by_id[key].origin.entity_id for key in selected & bindings_by_id.keys()
                if bindings_by_id[key].unit == source_name and bindings_by_id[key].origin.address in measured_ids}),
            "scope_complete": False, "ready_for_collection": False,
            "persisted": False, "acceptance": "not_evaluated",
            "limitations": ["exploratory relocation-masked byte comparison only",
                            "strict reference targets, structure, ownership, lifecycle, preservation, full build and runtime remain unknown",
                            "one authorized product TU and selected task functions in that TU only",
                            "no source is written and no claims, symbols, or collection decisions are published"]}


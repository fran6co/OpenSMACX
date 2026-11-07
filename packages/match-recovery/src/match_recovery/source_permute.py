"""Seeded source-spelling search for one near-miss function body.

The search recompiles only the target's configured TU, through the same
compiler interaction and per-TU comparer that ``measure`` uses, with the
candidate host text supplied as a compiler overlay. The real source file is
never written. ``exact`` is the unchanged masked-byte comparer's BYTE_EXACT
verdict; the finer distance only orders non-exact candidates and never admits
one. Nothing is recorded, banked or published.
"""

from __future__ import annotations

import base64
import difflib
import json
from hashlib import file_digest
from pathlib import Path
import time

from .coff_replica import object_signature
from .compiler import Compiler
from .measurement import Measurement
from .pe_coff import Coff
from .permute import Score, Unsupported, search, tokenize
from .source import Mode, State
from .source.asm import (AsmComparison, Listing, RegionCandidate, _data_divergence, _split_at,
                         compare_unit, original_asm, shared_spans, subject_asm)
from .source.body import resolve_body
from .source.mangled import qualified_name
from .source.model import Tier
from .source.reader import read_text
from .source_inventory import image_pin, source_snapshot
from .source_selection import (compilation_context, measurement_reference_targets, select_records,
                               selected_source_records, snapshot_mappings)

_SCALE = 1000          # one differing instruction outweighs any byte tie-break
_PROTECTED = ("ORIGINAL:", "BYTE_EXACT")


# ---------------------------------------------------------------- distance

def _byte_difference(original: Listing, compiled: Listing, left: int, right: int) -> int:
    """Differing bytes of one aligned instruction pair, relocations masked on either side."""
    a, b = original.instructions[left], compiled.instructions[right]
    if a.size != b.size:
        return max(a.size, b.size)
    a_offset, b_offset = a.address - original.base, b.address - compiled.base
    return sum(1 for step in range(a.size)
               if a_offset + step not in original.mask and b_offset + step not in compiled.mask
               and a.bytes[step] != b.bytes[step])


def listing_distance(original: Listing, compiled: Listing) -> tuple[int, int]:
    """(differing instructions, differing bytes) between two listings; (0, 0) iff they agree.

    Instructions are aligned on mnemonic+register/addressing shape, so a
    register swap or reordering counts, then aligned pairs are compared byte
    for byte with relocated bytes masked exactly as ``compare_asm`` masks them.
    An unmatched instruction counts once on each side it appears.
    """
    compiled = _split_at(compiled, original.instruction_bytes)
    left, right = original.instructions, compiled.instructions
    matcher = difflib.SequenceMatcher(None, original.shapes, compiled.shapes, autojunk=False)
    matched = differing = 0
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag == "equal":
            for x, y in zip(range(i1, i2), range(j1, j2)):
                count = _byte_difference(original, compiled, x, y)
                matched += count == 0
                differing += count
        else:
            differing += max(sum(item.size for item in left[i1:i2]),
                             sum(item.size for item in right[j1:j2]))
    instructions = len(left) + len(right) - 2 * matched
    if _data_divergence(original, compiled) is not None:
        instructions += 1
        differing += max(1, abs(len(original.data) - len(compiled.data)))
    return instructions, differing


def comparison_distance(comparison: AsmComparison) -> tuple[int, int]:
    """Coarser fallback for EH region pairs: positional instruction disagreement."""
    return (comparison.original_instructions + comparison.compiled_instructions
            - 2 * comparison.matching_instructions, 0)


# ----------------------------------------------------------------- source

def _body_bounds(text: str, first_line: int, end_line: int) -> tuple[int, int, str]:
    """(open brace, close brace, parameter text) of the definition in a line range."""
    lines = text.splitlines(keepends=True)
    low = sum(len(line) for line in lines[:first_line - 1])
    high = sum(len(line) for line in lines[:end_line])
    tokens = [t for t in tokenize(text[low:high]) if t.kind not in ("ws", "comment")]
    depth, opening, parameters = 0, None, None
    for index, token in enumerate(tokens):
        if opening is None:
            if token.text == "(":
                if depth == 0 and parameters is None:
                    parameters = [index, None]
                depth += 1
            elif token.text == ")":
                depth -= 1
                if depth == 0 and parameters is not None and parameters[1] is None:
                    parameters[1] = index
            elif token.text == "{" and depth == 0:
                opening, depth = index, 1
            continue
        if token.text == "{":
            depth += 1
        elif token.text == "}":
            depth -= 1
            if depth == 0:
                params = ""
                if parameters is not None and parameters[1] is not None:
                    params = text[low + tokens[parameters[0]].end:low + tokens[parameters[1]].start]
                return low + tokens[opening].start, low + token.start, params
    raise ValueError("target definition has no balanced body in its resolved line range")


def _locate_body(record, relative: str, texts: dict, symbol_format: str, root: Path):
    """(host path, open brace offset, close brace offset, parameters)."""
    resolution = resolve_body(record, marker_path=relative, source_texts=texts, symbol_format=symbol_format)
    if resolution.status == "candidate":
        host = resolution.path
        return (host, *_body_bounds(texts[host], resolution.line, resolution.end_line))
    # Overloads and FILE mappings: the marker-bound lexical walk used by local screening.
    from .local_variants import _bound_function_body_bytes, _marker_identity
    text = texts[relative]
    markers = tuple(_marker_identity(row) for row in read_text(text, record.path))
    names = [qualified_name(record.symbol or ""), record.name]
    for name in dict.fromkeys(name for name in names if name):
        try:
            low, high = _bound_function_body_bytes(text, name, record.path, record, markers)
        except ValueError:
            continue
        encoded = text.encode("utf-8")
        low, high = len(encoded[:low].decode("utf-8")), len(encoded[:high].decode("utf-8"))
        prefix = text[:low]
        close = prefix.rfind(")")
        opening = prefix.rfind("(", 0, close) if close >= 0 else -1
        return relative, low, high - 1, prefix[opening + 1:close] if opening >= 0 else ""
    raise ValueError("cannot locate the target body: " + resolution.reason)


# ------------------------------------------------------------------ driver

def _ignored_destination(root: Path, path: Path, settings) -> Path:
    path = (path if path.is_absolute() else root / path).absolute()
    for name in settings.source.roots:
        if path.resolve().is_relative_to(settings.source.path(root, name).resolve()):
            raise ValueError("--keep-best must not point into a source root; the real source is never edited")
    if path.is_symlink() or path.is_dir():
        raise ValueError("--keep-best must name a regular file path")
    return path


def permute_source(root: Path, target: str, *, image: Path, in_file=None, iterations=200, seed=0,
                   time_limit=None, keep_best=None, operators=None, progress=None, compiler=None):
    root = Path(root).resolve()
    settings = Measurement.read(root)
    image = (image if image.is_absolute() else root / image).resolve()
    with image.open("rb") as stream:
        if file_digest(stream, "sha256").hexdigest() != image_pin(root, settings.source):
            raise ValueError("permute requires the configured pinned original image")
    metadata = {"operation": "permute", "target": target}
    before = settings.snapshot(root, metadata)
    snapshot = source_snapshot(root, allow_empty=True)
    mappings = snapshot_mappings(root, snapshot)
    host_filter = settings.source.path(root, in_file) if in_file else None
    selected = select_records(mappings, [target], host_filter, settings.source.symbol_format)
    if len(selected) != 1:
        raise ValueError("permute requires exactly one unambiguous source mapping; use --in")
    record, = selected_source_records(root, snapshot, selected)
    if record.mode is Mode.UNBOUND or record.state is State.PLACEHOLDER or record.function_fragment:
        raise ValueError("permute requires a complete implemented source-owned function")
    unit = compilation_context(root, record, settings)
    unit_relative = unit.relative_to(root).as_posix()
    if unit.suffix not in settings.unit_extensions:
        raise ValueError("mapping host is not a configured translation unit")
    grid = tuple(tuple(flags) for flags in settings.flags_for(unit_relative))
    texts = {name: base64.b64decode(info["content_base64"]).decode("utf-8", errors="replace")
             for name, info in snapshot["files"].items()}
    relative = record.path.relative_to(root).as_posix()
    host, opening, closing, parameters = _locate_body(record, relative, texts,
                                                       settings.source.symbol_format, root)
    host_path = settings.source.path(root, host)
    host_text = host_path.read_text(encoding="utf-8")
    if host_text != texts[host]:
        raise ValueError("source host changed after the inventory snapshot")
    prefix, original_body, suffix = host_text[:opening + 1], host_text[opening + 1:closing], host_text[closing:]
    body_first = host_text.count("\n", 0, opening) + 1
    body_last = host_text.count("\n", 0, closing) + 1
    if any(body_first <= row.line <= body_last for row in read_text(host_text, host_path)):
        raise ValueError("target body contains source mapping markers; refusing to mutate it")
    if any(word in original_body for word in _PROTECTED):
        raise ValueError("target body contains ORIGINAL/BYTE_EXACT text; refusing to mutate it")
    try:
        tokenize(original_body)
    except Unsupported as error:
        raise ValueError(f"target body is outside the supported token grammar: {error}") from error
    destination = _ignored_destination(
        root, Path(keep_best) if keep_best else Path("build/recovery/permute") / record.address_hex / host_path.name,
        settings)

    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    shared = shared_spans(mappings)
    references = measurement_reference_targets(root, settings, snapshot)
    timings = {"compile_seconds": 0.0}
    failures = []
    originals = {}

    def compile_text(text: str, flags):
        started = time.monotonic()
        try:
            return bytes(compiler.overlay(unit, {host: text.encode("utf-8")}, (), flags))
        finally:
            timings["compile_seconds"] += time.monotonic() - started

    # Control: the overlay route must reproduce the direct compile before any score means anything.
    direct = bytes(compiler(unit, (), grid[0]))
    replica = compile_text(host_text, grid[0])
    if object_signature(Coff(direct)) != object_signature(Coff(replica)):
        raise ValueError("unchanged overlay replica differs physically from the direct compile")

    def splice(body_text: str) -> str:
        if any(body_text.count(word) != original_body.count(word) for word in _PROTECTED):
            raise ValueError("a mutation touched ORIGINAL/BYTE_EXACT text")
        return prefix + body_text + suffix

    def score(body_text: str):
        text = splice(body_text)
        raws = {}
        for flags in grid:
            try:
                raws[flags] = compile_text(text, flags)
            except ValueError as error:
                if not str(error).startswith("compiler command failed"):
                    raise
                failures.append(str(error)[:300])
                return None
        measured = compare_unit([record], image, unit, (), grid, shared,
                                compiler=lambda _path, _arguments, trial: raws[tuple(trial)],
                                root=root, targets=references)
        comparison = measured.results[0][1]
        if isinstance(comparison, str):
            failures.append(comparison[:300])
            return None
        exact = comparison.verdict is Tier.BYTE_EXACT
        best = None
        for flags in grid:
            try:
                listing = subject_asm(raws[flags], record, " ".join(flags))
            except ValueError:
                continue
            if isinstance(listing, RegionCandidate):
                continue
            original = originals.setdefault(None, original_asm(record, image))
            if len(listing.code) > len(original.code):
                try:
                    if len(listing.code) not in originals:
                        originals[len(listing.code)] = original_asm(record, image, extent=len(listing.code))
                    original = originals[len(listing.code)]
                except ValueError:
                    pass
            value = listing_distance(original, listing)
            best = value if best is None or value < best else best
        instructions, differing = best if best is not None else comparison_distance(comparison)
        distance = 0 if exact else max(1, instructions * _SCALE + min(differing, _SCALE - 1))
        return Score(distance, exact, {
            "verdict": comparison.verdict.value, "instruction_distance": instructions,
            "byte_distance": differing, "matching_instructions": comparison.matching_instructions,
            "original_instructions": comparison.original_instructions,
            "compiled_instructions": comparison.compiled_instructions,
            "first_divergence": comparison.first_divergence,
            "allocation_only": comparison.allocation_only})

    def unified(body_text: str) -> str:
        return "".join(difflib.unified_diff(
            host_text.splitlines(keepends=True), splice(body_text).splitlines(keepends=True),
            fromfile="a/" + host, tofile="b/" + host))

    outputs = {"source": str(destination), "diff": str(destination) + ".diff"}

    def keep(body_text: str, value: Score):
        destination.parent.mkdir(parents=True, exist_ok=True)
        for path, content in ((destination, splice(body_text)),
                              (Path(outputs["diff"]), unified(body_text))):
            temporary = path.with_name(path.name + ".tmp")
            temporary.write_text(content, encoding="utf-8")
            temporary.replace(path)

    def trial(iteration, value, applied):
        if progress is not None:
            progress({"iteration": iteration, "distance": None if value is None else value.distance,
                      "exact": bool(value and value.exact),
                      "mutations": [item.description for item in applied]})

    started = time.monotonic()
    result = search(original_body, score, iterations=iterations, seed=seed, time_limit=time_limit,
                    parameters=parameters, operators=operators, scale=_SCALE,
                    on_improve=keep, on_trial=trial)
    elapsed = time.monotonic() - started
    keep(result.best_text, result.best)
    # A changed input makes the scores stale; keep the variant for review but never call it exact.
    stale = (settings.snapshot(root, metadata) != before
             or host_path.read_text(encoding="utf-8") != host_text)
    compiles = result.compiles * len(grid)
    outputs["summary"] = str(destination) + ".json"
    summary = {
        "schema_version": 1, "operation": "permute",
        "result": "unknown" if stale else "pass" if result.best.exact else "fail",
        "inputs_changed": stale,
        "address": record.address_hex, "name": record.name, "symbol": record.symbol or record.name,
        "source": host, "translation_unit": unit_relative, "flag_sets": [list(flags) for flags in grid],
        "seed": seed, "iterations": result.iterations, "iteration_limit": iterations,
        "time_limit": time_limit, "stop_reason": result.stop_reason, "interrupted": result.interrupted,
        "exact": result.best.exact,
        "start": {"distance": result.start.distance, **result.start.detail},
        "best": {"distance": result.best.distance, **result.best.detail},
        "improved": result.best.distance < result.start.distance,
        "compiles": compiles + 2, "search_compiles": compiles, "control_compiles": 2,
        "compile_failures": result.compile_failures, "cache_hits": result.cache_hits,
        "minimize_compiles": result.minimize_compiles * len(grid), "reverted_hunks": result.reverted_hunks,
        "accepted_moves": result.accepted, "elapsed_seconds": round(elapsed, 3),
        "compile_seconds": round(timings["compile_seconds"], 3),
        "compiles_per_minute": round(60 * compiles / elapsed, 2) if elapsed > 0 else None,
        "improvements": result.improvements, "operator_stats": result.operator_stats,
        "sample_compile_failures": failures[:3], "outputs": outputs,
        "persisted": False, "ready_for_collection": False,
        "limitations": [
            "exact is the unchanged relocation-masked BYTE_EXACT comparer verdict; the distance only orders misses",
            "the real source was not edited; review the variant, apply it by hand, then measure and record",
            "mutations are spelling transforms, not equivalence proofs; the byte match is the behavioral test",
            "no claims are banked; strict references, structure and linked preservation are not evaluated"]}
    Path(outputs["summary"]).write_text(json.dumps(summary, indent=2, sort_keys=True))
    return summary

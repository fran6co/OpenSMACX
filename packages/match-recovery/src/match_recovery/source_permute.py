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
from .permute import (PROBE_FORMS, PROBE_SIZES, Score, Unsupported, probe_calls, probe_helpers,
                      probe_variants, search, tokenize)
from .source import Mode, State
from .source.asm import (AsmComparison, Listing, RegionCandidate, _data_divergence, _image_bytes, _split_at,
                         compare_unit, eh_region_listings, original_asm, shared_spans, subject_asm)
from .source.eh_regions import bind_eh_regions
from .source.body import resolve_body
from .source.mangled import qualified_name
from .source.model import Tier
from .source.reader import read_text
from .source_inventory import image_pin, source_snapshot
from .source_selection import (compilation_context, measurement_reference_targets, select_records,
                               selected_source_records, snapshot_mappings)

_SCALE = 1000          # one differing instruction outweighs any byte tie-break
_TOPOLOGY = 1000       # one differing EH unwind state outweighs any instruction distance
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


def eh_distance(candidate: RegionCandidate, record, image) -> dict | None:
    """EH unwind topology and per-region instruction distance; None when the regions do not bind.

    The topology distance counts unmatched predecessor states on either side
    (aligned like a sequence diff) plus any cleanup-action count difference,
    so an extra or missing teardown state costs one. Region instruction
    distances are summed with ``listing_distance`` even when the topology
    differs, so a near miss still has a gradient. Ordering only, never a verdict.
    """
    from .pe_coff import PeImage
    import hashlib
    raw = _image_bytes(Path(image))
    view = PeImage(raw, hashlib.sha256(raw).hexdigest())
    try:
        bound = bind_eh_regions(candidate.object_bytes, candidate.emitted_symbol, view, record.image_spans)
        pairs = eh_region_listings(candidate, bound, view)
    except ValueError:
        return None
    ownership = bound[0].ownership
    original, compiled = ownership.original_predecessors, ownership.compiled_predecessors
    matcher = difflib.SequenceMatcher(None, original, compiled, autojunk=False)
    matched = sum(block.size for block in matcher.get_matching_blocks())
    topology = (len(original) + len(compiled) - 2 * matched
                + abs(len(ownership.original_actions) - len(ownership.compiled_actions)))
    instructions = differing = 0
    for left, right in pairs:
        count, size = listing_distance(left, right)
        instructions += count
        differing += size
    return {"eh_topology_distance": topology, "original_eh_states": len(original),
            "compiled_eh_states": len(compiled), "instruction_distance": instructions,
            "byte_distance": differing}


# ------------------------------------------------------- parallel compiles

_SLOT = {}


def _slot_initialize(root: str, profile: str, environment: dict, binding) -> None:
    """Executor process bound to exactly one compiler slot for its whole life."""
    from .framework_runtime import initialize_worker
    import os
    initialize_worker(binding)
    os.environ.update(environment)
    _SLOT.update(root=root, profile=profile)


def _slot_compiler() -> Compiler:
    if "compiler" not in _SLOT:
        _SLOT["compiler"] = Compiler(Path(_SLOT["root"]), profile=_SLOT["profile"])
    return _SLOT["compiler"]


def _slot_compile(unit: str, host: str | None, data: bytes | None, grid) -> list:
    """Compile one overlay text for each flag set; ``("failed", diagnostic)`` ends the grid."""
    _SLOT_COMPILER = _slot_compiler()
    results = []
    for flags in grid:
        try:
            if host is None:
                results.append(("object", bytes(_SLOT_COMPILER(Path(unit), (), tuple(flags)))))
            else:
                results.append(("object", bytes(_SLOT_COMPILER.overlay(Path(unit), {host: data}, (),
                                                                       tuple(flags)))))
        except ValueError as error:
            if not str(error).startswith("compiler command failed"):
                raise
            results.append(("failed", str(error)[:300]))
            break
    return results


class SlotPool:
    """One single-process executor per compiler slot; jobs go to whichever slot is free.

    Results come back in submission order, so a caller's trajectory never
    depends on which slot finished first. Every overlay compile already runs
    in the compiler adapter's private temporary tree; the slot only separates
    the compiler installation/prefix state between concurrent compiles.
    """

    def __init__(self, root: Path, profile: str, slots):
        from concurrent.futures import ProcessPoolExecutor
        import multiprocessing
        import os
        from .framework_runtime import export_binding
        self.names = [name for name, _environment in slots]
        binding = export_binding()
        # Workers start lazily from the dispatching threads: never fork a threaded parent.
        context = multiprocessing.get_context("spawn" if os.name == "nt" else "forkserver")
        self.executors = [ProcessPoolExecutor(max_workers=1, mp_context=context, initializer=_slot_initialize,
                                              initargs=(str(root), profile, dict(environment), binding))
                          for _name, environment in slots]
        self.busy = [0.0] * len(self.executors)

    def __len__(self):
        return len(self.executors)

    def run(self, function, jobs, *, slot: int | None = None) -> list:
        """``function(*job)`` for each job; with ``slot`` every job runs there."""
        if slot is not None:
            return [self.executors[slot].submit(function, *job).result() for job in jobs]
        import queue
        import threading
        work = queue.SimpleQueue()
        for index, job in enumerate(jobs):
            work.put((index, job))
        results, errors = [None] * len(jobs), []

        def drain(number):
            while not errors:
                try:
                    index, job = work.get_nowait()
                except queue.Empty:
                    return
                started = time.monotonic()
                try:
                    results[index] = self.executors[number].submit(function, *job).result()
                except BaseException as error:
                    errors.append(error)
                    return
                finally:
                    self.busy[number] += time.monotonic() - started
        threads = [threading.Thread(target=drain, args=(number,), daemon=True)
                   for number in range(len(self.executors))]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        if errors:
            raise errors[0]
        return results

    def close(self, *, cancel: bool = False):
        for executor in self.executors:
            executor.shutdown(wait=not cancel, cancel_futures=cancel)


def _checked_slots(root: Path, slots) -> list:
    if not slots:
        return []
    slots = [(name, dict(environment)) for name, environment in slots]
    declared = read_json_compiler_environment(root)
    keys = None
    for name, environment in slots:
        if not environment or any(type(key) is not str or type(value) is not str
                                  for key, value in environment.items()):
            raise ValueError("compiler slot environment requires string variables")
        variables = set(environment) - {"MATCH_RECOVERY_RESOURCE_LEASE"}
        if not variables <= declared:
            raise ValueError("compiler slot variables must be declared compiler environment inputs")
        if keys is not None and keys != variables:
            raise ValueError("all compiler slots must set the same environment variables")
        keys = variables
    if len({tuple(sorted(environment.items())) for _name, environment in slots}) != len(slots):
        raise ValueError("concurrent compiler slots must have distinct environments")
    if len({name for name, _environment in slots}) != len(slots):
        raise ValueError("compiler slot names must be distinct")
    return slots


def read_json_compiler_environment(root: Path) -> set:
    from .records import read_json
    compiler = read_json(root / "recovery/project.json").get("compiler", {})
    declared = compiler.get("environment") if isinstance(compiler, dict) else None
    if not isinstance(declared, list):
        raise ValueError("project compiler configuration declares no environment inputs")
    return {"PATH", *declared}


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
    # The mapping row's name is usually its mangled symbol; the lexical walk
    # needs the qualified C++ name. The walk itself stays bound to this
    # record's ORIGINAL address and marker interval, so overloads resolve.
    names = [qualified_name(record.symbol or ""), qualified_name(record.name or ""), record.name]
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

def _trial_object(unit: Path, raws: dict, dependencies: dict, compile_dependency, path, arguments, trial) -> bytes:
    """The trial's own TU object, or an unchanged dependency TU compiled once.

    Strict associated-data verification also compiles the selected evidence
    closure: other translation units under their own recipes. Those files are
    not mutated, so their real objects are compiled once and reused; the trial
    overlay never stands in for another unit or another flag set.
    """
    trial = tuple(trial)
    if Path(path).resolve() == Path(unit).resolve():
        if tuple(arguments) or trial not in raws:
            raise ValueError("compiler command failed: permute trial grid lacks "
                             f"the requested target flags {' '.join(trial)}")
        return raws[trial]
    key = (Path(path).resolve(), tuple(arguments), trial)
    if key not in dependencies:
        dependencies[key] = compile_dependency(*key)
    return dependencies[key]


def _ignored_destination(root: Path, path: Path, settings) -> Path:
    path = (path if path.is_absolute() else root / path).absolute()
    for name in settings.source.roots:
        if path.resolve().is_relative_to(settings.source.path(root, name).resolve()):
            raise ValueError("--keep-best must not point into a source root; the real source is never edited")
    if path.is_symlink() or path.is_dir():
        raise ValueError("--keep-best must name a regular file path")
    return path


def _helper_offset(prefix: str) -> int:
    """A file-scope line start before the target definition and its comments/markers.

    Just after the last top-level delimiter (``;``, ``}``, ``{`` or a
    preprocessor line) preceding the definition, at the next line start that
    is not inside a comment, so marker comments stay attached to their
    definition and nothing lands inside a block comment.
    """
    tokens = tokenize(prefix)
    significant = [token for token in tokens if token.kind not in ("ws", "comment")]
    # Drop the definition head (everything after the last delimiter).
    offset = 0
    for token in reversed(significant[:-1] if significant and significant[-1].text == "{" else significant):
        if token.kind == "pp" or token.text in (";", "}", "{"):
            offset = token.end
            break
    if offset:
        newline = prefix.find("\n", offset)
        offset = len(prefix) if newline < 0 else newline + 1
    for token in tokens:
        if token.kind == "comment" and token.start < offset < token.end:
            newline = prefix.find("\n", token.end)
            offset = len(prefix) if newline < 0 else newline + 1
    return offset


def permute_source(root: Path, target: str, *, image: Path, in_file=None, iterations=200, seed=0,
                   time_limit=None, keep_best=None, operators=None, progress=None, compiler=None,
                   slots=None, probe_sweep=False, probe_sizes=None, probe_forms=None):
    """Search body spellings of one mapped function; ``slots`` compiles trials concurrently.

    ``slots`` is ``[(name, environment)]``, one distinct provisioned compiler
    slot per concurrent compile (leased by the caller or explicitly supplied).
    ``probe_sweep`` replaces the random search by an exhaustive ranking of
    single dead-budget-probe placements (marked stand-ins, never recovery).
    """
    root = Path(root).resolve()
    settings = Measurement.read(root)
    image = (image if image.is_absolute() else root / image).resolve()
    with image.open("rb") as stream:
        if file_digest(stream, "sha256").hexdigest() != image_pin(root, settings.source):
            raise ValueError("permute requires the configured pinned original image")
    slots = _checked_slots(root, slots)
    if compiler is not None and slots:
        raise ValueError("an injected compiler and compiler slots are alternatives")
    probe_sizes = tuple(probe_sizes or PROBE_SIZES)
    probe_forms = tuple(probe_forms or PROBE_FORMS)
    if any(type(size) is not int or size < 1 for size in probe_sizes) or set(probe_forms) - set(PROBE_FORMS):
        raise ValueError("probe sizes must be positive integers and forms one of " + ", ".join(PROBE_FORMS))
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
    if probe_calls(original_body):
        raise ValueError("target body already contains permute budget probes; remove them first")
    try:
        tokenize(original_body)
    except Unsupported as error:
        raise ValueError(f"target body is outside the supported token grammar: {error}") from error
    helper_at = []
    destination = _ignored_destination(
        root, Path(keep_best) if keep_best else Path("build/recovery/permute") / record.address_hex / host_path.name,
        settings)

    pool = SlotPool(root, settings.compiler_profile, slots) if slots else None
    if pool is None:
        compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    shared = shared_spans(mappings)
    references = measurement_reference_targets(root, settings, snapshot)
    timings = {"compile_seconds": 0.0}
    failures = []
    originals = {}

    def compile_grid(text: str | None, flag_sets=grid) -> list:
        """[("object", bytes) | ("failed", diagnostic)] per flag set; None text compiles the real file."""
        if pool is not None:
            return compile_texts([text])[0]
        started = time.monotonic()
        results = []
        try:
            for flags in flag_sets:
                try:
                    results.append(("object", bytes(compiler(unit, (), flags) if text is None else
                                                     compiler.overlay(unit, {host: text.encode("utf-8")}, (), flags))))
                except ValueError as error:
                    if not str(error).startswith("compiler command failed"):
                        raise
                    results.append(("failed", str(error)[:300]))
                    break
            return results
        finally:
            timings["compile_seconds"] += time.monotonic() - started

    def compile_texts(text_list) -> list:
        if pool is None:
            return [compile_grid(text) for text in text_list]
        started = time.monotonic()
        try:
            return pool.run(_slot_compile, [(str(unit), None if text is None else host,
                                             None if text is None else text.encode("utf-8"), grid)
                                            for text in text_list])
        finally:
            timings["compile_seconds"] += time.monotonic() - started

    def splice(body_text: str) -> str:
        if any(body_text.count(word) != original_body.count(word) for word in _PROTECTED):
            raise ValueError("a mutation touched ORIGINAL/BYTE_EXACT text")
        helpers = probe_helpers(body_text)
        if not helpers:
            return prefix + body_text + suffix
        if not helper_at:
            try:
                helper_at.append(_helper_offset(prefix))
            except Unsupported as error:
                raise ValueError(f"cannot place budget probe helpers before the target: {error}") from error
        at = helper_at[0]
        return prefix[:at] + helpers + "\n" + prefix[at:] + body_text + suffix

    dependencies = {}

    def compile_dependency(path, arguments, trial):
        started = time.monotonic()
        try:
            if pool is None:
                return bytes(compiler(path, arguments, trial))
            if arguments:
                raise ValueError("compiler slot dependencies take no extra arguments")
            (kind, data), = pool.run(_slot_compile, [(str(path), None, None, (trial,))], slot=0)[0]
            if kind != "object":
                raise ValueError(data)
            return data
        finally:
            timings["compile_seconds"] += time.monotonic() - started

    def trial_object(raws, path, arguments, trial):
        return _trial_object(unit, raws, dependencies, compile_dependency, path, arguments, trial)

    def score_objects(results):
        if not results or results[-1][0] == "failed" or len(results) != len(grid):
            failures.append(results[-1][1] if results else "no compile result")
            return None
        raws = {flags: data for flags, (_kind, data) in zip(grid, results)}
        measured = compare_unit([record], image, unit, (), grid, shared,
                                compiler=lambda path, arguments, trial: trial_object(raws, path, arguments, trial),
                                root=root, targets=references)
        comparison = measured.results[0][1]
        if isinstance(comparison, str):
            failures.append(comparison[:300])
            return None
        exact = comparison.verdict is Tier.BYTE_EXACT
        best = None
        eh = None
        for flags in grid:
            try:
                listing = subject_asm(raws[flags], record, " ".join(flags))
            except ValueError:
                continue
            if isinstance(listing, RegionCandidate):
                found = eh_distance(listing, record, image)
                if found is not None:
                    value = (found["eh_topology_distance"], found["instruction_distance"], found["byte_distance"])
                    if best is None or value < best:
                        best, eh = value, found
                continue
            original = originals.setdefault(None, original_asm(record, image))
            if len(listing.code) > len(original.code):
                try:
                    if len(listing.code) not in originals:
                        originals[len(listing.code)] = original_asm(record, image, extent=len(listing.code))
                    original = originals[len(listing.code)]
                except ValueError:
                    pass
            value = (0, *listing_distance(original, listing))
            best = value if best is None or value < best else best
        topology, instructions, differing = best if best is not None else (0, *comparison_distance(comparison))
        distance = 0 if exact else max(1, (topology * _TOPOLOGY + instructions) * _SCALE
                                       + min(differing, _SCALE - 1))
        detail = {
            "verdict": comparison.verdict.value, "instruction_distance": instructions,
            "byte_distance": differing, "matching_instructions": comparison.matching_instructions,
            "original_instructions": comparison.original_instructions,
            "compiled_instructions": comparison.compiled_instructions,
            "first_divergence": comparison.first_divergence,
            "allocation_only": comparison.allocation_only}
        if eh is not None:
            detail.update(eh_topology_distance=eh["eh_topology_distance"],
                          original_eh_states=eh["original_eh_states"], compiled_eh_states=eh["compiled_eh_states"])
        if comparison.diagnostic:
            detail["diagnostic"] = comparison.diagnostic[:300]
        return Score(distance, exact, detail)

    def score(body_text: str):
        return score_objects(compile_grid(splice(body_text)))

    def score_many(body_texts):
        return [score_objects(results) for results in compile_texts([splice(text) for text in body_texts])]

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

    sweep = None
    try:
        # Control: the overlay route must reproduce the direct compile before any score means anything,
        # on every slot, so concurrent trials are scored against one compiler behavior.
        if pool is None:
            direct = compile_grid(None, grid[:1])
            replicas = [compile_grid(host_text, grid[:1])]
        else:
            direct = pool.run(_slot_compile, [(str(unit), None, None, grid[:1])], slot=0)[0]
            replicas = [pool.run(_slot_compile, [(str(unit), host, host_text.encode("utf-8"), grid[:1])],
                                 slot=number)[0] for number in range(len(pool))]
        if direct[0][0] != "object" or any(row[0][0] != "object" for row in replicas):
            raise ValueError("unchanged source does not compile through the configured compiler")
        signature = object_signature(Coff(direct[0][1]))
        if any(object_signature(Coff(row[0][1])) != signature for row in replicas):
            raise ValueError("unchanged overlay replica differs physically from the direct compile"
                             + (" on some compiler slot" if pool is not None else ""))
        control_compiles = 1 + len(replicas)

        started = time.monotonic()
        if probe_sweep:
            sweep = _probe_sweep(original_body, parameters, probe_sizes, probe_forms, score, score_many,
                                 pool is not None, keep, time_limit, progress)
            result = sweep["result"]
        else:
            result = search(original_body, score, iterations=iterations, seed=seed, time_limit=time_limit,
                            parameters=parameters, operators=operators, scale=_SCALE,
                            on_improve=keep, on_trial=trial,
                            batch_scorer=score_many if pool is not None else None,
                            batch=len(pool) if pool is not None else 1)
        elapsed = time.monotonic() - started
    except BaseException:
        if pool is not None:
            pool.close(cancel=True)
            pool = None
        raise
    finally:
        if pool is not None:
            busy = list(pool.busy)
            pool.close()
    keep(result.best_text, result.best)
    # A changed input makes the scores stale; keep the variant for review but never call it exact.
    stale = (settings.snapshot(root, metadata) != before
             or host_path.read_text(encoding="utf-8") != host_text)
    compiles = result.compiles * len(grid)
    outputs["summary"] = str(destination) + ".json"
    stand_in = bool(probe_calls(result.best_text))
    summary = {
        "schema_version": 1, "operation": "permute",
        "result": "unknown" if stale else "pass" if result.best.exact else "fail",
        "inputs_changed": stale,
        "address": record.address_hex, "name": record.name, "symbol": record.symbol or record.name,
        "source": host, "translation_unit": unit_relative, "flag_sets": [list(flags) for flags in grid],
        "mode": "probe_sweep" if probe_sweep else "search",
        "seed": seed, "iterations": result.iterations, "iteration_limit": iterations,
        "time_limit": time_limit, "stop_reason": result.stop_reason, "interrupted": result.interrupted,
        "exact": result.best.exact,
        "start": {"distance": result.start.distance, **result.start.detail},
        "best": {"distance": result.best.distance, **result.best.detail},
        "improved": result.best.distance < result.start.distance,
        "best_contains_stand_in_probe": stand_in,
        "compiler_slots": [name for name, _environment in slots] if slots else None,
        "jobs": len(slots) if slots else 1,
        "compiles": compiles + control_compiles, "search_compiles": compiles,
        "control_compiles": control_compiles,
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
    if slots:
        summary["slot_busy_seconds"] = [round(value, 3) for value in busy]
        summary["limitations"].append(
            "concurrent trials are batched per round: the trajectory is reproducible for one seed and job count, "
            "but differs from a run with another job count")
    if sweep is not None:
        counts = {}
        for row in sweep["ranking"]:
            key = (row["eh_topology_distance"], row["compiled_eh_states"], row["distance"])
            counts[key] = counts.get(key, 0) + 1
        summary["probe_sweep"] = {
            "variants": sweep["variants"], "sizes": list(probe_sizes), "forms": list(probe_forms),
            "ranking": sweep["ranking"][:25], "complete": sweep["complete"],
            "unchanged": sum(row["distance"] == result.start.distance for row in sweep["ranking"]),
            "outcomes": [{"eh_topology_distance": topology, "compiled_eh_states": states, "distance": distance,
                          "variants": number}
                         for (topology, states, distance), number in sorted(
                             counts.items(), key=lambda item: (item[0][2], item[0][0] or 0))][:20]}
    if stand_in:
        summary["limitations"].append(
            "the best variant contains a marked stand-in dead budget probe: it is evidence about the original's "
            "/Ob2 inline budget, not recovered code; replace it with a genuine spelling before recording")
    Path(outputs["summary"]).write_text(json.dumps(summary, indent=2, sort_keys=True))
    return summary


def _probe_sweep(original_body, parameters, sizes, forms, score, score_many, parallel, keep, time_limit, progress):
    """Score every single-probe placement; deterministic ranking by (distance, size, order)."""
    from .permute import SearchResult
    started = time.monotonic()
    start = score(original_body) if not parallel else score_many([original_body])[0]
    if start is None:
        raise ValueError("the unchanged body does not compile through the scorer")
    result = SearchResult(start=start, best=start, best_text=original_body, compiles=1, stop_reason="sweep")
    variants = probe_variants(original_body, parameters, sizes=sizes, forms=forms)
    ranking, complete = [], True
    chunk = 32
    try:
        for low in range(0, len(variants), chunk):
            if time_limit is not None and time.monotonic() - started >= time_limit:
                complete, result.stop_reason = False, "time_limit"
                break
            part = variants[low:low + chunk]
            texts = [text for _description, text in part]
            scores = score_many(texts) if parallel else [score(text) for text in texts]
            for offset, ((description, text), value) in enumerate(zip(part, scores)):
                number = low + offset + 1
                result.iterations = number
                result.compiles += 1
                if progress is not None:
                    progress({"iteration": number, "distance": None if value is None else value.distance,
                              "exact": bool(value and value.exact), "mutations": [description]})
                if value is None:
                    result.compile_failures += 1
                    continue
                ranking.append({"order": number, "probe": description, "distance": value.distance,
                                "exact": value.exact, **{key: value.detail.get(key) for key in (
                                    "eh_topology_distance", "compiled_eh_states", "original_eh_states",
                                    "instruction_distance", "byte_distance", "verdict")}})
                if value.distance < result.best.distance or (value.exact and not result.best.exact):
                    result.best, result.best_text = value, text
                    result.improvements.append({"iteration": number, "distance": value.distance,
                                                "exact": value.exact, "mutations": [description]})
                    keep(text, value)
    except KeyboardInterrupt:
        complete, result.interrupted, result.stop_reason = False, True, "interrupted"
    ranking.sort(key=lambda row: (row["distance"], row["order"]))
    result.elapsed_seconds = time.monotonic() - started
    return {"result": result, "ranking": ranking, "variants": len(variants), "complete": complete}

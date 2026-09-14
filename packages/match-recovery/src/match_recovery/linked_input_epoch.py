"""Shared live native input epoch for section and selected-family observations.

No persisted report creates a handle. Physical compiler/link inputs and native
selection indices remain guarded through every use and final context exit.
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path
from weakref import WeakKeyDictionary

from .build_graph_commands import _context
from .components import relative_path
from .linked_code_ownership import _compilations, _Inputs
from .link_file_evidence import parse_link_files
from .link_input_coverage import _input_inventory
from .linked_objects import SelectedObjects
from .model import digest


@dataclass(frozen=True)
class _InputState:
    root: object
    directory: object
    compiled: dict
    coverage: dict
    native: object
    inputs: object
    evidence: dict
    records: list


@contextmanager
def _validated_inputs(context, objects):
    inputs = _Inputs()
    report = digest(context.report)
    context_identity = (context.root, context.directory, context.check_current)
    population_fresh = None
    selected_fresh = None

    def fresh():
        try:
            if selected_fresh is not None:
                selected_fresh()
            elif population_fresh is not None:
                population_fresh()
        finally:
            try:
                inputs.check_current()
            finally:
                try:
                    context_identity[2]()
                finally:
                    if ((context.root, context.directory, context.check_current) != context_identity
                            or digest(context.report) != report):
                        raise ValueError("data producer native context report changed")

    try:
        context.check_current()
        if type(objects) is not SelectedObjects:
            raise ValueError("data producer requires native selected physical inputs")
        root, directory, compiled = _compilations(context)
        _, _, _, command = _context(context)
        artifacts = {}
        for kind in ("link-input-files", "msvc-link-log"):
            rows = [row for row in context.report.get("outputs", ()) if row.get("format") == kind]
            if len(rows) != 1:
                raise ValueError("data producer requires its native link input and final-pass artifacts")
            row, = rows
            path = directory / relative_path(row["path"])
            raw = inputs.read(path)
            if inputs.files[path][1] != {key: row[key] for key in ("sha256", "size")}:
                raise ValueError("data producer native artifact changed")
            artifacts[kind] = raw
        evidence = parse_link_files(artifacts["link-input-files"])
        if (digest(evidence) != digest(objects._constant_evidence)
                or list(command.arguments) != evidence["arguments"] or str(command.cwd) != evidence["cwd"]):
            raise ValueError("data producer objects differ from this native command/input epoch")
        coverage, records = _input_inventory(artifacts["link-input-files"], artifacts["msvc-link-log"])
        native = SelectedObjects(evidence, coverage, records)
        population, population_fresh = objects._constant_population()
        if digest(sorted(population, key=digest)) != digest(sorted(records, key=digest)):
            raise ValueError("data producer population differs from the native physical input inventory")
        # Read actual compiler/source/header inputs, not merely receipt fields.
        for row in context.report["compile_link"]["compilations"]:
            for item in row["compilation"]["inputs"]:
                path = Path(item["path"])
                inputs.read(path)
                if (item["before"] != item["after"] or inputs.files[path][1] != item["after"]):
                    raise ValueError("data producer compiler/source input changed from its receipt")
        from .coff_archive import CoffArchive
        from .common_allocation import _policy
        from .linked_context import _same
        # Initialize legitimate lazy policy state before pinning the immutable
        # native indices. Growing byte/archive/owner caches remain independently
        # checked against actual physical files, not copied verdicts.
        policy = _policy(evidence, records)
        if objects._constant_policy_gaps is None:
            objects._constant_policy_gaps = policy
        elif not _same(objects._constant_policy_gaps, policy):
            raise ValueError("data producer selected policy differs from native physical inputs")
        mutable = {"_files", "_archives", "_resolved"}
        resolvers = (objects, native)
        pinned = [deepcopy({key: value for key, value in resolver.__dict__.items() if key not in mutable})
                  for resolver in resolvers]
        witness = SelectedObjects(evidence, coverage, records)
        state = _InputState(root, directory, compiled, coverage, native, inputs, evidence, records)
        def indices():
            return (state.root, state.directory, state.compiled, state.coverage, state.evidence, state.records)
        index_before = deepcopy(indices())
        archives = {}
        archive_members = {}
        memo_files = {}

        def selected_fresh():
            if state.native is not native or state.inputs is not inputs or not _same(indices(), index_before):
                raise ValueError("native input derived indices changed")
            population_fresh()
            if set(memo_files) - inputs.files.keys():
                raise ValueError("native input byte memo lost a captured file")
            for path, value in inputs.files.items():
                raw, stamp = value
                if (not _same(stamp, {"sha256": sha256(raw).hexdigest(), "size": len(raw)})
                        or path in memo_files and not _same(value, memo_files[path])):
                    raise ValueError("native input byte memo changed")
                memo_files[path] = deepcopy(value)
            if set(inputs.archives) - inputs.files.keys():
                raise ValueError("native input archive memo has an unobserved key")
            for path, members in inputs.archives.items():
                if path not in archive_members:
                    archive_members[path] = deepcopy(CoffArchive(inputs.files[path][0]).members)
                if not _same(members, archive_members[path]):
                    raise ValueError("native input archive memo changed")
            for resolver, before in zip(resolvers, pinned):
                if not _same({key: value for key, value in resolver.__dict__.items() if key not in mutable}, before):
                    raise ValueError("data producer selected-object indices changed")
                if set(resolver._files) - resolver._stamps.keys() or set(resolver._archives) - resolver._files.keys():
                    raise ValueError("data producer has an unobserved native cache key")
                for name, raw in resolver._files.items():
                    path = Path(name)
                    actual = inputs.read(path)
                    if inputs.files[path][1] != resolver._stamps[name] or not _same(actual, raw):
                        raise ValueError("data producer physical file cache changed")
                for path, archive in resolver._archives.items():
                    if path not in archives:
                        archives[path] = deepcopy(CoffArchive(resolver._files[path]).__dict__)
                    if type(archive) is not CoffArchive or not _same(archive.__dict__, archives[path]):
                        raise ValueError("data producer physical archive cache changed")
                for owner, value in resolver._resolved.items():
                    if not _same(value, witness.resolve(owner)):
                        raise ValueError("data producer selected owner cache changed")

        fresh()
        yield state, fresh
    except (KeyError, TypeError, AttributeError) as error:
        raise ValueError("incomplete native data producer evidence") from error
    finally:
        fresh()



class InputEpoch:
    """Opaque live parsing handle; not a scope or acceptance capability."""

    __slots__ = ("__weakref__",)

    def __new__(cls, *args, **kwargs):
        raise TypeError("native input epochs require their context manager")

    def __reduce__(self):
        raise TypeError("native input epochs are not serializable")

    def check_current(self):
        _, _, _, fresh = _entry(self)
        fresh()


_live = WeakKeyDictionary()


def _entry(epoch):
    if type(epoch) is not InputEpoch or epoch not in _live:
        raise ValueError("native input epoch is unrecognized or expired")
    return _live[epoch]


@contextmanager
def _borrow_epoch(epoch, context, objects):
    original_context, original_objects, state, fresh = _entry(epoch)
    try:
        fresh()
        if context is not original_context or objects is not original_objects:
            raise ValueError("native input epoch belongs to another context or selected-object instance")
        yield state
    finally:
        fresh()


@contextmanager
def input_epoch(context, objects):
    """Parse once, guard every consumer, then expire even after a refused close."""
    with _validated_inputs(context, objects) as (state, fresh):
        epoch = object.__new__(InputEpoch)
        _live[epoch] = (context, objects, state, fresh)
        try:
            fresh()
            yield epoch
        finally:
            try:
                fresh()
            finally:
                del _live[epoch]

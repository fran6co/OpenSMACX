"""Bind a preservation trial to a live application object, without a waiver.

Replica agreement is necessary before linked evidence can explain a compiled
residual. It is not itself proof that an initializer, datum or other residual
may be removed from preservation coverage. Reports cannot be replayed as inputs.
"""

from contextlib import contextmanager, ExitStack
from copy import deepcopy
from dataclasses import dataclass
from hashlib import sha256
from pathlib import Path

from .build_graph_commands import _context
from .coff_replica import object_signature
from .link_file_evidence import _stamp
from .linked_initializers import observe_build_initializers, _read
from .model import Result, digest
from .pe_coff import Coff
from .source_replica import snapshot_digest
from .startup_coverage import _mint_startup
from .startup_projection import project_startup


@dataclass(frozen=True)
class ObjectTrial:
    raw: bytes
    root: Path
    task_name: str
    baseline_revision: str
    authorization: str
    source_digest: str
    unit: str
    version: str
    flags: tuple[str, ...]
    input_fingerprint: str


@contextmanager
def application_objects(graph):
    """Index native membership once inside a single live preservation epoch.

    Individual comparisons remain provisional until normal context exit. Each
    reads its actual output against the original receipt; final graph freshness
    covers all source, policy, membership and input/output changes. Only pure
    whole-object signatures are memoized by exact raw SHA256, never verdicts.
    """
    live = False
    indexes, signatures, initializer_views = {}, {}, {}
    common_stack, common_views = ExitStack(), {}
    try:
        graph.check_current()
        root, task_name, baseline_revision = graph.root, graph.task_name, graph.baseline_revision
        authorization, sources = digest(graph.authorization), graph.source_versions
        source_digests = {version: snapshot_digest(sources[version]) for version in ("baseline", "current")}
        for version, context in (("baseline", graph.baseline), ("current", graph.current)):
            _, _, rows, _ = _context(context)
            receipts = {row["source"]: row for row in context.report["compile_link"]["compilations"]}
            indexes[version] = {}
            for unit, row in rows.items():
                receipt = receipts[str(context.root / unit)]["compilation"]["output"].get("after")
                _stamp(receipt)
                if receipt is None:
                    raise ValueError("application object lacks a fresh compiler receipt")
                indexes[version][unit] = (row["output"], dict(receipt))
        graph.check_current()
        live = True

        def signature(raw, identity):
            if identity not in signatures:
                signatures[identity] = object_signature(Coff(raw))
            return signatures[identity]

        def compare(trial, obj, *, unit, version, flags, input_fingerprint):
            if not live:
                raise ValueError("application object comparison context has expired")
            try:
                if (not isinstance(trial, ObjectTrial) or type(trial.raw) is not bytes
                        or trial.root != root or trial.task_name != task_name
                        or trial.baseline_revision != baseline_revision or trial.authorization != authorization
                        or version not in ("baseline", "current") or trial.version != version
                        or trial.unit != unit or trial.flags != tuple(flags)
                        or trial.input_fingerprint != input_fingerprint):
                    raise ValueError("preservation object trial identity or authorization differs from live graph")
                if unit not in sources[version] or trial.source_digest != source_digests[version]:
                    raise ValueError("preservation object source version differs from live graph")
                trial_sha256 = sha256(trial.raw).hexdigest()
                observed = signature(trial.raw, trial_sha256)
                if object_signature(obj) != observed:
                    raise ValueError("preservation object changed from captured compiler output")
                if unit not in indexes[version]:
                    raise ValueError("preservation object has no unique application compilation")
                output, expected = indexes[version][unit]
                if output.resolve() != output or not output.is_file() or output.stat().st_nlink != 1:
                    raise ValueError("application object must remain an unaliased physical file")
                raw = output.read_bytes()
                stamp = {"sha256": sha256(raw).hexdigest(), "size": len(raw)}
                if stamp != expected:
                    raise ValueError("application object differs from its fresh compiler receipt")
                linked = signature(raw, stamp["sha256"])
                same = observed == linked
                return {"check": "preservation_application_object", "unit": unit, "version": version,
                        "flags": list(flags), "source_digest": trial.source_digest,
                        "input_fingerprint": input_fingerprint,
                        "trial_sha256": trial_sha256, "application_object": str(output),
                        "application_sha256": stamp["sha256"], "trial_signature": observed,
                        "application_signature": linked, "result": Result.PASS if same else Result.UNKNOWN,
                        "linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                        "reason": ("same-source whole-object replica agreement; residual effects remain unproved" if same else
                                   "application output differs from this preservation trial; linked evidence cannot cover it")}
            except (KeyError, TypeError, AttributeError) as error:
                raise ValueError("incomplete preservation/application object binding") from error

        def coverage_fresh():
            if not live:
                raise ValueError("startup coverage application context has expired")
            graph.check_current()
            if digest(graph.authorization) != authorization:
                raise ValueError("startup coverage authorization changed")

        def cover_startup(trial, obj, *, unit, version, flags, input_fingerprint, changes):
            """Bind authorized exact generated slots, never a supplied report.

            Source ownership is independently validated by the task caller. The
            same selections must also belong to the graph's pinned permission;
            source editability alone cannot authorize generated effects.
            """
            linked_stack = ExitStack()
            native = locality = None
            try:
                coverage_fresh()
                fields = {"entity_id", "unit", "version", "symbol", "offset", "evidence"}
                permission = graph.authorization.get("permission", {}).get("generated_changes", ())
                permission_ids = ({digest(item) for item in permission}
                                  if isinstance(permission, (tuple, list)) else set())
                if (not isinstance(changes, (tuple, list)) or not changes or len(changes) > 4096
                        or not isinstance(permission, (tuple, list))
                        or any(not isinstance(change, dict) or set(change) != fields
                               or change.get("unit") != unit or change.get("version") != version
                               or not isinstance(change.get("symbol"), str) or not change["symbol"]
                               or type(change.get("offset")) is not int or change["offset"] < 0
                               or digest(change) not in permission_ids for change in changes)):
                    raise ValueError("startup selections differ from exact generated-change permission")
                changes = deepcopy(changes)
                if len({(change["offset"], change["symbol"]) for change in changes}) != len(changes):
                    raise ValueError("duplicate startup coverage selection")
                observed = compare(trial, obj, unit=unit, version=version, flags=flags,
                                   input_fingerprint=input_fingerprint)
                if observed["result"] != Result.PASS:
                    raise ValueError("startup coverage requires this trial's whole application-object replica")
                output, expected = indexes[version][unit]
                raw = _read(output, expected)
                application_obj = Coff(raw)
                context = graph.baseline if version == "baseline" else graph.current
                if version not in initializer_views:
                    initializer_views[version] = observe_build_initializers(context)
                    coverage_fresh()
                linked = initializer_views[version]
                dependencies = linked.get("code_dependencies", {})
                if (linked.get("result") != Result.PASS or not isinstance(dependencies, dict)
                        or dependencies.get("result") != Result.PASS):
                    raise ValueError("startup coverage lacks consistent fresh linked initializer evidence")
                if object_signature(application_obj) != observed["application_signature"]:
                    raise ValueError("startup application object changed from its replica observation")
                application_tables = [number for number, section in enumerate(application_obj.sections, 1)
                                      if section.name == ".CRT$XCU"]
                trial_tables = [number for number, section in enumerate(obj.sections, 1)
                                if section.name == ".CRT$XCU"]
                if len(application_tables) != 1 or len(trial_tables) != 1:
                    raise ValueError("startup coverage requires a unique table in both actual compiler objects")
                application_table, = application_tables
                trial_table, = trial_tables
                slots, fpo = [], []
                for change in changes:
                    entries = [row for row in linked.get("entries", ())
                               if row.get("input") == {"path": str(output)}
                               and row.get("offset") == change["offset"]
                               and row.get("function", {}).get("symbol") == change["symbol"]]
                    if len(entries) != 1:
                        raise ValueError("startup selection has no unique actual compilation/table entry")
                    entry, = entries
                    if entry.get("object_sha256") != expected["sha256"] or entry.get("section") != application_table:
                        raise ValueError("startup entry differs from its actual compiler object receipt")
                    emitted = application_obj.function(change["symbol"])
                    local = obj.function(change["symbol"])
                    for subject, number, symbol in ((application_obj, application_table, emitted),
                                                    (obj, trial_table, local)):
                        matches = [row for row in subject.sections[number - 1].relocations
                                   if row[0] == change["offset"]]
                        if symbol.storage != 3 or matches != [(change["offset"], symbol.index, 6)]:
                            raise ValueError("startup slot does not retain its exact static root identity")
                    slots.append((trial_table, change["offset"], local.index))
                    children = [number for number, parent in application_obj.associations.items()
                                if parent == emitted.section and application_obj.sections[number - 1].name == ".debug$F"]
                    if children:
                        from .initializer_order import _project, _LocalRoots
                        from .linked_context import linked_build
                        from .linked_fpo import verify_fpo
                        from .startup_local import slot_local_root
                        if locality is None:
                            _, projection = _project(context, linked, changes)
                            locality = _LocalRoots(context, linked, projection)
                        locality.check(unit, change["symbol"])
                        trial_local = slot_local_root(obj, table=trial_table, offset=change["offset"], symbol=local)
                        trial_children = trial_local["recognized_debug_children"]
                        if len(children) != 1 or len(trial_children) != 1:
                            raise ValueError("startup FPO lacks a unique trial/application child")
                        trial_child = trial_children[0]["section"]
                        if native is None:
                            native = linked_stack.enter_context(linked_build(context))
                        try:
                            native.check_current()
                            frame = verify_fpo(application_obj, children[0], emitted, native.inputs.image,
                                               entry["target"], native.inputs.fixups)
                            replica_frame = verify_fpo(obj, trial_child, local, native.inputs.image,
                                                       entry["target"], native.inputs.fixups)
                            if frame != replica_frame:
                                raise ValueError("startup trial FPO differs from its selected native frame")
                        finally:
                            native.check_current()
                        fpo.append((trial_child, local.index, tuple(sorted(frame.items()))))
                # Validate the exact projection algebra now. The consumer must
                # revalidate it alongside ordinary mapped-function subtraction.
                project_startup(obj, tuple(slots), fpo=tuple(fpo))
                repeated = compare(trial, obj, unit=unit, version=version, flags=flags,
                                   input_fingerprint=input_fingerprint)
                if repeated != observed:
                    raise ValueError("startup replica evidence changed during linked observation")
                coverage_fresh()
                return _mint_startup(obj, tuple(slots), coverage_fresh, fpo=tuple(fpo), trial=trial)
            except (KeyError, TypeError, AttributeError) as error:
                raise ValueError("incomplete native startup coverage binding") from error
            finally:
                try:
                    linked_stack.close()
                finally:
                    coverage_fresh()

        def common_fresh():
            try:
                coverage_fresh()
            finally:
                try:
                    if "baseline" in common_views:
                        common_views["baseline"].check_current()
                finally:
                    if "current" in common_views:
                        common_views["current"].check_current()

        def cover_common_requests(trial, obj, *, unit, version, flags, input_fingerprint, startup):
            """Explain only redundant requests from this epoch's covered roots.

            A surviving native witness is outside the entire pinned generated
            projection domain, preventing mutually borrowed or later-removed
            witnesses. No storage isolation or initializer effect is waived.
            """
            from .common_preservation import _compare_linked
            from .common_request_coverage import _requests, _mint_requests
            from .common_request_witnesses import retained_witnesses
            from .linked_context import linked_build
            from .startup_coverage import _consume_for_producer

            try:
                common_fresh()
                slots, fpo = _consume_for_producer(startup, obj, coverage_fresh, trial=trial)
                observed = compare(trial, obj, unit=unit, version=version, flags=flags,
                                   input_fingerprint=input_fingerprint)
                if observed["result"] != Result.PASS:
                    raise ValueError("common request coverage requires the whole application-object replica")
                _, projection = project_startup(obj, slots, fpo=fpo)
                indices = _requests(obj, projection)
                opposite = "current" if version == "baseline" else "baseline"
                if unit in indexes[opposite]:
                    output, expected = indexes[opposite][unit]
                    other = Coff(_read(output, expected))
                    # An unchanged declaration needs no waiver. This filter
                    # only narrows eligibility; a different opposite trial
                    # still has to pass the ordinary strict residual checks.
                    unchanged = {(symbol.name, symbol.value) for symbol in other.symbols.values()
                                 if symbol.section == 0 and symbol.storage == 2 and symbol.type == 0
                                 and symbol.value > 0 and not other.auxiliary.get(symbol.index)}
                    indices = tuple(index for index in indices
                                    if (obj.symbols[index].name, obj.symbols[index].value) not in unchanged)
                if not indices:
                    return None
                permission = graph.authorization.get("permission", {}).get("generated_changes", ())
                excluded_units = frozenset(change["unit"] for change in permission)
                if unit not in excluded_units:
                    raise ValueError("common request has no generated-change authorization")
                for name, context in (("baseline", graph.baseline), ("current", graph.current)):
                    if name not in common_views:
                        common_views[name] = common_stack.enter_context(linked_build(context))
                requests = {obj.symbols[index].name: obj.symbols[index].value for index in indices}
                proof = _compare_linked(common_views["baseline"], common_views["current"], tuple(requests))
                if (proof["result"] != Result.PASS
                        or any(row.get("baseline_size") != requests[row["symbol"]]
                               or row.get("current_size") != requests[row["symbol"]]
                               for row in proof["allocations"])):
                    raise ValueError("duplicate common request lacks unchanged native common allocation evidence")
                retained_witnesses(common_views["baseline"], common_views["current"],
                                   excluded_units=excluded_units, requests=requests)
                repeated = compare(trial, obj, unit=unit, version=version, flags=flags,
                                   input_fingerprint=input_fingerprint)
                if repeated != observed:
                    raise ValueError("common request replica evidence changed during native verification")
                return _mint_requests(obj, startup, indices, common_fresh)
            except (KeyError, TypeError, AttributeError) as error:
                raise ValueError("incomplete native duplicate common request binding") from error
            finally:
                common_fresh()

        compare.cover_startup = cover_startup
        compare.cover_common_requests = cover_common_requests
        yield compare
    except (KeyError, TypeError, AttributeError) as error:
        if live:
            raise  # Do not translate unrelated exceptions from the caller body.
        raise ValueError("incomplete preservation/application object binding") from error
    finally:
        try:
            if live and digest(graph.authorization) != authorization:
                raise ValueError("application object coverage authorization changed before final verification")
        finally:
            live = False
            try:
                common_stack.close()
            finally:
                graph.check_current()


def compare_application_object(graph, trial, obj, *, unit, version, flags, input_fingerprint):
    """Standalone observation with the same entry and mandatory exit guards."""
    with application_objects(graph) as compare:
        return compare(trial, obj, unit=unit, version=version, flags=flags, input_fingerprint=input_fingerprint)

"""Native task application-graph evidence, with no linked-effect waiver."""

from contextlib import ExitStack, contextmanager
from copy import deepcopy
from dataclasses import dataclass, replace
from pathlib import Path
import re
from typing import Callable
from weakref import WeakKeyDictionary

from .application_context import ApplicationBuild, application_build
from .build_check import Build
from .build_graph_commands import compare_build_commands
from .build_policy import build_policy_changes
from .driver import working_state
from .measurement import Measurement
from .generated_permissions import generated_sources
from .linked_initializers import compare_build_initializers
from .graph_limits import MAX_NODES
from .model import Result, digest
from .source_history import _tree, git, revision_files
from .source_inventory import image_pin
from .source_replica import replica_roots, replica_snapshot
from .tasks import TaskStore


@dataclass(frozen=True, eq=False)
class ApplicationGraph:
    """Live graph evidence; neither the report nor source copies grant scope."""

    root: Path
    task_name: str
    baseline_revision: str
    roots: tuple[str, ...]
    baseline: ApplicationBuild
    current: ApplicationBuild
    _sources: dict
    _report: dict
    _authorization: dict
    check_current: Callable[[], None]

    @property
    def source_versions(self):
        return deepcopy(self._sources)

    @property
    def report(self):
        return deepcopy(self._report)

    @property
    def authorization(self):
        return deepcopy(self._authorization)


_live_graphs = WeakKeyDictionary()


def require_bound_graph(session):
    """Verify exact native session identity and its originally supplied authority.

    A copied dataclass, a replacement callback or another valid permission may
    not borrow the original build epoch. The binding is never serialized.
    """
    if type(session) is not ApplicationGraph or session not in _live_graphs:
        raise ValueError("application graph requires its original live native authority binding")
    expected, sources, authority, report = _live_graphs[session]
    guard = expected[-1]
    try:
        guard()
        actual = (session.root, session.task_name, session.baseline_revision, session.roots,
                  session.baseline, session.current, session.check_current)
        if (actual[:4] != expected[:4] or any(a is not b for a, b in zip(actual[4:], expected[4:]))
                or session._sources != sources or digest(session._authorization) != authority
                or digest(session._report) != report):
            raise ValueError("application graph binding or authority changed during its native epoch")
    finally:
        guard()


@contextmanager
def _bound_graph(session):
    """Private native producer hook, not caller-supplied verification data."""
    if type(session) is not ApplicationGraph or session in _live_graphs:
        raise ValueError("application graph needs a new exclusive native binding")
    session.check_current()
    _live_graphs[session] = ((session.root, session.task_name, session.baseline_revision, session.roots,
                             session.baseline, session.current, session.check_current),
                            deepcopy(session._sources), digest(session._authorization), digest(session._report))
    try:
        require_bound_graph(session)
        yield session
    finally:
        try:
            require_bound_graph(session)
        finally:
            _live_graphs.pop(session, None)


@contextmanager
def application_graph(root, task_name, *, integration=None, permission=None):
    """Build authorized source/graph versions under the same current policy.

    Called by native migration verification. Command/membership agreement is
    necessary, not sufficient: linked initializer/common/COMDAT effects still
    require independent proof before this prerequisite can pass. Ordinary scoped
    source edits may use paired builds under unchanged policy without granting
    any generated-code exception.
    """
    root = Path(root).resolve()
    store, settings, build = TaskStore(root), Measurement.read(root), Build.read(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    scope, editable = store._scope(task, decisions)
    permission = deepcopy(permission)
    authorization = {"task": task.to_dict(), "decisions": decisions, "integration": integration}
    if permission is not None:
        from .migration_protocol import permissions
        permissions(permission, task, scope)
        if not permission.get("generated_changes"):
            raise ValueError("application graph permission requires explicit generated changes")
        authorization["permission"] = permission
    before, policy_before = working_state(root), build.snapshot(root)
    measured_before = settings.snapshot(root, {"task": task, "decisions": decisions, "integration": integration})
    policy = build_policy_changes(root, task, settings, build, integration=integration)
    if not policy["graph_only"] and not policy["unchanged"]:
        raise ValueError("application graph comparison requires exclusively versioned graph changes and unchanged compiler/build policy")
    if set(policy["changed_paths"]) - set(editable):
        raise ValueError("changed build graph inputs exceed authorized editable task files")
    if build.compile_link is None or "command_stages" not in build.compile_link:
        raise ValueError("application graph comparison requires native compile/link observations and declared command stages")
    baseline = policy["baseline_revision"]
    # Task history is never reset to HEAD or to a freshly built source tree.
    git(root, "merge-base", "--is-ancestor", task.base_revision, before["head"].decode().strip())
    roots = replica_roots(settings.source.roots)
    source_before = replica_snapshot(root, roots)
    current = {name: raw for name, raw in source_before.items() if raw is not None}
    prior = revision_files(root, baseline, roots)
    changed = sorted(name for name in current.keys() | prior.keys() if current.get(name) != prior.get(name))
    if set(changed) - set(editable):
        raise ValueError("application source changes exceed authorized editable task files")
    pin = image_pin(root, settings.source)
    if permission is not None:
        generated_sources(permission, task, scope, editable, {"baseline": prior, "current": current},
                          settings, pin, root)
    pin_entry = _tree(root, baseline, (settings.source.image_pin,)).get(settings.source.image_pin)
    pin_bytes = (git(root, "cat-file", "blob", pin_entry[2]) if pin_entry is not None
                 and pin_entry[:2] in (("100644", "blob"), ("100755", "blob")) else b"")
    if (any(subject.image_sha256 != pin or subject.address_space != settings.source.address_space for subject in task.subjects)
            or re.findall(rb"^sha256\s+([0-9a-f]{64})\s*$", pin_bytes, re.MULTILINE) != [pin.encode()]):
        raise ValueError("application graph baseline must retain the task's pinned original image")
    historical = _tree(root, baseline, build.versioned_inputs)
    old_graph, new_graph = {}, {}
    for name in build.versioned_inputs:
        entry = historical.get(name)
        if entry is None or entry[:2] not in (("100644", "blob"), ("100755", "blob")):
            raise ValueError("application graph baseline requires exact declared regular graph files")
        old_graph[name] = git(root, "cat-file", "blob", entry[2])
        new_graph[name] = (root / name).read_bytes()

    def fresh():
        if (working_state(root) != before or build.snapshot(root) != policy_before
                or replica_snapshot(root, roots) != source_before or image_pin(root, settings.source) != pin
                or store.read(task_name) != task or store.decisions(task_name) != decisions
                or settings.snapshot(root, {"task": task, "decisions": decisions, "integration": integration}) != measured_before
                or build_policy_changes(root, task, settings, build, integration=integration) != policy):
            raise ValueError("task application graph inputs or authorization changed")

    try:
        fresh()
        with ExitStack() as contexts:
            old = contexts.enter_context(application_build(root, roots, prior, versioned_files=old_graph))
            fresh()
            new = contexts.enter_context(application_build(root, roots, current, versioned_files=new_graph))
            fresh()
            commands = compare_build_commands(old, new, editable=editable)
            fresh()
            initializers = compare_build_initializers(old, new, max_functions=MAX_NODES, **(
                {"generated_changes": permission["generated_changes"]} if permission is not None else {}))
            fresh()
            # Reuse the actual current application build in migration; do not
            # compile a third copy simply to obtain another build success.
            result = {"check": "task_application_build_graph", "task": task_name,
                      "baseline_revision": baseline, "graph_inputs": policy["changed_paths"],
                      "source_changes": changed, "commands": commands,
                      "initializers": initializers,
                      "current_build": new.report, "baseline_build": old.report,
                      # A mismatch between selected input and its actual PE is
                      # corruption, unlike an intentional cross-build delta.
                      "result": Result.FAIL if commands["result"] == Result.FAIL or
                                initializers.get("input_consistency") == Result.FAIL or
                                permission is not None and initializers.get("projected_order", {}).get("result") == Result.FAIL
                                else Result.UNKNOWN,
                      "linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                      "reason": "command comparison does not establish remaining linked-effect preservation; "
                                + "initializer table/code comparison: " + initializers["result"]
                                + "; callback/storage/other residual effects remain unproved",
                      "limitations": ["bounded initializer table/code comparison is not startup effect, common allocation, COMDAT selection or residual proof",
                                      "source/graph versions retain original task and integration preservation baselines"]}
            live = True

            def session_fresh():
                if not live:
                    raise ValueError("application graph context has expired")
                # Refusals do not skip the other build or original authority
                # guard; no participant can refresh another participant's epoch.
                try:
                    old.check_current()
                finally:
                    try:
                        new.check_current()
                    finally:
                        fresh()

            try:
                session_fresh()
                session = ApplicationGraph(root, task_name, baseline, roots, old, new,
                                           deepcopy({"baseline": prior, "current": current}),
                                           deepcopy(result), deepcopy(authorization), session_fresh)
                from .application_residual import compare_application_residual
                try:
                    with _bound_graph(session):
                        residual = compare_application_residual(session)
                except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
                    residual = {"check": "application_task_residual", "result": Result.UNKNOWN,
                                "reason": str(error), "outside_scope_verification": Result.UNKNOWN,
                                "ready_for_collection": False}
                result["application_residual"] = residual
                if residual.get("scoped_code", {}).get("storage_residual", {}).get("input_consistency") == Result.FAIL:
                    result.update(result=Result.FAIL,
                                  reason="selected application data disagrees with its own linked image")
                session_fresh()
                session = replace(session, _report=deepcopy(result))
                if permission is not None:
                    from .generated_code_residual import compare_generated_code_residual
                    try:
                        with _bound_graph(session):
                            residual = compare_generated_code_residual(session)
                    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
                        residual = {"check": "application_generated_code_residual", "result": Result.UNKNOWN,
                                    "reason": str(error), "remaining_linked_effects": Result.UNKNOWN,
                                    "ready_for_collection": False}
                    # This dimension omits generated roots only, not all task
                    # functions. Neither a PASS nor a cross-build difference
                    # supplies the remaining scope/storage/effect composition.
                    result["generated_code_residual"] = residual
                    selection = residual.get("input_selection", {})
                    if selection.get("dependency_families") == Result.FAIL and result["result"] != Result.FAIL:
                        result["reason"] = ("selected dependency families changed outside the initializer-only proof; "
                                            "complete contribution preservation is required")
                    storage = residual.get("storage_residual", {})
                    # An intended source/root delta cannot justify a mismatch
                    # between an actual selected object and its own linked data.
                    # Cross-build storage changes need separate ownership/effect
                    # composition; their diagnostic FAIL alone is not this case.
                    if storage.get("input_consistency") == Result.FAIL:
                        result.update(result=Result.FAIL,
                                      reason="selected data contents or references disagree with the actual linked image")
                    session_fresh()
                    session = replace(session, _report=deepcopy(result))
                with _bound_graph(session):
                    yield session
            finally:
                try:
                    session_fresh()
                finally:
                    live = False
    finally:
        fresh()


def check_build_graph(root, task_name, *, integration=None):
    """Report a freshly verified graph context without retaining live files."""
    with application_graph(root, task_name, integration=integration) as session:
        return session.report

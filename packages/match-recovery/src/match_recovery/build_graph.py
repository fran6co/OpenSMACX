"""Native task application-graph evidence, with no linked-effect waiver."""

from contextlib import ExitStack
from pathlib import Path
import re

from .application_context import application_build
from .build_check import Build
from .build_graph_commands import compare_build_commands
from .build_policy import build_policy_changes
from .driver import working_state
from .measurement import Measurement
from .linked_initializers import compare_build_initializers
from .model import Result
from .source_history import _tree, git, revision_files
from .source_inventory import image_pin
from .source_replica import replica_roots, replica_snapshot
from .tasks import TaskStore


def check_build_graph(root, task_name, *, integration=None):
    """Build authorized source/graph versions under the same current policy.

    Called by native migration verification. Command/membership agreement is
    necessary, not sufficient: linked initializer/common/COMDAT effects still
    require independent proof before this prerequisite can pass.
    """
    root = Path(root).resolve()
    store, settings, build = TaskStore(root), Measurement.read(root), Build.read(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    _, editable = store._scope(task, decisions)
    before, policy_before = working_state(root), build.snapshot(root)
    measured_before = settings.snapshot(root, {"task": task, "decisions": decisions, "integration": integration})
    policy = build_policy_changes(root, task, settings, build, integration=integration)
    if not policy["graph_only"]:
        raise ValueError("application graph comparison requires exclusively versioned graph changes and unchanged compiler/build policy")
    if set(policy["changed_paths"]) - set(editable):
        raise ValueError("changed build graph inputs exceed authorized editable task files")
    if build.compile_link is None or "command_stages" not in build.compile_link:
        raise ValueError("application graph comparison requires native compile/link observations and declared command stages")
    baseline = policy["baseline_revision"]
    # Task history is never reset to HEAD or to a freshly built source tree.
    git(root, "merge-base", "--is-ancestor", task.base_revision, before["head"].decode().strip())
    roots = replica_roots(settings.source.roots)
    current = {name: raw for name, raw in replica_snapshot(root, roots).items() if raw is not None}
    prior = revision_files(root, baseline, roots)
    changed = sorted(name for name in current.keys() | prior.keys() if current.get(name) != prior.get(name))
    if set(changed) - set(editable):
        raise ValueError("application source changes exceed authorized editable task files")
    pin = image_pin(root, settings.source)
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
            initializers = compare_build_initializers(old, new)
            fresh()
            # Reuse the actual current application build in migration; do not
            # compile a third copy simply to obtain another build success.
            result = {"check": "task_application_build_graph", "task": task_name,
                      "baseline_revision": baseline, "graph_inputs": policy["changed_paths"],
                      "source_changes": changed, "commands": commands,
                      "initializers": initializers,
                      "current_build": new.report, "baseline_build": old.report,
                      "result": Result.FAIL if commands["result"] == Result.FAIL else Result.UNKNOWN,
                      "linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
                      "reason": "command comparison does not establish remaining linked-effect preservation; "
                                + "initializer table/code comparison: " + initializers["result"]
                                + "; callback/storage/other residual effects remain unproved",
                      "limitations": ["bounded initializer table/code comparison is not startup effect, common allocation, COMDAT selection or residual proof",
                                      "source/graph versions retain original task and integration preservation baselines"]}
        return result
    finally:
        fresh()

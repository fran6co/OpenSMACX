"""Explicit destination context for verification, never a replacement history.

The coordinator names the incoming source commit and the destination *before*
integration. Both must already be ancestors of the candidate checkout. This
module performs no merge, conflict resolution, publication or saved admission.
"""

import os
import re

from .model import require_text
from .reviews import ReviewStore
from .source_history import _tree, git
from .tasks import TaskStore


def decision(task, value):
    fields = {"author", "source_revision", "target_revision", "evidence"}
    if not isinstance(value, dict) or set(value) != fields or value["author"] != task.author:
        raise ValueError("integration context requires the task coordinator and exact source/target revisions")
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("integration context requires coordinator evidence")
    for item in value["evidence"]:
        require_text(item, "integration evidence")
    for key in ("source_revision", "target_revision"):
        revision = value[key]
        if not isinstance(revision, str) or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", revision):
            raise ValueError("integration context requires full immutable commit ids")
    return dict(value)


def validate(root, task, value):
    value = decision(task, value)
    head = git(root, "rev-parse", "HEAD").decode().strip()
    for key in ("source_revision", "target_revision"):
        revision = value[key]
        if git(root, "rev-parse", revision + "^{commit}").decode().strip() != revision:
            raise ValueError("integration revision must identify a commit, not an alias or tag object")
        try:
            git(root, "merge-base", "--is-ancestor", task.base_revision, revision)
            git(root, "merge-base", "--is-ancestor", revision, head)
        except ValueError as error:
            raise ValueError("integration revisions must retain the task baseline and both be incorporated in this checkout") from error
    source, target = value["source_revision"], value["target_revision"]
    if git(root, "merge-base", source, target).decode().strip() == source:
        raise ValueError("integration target must precede collection of the incoming source")
    store, reviews = TaskStore(root), ReviewStore(root, task.task_id)
    _, editable = store.scope(task.task_id)
    reviews.decisions()
    task_file = "recovery/tasks/" + task.task_id + ".json"
    paths = (task_file,
             store.journal.stream_directory("task:" + task.task_id).relative_to(root).as_posix(),
             reviews.journal.stream_directory(reviews.stream).relative_to(root).as_posix())
    # A merge resolution must not drop either branch's existing task decisions,
    # rewrite the task baseline, or discard review/interpretation history.
    for revision in (source, target):
        entries = _tree(root, revision, paths)
        if revision == source and task_file not in entries:
            raise ValueError("incoming source must contain the original task authorization")
        for name, (mode, kind, blob) in entries.items():
            path = root / name
            if (mode != "100644" or kind != "blob" or not path.is_file() or path.is_symlink()
                    or path.resolve() != path or path.read_bytes() != git(root, "cat-file", "blob", blob)):
                raise ValueError("integration changed or removed committed task/review history; preserve both branches")
    # Validate additions as real events, not arbitrary files under a history path.
    from .driver import _resumable_history
    allowed = set(editable) | {task_file} | _resumable_history(root, store, reviews, task.task_id)
    changed = git(root, "diff", "--name-only", "-z", target).split(b"\0")
    changed += git(root, "ls-files", "--others", "--exclude-standard", "-z").split(b"\0")
    if {os.fsdecode(name) for name in changed if name} - allowed:
        raise ValueError("integration changes exceed authorized task source and decision history")
    if git(root, "rev-parse", "HEAD").decode().strip() != head:
        raise ValueError("integration checkout changed during context validation")
    return dict(value)


def preservation_base(root, task, context=None):
    return task.base_revision if context is None else validate(root, task, context)["target_revision"]

"""Explicit local fast-forward collection with fresh checks on both checkouts.

No saved verdict is accepted. Git owns the checkout update; repairs, autostash,
hooks, checkout filters, force updates and automatic rollback are forbidden.
A post-update failure retains the destination and requires fresh verification.
"""

from contextlib import ExitStack
import os
from pathlib import Path
import re
import subprocess

from .checkpoints import _publication_changes, preflight
from .driver import drive, invoke, working_state
from .integration import decision, validate
from .integration_prepare import _assert_raw_checkout, _destination_state, _files, _run
from .locking import file_lock
from .model import canonical, require_text
from .preparation import prepare_project
from .records import read_json
from .tasks import TaskStore


def authorization(task, value):
    fields = {"author", "candidate_revision", "destination_branch", "integration", "message", "allow_migration"}
    if not isinstance(value, dict) or set(value) != fields or value["author"] != task.author:
        raise ValueError("destination publication requires explicit task coordinator authorization")
    if (not isinstance(value["candidate_revision"], str)
            or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", value["candidate_revision"])):
        raise ValueError("destination publication requires an immutable candidate commit")
    require_text(value["destination_branch"], "destination branch")
    require_text(value["message"], "collection message")
    if not value["destination_branch"].startswith("refs/heads/") or type(value["allow_migration"]) is not bool:
        raise ValueError("destination publication requires a full local branch ref and boolean migration permission")
    return {**value, "integration": decision(task, value["integration"])}


def _options(root):
    options = ["-c", "core.hooksPath=" + os.devnull, "-c", "core.fsmonitor=false",
               "-c", "merge.autostash=false", "-c", "submodule.recurse=false"]
    filters = _run(root, options, "config", "--name-only", "--get-regexp", r"^filter\..*\.(clean|smudge|process|required)$", codes=(0, 1))
    for prefix in sorted({line.rsplit(".", 1)[0] for line in filters.stdout.decode().splitlines()}):
        for field, value in (("clean", ""), ("smudge", ""), ("process", ""), ("required", "false")):
            options.extend(["-c", prefix + "." + field + "=" + value])
    if _run(root, options, "config", "--default", "false", "--bool", "--get", "core.sparseCheckout").stdout.strip() == b"true":
        raise ValueError("destination publication requires a complete, nonsparse checkout")
    return options


def _checkout_policy(root, revision, options):
    entries, _ = _files(root, revision)
    fields = _run(root, options, "check-attr", "--source=" + revision, "-z", "--stdin", "filter",
                  input=b"".join(os.fsencode(name) + b"\0" for name in entries)).stdout.split(b"\0")
    if len(fields[:-1]) != 3 * len(entries) or fields[-1:] != [b""]:
        raise ValueError("unsupported checkout attribute response")
    if any(value not in (b"unset", b"unspecified") for value in fields[2::3]):
        raise ValueError("destination publication refuses active checkout filters")
    return entries


def publish_integration(root, name, candidate, permission, *, configuration, worker_configuration,
                        reviewer_configuration, migration_configuration=None, execute=invoke):
    root, candidate = Path(root).resolve(), Path(candidate).resolve()
    if root == candidate:
        raise ValueError("collection requires separate destination and candidate checkouts")
    if any(key in os.environ for key in ("GIT_DIR", "GIT_WORK_TREE", "GIT_COMMON_DIR", "GIT_INDEX_FILE",
                                         "GIT_OBJECT_DIRECTORY", "GIT_ALTERNATE_OBJECT_DIRECTORIES", "GIT_NAMESPACE")):
        raise ValueError("destination publication refuses Git path overrides")
    with ExitStack() as stack:
        leases = {path: stack.enter_context(file_lock(path / "build/recovery/driver.lock", timeout=0))
                  for path in sorted((root, candidate))}
        options = {path: _options(path) for path in (root, candidate)}
        def git(path, *args):
            return _run(path, options[path], *args).stdout.decode().strip()
        common = [(path / git(path, "rev-parse", "--git-common-dir")).resolve() for path in (root, candidate)]
        if common[0] != common[1]:
            raise ValueError("collection currently requires worktrees of the same local repository")
        task = TaskStore(candidate).read(name)
        permission = authorization(task, permission)
        if (not isinstance(worker_configuration, dict) or not isinstance(reviewer_configuration, dict)
                or task.author in {worker_configuration.get("author"), reviewer_configuration.get("author")}
                or worker_configuration.get("author") == reviewer_configuration.get("author")):
            raise ValueError("collection needs an independent coordinator, worker identity and reviewer")
        branch, candidate_branch = preflight(root), preflight(candidate)
        target = permission["integration"]["target_revision"]
        if branch != permission["destination_branch"] or git(root, "rev-parse", "HEAD") != target:
            raise ValueError("destination branch or HEAD changed from the explicit collection authorization")
        if git(candidate, "rev-parse", "HEAD") != permission["candidate_revision"]:
            raise ValueError("candidate HEAD changed from the explicit collection authorization")
        _assert_raw_checkout(candidate, permission["candidate_revision"])
        _checkout_policy(candidate, permission["candidate_revision"], options[candidate])
        old_entries = _checkout_policy(root, target, options[root])
        _checkout_policy(root, permission["candidate_revision"], options[root])
        before = _destination_state(root, target, options[root])
        _publication_changes(root, before, set())  # Do not hide mode edits behind core.fileMode=false.
        validate(candidate, task, permission["integration"])
        publication = {"author": task.author, "message": permission["message"] + "\n\nCollection authorization:\n" + canonical(permission).decode(),
                       "allow_migration": permission["allow_migration"]}
        policy = {**configuration, "acceptance_scope": "change"}
        report = {"task_id": name, "authorization": permission, "destination": str(root), "candidate": str(candidate),
                  "destination_updated": False, "destination_verified": False, "ready_for_collection": False,
                  "acceptance_scope": "change", "persisted_results": False}

        def verify(path):
            if "preparation" in read_json(path / "recovery/project.json"):
                preparation = prepare_project(path)
                if preparation["result"] != "pass" or not preparation["published"]:
                    return {"stop_reason": "preparation_required", "preparation": preparation}
            return drive(path, name, policy, worker_configuration, iterations=1,
                         reviewer_configuration=reviewer_configuration, migration_configuration=migration_configuration,
                         checkpoint_configuration=publication, integration_configuration=permission["integration"],
                         verify_only=True, execute=execute, _held_lease=leases[path],
                         _collection={**permission, "destination": str(root), "candidate": str(candidate)})

        def admitted(result):
            return (result["stop_reason"] == "change_accepted" or
                    result["stop_reason"] == "migration_admitted" and permission["allow_migration"])

        # Nothing at the destination moves before fresh candidate admission.
        checked = verify(candidate)
        report["candidate_verification"] = checked
        if not admitted(checked):
            if working_state(root) != before or preflight(root) != branch:
                raise ValueError("destination changed while verifying the candidate; preserve work")
            return {**report, "stop_reason": "candidate_verification_required"}
        commit = git(candidate, "rev-parse", "HEAD")
        candidate_state = _assert_raw_checkout(candidate, commit)
        if preflight(candidate) != candidate_branch:
            raise ValueError("candidate branch changed during collection verification")
        validate(candidate, task, permission["integration"])
        git(root, "merge-base", "--is-ancestor", target, commit)
        entries = _checkout_policy(root, commit, options[root])
        if working_state(root) != before or preflight(root) != branch:
            raise ValueError("destination changed during candidate verification; preserve work and reauthorize")
        for filename in entries.keys() - old_entries.keys():
            path = root / filename
            if path.exists() or path.is_symlink():
                raise ValueError("collection would overwrite an untracked or ignored destination path: " + filename)
        for filename in old_entries:
            path = root / filename
            if old_entries[filename] != entries.get(filename) and path.lstat().st_nlink != 1:
                raise ValueError("collection refuses a hardlinked destination output: " + filename)
        report["verified_candidate_commit"] = commit
        report["update_attempted"] = True
        try:
            # Git refuses overwritten local work. No manual source/index writer,
            # stash, merge resolution, reset, or saved acceptance participates.
            _run(root, options[root], "merge", "--ff-only", "--no-autostash", "--no-overwrite-ignore", "--no-squash", "--no-edit", commit)
            report["destination_updated"] = True
            if git(root, "rev-parse", "HEAD") != commit or preflight(root) != branch:
                raise ValueError("destination changed during fast-forward; preserve work and inspect")
            after = _destination_state(root, commit, options[root])
            untouched = {name for name, value in old_entries.items() if entries.get(name) == value}
            if any(before["files"][name] != after["files"].get(name) for name in untouched):
                raise ValueError("fast-forward changed unrelated checkout bytes or modes; preserve work")
            checked = verify(root)
            report["destination_verification"] = checked
            if working_state(candidate) != candidate_state or preflight(candidate) != candidate_branch:
                raise ValueError("candidate changed during destination verification; preserve both checkouts")
            if preflight(root) != branch:
                raise ValueError("destination branch changed during final verification")
            report["final_commit"] = git(root, "rev-parse", "HEAD")
            if not admitted(checked):
                return {**report, "stop_reason": "destination_verification_required"}
            return {**report, "destination_verified": True,
                    "stop_reason": "destination_accepted" if checked["stop_reason"] == "change_accepted" else "destination_migration_admitted"}
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            # Publication and final checks are not a filesystem transaction.
            # A partial update or post-check failure must not discard recovery
            # or somebody else's work. The old target remains in Git history.
            try:
                observed = git(root, "rev-parse", "HEAD")
            except (OSError, ValueError, subprocess.SubprocessError):
                observed = None
            return {**report, "stop_reason": "destination_verification_required", "error": str(error),
                    "destination_updated": True if report["destination_updated"] else None,
                    "observed_head": observed,
                    "next_action": "preserve both checkouts; inspect the update and run fresh destination verification; never replay this report"}

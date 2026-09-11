"""Native parallel dispatch of disjoint, centrally admitted retained scopes."""

import json
import os
from copy import deepcopy
from pathlib import Path

import pytest

from match_recovery.dispatch import dispatch
from match_recovery.driver import working_state
from match_recovery.locking import file_lock
from match_recovery.model import Origin
from match_recovery.retained_scope import authorize
from match_recovery.tasks import TaskStore
from test_dispatch import _replace_worker, batch
from test_source_configuration import configured
from test_task_source import git, project


def expanded_batch(batch, *, corrupt_scope=False):
    root, plan = batch
    for name, address in (("owner", 48), ("other", 64)):
        (root / f"code/{name}_extra.cc").write_text(
            f"// ORIGINAL: 0x{address:08X} _{name}_extra 0x{address:08X}-0x{address + 8:08X}\n"
            f"void {name}_extra() {{}}\n")
    git(root, "add", "code/owner_extra.cc", "code/other_extra.cc")
    git(root, "commit", "-m", "Additional disjoint original mappings")
    worker = ("import json,sys; from pathlib import Path; p=json.load(sys.stdin); "
              "name=p['task']['task_id']; "
              "assert sorted(p['editable_files']) == [f'code/{name}.cc', f'code/{name}_extra.cc']; "
              "counter=Path('build/worker-rounds'); "
              "counter.write_text((counter.read_text() if counter.exists() else '') + 'worker\\n'); ")
    if corrupt_scope:
        worker += ("from match_recovery.tasks import TaskStore; store=TaskStore(Path.cwd()); "
                   "store.expand(name, request_id='forbidden-scope', author='coordinator', subjects=(), "
                   "editable_files=('code/unapproved.hpp',), evidence=('Worker cannot change ownership',), "
                   "expected_revision=1); ")
    worker += "print('{\"discoveries\": []}')"
    _replace_worker(batch, worker)
    revision = git(root, "rev-parse", "HEAD")
    peers = [{"task": job["task"], "root": job["root"], "branch": "refs/heads/" + job["task"]}
             for job in plan["jobs"]]
    for job, address in zip(plan["jobs"], (48, 64)):
        child, name = Path(job["root"]), job["task"]
        store = TaskStore(child)
        store.expand(name, request_id="expanded", author="coordinator",
                     subjects=(Origin("a" * 64, "custom:offset", address),),
                     editable_files=(f"code/{name}_extra.cc",),
                     evidence=("Retained evidence establishes this additional owned function",), expected_revision=0)
        path = child / f"code/{name}_extra.cc"
        path.write_text(path.read_text() + "// preserve retained scoped work\n")
    for job in plan["jobs"]:
        job["retained_scope"] = authorize(root, Path(job["root"]), job["task"], revision=revision,
                                           branch="refs/heads/" + job["task"], peers=peers, reservations=[])
    return root, plan


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_native_parallel_workers_use_expanded_retained_scopes_without_changing_root_tasks(batch):
    root, plan = expanded_batch(batch)
    before = working_state(root)
    retained = {job["task"]: (TaskStore(Path(job["root"])).read(job["task"]),
                              TaskStore(Path(job["root"])).decisions(job["task"])) for job in plan["jobs"]}
    events = []
    report = dispatch(root, plan, progress=events.append)
    assert [event["event"] for event in events[:2]] == ["started", "started"]
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert working_state(root) == before
    assert len({(root / "build/signals" / name).read_text() for name in ("owner", "other")}) == 2
    for row, job in zip(report["jobs"], plan["jobs"]):
        child, name = Path(job["root"]), job["task"]
        assert row["scope_intact"] and row["returncode"] == 3 and not row["timed_out"]
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert receipt["worker_rounds"] == 1 and receipt["stop_reason"] == "no_source_progress"
        assert not receipt["ready_for_collection"]
        assert (child / "build/worker-rounds").read_text() == "worker\n"
        assert (TaskStore(child).read(name), TaskStore(child).decisions(name)) == retained[name]
        assert TaskStore(child).read(name).base_revision == TaskStore(root).read(name).base_revision
        assert not TaskStore(root).decisions(name)
        assert TaskStore(root).scope(name)[1] == frozenset((f"code/{name}.cc",))
        assert "preserve retained scoped work" in (child / f"code/{name}_extra.cc").read_text()
        request_id = job["retained_scope"]["request_id"]
        assert (child / "build/recovery/retained-scope-uses" / (request_id + ".json")).is_file()


@pytest.mark.parametrize("damage", ["omitted", "source", "history"])
def test_expanded_scope_needs_fresh_admission_before_any_hooks_or_workers_start(batch, damage):
    root, plan = expanded_batch(batch)
    job = plan["jobs"][1]
    child, name = Path(job["root"]), job["task"]
    if damage == "omitted":
        del job["retained_scope"]
    elif damage == "source":
        path = child / f"code/{name}_extra.cc"
        path.write_text(path.read_text() + "// newer retained source change\n")
    else:
        store = TaskStore(child)
        store.discover(name, request_id="new", author="worker", hypothesis="New observation since admission",
                       affects=[store.read(name).subjects[0].entity_id], evidence=["Current source"], expected_revision=1)
    events = []
    with pytest.raises(ValueError):
        dispatch(root, plan, progress=events.append)
    assert not events and not list((root / "build/signals").iterdir())
    assert all(not (Path(row["root"]) / "build/worker-rounds").exists() for row in plan["jobs"])


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_consumed_retained_admission_cannot_execute_another_worker_round(batch):
    root, plan = expanded_batch(batch)
    first = dispatch(root, plan)
    assert all(json.loads(Path(row["stdout"]).read_text())["worker_rounds"] == 1 for row in first["jobs"])
    repeated = dispatch(root, plan)
    assert not repeated["ready_for_collection"] and not repeated["batch_completed"]
    for row in repeated["jobs"]:
        assert row["returncode"] == 3 and not row["timed_out"]
        assert "already consumed" in Path(row["stderr"]).read_text()
        assert (Path(row["root"]) / "build/worker-rounds").read_text() == "worker\n"


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_scope_mutation_during_worker_is_retained_and_refused_after_run(batch):
    root, plan = expanded_batch(batch, corrupt_scope=True)
    report = dispatch(root, plan)
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert all(not row["scope_intact"] and row["returncode"] == 3 for row in report["jobs"])
    for row in report["jobs"]:
        child, name = Path(row["root"]), row["task"]
        assert "code/unapproved.hpp" in TaskStore(child).scope(name)[1]
        assert len(TaskStore(child).decisions(name)) == 2
        assert not TaskStore(root).decisions(name)


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_scope_change_after_both_children_exit_invalidates_dispatch_ownership(batch):
    root, plan = expanded_batch(batch)
    finished = []

    def progress(event):
        if event["event"] != "finished": return
        finished.append(event["job"])
        if len(finished) == 2:
            job = plan["jobs"][0]
            TaskStore(Path(job["root"])).expand(
                job["task"], request_id="postrun-scope", author="coordinator", subjects=(),
                editable_files=("code/unapproved.hpp",), evidence=("Late ownership mutation retained",),
                expected_revision=1)

    report = dispatch(root, plan, progress=progress)
    assert len(finished) == 2
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert all(not row["scope_intact"] for row in report["jobs"])
    for row in report["jobs"]:
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert receipt["worker_rounds"] == 1 and receipt["stop_reason"] == "no_source_progress"
        assert not TaskStore(root).decisions(row["task"])
    assert "code/unapproved.hpp" in TaskStore(Path(plan["jobs"][0]["root"])).scope("owner")[1]


def resume_expanded_batch(root, plan, assignments, *, announce=lambda *args, **kwargs: None):
    from match_recovery.campaign import _resume_round

    directory = root / "build/expanded-campaign-resume"
    directory.mkdir()
    before = working_state(root)

    def fresh():
        assert working_state(root) == before

    profiles = {"resources": json.loads((root / "recovery/resources.json").read_text())}
    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease:
        return _resume_round(root, plan, profiles, directory, 0, git(root, "rev-parse", "HEAD"),
                             execute=lambda *args: pytest.fail("No discovery needs no coordinator provider"),
                             fresh=fresh, announce=announce, lease=lease,
                             assignments=assignments, scope_reservations=[])


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_campaign_derives_fresh_admission_for_two_expanded_retained_workers(batch):
    root, admitted_plan = expanded_batch(batch)
    plan = {**admitted_plan, "jobs": [{key: value for key, value in job.items() if key != "retained_scope"}
                                    for job in admitted_plan["jobs"]]}
    assignments = [{"task": job["task"], "root": job["root"], "branch": "refs/heads/" + job["task"]}
                   for job in plan["jobs"]]
    original_plan, original_assignments = deepcopy(plan), deepcopy(assignments)
    before = working_state(root)
    children = {job["task"]: Path(job["root"]) for job in plan["jobs"]}
    retained = {name: (TaskStore(child).read(name), TaskStore(child).decisions(name),
                       (child / f"code/{name}_extra.cc").read_bytes()) for name, child in children.items()}
    events = []
    report, decisions, deferred = resume_expanded_batch(root, plan, assignments,
                                                       announce=lambda stage, **details: events.append((stage, details)))
    assert not decisions and not deferred
    assert plan == original_plan and assignments == original_assignments
    assert working_state(root) == before
    assert not report["batch_completed"] and not report["ready_for_collection"]
    started = [details for stage, details in events if stage == "worker" and details["event"] == "started"]
    assert len(started) == 2
    assert len({(root / "build/signals" / name).read_text() for name in children}) == 2
    for row in report["jobs"]:
        child, name = Path(row["root"]), row["task"]
        assert row["scope_intact"] and row["returncode"] == 3 and not row["timed_out"]
        receipt = json.loads(Path(row["stdout"]).read_text())
        assert receipt["worker_rounds"] == 1 and receipt["stop_reason"] == "no_source_progress"
        assert (TaskStore(child).read(name), TaskStore(child).decisions(name),
                (child / f"code/{name}_extra.cc").read_bytes()) == retained[name]
        assert TaskStore(child).read(name).base_revision == TaskStore(root).read(name).base_revision
        assert TaskStore(root).scope(name)[1] == frozenset((f"code/{name}.cc",))
        assert not TaskStore(root).decisions(name)


def test_campaign_refuses_expansion_overlap_with_an_inactive_original_assignment(batch):
    root, admitted_plan = expanded_batch(batch)
    assignments = [{"task": job["task"], "root": job["root"], "branch": "refs/heads/" + job["task"]}
                   for job in admitted_plan["jobs"]]
    only_job = {key: value for key, value in admitted_plan["jobs"][0].items() if key != "retained_scope"}
    plan = {**admitted_plan, "jobs": [only_job]}
    inactive = TaskStore(Path(assignments[1]["root"]))
    inactive.expand("other", request_id="inactive-overlap", author="coordinator", subjects=(),
                    editable_files=("code/owner_extra.cc",), evidence=("Retained conflicting ownership",),
                    expected_revision=1)
    events = []
    with pytest.raises(ValueError, match="overlaps"):
        resume_expanded_batch(root, plan, assignments,
                              announce=lambda stage, **details: events.append((stage, details)))
    assert not events and not list((root / "build/signals").iterdir())
    assert "code/owner_extra.cc" in inactive.scope("other")[1]
    assert all(not (Path(job["root"]) / "build/worker-rounds").exists() for job in assignments)
    assert not TaskStore(root).decisions("owner") and not TaskStore(root).decisions("other")


def test_campaign_holds_parked_assignment_lease_through_filtered_dispatch(batch, monkeypatch):
    import match_recovery.campaign as campaign

    root, admitted_plan = expanded_batch(batch)
    assignments = [{"task": job["task"], "root": job["root"], "branch": "refs/heads/" + job["task"]}
                   for job in admitted_plan["jobs"]]
    only_job = {key: value for key, value in admitted_plan["jobs"][0].items() if key != "retained_scope"}
    # A retained/retired assignment is absent from active_jobs but still owns its
    # original checkout. Admission-time locking alone does not protect dispatch.
    plan = {**admitted_plan, "jobs": [only_job]}
    parked = Path(assignments[1]["root"])
    paths = [root, *(Path(row["root"]) for row in assignments)]
    before = [working_state(path) for path in paths]
    calls = []

    def dispatch_under_leases(current, selected_plan, **kwargs):
        assert current == root
        assert [row["task"] for row in selected_plan["jobs"]] == ["owner"]
        calls.append("dispatch")
        with pytest.raises(TimeoutError):
            with file_lock(parked / "build/recovery/driver.lock", timeout=0):
                pytest.fail("Parked assignment must remain leased throughout native dispatch")
        return {"jobs": [], "batch_completed": False, "ready_for_collection": False}

    monkeypatch.setattr(campaign, "dispatch", dispatch_under_leases)
    report, decisions, deferred = resume_expanded_batch(root, plan, assignments)
    assert calls == ["dispatch"] and not decisions and not deferred
    assert not report["batch_completed"] and not report["ready_for_collection"]
    assert [working_state(path) for path in paths] == before
    assert not list((root / "build/signals").iterdir())
    with file_lock(parked / "build/recovery/driver.lock", timeout=0):
        pass  # A completed dispatch releases its inactive-peer lease normally.

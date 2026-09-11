"""Worker metadata continuity is independent of matching or provider success."""

import base64
import hashlib
import json
import subprocess
from collections import Counter
from itertools import combinations_with_replacement, permutations

import pytest

from match_recovery.binding_source import render
from match_recovery.driver import drive
from test_agent_command import configure, request
from test_binding import project as binding_project
from test_driver import CONFIG, WORKER, project as command_project
from test_source_configuration import configured
from test_task_source import git


PRODUCT = "code/owner.cc"
SEED = "hints/attempt.cc"
RATIONALE = render("Synthetic original binding", ["fixture:original", "source:code/owner.cc"])
FAILURES = "build/recovery/worker-source-failures"


def body(flags="", *, name="_owner"):
    return (f"// ORIGINAL: 0x00000010 {name} 0x00000010-0x00000018{flags}\n"
            + RATIONALE + "\nvoid owner() {}\n")


@pytest.fixture
def mapped(configured):
    (configured / PRODUCT).write_text(body())
    (configured / SEED).write_text(body())
    return configured


def capture(root):
    from match_recovery.worker_bindings import capture as implementation
    return implementation(root)


def validate(root, before, editable=(PRODUCT,)):
    from match_recovery.worker_bindings import validate as implementation
    return implementation(root, before, frozenset(editable))


def saved_sources(root, source, expected):
    paths = list((root / FAILURES).glob("*.json"))
    assert paths
    for path in paths:
        saved = json.loads(path.read_text())
        assert saved["verification"] == "not_measured" and saved["ready_for_collection"] is False
        assert base64.b64decode(saved["before"]["files"][source]["content_base64"]) == expected
        assert path.stat().st_mode & 0o777 == 0o600
        assert git(root, "check-ignore", path.relative_to(root).as_posix())
    return paths


def test_command_only_configuration_has_no_invented_source_provider(tmp_path):
    assert capture(tmp_path) is None
    (tmp_path / "recovery").mkdir()
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1}))
    assert capture(tmp_path) is None
    assert validate(tmp_path, None, ()) is None


@pytest.mark.parametrize("configuration", ["{", "[]", '{"schema_version":1,"source":null}',
                                          '{"schema_version":1,"source":{}}',
                                          '{"schema_version":2,"source":{}}'])
def test_malformed_declared_source_is_not_a_command_only_fallback(tmp_path, configuration):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "recovery/project.json").write_text(configuration)
    with pytest.raises(ValueError):
        capture(tmp_path)


def test_configured_empty_inventory_is_still_guarded(mapped):
    (mapped / PRODUCT).write_text("void owner() {}\n")
    (mapped / SEED).write_text("// evidence only\n")
    before = capture(mapped)
    assert before is not None
    assert validate(mapped, before) is None


@pytest.mark.parametrize("damage", ["delete_pair", "delete_marker", "delete_rationale",
                                    "reason", "evidence", "orphan_rationale"])
def test_unclaimed_product_and_provenance_cannot_disappear_behind_its_seed(mapped, damage):
    before = capture(mapped)
    path = mapped / PRODUCT
    text = path.read_text()
    if damage == "delete_pair":
        text = "void owner() {}\n"
    elif damage == "delete_marker":
        text = "\n".join(text.splitlines()[1:]) + "\n"
    elif damage == "delete_rationale":
        text = text.replace(RATIONALE + "\n", "")
    elif damage == "reason":
        text = text.replace("Synthetic original binding", "Different interpretation")
    elif damage == "evidence":
        text = text.replace("fixture:original", "fixture:invented")
    else:
        text = text.replace(RATIONALE, "\n" + RATIONALE)
    path.write_text(text)
    with pytest.raises(ValueError):
        validate(mapped, before)
    assert path.read_text() == text
    assert (mapped / SEED).read_text() == body()


@pytest.mark.parametrize("old,new", [
    ("0x00000010", "0x00000020"), ("0x00000018", "0x00000019"),
    ("_owner", "_different_original"), ("0x00000018", "0x00000018 FILE"),
    ("0x00000018", "0x00000018 EXCLUDED S1"),
    ("// kind game", "// kind library"),
    ("// body code/owner.hpp", "// body code/different.hpp"),
    ("// body code/owner.hpp\n", ""),
])
def test_original_interpretation_and_explicit_body_edges_are_preserved(mapped, old, new):
    path = mapped / PRODUCT
    path.write_text(body().replace("void owner()", "// kind game\n// body code/owner.hpp\nvoid owner()"))
    before = capture(mapped)
    path.write_text(path.read_text().replace(old, new))
    with pytest.raises(ValueError):
        validate(mapped, before)


@pytest.mark.parametrize("destination", ["code/moved.cc", "code/nested/moved.cc"])
def test_intact_product_rehoming_inside_scope_preserves_header_reference(mapped, destination):
    path = mapped / PRODUCT
    path.write_text(body().replace("void owner()", "// body code/owner.hpp\nvoid owner()"))
    before = capture(mapped)
    moved = mapped / destination
    moved.parent.mkdir(exist_ok=True)
    path.rename(moved)
    assert validate(mapped, before, (PRODUCT, destination)) is None
    assert "// body code/owner.hpp" in moved.read_text()


@pytest.mark.parametrize("editable", [(PRODUCT,), ("code/moved.cc",)])
def test_both_rehoming_endpoints_must_be_authorized(mapped, editable):
    before = capture(mapped)
    (mapped / PRODUCT).rename(mapped / "code/moved.cc")
    with pytest.raises(ValueError):
        validate(mapped, before, editable)


def test_source_to_seed_move_is_not_product_preservation_even_inside_scope(mapped):
    before = capture(mapped)
    (mapped / PRODUCT).rename(mapped / "hints/product.cc")
    with pytest.raises(ValueError):
        validate(mapped, before, (PRODUCT, "hints/product.cc"))


def test_same_role_seed_rehoming_inside_scope_does_not_erase_a_candidate(mapped):
    before = capture(mapped)
    (mapped / SEED).rename(mapped / "hints/moved.cc")
    assert validate(mapped, before, (SEED, "hints/moved.cc")) is None


@pytest.mark.parametrize("flags", [" BYTE_EXACT", " SEMANTIC", " BYTE_EXACT SEMANTIC"])
def test_existing_protection_cannot_be_dropped(mapped, flags):
    path = mapped / PRODUCT
    path.write_text(body(flags))
    before = capture(mapped)
    path.write_text(body())
    with pytest.raises(ValueError):
        validate(mapped, before)


@pytest.mark.parametrize("before_flags,after_flags", [
    ("", " BYTE_EXACT"), (" SEMANTIC", " BYTE_EXACT SEMANTIC"),
    (" BYTE_EXACT", " BYTE_EXACT SEMANTIC"),
])
def test_monotonic_claim_upgrade_does_not_freeze_current_matching_state(mapped, before_flags, after_flags):
    path = mapped / PRODUCT
    path.write_text(body(before_flags))
    before = capture(mapped)
    path.write_text(body(after_flags))
    assert validate(mapped, before) is None


def test_one_upgraded_duplicate_cannot_cover_two_previously_protected_candidates(mapped):
    path = mapped / PRODUCT
    path.write_text(body(" BYTE_EXACT") + body(" SEMANTIC"))
    before = capture(mapped)
    # Same total and per-flag totals, but there is no one-to-one preservation.
    path.write_text(body(" BYTE_EXACT SEMANTIC") + body())
    with pytest.raises(ValueError):
        validate(mapped, before)


def test_duplicate_multiplicity_cannot_be_replaced_by_a_different_original(mapped):
    path = mapped / PRODUCT
    path.write_text(body() + body())
    before = capture(mapped)
    path.write_text(body() + body(name="_other"))
    with pytest.raises(ValueError):
        validate(mapped, before)


def test_duplicate_matching_finds_valid_monotonic_assignment_without_greedy_loss(mapped):
    path = mapped / PRODUCT
    path.write_text(body() + body(" BYTE_EXACT"))
    before = capture(mapped)
    path.write_text(body(" BYTE_EXACT") + body())
    assert validate(mapped, before) is None


def test_claim_matching_agrees_with_independent_one_to_one_oracle():
    from match_recovery.worker_bindings import _retained
    populations = [population for size in range(4)
                   for population in combinations_with_replacement(range(4), size)]
    for prior in populations:
        for current in populations:
            expected = any(all(new & old == old for old, new in zip(prior, assignment))
                           for assignment in permutations(current, len(prior)))
            assert _retained(Counter(prior), Counter(current)) is expected, (prior, current)


def test_cpp_body_members_emitted_name_and_line_numbers_are_not_frozen(mapped):
    before = capture(mapped)
    path = mapped / PRODUCT
    path.write_text("// Fresh class evidence\n\n" + body().replace(
        "void owner() {}", "// symbol _better_name\nvoid better_name() { this->member = 1; }"))
    assert validate(mapped, before) is None


@pytest.mark.parametrize("ignored", [False, True])
def test_outside_scope_source_bytes_are_preserved_even_without_mappings(mapped, ignored):
    path = mapped / "code/outside.hpp"
    path.write_text("struct Outside {};\n")
    if ignored:
        (mapped / ".gitignore").write_text("code/outside.hpp\n")
        git(mapped, "init")
        assert git(mapped, "check-ignore", "code/outside.hpp") == "code/outside.hpp"
    before = capture(mapped)
    path.write_text("struct Outside { int changed; };\n")
    with pytest.raises(ValueError):
        validate(mapped, before)
    assert "changed" in path.read_text()


@pytest.mark.parametrize("damage", ["source_configuration", "image_pin", "configuration_removed", "revision"])
def test_source_selection_image_and_revision_cannot_change_under_worker(mapped, damage):
    if damage == "revision":
        git(mapped, "init")
        git(mapped, "add", ".")
        git(mapped, "commit", "-m", "Synthetic binding baseline")
    before = capture(mapped)
    config = mapped / "recovery/project.json"
    if damage == "source_configuration":
        value = json.loads(config.read_text())
        value["source"]["address_space"] = "different:space"
        config.write_text(json.dumps(value))
    elif damage == "image_pin":
        (mapped / "original/identity").write_text("sha256 " + "b" * 64 + "\n")
    elif damage == "configuration_removed":
        config.unlink()
    else:
        git(mapped, "commit", "--allow-empty", "-m", "Synthetic moved revision")
    with pytest.raises(ValueError):
        validate(mapped, before)


def native_command_project(command_project):
    root, task = command_project
    (root / "src/f.cpp").write_text(body())
    (root / "src/IMAGE_PIN").write_text("sha256 " + "a" * 64 + "\n")
    (root / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "source": {
        "image_pin": "src/IMAGE_PIN", "address_space": "fixture", "roots": ["src"]}}))
    return root, task


@pytest.mark.parametrize("outcome", ["success", "nonzero", "timeout"])
def test_driver_refuses_binding_loss_before_discovery_publication_on_every_exit(command_project, outcome):
    root, task = native_command_project(command_project)
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic configured binding")
    calls = []
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context":
            return 0, {}
        if name == "check":
            return 1, {"result": "fail"}
        assert name == "worker"
        (root / "src/f.cpp").write_text("void owner() {}\n")
        if outcome == "timeout":
            raise subprocess.TimeoutExpired("synthetic-worker", 1)
        return (3 if outcome == "nonzero" else 0), {"discoveries": [{
            "hypothesis": "The product is now seed-only", "affects": [task.subjects[0].entity_id],
            "evidence": ["Synthetic worker removed its original binding"]}]}
    with pytest.raises(ValueError):
        drive(root, task.task_id, CONFIG, WORKER, iterations=1, execute=execute)
    assert calls == ["context", "check", "worker"]
    assert (root / "src/f.cpp").read_text() == "void owner() {}\n"
    from match_recovery.tasks import TaskStore
    assert not TaskStore(root).decisions(task.task_id)
    saved_sources(root, "src/f.cpp", body().encode())


@pytest.mark.parametrize("outcome", ["success", "nonzero", "timeout", "invalid_response"])
def test_real_agent_bridge_guards_bindings_even_when_synthetic_subprocess_fails(command_project, outcome):
    from match_recovery.agent_command import run_agent
    root, task = native_command_project(command_project)
    ending = {
        "success": "print('{\"discoveries\":[]}')",
        "nonzero": "raise SystemExit(2)",
        "timeout": "import time; time.sleep(5)",
        "invalid_response": "print('not json')",
    }[outcome]
    script = "from pathlib import Path; Path('src/f.cpp').write_text('void owner() {}\\n'); " + ending
    config = configure(root, script=script, response="stdout", timeout=1)
    with pytest.raises(ValueError):
        run_agent(root, "worker", config, request(task))
    assert (root / "src/f.cpp").read_text() == "void owner() {}\n"
    saved_sources(root, "src/f.cpp", body().encode())


def test_real_agent_bridge_allows_cpp_edits_with_existing_mapping_intact(command_project):
    from match_recovery.agent_command import run_agent
    root, task = native_command_project(command_project)
    script = ("from pathlib import Path; p=Path('src/f.cpp'); "
              "p.write_text(p.read_text().replace('void owner() {}', 'void owner() { int member = 1; }')); "
              "print('{\"discoveries\":[]}')")
    config = configure(root, script=script, response="stdout")
    assert run_agent(root, "worker", config, request(task)) == (0, {"discoveries": []})
    assert "int member = 1" in (root / "src/f.cpp").read_text()
    assert not (root / FAILURES).exists()


def test_repeat_refusals_preserve_separate_non_overwriting_source_backups(command_project):
    root, _ = native_command_project(command_project)
    before = capture(root)
    path = root / "src/f.cpp"
    path.write_text("void owner() { /* preserve this WIP */ }\n")
    originals = {}
    for _ in range(2):
        with pytest.raises(ValueError, match="pre-worker source saved at"):
            validate(root, before, ("src/f.cpp",))
        for previous, contents in originals.items():
            assert previous.read_bytes() == contents
        paths = saved_sources(root, "src/f.cpp", body().encode())
        assert len(paths) == len(originals) + 1
        originals = {backup: backup.read_bytes() for backup in paths}
    assert "preserve this WIP" in path.read_text()


@pytest.mark.parametrize("unsafe", ["unignored", "symlink", "non_directory", "tracked"])
def test_unsafe_backup_storage_does_not_write_or_bypass_binding_refusal(command_project, unsafe, monkeypatch):
    monkeypatch.setenv("RECOVERY_PARSE_CACHE", "off")
    root, _ = native_command_project(command_project)
    parent = root / FAILURES
    if unsafe == "unignored":
        (root / ".gitignore").write_text("")
    elif unsafe == "symlink":
        target = root / "backup-target"
        target.mkdir()
        (root / "build/recovery").mkdir(parents=True)
        parent.symlink_to(target, target_is_directory=True)
    elif unsafe == "non_directory":
        (root / "build/recovery").mkdir(parents=True)
        parent.write_text("Existing ordinary file\n")
    else:
        parent.mkdir(parents=True)
        (parent / "existing.json").write_text("Existing tracked evidence\n")
        git(root, "add", "-f", str((parent / "existing.json").relative_to(root)))
    before = capture(root)
    old_files = {path.relative_to(root): path.read_bytes() for path in root.rglob("*")
                 if path.is_file() and ".git" not in path.relative_to(root).parts}
    (root / "src/f.cpp").write_text("void owner() { /* retained WIP */ }\n")
    with pytest.raises(ValueError, match="pre-worker source backup unavailable"):
        validate(root, before, ("src/f.cpp",))
    after = {path.relative_to(root): path.read_bytes() for path in root.rglob("*")
             if path.is_file() and ".git" not in path.relative_to(root).parts}
    old_files[root.joinpath("src/f.cpp").relative_to(root)] = (root / "src/f.cpp").read_bytes()
    assert after == old_files
    if unsafe == "symlink":
        assert not list(target.iterdir())


def test_without_git_safe_backup_cannot_be_invented_but_refusal_keeps_wip(mapped):
    before = capture(mapped)
    (mapped / PRODUCT).write_text("void owner() {}\n")
    with pytest.raises(ValueError, match="pre-worker source backup unavailable"):
        validate(mapped, before)
    assert not (mapped / "build").exists()
    assert (mapped / PRODUCT).read_text() == "void owner() {}\n"


@pytest.mark.parametrize("boundary", ["driver", "bridge"])
def test_binding_backup_precedes_unrelated_out_of_scope_change_refusal(command_project, boundary):
    from match_recovery.agent_command import run_agent
    root, task = native_command_project(command_project)
    script = ("from pathlib import Path; Path('src/f.cpp').write_text('void owner() {}\\n'); "
              "Path('outside.txt').write_text('preserve unrelated violation'); print('{\"discoveries\":[]}')")
    config = configure(root, script=script, response="stdout")
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic combined violation boundary")
    def execute(command, root, task_name, payload):
        if command.argv[0] == "context":
            return 0, {}
        if command.argv[0] == "check":
            return 1, {"result": "fail"}
        assert command.argv[0] == "worker"
        (root / "src/f.cpp").write_text("void owner() {}\n")
        (root / "outside.txt").write_text("preserve unrelated violation")
        return 0, {"discoveries": []}
    with pytest.raises(ValueError, match="pre-worker source saved at"):
        if boundary == "bridge":
            run_agent(root, "worker", config, request(task))
        else:
            drive(root, task.task_id, CONFIG, WORKER, iterations=1, execute=execute)
    assert (root / "outside.txt").read_text() == "preserve unrelated violation"
    assert (root / "src/f.cpp").read_text() == "void owner() {}\n"
    saved_sources(root, "src/f.cpp", body().encode())


def test_native_coordinator_binding_between_workers_becomes_protected_without_restarting_driver(binding_project):
    from match_recovery.model import Origin
    from match_recovery.source_inventory import source_snapshot
    from match_recovery.tasks import Task, TaskStore
    root = binding_project
    config_path = root / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["seed_roots"] = ["src/seeds"]
    config_path.write_text(json.dumps(config))
    (root / "src/seeds").mkdir()
    seed = root / "src/seeds/f.cpp"
    seed_text = "// ORIGINAL: 0x00401000 _hint 0x00401000-0x00401001\nint hint() { return 0; }\n"
    seed.write_text(seed_text)
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic seed-only original")
    row, = source_snapshot(root)["records"]
    task = Task("seed-owner", "binding-control", "coordinator", "Recover original owner",
                git(root, "rev-parse", "HEAD"), (Origin(**row["origin"]),),
                ("src/f.cpp",), ("Synthetic original seed",))
    store = TaskStore(root)
    store.create(task)
    store.discover(task.task_id, request_id="seed-body", author="coordinator",
                   hypothesis="Recover then bind product", affects=[row["entity_id"]],
                   evidence=["Synthetic seed"], expected_revision=0)
    git(root, "add", "recovery")
    git(root, "commit", "-m", "Synthetic seed recovery authorization")
    command = lambda name: {"argv": [name], "timeout": 10}
    driver = {"schema_version": 1, "target": task.target,
              "context": command("context"), "check": command("check")}
    delegation = {"schema_version": 1, "author": task.author, "command": command("coordinate"),
                  "subjects": [], "editable_files": [], "evidence": ["Synthetic binding authority"],
                  "seed_bindings": {"image": "original.exe"}, "max_decisions": 4}
    product = root / "src/f.cpp"
    actions, worker_sources = [], []
    def execute(command, root, name, payload):
        role = command.argv[0]
        if role == "context":
            return 0, {"mapped": "ORIGINAL:" in product.read_text()}
        if role == "check":
            return 1, {"result": "fail"}
        if role == "coordinate":
            text = product.read_text()
            action = "investigate_local" if "ORIGINAL:" in text else "bind_seed" if "return 2" in text else "investigate"
            actions.append(action)
            answer = {"snapshot": payload["snapshot"], "action": action, "discovery_id": "seed-body",
                      "subjects": [], "editable_files": [], "evidence": ["Current synthetic owner"]}
            if action == "bind_seed":
                answer["binding"] = {"entity_id": row["entity_id"], "source": "src/f.cpp", "line": 1,
                                     "source_sha256": hashlib.sha256(product.read_bytes()).hexdigest(),
                                     "name": "_f", "reason": "Coordinator-owned uncommitted rationale"}
            return 0, answer
        assert role == "worker"
        worker_sources.append(product.read_bytes())
        if len(worker_sources) == 1:
            assert "ORIGINAL:" not in product.read_text()
            product.write_text("int f() { return 2; }\n")
            return 0, {"discoveries": []}
        assert len(worker_sources) == 2 and "RECOVERY_BINDING:" in product.read_text()
        assert "Coordinator-owned uncommitted rationale" in product.read_text()
        product.write_text("int f() { return 3; }\n")
        return 0, {"discoveries": [{"hypothesis": "Invented seed-only status after deletion",
                                   "affects": [row["entity_id"]], "evidence": ["Synthetic worker deletion"]}]}
    with pytest.raises(ValueError, match="original source binding"):
        drive(root, task.task_id, driver, {"author": "worker", "command": command("worker")},
              iterations=2, coordinator_configuration=delegation, execute=execute)
    assert actions == ["investigate", "bind_seed", "investigate_local"]
    assert len(worker_sources) == 2
    assert product.read_text() == "int f() { return 3; }\n" and seed.read_text() == seed_text
    assert len(store.decisions(task.task_id)) == 1
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
    saved_sources(root, "src/f.cpp", worker_sources[1])

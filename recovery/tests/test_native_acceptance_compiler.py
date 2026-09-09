"""Real native gate/restoration/preservation/build/runtime acceptance control.

The default controls use deterministic worker/reviewer doubles. An opt-in live
control uses the configured provider and the package's delegated coordinator. The
test-owned reference is freshly compiled, linked and run; no proprietary fixture
or precomputed verifier response participates. Not an Alpha Centauri pilot.
"""

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.binding import bind
from match_recovery.driver import drive, invoke
from match_recovery.image_survey import survey_image
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff
from match_recovery.project import initialize
from match_recovery.recording import record
from match_recovery.source_inventory import source_snapshot
from match_recovery.tasks import Task, TaskStore
from recovery.environment.wine import VC6_ROOT


CLASSES = """struct Base { int bias; };
struct Member { int value; };
struct Owner : Base {
    Member member;
    void assign(int a, int b) { bias = a; member.value = b; }
    int add(int input);
};
"""
FLAT = """struct Owner {
    int raw_[2];
    void assign(int a, int b) { raw_[0] = a; raw_[1] = b; }
    int add(int input);
};
"""
BODY = '#include "owner.h"\nint Owner::add(int input) { return bias + member.value + input; }\n'
ENTRY = """#include "owner.h"
extern "C" __declspec(dllimport) void __stdcall ExitProcess(unsigned int);
extern "C" void entry() {
    Owner owner;
    owner.assign(3, 4);
    ExitProcess(owner.add(5));
}
"""
FLAGS = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Oy-", "/Z7")
METHOD = "?add@Owner@@QAEHH@Z"


def prepared_project(tmp_path, monkeypatch, *, damage=None, provider=None, authorize=True, proofs=True, preparation=False,
                     normalized_checkout=False, generated_context=None, seed_owner=False):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    root = tmp_path / "unregistered-project"
    root.mkdir()
    product = root / "src"
    product.mkdir()
    local = root / "recovery"
    local.mkdir()
    integration = Path(__file__).resolve().parents[1]
    for source, target in ((integration / "environment/compile.py", "compiler_support.py"),
                           (integration / "environment/wine.py", "local_wine.py"),
                           (Path(__file__).with_name("fixtures") / "acceptance_agents.py", "local_agents.py"),
                           (Path(__file__).with_name("fixtures") / "acceptance_environment.py", "environment.py")):
        (local / target).write_bytes(source.read_bytes())
    (product / "owner.h").write_text(CLASSES)
    (product / "owner.cpp").write_text(BODY)
    entry = ENTRY.replace("owner.assign(3, 4)", "owner.assign(TestBias, 4)")
    # An intentionally different application-build flag can produce a faulty
    # executable while independently measured source objects still match. The
    # runtime verifier must observe the retained artifact, not those objects.
    entry = entry.replace("    ExitProcess", "#ifdef TEST_RUNTIME_DAMAGE\n    owner.assign(TestBias, 5);\n#endif\n    ExitProcess")
    entry = entry.replace('extern "C" void entry()', 'int TestBias = 3;\nextern "C" void entry()')
    if generated_context is not None:
        assert preparation
        (local / "generated.json").write_text(json.dumps({"flat": 1 if generated_context == "collateral" else 2,
                                                         "structured": 2}))
        entry += "\nint GeneratedSentinel = GENERATED_SENTINEL;\n"
    (product / "entry.cpp").write_text(entry)
    original = tmp_path / "original"
    original.mkdir()
    script = [sys.executable, "recovery/environment.py"]
    built = subprocess.run([*script, "build", str(root), str(original)], cwd=root, capture_output=True, timeout=90)
    assert built.returncode == 0, built.stdout.decode() + built.stderr.decode()
    image = original / "sample.exe"
    pin = hashlib.sha256(image.read_bytes()).hexdigest()
    addresses = {}
    for line in (original / "sample.map").read_text().splitlines():
        words = line.split()
        if len(words) >= 3 and words[1] in (METHOD, "_entry", "?TestBias@@3HA"):
            addresses[words[1]] = int(words[2], 16)
    assert len(addresses) == 3
    spans = {}
    for name, unit in ((METHOD, "owner"), ("_entry", "entry")):
        obj = Coff((original / (unit + ".obj")).read_bytes())
        data, _, _ = obj.subject(obj.function(name))
        decoded = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(data, addresses[name]))
        returns = [ins for ins in decoded if ins.mnemonic == "ret"]
        assert len(returns) == 1 and not any(ins.mnemonic.startswith("j") for ins in decoded)
        end = returns[0].address + returns[0].size
        assert set(data[end - addresses[name]:]) <= {0x90, 0xCC}
        spans[name] = Span(addresses[name], end)
    initialize(root, "native-control", sys.executable, image=image)
    driver_path = local / "driver.json"
    driver_path.write_text(json.dumps({**json.loads(driver_path.read_text()), "acceptance_scope": "full_project"}))
    survey = survey_image(root, image)
    assert any(row["origin"]["address"] == addresses[METHOD] for row in survey["candidates"])
    (root / ".gitignore").write_text("build/\n__pycache__/\n")
    if normalized_checkout:
        (root / ".gitattributes").write_text("*.csv text=auto\n")
        (root / "notes.csv").write_bytes(b"owner,evidence\nOwner,test-owned reference\n")
    (root / "build").mkdir()
    (root / "build/compile_commands.json").write_text(json.dumps([
        {"directory": str(root), "file": "src/" + unit + ".cpp", "arguments": ["local-compiler"]}
        for unit in ("owner", "entry")]))
    command = lambda mode: {"argv": [*script, mode], "timeout": 300}
    inputs = ["recovery/environment.py", "recovery/compiler_support.py", "recovery/local_wine.py"]
    if generated_context is not None:
        inputs.append("recovery/generated.json")
    env = ["VC6_PREFIX", "VC6_ROOT", "WINEARCH", "CL", "_CL_", "VC6_INCLUDE", "VC6_LIB"]
    config_file = local / "project.json"
    config = json.loads(config_file.read_text())
    # Keep the actual starter's mappings policy; the generated database supplies
    # local translation units, not another handwritten source/gate catalogue.
    config.update(compiler={"command": command("compile"), "inputs": inputs, "environment": env},
                  measurement={**config["measurement"], "flag_sets": [list(FLAGS[:-1])], "layout_flags": ["/Z7"]},
                  acceptance={"checks": {name: command(name) for name in ("structure", "runtime")} if proofs else {},
                              "inputs": inputs, "environment": env})
    if preparation:
        config["preparation"] = "recovery/preparation.json"
        (local / "preparation.json").write_text(json.dumps({
            "schema_version": 1, "artifacts": "build/preparation", "input_roots": ["src"],
            "inputs": inputs, "environment": [],
            "commands": [{"argv": [*script, "prepare", "{root}", "{build}"], "timeout": 30}],
            "outputs": [{"path": "compile_commands.json", "format": "compilation-database"}]}))
    config_file.write_text(json.dumps(config))
    if provider is not None:
        (local / "agents.json").write_text(json.dumps(provider))
        instructions = (
            "This is a bounded test-owned C++ application, not Alpha Centauri. "
            "Recover the authorized Owner hierarchy and method from original evidence. "
            "The original reference COFF with CodeView class names/types is at " + str(original / "owner.obj") + ". "
            "It is a supplied seed/hint; the pinned executable remains the byte reference. "
            "Read it with match_recovery.pe_coff.Coff and match_recovery.codeview.class_layout. "
            "The Python interpreter with the package installed is " + sys.executable + ". "
            "Do not change source annotations, task decisions, compiler/build/proof policy or Git state. "
            "Do not start other agents or invoke recover run recursively. "
            "Return the required final JSON. Original names and declared member/base evidence "
            "are important; a flattened raw array is not an accepted model. "
        )
        (local / "worker.md").write_text(instructions + "Edit only src/owner.h and src/owner.cpp; preserve structural progress on a byte miss.\n")
        (local / "reviewer.md").write_text(instructions + "Remain read-only. Inspect source and original evidence independently; approval is not acceptance.\n")
        (local / "coordinator.md").write_text(instructions +
            "Remain read-only. Inspect each discovery and source evidence. Propose expansion only within the delegated originals, "
            "then incorporation only once the insight is expressed in source. Never waive proof or declare acceptance.\n")
    (local / "build.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/products",
        "commands": [{"argv": [*script, "build-" + damage + "-damage" if damage else "build", "{root}", "{build}"], "timeout": 90}],
        "outputs": [{"path": "sample.exe", "format": "pe32-i386"}, {"path": "owner.obj", "format": "file"}],
        "inputs": inputs, "input_roots": ["src"], "environment": env}))
    def git(*args):
        return subprocess.run(["git", *args], cwd=root, capture_output=True, check=True).stdout.decode().strip()
    git("init")
    git("config", "user.name", "Native acceptance fixture")
    git("config", "user.email", "fixture@example.invalid")
    git("add", ".")
    git("commit", "-m", "Test-owned reference configuration")
    config["source_history"] = {"baseline_revision": git("rev-parse", "HEAD"),
                                "evidence": ["test-owned source before first original mapping"]}
    config_file.write_text(json.dumps(config))
    # Seed the mistaken but byte-identical model while preserving independently
    # established original structure evidence as source-owned requirements.
    evidence = (f"// RECOVERY_BASE: v1 Owner Base 0x0 {addresses[METHOD]:#x}\n"
                f"// RECOVERY_MEMBER: v2 Owner member Member 0x4 {addresses[METHOD]:#x}\n")
    (product / "owner.h").write_text(evidence + FLAT)
    (product / "owner.cpp").write_text(BODY.replace("bias + member.value", "raw_[0] + raw_[1]"))
    entry = entry.replace('extern "C" __declspec',
                          '// RECOVERY_IMPORT: v1 __imp__ExitProcess@4 KERNEL32.dll name:ExitProcess\nextern "C" __declspec')
    entry = entry.replace("int TestBias = 3;", f"// RECOVERY_OBJECT: v2 {addresses['?TestBias@@3HA']:#x} "
                         f"{addresses['?TestBias@@3HA'] + 4:#x} ?TestBias@@3HA {addresses['_entry']:#x}\nint TestBias = 3;")
    (product / "entry.cpp").write_text(entry)
    for name, unit, definition in ((METHOD, "owner", "int Owner::add"), ("_entry", "entry", 'extern "C" void entry')):
        path = product / (unit + ".cpp")
        line = next(i for i, text in enumerate(path.read_text().splitlines(), 1) if text.startswith(definition))
        bind(root, image=image, address=addresses[name], name=name, spans=(spans[name],), source="src/" + unit + ".cpp",
             line=line, source_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
             reason="Test-owned original MAP entry and straight-line reference return establish this extent.",
             evidence=["test-owned original source, COFF and MAP"])
    git("add", ".")
    git("commit", "-m", "Bind source-owned original evidence and flat candidate")
    if preparation:
        from match_recovery.preparation import prepare_project
        assert prepare_project(root)["published"]
    code, report = record(root, [hex(addresses["_entry"])] if seed_owner else
                          [hex(addresses[METHOD]), hex(addresses["_entry"])], image=image)
    assert code == 0, report
    git("add", "src")
    git("commit", "-m", "Measure original byte obligations before structural recovery")
    if seed_owner:
        config["source"]["seed_roots"] = ["src/seeds"]
        config_file.write_text(json.dumps(config))
        (product / "seeds").mkdir()
        # Test-owned seed has no matching claim. Product code starts unbound;
        # the delegated coordinator must use the real writer after worker repair.
        (product / "seeds/owner.cpp").write_bytes((product / "owner.cpp").read_bytes())
        (product / "owner.cpp").write_text(BODY.replace("bias + member.value", "raw_[0] + raw_[1]"))
        git("add", ".")
        git("commit", "-m", "Supply an unclaimed seed and unbound product body")
    task = Task("owner", "native-control", "coordinator", "Recover real base and member while preserving bytes",
                git("rev-parse", "HEAD"), (Origin(pin, "pe:va", addresses[METHOD]),),
                ("src/owner.cpp", "src/owner.h"), ("test-owned original class and lifetime model",))
    if authorize:
        TaskStore(root).create(task)
    if provider is not None:
        (local / "coordinator.json").write_text(json.dumps({
            "schema_version": 1, "author": task.author,
            "command": {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "coordinator"], "timeout": 300},
            "subjects": [Origin(pin, "pe:va", addresses["_entry"]).entity_id], "editable_files": [],
            "evidence": ["Authorize the independently mapped caller's verification scope; existing owner editing files remain unchanged."],
            "max_decisions": 6 if seed_owner else 4,
            **({"seed_bindings": {"image": str(image)}} if seed_owner else {})}))
    git("add", "recovery")
    git("commit", "-m", "Authorize ownership task")
    if normalized_checkout:
        notes = root / "notes.csv"
        notes.write_bytes(notes.read_bytes().replace(b"\n", b"\r\n"))
        assert not git("diff", "--name-only", "HEAD")
    return root, image, task, git, evidence


@pytest.mark.parametrize("publish", [False, True, "resources"])
def test_real_change_acceptance_does_not_require_application_proof_capabilities(tmp_path, monkeypatch, publish):
    provider = {"schema_version": 1, "artifacts": "build/agents"}
    for role in ("worker", "coordinator", "reviewer"):
        provider[role] = {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                          "instructions": [], "response": "stdout"}
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, provider=provider, proofs=False,
                                                       normalized_checkout=publish)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    head = git("rev-parse", "HEAD")
    options = []
    if publish:
        notes_before = (root / "notes.csv").read_bytes()
        notes_blob = git("rev-parse", "HEAD:notes.csv")
        authorization = tmp_path / "checkpoint.json"
        authorization.write_text(json.dumps({"author": task.author, "message": "Recover real Base and Member ownership",
                                             "allow_migration": False}))
        options = ["--checkpoint", str(authorization)]
    if publish == "resources":
        prefix = Path(os.environ.get("VC6_PREFIX", VC6_ROOT / ".wineprefix")).resolve()
        resources = tmp_path / "resources.json"
        resources.write_text(json.dumps({"schema_version": 1, "slots": [
            {"name": "test-compiler", "directory": str(prefix), "environment": {"VC6_PREFIX": "{resource}"}}
        ]}))
        options += ["--resources", str(resources)]
    result = subprocess.run([sys.executable, "-m", "match_recovery.cli", "run", task.task_id,
                             "--root", str(root), "--acceptance-scope", "change", "--iterations", "2",
                             "--reviewer", "recovery/reviewer.json", "--coordinator", "recovery/coordinator.json", *options],
                            cwd=tmp_path, capture_output=True, timeout=180)
    assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
    report = json.loads(result.stdout)
    assert report["stop_reason"] == "change_accepted", report
    assert report["ready_for_checkpoint"] and not report["ready_for_collection"]
    assert report["migration"] == {"exit_code": 0, "failed": [], "unknown": [], "tracked": []}
    assert "acceptance" not in report
    assert report["worker_rounds"] == 2 and report["coordinator_decisions"] == 2
    if publish == "resources":
        from match_recovery.resources import ACTIVE, LOCK
        assert (prefix / LOCK).is_file() and not (prefix / ACTIVE).exists()
    assert (root / "src/owner.h").read_text() == evidence + CLASSES
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    if publish:
        assert report["checkpoint"]["published"]
        assert report["checkpoint"]["parent"] == head
        assert git("rev-parse", "HEAD") == report["checkpoint"]["commit"]
        assert git("show", "HEAD:src/owner.h") + "\n" == evidence + CLASSES
        assert "notes.csv" not in report["checkpoint"]["files"]
        assert (root / "notes.csv").read_bytes() == notes_before
        assert git("rev-parse", "HEAD:notes.csv") == notes_blob
        assert not git("status", "--porcelain")
    else:
        assert git("rev-parse", "HEAD") == head


def test_real_preservation_refusal_drives_scoped_repair_without_reverting_structure(tmp_path, monkeypatch):
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, proofs=False)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    (root / "src/owner.h").write_text(evidence + CLASSES)
    body = root / "src/owner.cpp"
    restored = body.read_text().replace("raw_[0] + raw_[1]", "bias + member.value")
    helper = "\nint accidental_unmapped_helper() { return 7; }\n"
    body.write_text(restored + helper)
    config = json.loads((root / "recovery/driver.json").read_text())
    snapshots, feedback = [], []
    def execute(command, root, name, payload):
        if command.argv[0] == "test-worker":
            feedback.append(payload["checkpoint_feedback"])
            assert "accidental_unmapped_helper" in str(feedback[-1]["report"]["diagnostics"])
            assert (root / "src/owner.h").read_text() == evidence + CLASSES
            body.write_text(body.read_text().replace(helper, ""))
            return 0, {"discoveries": []}
        if command.argv[0] == "test-reviewer":
            snapshots.append(payload["snapshot"])
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["test-owned Base/Member source declarations and original reference"],
                       "feedback": "Check compiler preservation independently; keep the recovered hierarchy."}
        return invoke(command, root, name, payload)
    head = git("rev-parse", "HEAD")
    verified = drive(root, task.task_id, {**config, "acceptance_scope": "change"},
                     {"author": "worker", "command": {"argv": ["test-worker"], "timeout": 30}}, iterations=2,
                     reviewer_configuration={"author": "reviewer", "command": {"argv": ["test-reviewer"], "timeout": 30}},
                     verify_only=True, execute=execute)
    assert verified["stop_reason"] == "change_fail" and verified["verification_only"]
    assert verified["worker_rounds"] == 0 and not feedback and len(snapshots) == 1
    assert body.read_text() == restored + helper and (root / "src/owner.h").read_text() == evidence + CLASSES
    assert git("rev-parse", "HEAD") == head
    report = drive(root, task.task_id, {**config, "acceptance_scope": "change"},
                   {"author": "worker", "command": {"argv": ["test-worker"], "timeout": 30}}, iterations=2,
                   reviewer_configuration={"author": "reviewer", "command": {"argv": ["test-reviewer"], "timeout": 30}},
                   execute=execute)
    assert report["stop_reason"] == "change_accepted", report
    assert report["worker_rounds"] == 1 and len(feedback) == 1 and len(set(snapshots)) == 3
    assert feedback[0]["report"]["outside_scope"] == "fail"
    assert body.read_text() == restored and (root / "src/owner.h").read_text() == evidence + CLASSES
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    assert git("rev-parse", "HEAD") == head


@pytest.mark.parametrize("mode", ["existing", "isolated", "destination_failure"])
def test_installed_integration_recheck_uses_destination_without_resetting_task_history(tmp_path, monkeypatch, mode):
    isolated = mode != "existing"
    provider = {"schema_version": 1, "artifacts": "build/agents"}
    for role in ("worker", "coordinator", "reviewer"):
        provider[role] = {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                          "instructions": [], "response": "stdout"}
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, provider=provider, proofs=False,
                                                       preparation=isolated, normalized_checkout=isolated)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    fork = git("rev-parse", "HEAD")
    git("switch", "-c", "incoming")
    (root / "src/owner.h").write_text(evidence + CLASSES)
    body = root / "src/owner.cpp"
    body.write_text(body.read_text().replace("raw_[0] + raw_[1]", "bias + member.value"))
    git("add", "src/owner.h", "src/owner.cpp")
    git("commit", "-m", "Incoming real hierarchy; verification still required")
    source = git("rev-parse", "HEAD")
    git("switch", "-c", "destination", fork)
    entry = root / "src/entry.cpp"
    entry.write_text(entry.read_text() + "\nint independent_destination_helper() { return 7; }\n")
    build_policy = root / "recovery/build.json"
    build_policy.write_text(build_policy.read_text() + "\n")
    git("add", "src/entry.cpp", "recovery/build.json")
    git("commit", "-m", "Independent destination source and build-policy input")
    target = git("rev-parse", "HEAD")
    context = {"author": task.author, "source_revision": source, "target_revision": target,
               "evidence": ["Test-owned incoming hierarchy and separately advanced destination"]}
    integration = tmp_path / "integration.json"
    integration.write_text(json.dumps(context))
    if isolated:
        destination = root
        root = tmp_path / "isolated-candidate"
        result = subprocess.run([sys.executable, "-m", "match_recovery.cli", "integrate", "prepare", task.task_id,
                                 "--root", str(destination), "--context", str(integration), "--worktree", str(root),
                                 "--branch", "integration-candidate"], cwd=tmp_path, capture_output=True, timeout=60)
        assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
        prepared = json.loads(result.stdout)
        candidate = prepared["commit"]
        assert prepared["candidate_created"] and prepared["verification"] == "not_measured"
        assert git("rev-parse", "HEAD") == target and not git("diff", "--name-only", "HEAD")
        assert not (root / "build/compile_commands.json").exists()
        generated = subprocess.run([sys.executable, "-m", "match_recovery.cli", "prepare", "--root", str(root)],
                                   cwd=tmp_path, capture_output=True, timeout=60)
        assert generated.returncode == 0, generated.stdout.decode() + generated.stderr.decode()
        assert json.loads(generated.stdout)["published"]
        assert all(str(root) in row["directory"] for row in json.loads((root / "build/compile_commands.json").read_text()))
    else:
        git("switch", "-c", "integration-candidate")
        git("merge", "--no-ff", source, "-m", "Unverified candidate; destination remains unchanged")
        candidate = git("rev-parse", "HEAD")
    from match_recovery.preservation import check_preservation
    outside = check_preservation(root, task.task_id)
    assert outside["result"] == "fail"
    if isolated:
        # Fresh recipes are regenerated independently for historical/current
        # source and each damage control, never copied from today's cache.
        preservation = check_preservation(root, task.task_id, integration=context)
        assert preservation["result"] == "pass", preservation
        assert preservation["materialization"] == "fresh-prepared-project"
        assert preservation["prepared_contexts"] >= 4
        assert all(row["result"] == "pass" for control in preservation["controls"] for row in control["files"])
        assert all(row["compiled_comparison"] == "pass" for row in preservation["units"])
    checkpoint = tmp_path / "checkpoint.json"
    checkpoint.write_text(json.dumps({"author": task.author, "message": "Verify merged hierarchy in destination context",
                                      "allow_migration": False}))
    result = subprocess.run([sys.executable, "-m", "match_recovery.cli", "run", task.task_id, "--root", str(root),
                             "--acceptance-scope", "change", "--iterations", "1", "--reviewer", "recovery/reviewer.json",
                             "--integration", str(integration), "--checkpoint", str(checkpoint), "--verify-only"],
                            cwd=tmp_path, capture_output=True, timeout=180)
    assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
    report = json.loads(result.stdout)
    assert report["verification_only"] and report["worker_rounds"] == 0
    if isolated:
        assert report["stop_reason"] == "change_accepted" and report["checkpoint"]["published"]
        assert report["checkpoint"]["parent"] == candidate and not report["ready_for_collection"]
        assert TaskStore(root).read(task.task_id) == task
        assert (root / "src/owner.h").read_text() == evidence + CLASSES
        assert git("rev-parse", "HEAD") == target and not git("diff", "--name-only", "HEAD")
        assert subprocess.run(["git", "rev-parse", "HEAD"], cwd=root, capture_output=True, check=True).stdout.decode().strip() == report["checkpoint"]["commit"]
        assert report["integration"] == context and git("rev-parse", "incoming") == source
        # Collection must reverify rather than accepting the report above.
        notes = (destination / "notes.csv").read_bytes()
        publication = tmp_path / "collection.json"
        publication.write_text(json.dumps({"author": task.author, "candidate_revision": report["checkpoint"]["commit"],
                                            "destination_branch": "refs/heads/destination", "integration": context,
                                            "message": "Collect verified test-owned hierarchy", "allow_migration": False}))
        if mode == "destination_failure":
            (destination / "build/unavailable-toolchain").write_text("test-owned unavailable build resource\n")
        collected = subprocess.run([sys.executable, "-m", "match_recovery.cli", "integrate", "publish", task.task_id,
                                    "--root", str(destination), "--candidate", str(root), "--authorization", str(publication),
                                    "--reviewer", "recovery/reviewer.json"], cwd=tmp_path, capture_output=True, timeout=180)
        assert collected.returncode == (3 if mode == "destination_failure" else 0), collected.stdout.decode() + collected.stderr.decode()
        final = json.loads(collected.stdout)
        if mode == "destination_failure":
            assert final["stop_reason"] == "destination_verification_required" and final["destination_updated"]
            assert not final["destination_verified"] and not final["ready_for_collection"]
            assert final["candidate_verification"]["stop_reason"] == "change_accepted"
            assert final["destination_verification"]["check"]["build_check"]["result"] == "fail"
            assert git("rev-parse", "HEAD") == final["verified_candidate_commit"]
            assert (destination / "src/owner.h").read_text() == evidence + CLASSES
            assert TaskStore(destination).read(task.task_id) == task
            (destination / "build/unavailable-toolchain").unlink()
            resumed = subprocess.run([sys.executable, "-m", "match_recovery.cli", "run", task.task_id,
                                      "--root", str(destination), "--acceptance-scope", "change", "--verify-only",
                                      "--reviewer", "recovery/reviewer.json", "--integration", str(integration),
                                      "--checkpoint", str(checkpoint)], cwd=tmp_path, capture_output=True, timeout=180)
            assert resumed.returncode == 0, resumed.stdout.decode() + resumed.stderr.decode()
            assert json.loads(resumed.stdout)["stop_reason"] == "change_accepted"
            assert TaskStore(destination).read(task.task_id) == task
            return  # A fresh retry, never rollback or replay of the failed report.
        assert final["stop_reason"] == "destination_accepted" and final["destination_updated"] and final["destination_verified"]
        assert not final["ready_for_collection"] and git("rev-parse", "HEAD") == final["final_commit"]
        for key in ("candidate_verification", "destination_verification"):
            assert final[key]["stop_reason"] == "change_accepted" and final[key]["worker_rounds"] == 0
            assert final[key]["verification_only"] and final[key]["migration"]["exit_code"] == 0
        assert TaskStore(destination).read(task.task_id) == task and TaskStore(root).read(task.task_id) == task
        assert (destination / "src/owner.h").read_text() == evidence + CLASSES
        assert (destination / "notes.csv").read_bytes() == notes
        assert "return 7;" in (destination / "src/entry.cpp").read_text()
        assert git("rev-parse", "incoming") == source and not git("diff", "--name-only", "HEAD")
        assert not git("ls-files", "--others", "--exclude-standard")
        assert all(row["directory"] == str(destination) for row in json.loads((destination / "build/compile_commands.json").read_text()))
        return  # Fresh candidate and destination checks, not game/provider acceptance.
    assert report["stop_reason"] == "change_accepted" and report["worker_rounds"] == 0
    assert report["integration"] == context and report["checkpoint"]["published"]
    assert report["checkpoint"]["parent"] == candidate and not report["ready_for_collection"]
    assert TaskStore(root).read(task.task_id) == task
    assert git("rev-parse", "destination") == target and git("rev-parse", "incoming") == source
    assert (root / "src/owner.h").read_text() == evidence + CLASSES
    assert "return 7;" in (root / "src/entry.cpp").read_text() and not git("status", "--porcelain")


@pytest.mark.parametrize("context", ["stable", "collateral", "stale_cache"])
def test_regenerated_header_preservation_detects_source_dependent_collateral_and_stale_cache(tmp_path, monkeypatch, context):
    from match_recovery.preparation import prepare_project
    from match_recovery.preservation import check_preservation
    from match_recovery.driver import working_state
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, proofs=False,
                                                       preparation=True, generated_context=context)
    (root / "src/owner.h").write_text(evidence + CLASSES)
    body = root / "src/owner.cpp"
    body.write_text(body.read_text().replace("raw_[0] + raw_[1]", "bias + member.value"))
    prepared = prepare_project(root)
    assert prepared["published"]
    if context == "stale_cache":
        (root / prepared["directory"] / "generated.h").write_text("#define GENERATED_SENTINEL 9\n")
    before = working_state(root)
    report = check_preservation(root, task.task_id)
    assert report["result"] == {"stable": "pass", "collateral": "fail", "stale_cache": "unknown"}[context], report
    assert report["changed_files"] == ["src/owner.cpp", "src/owner.h"]
    assert report["prepared_contexts"] >= 4
    assert all(row["result"] == "pass" for control in report["controls"] for row in control["files"])
    if context == "collateral":
        assert any(row["result"] == "fail" and "GeneratedSentinel" in str(row["changed_sections"]) for row in report["units"])
    elif context == "stale_cache":
        assert any("current replica does not agree" in row.get("reason", "") for row in report["units"])
    assert working_state(root) == before and TaskStore(root).read(task.task_id) == task
    assert (root / "src/owner.h").read_text() == evidence + CLASSES


@pytest.mark.parametrize("damage,investigate,seed_owner", [(None, False, False), ("runtime", False, False),
                                                         ("structure", False, False), (None, True, False), (None, True, True)])
def test_real_native_acceptance_preserves_structure_through_byte_regression(tmp_path, monkeypatch, damage, investigate, seed_owner):
    provider = {"schema_version": 1, "artifacts": "build/agents"}
    for role in ("worker", "coordinator", "reviewer"):
        provider[role] = {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                          "instructions": [], "response": "stdout"}
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, damage=damage, provider=provider, seed_owner=seed_owner)
    product, local, original = root / "src", root / "recovery", image.parent
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    configuration = lambda name: json.loads((local / (name + ".json")).read_text())
    store = TaskStore(root)
    if investigate:
        store.discover(task.task_id, request_id="owner-model", author="coordinator",
                       hypothesis="The byte-identical flat array must become the original Base and Member model",
                       affects=[task.subjects[0].entity_id], evidence=["Source-owned base/member original evidence"],
                       expected_revision=0)
        git("add", "recovery")
        git("commit", "-m", "Preserve unresolved structural finding before worker dispatch")
    head = git("rev-parse", "HEAD")
    result = drive(root, task.task_id, configuration("driver"), configuration("worker"), iterations=2,
                   reviewer_configuration=configuration("reviewer"), coordinator_configuration=configuration("coordinator"))
    (root / "build/driver-result.json").write_text(json.dumps(result))
    assert result["ready_for_collection"] == (damage is None), result
    assert result["worker_rounds"] == 2 and result["coordinator_decisions"] == (5 if seed_owner else 4 if investigate else 2)
    if investigate:
        grants = json.loads((root / "build/investigation-worker.json").read_text())
        assert len(grants) == 1 and grants[0]["action"] == "investigate" and not grants[0]["published"]
        assert not store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
        # Investigation is a transient worker permission, never a journal resolution.
        decisions = store.decisions(task.task_id)
        assert [item["kind"] for item in decisions].count("discovery_resolved") == 2
        assert all(item["kind"] != "investigate" for item in decisions)
    outcomes = [json.loads((root / f"build/worker-{step}.json").read_text()) for step in (1, 2)]
    assert outcomes[0]["candidate"]["checks"]["binary"]["result"] == ("unknown" if seed_owner else "pass")
    assert outcomes[0]["candidate"]["checks"]["layout"]["result"] == "fail"
    assert outcomes[1]["candidate"]["checks"]["binary"]["result"] == "fail"
    assert outcomes[1]["candidate"]["checks"]["layout"]["result"] == "pass"
    receipt = result["acceptance"]
    assert len(receipt["artifacts"]) == 2
    for name, fingerprint in receipt["artifacts"].items():
        assert hashlib.sha256((root / name).read_bytes()).hexdigest() == fingerprint
    artifact, = [root / name for name in receipt["artifacts"] if name.endswith("/owner.obj")]
    obj = Coff(artifact.read_bytes())
    original_obj = Coff((original / "owner.obj").read_bytes())
    # The damaged structural build really does preserve the method's bytes;
    # rejecting it must come from structure evidence, not a convenient mismatch.
    assert obj.subject(obj.function(METHOD))[0] == original_obj.subject(original_obj.function(METHOD))[0]
    assert receipt["build"] == receipt["references"] == receipt["obligations"] == "pass"
    assert receipt["runtime"] == ("fail" if damage == "runtime" else "pass")
    assert receipt["structure"] == ("fail" if damage == "structure" else "pass")
    if damage is not None:
        assert result["stop_reason"] == "acceptance_fail"
    if damage == "runtime":
        assert "retained candidate exit=13" in receipt["findings"][0]["evidence"][0]
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    assert git("rev-parse", "HEAD") == head
    assert (product / "owner.h").read_text() == evidence + CLASSES
    if seed_owner:
        records = [row for row in source_snapshot(root)["records"] if row["origin"]["address"] == task.subjects[0].address]
        assert {row["path"] for row in records} == {"src/owner.cpp", "src/seeds/owner.cpp"}
        assert all(not row["legacy_byte_exact"] for row in records)
        assert (product / "seeds/owner.cpp").read_text() == git("show", "HEAD:src/seeds/owner.cpp") + "\n"
        assert "binding_evidence" in next(row for row in records if row["path"] == "src/owner.cpp")
    assert git("diff", "--name-only", "HEAD").splitlines() == ["src/owner.cpp", "src/owner.h"]


def test_native_acceptance_resumes_uncommitted_scope_decisions_and_rechecks_review(tmp_path, monkeypatch):
    provider = {"schema_version": 1, "artifacts": "build/agents"}
    for role in ("worker", "coordinator", "reviewer"):
        provider[role] = {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                          "instructions": [], "response": "stdout"}
    root, image, task, git, evidence = prepared_project(tmp_path, monkeypatch, provider=provider)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    configuration = lambda name: json.loads((root / "recovery" / (name + ".json")).read_text())
    head, index = git("rev-parse", "HEAD"), git("ls-files", "--stage")
    # One decision per invocation forces a real stop between native expansion
    # and incorporation, while the good hierarchy still has mismatching bytes.
    delegation = {**configuration("coordinator"), "max_decisions": 1}
    def attempt():
        return drive(root, task.task_id, configuration("driver"), configuration("worker"), iterations=1,
                     reviewer_configuration=configuration("reviewer"), coordinator_configuration=delegation)
    first = attempt()
    assert first["stop_reason"] == "coordination_budget_exhausted", first
    assert first["worker_rounds"] == first["coordinator_decisions"] == 1
    assert not first["ready_for_collection"]
    assert (root / "src/owner.h").read_text() == evidence + CLASSES
    assert "- input" in (root / "src/owner.cpp").read_text()
    store = TaskStore(root)
    assert [item["kind"] for item in store.decisions(task.task_id)] == ["discovery", "scope_expanded"]
    before = {path: path.read_bytes() for path in (root / "recovery/history").rglob("*.json")}
    second = attempt()
    assert second["ready_for_collection"], second
    assert second["worker_rounds"] == second["coordinator_decisions"] == 1
    assert all(path.read_bytes() == contents for path, contents in before.items())
    assert [item["kind"] for item in store.decisions(task.task_id)] == ["discovery", "scope_expanded", "discovery_resolved"]
    # A stored review can be reused when fresh; it is never a cached acceptance.
    before = {path: path.read_bytes() for path in (root / "recovery/history").rglob("*.json")}
    third = attempt()
    assert third["ready_for_collection"] and third["worker_rounds"] == third["coordinator_decisions"] == 0, third
    assert {path: path.read_bytes() for path in (root / "recovery/history").rglob("*.json")} == before
    assert second["acceptance"]["request_fingerprint"] != third["acceptance"]["request_fingerprint"]
    assert second["acceptance"]["artifacts"].keys().isdisjoint(third["acceptance"]["artifacts"])
    for report in (second, third):
        assert all(report["acceptance"][key] == "pass" for key in (
            "build", "structure", "references", "obligations", "runtime"))
        for name, fingerprint in report["acceptance"]["artifacts"].items():
            assert hashlib.sha256((root / name).read_bytes()).hexdigest() == fingerprint
    assert store.read(task.task_id) == task
    assert git("rev-parse", "HEAD") == head and git("ls-files", "--stage") == index


@pytest.mark.parametrize("publish", [False, True])
def test_installed_start_selects_checkpoints_recovers_and_accepts_without_a_precreated_task(tmp_path, monkeypatch, publish):
    provider = {"schema_version": 1, "artifacts": "build/agents"}
    for role in ("planner", "worker", "coordinator", "reviewer"):
        provider[role] = {"command": {"argv": [sys.executable, "recovery/local_agents.py", role, "{request}"], "timeout": 30},
                          "instructions": [], "response": "stdout"}
    root, image, _, git, evidence = prepared_project(tmp_path, monkeypatch, provider=provider, authorize=False)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    (root / "recovery/planner.json").write_text(json.dumps({
        "schema_version": 1, "author": "coordinator",
        "command": {"argv": [sys.executable, "-m", "match_recovery.cli", "agent", "planner"], "timeout": 60},
        "max_entities": 4, "max_editable_files": 4, "commit_decision": True,
        "evidence": ["Authorize selection of test-owned application ownership groups and task-only checkpoint commits"]}))
    git("add", "recovery/planner.json")
    git("commit", "-m", "Configure planner without preselecting a task")
    baseline = git("rev-parse", "HEAD")
    assert not list((root / "recovery/tasks").glob("*.json"))
    command = [sys.executable, "-m", "match_recovery.cli", "start", "--root", str(root),
               "--planner", "recovery/planner.json", "--reviewer", "recovery/reviewer.json",
               "--coordinator", "recovery/coordinator.json", "--iterations", "2"]
    if publish:
        permission = tmp_path / "checkpoint.json"
        permission.write_text(json.dumps({"author": "coordinator", "message": "Recover owner from planned task",
                                         "allow_migration": False}))
        command.extend(["--acceptance-scope", "change", "--checkpoint", str(permission)])
    def attempt():
        result = subprocess.run(command, cwd=tmp_path, capture_output=True, timeout=180)
        assert result.returncode == 0, result.stdout.decode() + result.stderr.decode()
        return json.loads(result.stdout)
    report = attempt()
    assert report["ready_for_collection"] == (not publish) and report["selection"]["action"] == "create"
    assert report["worker_rounds"] == report["coordinator_decisions"] == 2
    task = TaskStore(root).read(report["task_id"])
    assert task.base_revision == baseline
    task_commit = report["selection"]["checkpoint"]
    assert git("rev-parse", task_commit + "^") == baseline
    assert git("diff-tree", "--no-commit-id", "--name-only", "-r", task_commit) == "recovery/tasks/" + task.task_id + ".json"
    if publish:
        assert report["stop_reason"] == "change_accepted" and report["checkpoint"]["published"]
        assert report["checkpoint"]["parent"] == task_commit
        assert not git("status", "--porcelain")
    assert (root / "src/owner.h").read_text() == evidence + CLASSES
    checkpoint = git("rev-parse", "HEAD")
    resumed = attempt()
    assert resumed["selection"]["action"] == "resume" and resumed["selection"]["checkpoint"] is None
    assert resumed["worker_rounds"] == resumed["coordinator_decisions"] == 0
    assert resumed["ready_for_collection"] == (not publish) and git("rev-parse", "HEAD") == checkpoint
    if publish:
        assert resumed["stop_reason"] == "change_accepted"
        assert resumed["checkpoint"] == {"published": False, "reason": "no_changes"}
        assert TaskStore(root).read(task.task_id).base_revision == baseline
        assert len(list((root / "recovery/tasks").glob("*.json"))) == 1
        return  # This branch verifies per-change publication, not full acceptance.
    assert resumed["acceptance"]["request_fingerprint"] != report["acceptance"]["request_fingerprint"]
    assert len(list((root / "recovery/tasks").glob("*.json"))) == 1
    for result in (report, resumed):
        assert all(result["acceptance"][key] == "pass" for key in ("build", "structure", "references", "obligations", "runtime"))
        for name, fingerprint in result["acceptance"]["artifacts"].items():
            assert hashlib.sha256((root / name).read_bytes()).hexdigest() == fingerprint


@pytest.mark.skipif(os.environ.get("OPENSMACX_LIVE_AGENT_CONTROL") != "1", reason="opt-in live provider recovery and acceptance")
def test_live_worker_and_independent_reviewer_reach_native_acceptance(tmp_path, monkeypatch):
    # Deliberately reuse the project's existing provider settings and models.
    # Only instructions are replaced with this unrelated application's evidence.
    provider = json.loads((Path(__file__).resolve().parents[1] / "agents.json").read_text())
    provider["coordinator"] = json.loads(json.dumps(provider["reviewer"]))
    for role in ("worker", "reviewer", "coordinator"):
        provider[role]["instructions"] = ["recovery/" + role + ".md"]
    root, image, task, git, _ = prepared_project(tmp_path, monkeypatch, provider=provider)
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    print("Live native acceptance project: " + str(root), flush=True)
    local = root / "recovery"
    def configuration(name):
        return json.loads((local / (name + ".json")).read_text())
    # No execute override or fixture coordinator. All three agent roles and
    # native verification use the installed path, in ONE bounded driver run.
    head = git("rev-parse", "HEAD")
    report = drive(root, task.task_id, configuration("driver"), configuration("worker"),
                   iterations=3, reviewer_configuration=configuration("reviewer"),
                   coordinator_configuration=configuration("coordinator"))
    (root / "build/live-result.json").write_text(json.dumps(report, indent=2))
    assert report["ready_for_collection"], report
    assert 1 <= report["worker_rounds"] <= 3
    assert 2 <= report["coordinator_decisions"] <= 4
    assert all(report["acceptance"][key] == "pass" for key in (
        "build", "structure", "references", "obligations", "runtime"))
    for name, fingerprint in report["acceptance"]["artifacts"].items():
        assert hashlib.sha256((root / name).read_bytes()).hexdigest() == fingerprint
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    assert git("rev-parse", "HEAD") == head
    assert set(git("diff", "--name-only", "HEAD").splitlines()) <= {"src/owner.cpp", "src/owner.h"}
    store = TaskStore(root)
    known = {row["entity_id"] for row in source_snapshot(root)["records"]}
    assert not store.status(task.task_id, known)["pending_discoveries"]
    assert store.scope(task.task_id)[0] <= known
    assert store.scope(task.task_id)[1] == frozenset(("src/owner.cpp", "src/owner.h"))

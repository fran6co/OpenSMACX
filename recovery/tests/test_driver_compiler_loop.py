"""Real compiler/image/layout/runtime control for the generic recovery driver.

The worker and reviewer are deterministic test doubles, not a live-model pilot.
The tiny reference program is ours; no proprietary executable is used. Passing
this bounded fixture says nothing about full application/lifetime acceptance.
"""

import hashlib
from dataclasses import asdict
from pathlib import Path
import subprocess
import sys
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.application_layout import check_base, check_member
from match_recovery.binding import bind
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import class_layout
from match_recovery.driver import drive
from match_recovery.image_survey import survey_image
from match_recovery.layout_source import parse_bases, parse_members
from match_recovery.model import Dimension, Obligation, Origin, Span, digest
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.processes import run_bounded
from match_recovery.project import initialize
from match_recovery.tasks import Task, TaskStore
from recovery.environment.compile import compile_object
from recovery.environment.wine import VC6_ROOT, environment


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


def test_compiled_wrong_structure_exact_bytes_migration_and_final_handoff(tmp_path, monkeypatch):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    original = tmp_path / "original"
    root = tmp_path / "candidate"
    original.mkdir()
    root.mkdir()
    product = root / "src"
    product.mkdir()

    def compile_units(directory):
        return {name: Coff(compile_object(directory / name, command, FLAGS))
                for name in ("owner.cpp", "entry.cpp")}

    def build(directory, output):
        output.mkdir()
        for name in ("owner", "entry"):
            (output / (name + ".obj")).write_bytes(compile_object(directory / (name + ".cpp"), command, FLAGS))
        result = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo", "/nodefaultlib",
                              "/subsystem:console", "/entry:entry", "/incremental:no", "/opt:noref", "/fixed:no",
                              "/out:sample.exe", "/map:sample.map", "owner.obj", "entry.obj", "kernel32.lib"],
                             cwd=output, timeout=60)
        assert result.returncode == 0, (result.stdout + result.stderr).decode(errors="replace")
        return output / "sample.exe"

    def run(executable):
        # Background Wine services must not retain a captured diagnostic pipe.
        with tempfile.TemporaryFile() as diagnostics:
            result = subprocess.run(["wine32", str(executable)], env=environment(),
                                    stdout=diagnostics, stderr=diagnostics, timeout=30)
        return result.returncode

    for directory in (original, product):
        (directory / "owner.h").write_text(CLASSES)
        (directory / "owner.cpp").write_text(BODY)
        (directory / "entry.cpp").write_text(ENTRY)
    reference_exe = build(original, original / "build")
    reference_exit = run(reference_exe)
    assert reference_exit == 12
    image_bytes = reference_exe.read_bytes()
    pin = hashlib.sha256(image_bytes).hexdigest()
    image = PeImage(image_bytes, pin)
    objects = compile_units(original)
    addresses = {}
    for line in reference_exe.with_suffix(".map").read_text().splitlines():
        words = line.split()
        if len(words) >= 3 and words[1] in (METHOD, "_entry"):
            addresses[words[1]] = int(words[2], 16)
    assert set(addresses) == {METHOD, "_entry"}
    bindings = []
    for name, unit in ((METHOD, "owner.cpp"), ("_entry", "entry.cpp")):
        code, _, _ = objects[unit].subject(objects[unit].function(name))
        # Both known reference bodies are straight-line. Bound their return,
        # not the trailing COMDAT alignment or anything in the candidate.
        instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, addresses[name]))
        returns = [ins for ins in instructions if ins.mnemonic == "ret"]
        assert len(returns) == 1 and not any(ins.mnemonic.startswith("j") for ins in instructions)
        end = returns[0].address + returns[0].size
        assert set(code[end - addresses[name]:]) <= {0x90, 0xCC}
        span = Span(addresses[name], end)
        bindings.append(FunctionBinding(Origin(pin, "pe:va", span.start), (span,), name, unit))
    imports = {"__imp__ExitProcess@4": next(item.address for item in image.imports()
                                         if item.name == "ExitProcess")}
    assert all(row["result"] == "pass" for row in check_objects(bindings, objects, image, import_targets=imports))
    base, = parse_bases(f"// RECOVERY_BASE: v1 Owner Base 0x0 {addresses[METHOD]:#x}",
                        "owner.h", pin, address_space="pe:va")
    member, = parse_members(f"// RECOVERY_MEMBER: v2 Owner member Member 0x4 {addresses[METHOD]:#x}",
                            "owner.h", pin, address_space="pe:va")

    flat_body = BODY.replace("bias + member.value", "raw_[0] + raw_[1]")
    (product / "owner.h").write_text(FLAT)
    (product / "owner.cpp").write_text(flat_body)
    initialize(root, "controlled-cpp", sys.executable, image=reference_exe)
    (root / ".gitignore").write_text("build/\n")

    def git(*args):
        return subprocess.run(["git", *args], cwd=root, check=True, capture_output=True).stdout.decode().strip()
    git("init")
    git("config", "user.name", "Recovery fixture")
    git("config", "user.email", "fixture@example.invalid")
    git("add", ".")
    git("commit", "-m", "unmapped flat source candidate")
    survey = survey_image(root, reference_exe)
    assert any(row["origin"]["address"] == addresses[METHOD] for row in survey["candidates"])
    # The survey supplied a starting hint, not the extent: use independently
    # bounded reference code above, never candidate size or the survey's walk.
    bind(root, image=reference_exe, address=addresses[METHOD], name=METHOD, spans=bindings[0].spans,
         source="src/owner.cpp", line=2, source_sha256=hashlib.sha256((product / "owner.cpp").read_bytes()).hexdigest(),
         reason="Controlled reference MAP identifies entry; known straight-line reference body bounds exclude padding.",
         evidence=["test-owned original source/object/MAP", "source:src/owner.cpp"])
    bound_flat = (product / "owner.cpp").read_text()
    bound_real = bound_flat.replace("raw_[0] + raw_[1]", "bias + member.value")
    git("add", "src")
    git("commit", "-m", "bind independently bounded original method")
    task = Task("owner", "controlled-cpp", "coordinator", "Recover the real base and embedded member",
                git("rev-parse", "HEAD"), (bindings[0].origin,), ("src/owner.h", "src/owner.cpp"),
                ("Known reference class model; source markers retain the original method identity.",))
    TaskStore(root).create(task)
    git("add", "recovery")
    git("commit", "-m", "authorize ownership group")

    def observe():
        compiled = compile_units(product)
        rows = check_objects(bindings, compiled, image, import_targets=imports)
        layout = class_layout(compiled["owner.cpp"], "Owner")
        base_result, member_result = check_base(base, layout), check_member(member, layout)
        return rows, base_result, member_result

    calls, observations = [], []
    obligation = Obligation(bindings[0].origin.entity_id, Dimension.BYTES,
                            "test-owned reference:" + hashlib.sha256(reference_exe.read_bytes()).hexdigest())
    def execute(cmd, directory, name, payload):
        stage = cmd.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {"evidence": task.evidence}
        if stage == "obligations":
            return 0, {"obligations": [{**asdict(obligation), "obligation_id": obligation.obligation_id}]}
        if stage == "check":
            rows, base_row, member_row = observe()
            observations.append((rows[0]["result"], base_row["result"], member_row["result"]))
            good = all(row["result"] == "pass" for row in [*rows, base_row, member_row])
            return (3 if good else 1), {"result": "unknown" if good else "fail",
                                         "next_action": "review_structure" if good else "recover"}
        if stage == "worker":
            if payload["round"] == 1:
                assert observations[-1][0:2] == ("pass", "fail")
                (product / "owner.h").write_text(CLASSES)
                (product / "owner.cpp").write_text(bound_real.replace("+ input", "- input"))
            else:
                assert observations[-1] == ("fail", "pass", "pass")
                assert (product / "owner.h").read_text() == CLASSES  # No automatic structural rollback.
                (product / "owner.cpp").write_text(bound_real)
            return 0, {"discoveries": []}
        if stage == "review":
            assert (product / "owner.h").read_text() == CLASSES
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["controlled reference declarations", "src/owner.h"],
                       "feedback": "The real Base and Member are restored; final verification must run."}
        assert stage == "acceptance"
        assert payload["protected"] == [{**asdict(obligation), "obligation_id": obligation.obligation_id}]
        def measurement_inputs():
            return digest({"source": {name: hashlib.sha256((product / name).read_bytes()).hexdigest()
                                      for name in ("owner.cpp", "owner.h", "entry.cpp")},
                           "image": hashlib.sha256(reference_exe.read_bytes()).hexdigest(), "flags": FLAGS,
                           "compiler": hashlib.sha256((VC6_ROOT / "BIN/CL.EXE").read_bytes()).hexdigest()})
        inputs = measurement_inputs()
        rows, base_row, member_row = observe()  # Fresh, not the earlier check's objects.
        assert all(row["result"] == "pass" for row in [*rows, base_row, member_row])
        candidate_exe = build(product, root / "build" / "acceptance")
        assert run(candidate_exe) == reference_exit
        assert measurement_inputs() == inputs
        return 0, {"scope": "full_project", "result": "pass", "build": "pass", "structure": "pass",
                   "references": "pass", "obligations": "pass", "runtime": "pass",
                   "request_fingerprint": digest(payload),
                   "measurements": [{"obligation_id": obligation.obligation_id, "fingerprint": inputs,
                                     "result": rows[0]["result"],
                                     "evidence": "fresh strict comparison against the test-owned original image"}],
                   "current": {obligation.obligation_id: inputs}}

    configured = lambda name: {"argv": [name], "timeout": 60}
    result = drive(root, task.task_id, {"schema_version": 1, "target": task.target,
                                      **{name: configured(name) for name in ("context", "check", "obligations", "acceptance")}},
                   {"author": "fixture-worker", "command": configured("worker")}, iterations=3,
                   reviewer_configuration={"author": "fixture-reviewer", "command": configured("review")}, execute=execute)
    assert result["ready_for_collection"] and result["worker_rounds"] == 2
    assert [name for name in calls if name != "obligations"] == [
        "context", "check", "worker", "context", "check", "worker", "context", "check", "review", "acceptance"]
    assert calls.count("obligations") == 4
    assert observations[-1] == ("pass", "pass", "pass")
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    assert git("diff", "--name-only", "HEAD").splitlines() == ["src/owner.cpp", "src/owner.h"]

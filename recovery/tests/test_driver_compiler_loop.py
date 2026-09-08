"""Real compiler/image/layout/runtime control for the generic recovery driver.

The worker and reviewer are deterministic test doubles, not a live-model pilot.
The tiny reference program is ours; no proprietary executable is used. Passing
this bounded fixture says nothing about full application/lifetime acceptance.
"""

import hashlib
from pathlib import Path
import subprocess
import sys
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.application_layout import check_base, check_member
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import class_layout
from match_recovery.driver import drive
from match_recovery.layout_source import parse_bases, parse_members
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.processes import run_bounded
from match_recovery.source.model import DecompilationState, Mode, State
from match_recovery.source.writer import write
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

    for directory in (original, root):
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
    (root / "owner.h").write_text(FLAT)
    record = DecompilationState(addresses[METHOD], Mode.BODY, State.IMPLEMENTED,
                                root / "owner.cpp", 2, METHOD,
                                ((bindings[0].spans[0].start, bindings[0].spans[0].end),))
    (root / "owner.cpp").write_text(write(flat_body, [record]))
    (root / ".gitignore").write_text("build/\n")

    def git(*args):
        return subprocess.run(["git", *args], cwd=root, check=True, capture_output=True).stdout.decode().strip()
    git("init")
    git("config", "user.name", "Recovery fixture")
    git("config", "user.email", "fixture@example.invalid")
    git("add", ".")
    git("commit", "-m", "flat byte-exact seed")
    task = Task("owner", "controlled-cpp", "coordinator", "Recover the real base and embedded member",
                git("rev-parse", "HEAD"), (bindings[0].origin,), ("owner.h", "owner.cpp"),
                ("Known reference class model; source markers retain the original method identity.",))
    TaskStore(root).create(task)
    git("add", "recovery")
    git("commit", "-m", "authorize ownership group")

    def observe():
        compiled = compile_units(root)
        rows = check_objects(bindings, compiled, image, import_targets=imports)
        layout = class_layout(compiled["owner.cpp"], "Owner")
        base_result, member_result = check_base(base, layout), check_member(member, layout)
        return rows, base_result, member_result

    calls, observations = [], []
    def execute(cmd, directory, name, payload):
        stage = cmd.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {"evidence": task.evidence}
        if stage == "check":
            rows, base_row, member_row = observe()
            observations.append((rows[0]["result"], base_row["result"], member_row["result"]))
            good = all(row["result"] == "pass" for row in [*rows, base_row, member_row])
            return (3 if good else 1), {"result": "unknown" if good else "fail",
                                         "next_action": "review_structure" if good else "recover"}
        if stage == "worker":
            if payload["round"] == 1:
                assert observations[-1][0:2] == ("pass", "fail")
                (root / "owner.h").write_text(CLASSES)
                (root / "owner.cpp").write_text(write(BODY.replace("+ input", "- input"), [record]))
            else:
                assert observations[-1] == ("fail", "pass", "pass")
                assert (root / "owner.h").read_text() == CLASSES  # No automatic structural rollback.
                (root / "owner.cpp").write_text(write(BODY, [record]))
            return 0, {"discoveries": []}
        if stage == "review":
            assert (root / "owner.h").read_text() == CLASSES
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["controlled reference declarations", "owner.h"],
                       "feedback": "The real Base and Member are restored; final verification must run."}
        assert stage == "acceptance"
        rows, base_row, member_row = observe()  # Fresh, not the earlier check's objects.
        assert all(row["result"] == "pass" for row in [*rows, base_row, member_row])
        candidate_exe = build(root, root / "build" / "acceptance")
        assert run(candidate_exe) == reference_exit
        return 0, {"scope": "full_project", "result": "pass", "build": "pass", "structure": "pass",
                   "references": "pass", "obligations": "pass", "runtime": "pass"}

    configured = lambda name: {"argv": [name], "timeout": 60}
    result = drive(root, task.task_id, {"schema_version": 1, "target": task.target,
                                      **{name: configured(name) for name in ("context", "check", "acceptance")}},
                   {"author": "fixture-worker", "command": configured("worker")}, iterations=3,
                   reviewer_configuration={"author": "fixture-reviewer", "command": configured("review")}, execute=execute)
    assert result["ready_for_collection"] and result["worker_rounds"] == 2
    assert calls == ["context", "check", "worker", "context", "check", "worker",
                     "context", "check", "review", "acceptance"]
    assert observations[-1] == ("pass", "pass", "pass")
    assert TaskStore(root).read(task.task_id).base_revision == task.base_revision
    assert git("diff", "--name-only", "HEAD").splitlines() == ["owner.cpp", "owner.h"]

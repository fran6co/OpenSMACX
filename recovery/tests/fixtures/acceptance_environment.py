"""Local VC6/compiler/build/proof boundary for a test-owned tiny application.

Copied into an unrelated temporary project. This is a bounded integration-test
oracle, not an application adapter or a universal structure/runtime verifier.
"""

import base64
import json
from pathlib import Path
import subprocess
import sys
import tempfile

from match_recovery.codeview import class_layout
from match_recovery.compiler import CompilationRequest
from match_recovery.model import digest
from match_recovery.pe_coff import Coff
from match_recovery.processes import run_bounded
from compiler_support import compile_object, compile_overlay
from local_wine import environment


FLAGS = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Oy-", "/Z7")
WRAPPER = Path(__file__).with_name("local_wine.py")
COMMAND = (sys.executable, str(WRAPPER), "--tool=cl", "/nologo")


def build(root, output, *, damage=None):
    flags = FLAGS + (("/DTEST_RUNTIME_DAMAGE",) if damage == "runtime" else ())
    replacements = None
    if damage == "structure":
        # Simulate a faulty build generator retaining a flattened model. Only
        # a private compiler overlay changes; the candidate source stays intact.
        header = (root / "src/owner.h").read_text().replace(
            "struct Owner : Base {\n    Member member;", "struct Owner {\n    int raw_[2];").replace(
            "bias = a; member.value = b;", "raw_[0] = a; raw_[1] = b;")
        body = (root / "src/owner.cpp").read_text().replace("bias + member.value", "raw_[0] + raw_[1]")
        replacements = {"src/owner.h": header.encode(), "src/owner.cpp": body.encode()}
    for name in ("owner", "entry"):
        source = root / "src" / (name + ".cpp")
        obj = (compile_overlay(root, source, replacements, COMMAND, flags) if replacements else
               compile_object(source, COMMAND, flags))
        (output / (name + ".obj")).write_bytes(obj)
    result = run_bounded([sys.executable, str(WRAPPER), "--tool=link", "/nologo", "/nodefaultlib",
                          "/subsystem:console", "/entry:entry", "/incremental:no", "/opt:noref", "/fixed:no",
                          "/out:sample.exe", "/map:sample.map", "owner.obj", "entry.obj", "kernel32.lib"],
                         cwd=output, timeout=60)
    if result.returncode:
        raise ValueError((result.stdout + result.stderr).decode(errors="replace"))


def run(executable):
    with tempfile.TemporaryFile() as diagnostics:
        return subprocess.run(["wine32", str(executable)], env=environment(),
                              stdout=diagnostics, stderr=diagnostics, timeout=30).returncode


def main():
    mode = sys.argv[1]
    root = Path.cwd()
    if mode in ("build", "build-runtime-damage", "build-structure-damage"):
        build(Path(sys.argv[2]), Path(sys.argv[3]), damage=mode.split("-")[1] if "-" in mode else None)
        return 0
    payload = json.load(sys.stdin)
    if mode == "compile":
        request = CompilationRequest.read(root, payload)
        flags = (*request.arguments, *request.flags)
        if request.tree_roots is not None:
            obj = compile_overlay(root, request.source, request.tree_files, COMMAND, flags,
                                  tree_roots=request.tree_roots)
        elif request.replacements is not None:
            obj = compile_overlay(root, request.source, request.replacements, COMMAND, flags)
        else:
            obj = compile_object(request.source, COMMAND, flags)
        print(json.dumps({"object_base64": base64.b64encode(obj).decode()}))
        return 0
    assert mode == payload["dimension"] and mode in ("structure", "runtime")
    directory = root / payload["build"]["directory"]
    if mode == "structure":
        layout = class_layout(Coff((directory / "owner.obj").read_bytes()), "Owner")
        # Independent reference declarations specify the complete tiny model.
        # Check actual emitted types from the SAME retained application build.
        good = (layout["size"] == 8 and layout["members"] == {"member": 4}
                and layout["member_types"].get("member") == {"kind": "class_value", "class": "Member"}
                and len(layout["bases"]) == 1 and layout["bases"][0]["class"] == "Base"
                and layout["bases"][0]["offset"] == 0 and not layout["bases"][0]["virtual"])
        evidence = ["test-owned reference model: Owner : Base with one Member; emitted " + json.dumps(layout)]
    else:
        original = run(Path(payload["image"]["path"]))
        candidate = run(directory / "sample.exe")
        good = original == candidate == 12
        evidence = [f"test-owned executable control: original exit={original}, retained candidate exit={candidate}"]
    result = "pass" if good else "fail"
    fingerprint = digest(payload)
    print(json.dumps({"request_fingerprint": fingerprint, "result": result, "evidence": evidence,
                      "measurements": [{"obligation_id": row["obligation_id"], "fingerprint": fingerprint,
                                        "result": result, "evidence": evidence[0]} for row in payload["protected"]],
                      "current": {row["obligation_id"]: fingerprint for row in payload["protected"]}}))
    return 0 if good else 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        print(json.dumps({"error": str(error)}))
        raise SystemExit(3)

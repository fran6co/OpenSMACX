"""Real native gate/restoration/preservation/build/runtime acceptance control.

Only the worker and independent reviewer are deterministic test doubles. The
test-owned reference is freshly compiled, linked and run; no proprietary fixture
or precomputed verifier response participates. Not a live-model/game pilot.
"""

import hashlib
import json
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


@pytest.mark.parametrize("damage", [None, "runtime", "structure"])
def test_real_native_acceptance_preserves_structure_through_byte_regression(tmp_path, monkeypatch, damage):
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
    survey = survey_image(root, image)
    assert any(row["origin"]["address"] == addresses[METHOD] for row in survey["candidates"])
    (root / ".gitignore").write_text("build/\n__pycache__/\n")
    (root / "build").mkdir()
    (root / "build/compile_commands.json").write_text(json.dumps([
        {"directory": str(root), "file": "src/" + unit + ".cpp", "arguments": ["local-compiler"]}
        for unit in ("owner", "entry")]))
    command = lambda mode: {"argv": [*script, mode], "timeout": 300}
    inputs = ["recovery/environment.py", "recovery/compiler_support.py", "recovery/local_wine.py"]
    env = ["VC6_PREFIX", "VC6_ROOT", "WINEARCH", "CL", "_CL_", "VC6_INCLUDE", "VC6_LIB"]
    config_file = local / "project.json"
    config = json.loads(config_file.read_text())
    # Keep the actual starter's mappings policy; the generated database supplies
    # local translation units, not another handwritten source/gate catalogue.
    config.update(compiler={"command": command("compile"), "inputs": inputs, "environment": env},
                  measurement={**config["measurement"], "flag_sets": [list(FLAGS[:-1])], "layout_flags": ["/Z7"]},
                  acceptance={"checks": {name: command(name) for name in ("structure", "runtime")},
                              "inputs": inputs, "environment": env})
    config_file.write_text(json.dumps(config))
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
    code, report = record(root, [hex(addresses[METHOD]), hex(addresses["_entry"])], image=image)
    assert code == 0, report
    git("add", "src")
    git("commit", "-m", "Measure original byte obligations before structural recovery")
    task = Task("owner", "native-control", "coordinator", "Recover real base and member while preserving bytes",
                git("rev-parse", "HEAD"), (Origin(pin, "pe:va", addresses[METHOD]),),
                ("src/owner.cpp", "src/owner.h"), ("test-owned original class and lifetime model",))
    TaskStore(root).create(task)
    git("add", "recovery")
    git("commit", "-m", "Authorize ownership task")
    flat = (product / "owner.cpp").read_text()
    real = flat.replace("raw_[0] + raw_[1]", "bias + member.value")
    config_driver = json.loads((local / "driver.json").read_text())
    monkeypatch.setenv("RECOVERY_IMAGE", str(image))
    outcomes, acceptance_reports = [], []
    def execute(cmd, directory, name, payload):
        if cmd.argv[0] == "worker":
            outcomes.append(payload["check"])
            if len(outcomes) > 1:
                assert (product / "owner.h").read_text() == evidence + CLASSES
            (product / "owner.h").write_text(evidence + CLASSES)
            (product / "owner.cpp").write_text(real.replace("+ input", "- input") if len(outcomes) == 1 else real)
            return 0, {"discoveries": []}
        if cmd.argv[0] == "review":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["test-owned original Base and Member"], "feedback": "Run native full acceptance"}
        code, report = invoke(cmd, directory, name, payload)
        if "check-acceptance" in cmd.argv:
            acceptance_reports.append(report)
        return code, report
    result = drive(root, task.task_id, config_driver,
                   {"author": "worker", "command": {"argv": ["worker"], "timeout": 30}}, iterations=2,
                   reviewer_configuration={"author": "reviewer", "command": {"argv": ["review"], "timeout": 30}},
                   execute=execute)
    assert result["ready_for_collection"] == (damage is None), result
    assert len(outcomes) == 2 and len(acceptance_reports) == 1
    assert outcomes[0]["candidate"]["checks"]["binary"]["result"] == "pass"
    assert outcomes[0]["candidate"]["checks"]["layout"]["result"] == "fail"
    assert outcomes[1]["candidate"]["checks"]["binary"]["result"] == "fail"
    assert outcomes[1]["candidate"]["checks"]["layout"]["result"] == "pass"
    receipt = acceptance_reports[0]
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
    assert git("diff", "--name-only", "HEAD").splitlines() == ["src/owner.cpp", "src/owner.h"]

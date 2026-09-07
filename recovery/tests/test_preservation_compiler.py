"""Real local compiler protocol controls for the package preservation checker."""

import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.model import Origin
from match_recovery.preservation import check_preservation
from match_recovery.tasks import Task, TaskStore


def test_real_vc6_preserves_scoped_change_and_detects_collateral_header_change(tmp_path, monkeypatch):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    root = tmp_path
    repository = Path(__file__).resolve().parents[2]
    for name in ("code", "recovery", "build"):
        (root / name).mkdir()
    (root / ".gitignore").write_text("build/\n")
    (root / "code/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (root / "code/model.h").write_text("struct Model { int value; };\n")
    (root / "code/owner.cpp").write_text('// ORIGINAL: 0x00401000 ?scoped@@YAHPAUModel@@@Z 0x00401000-0x00401008\n#include "model.h"\nint scoped(Model* m) { return m->value; }\n')
    (root / "code/caller.cpp").write_text('// ORIGINAL: 0x00401010 ?outside@@YAHPAUModel@@@Z 0x00401010-0x00401018\n#include "model.h"\nint outside(Model* m) { return m->value; }\n')
    config = {"schema_version": 1, "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
              "compiler": {"command": {"argv": [sys.executable, str(repository / "recovery/environment/adapter.py")], "timeout": 60},
                           "inputs": [], "environment": ["VC6_PREFIX", "VC6_ROOT", "WINEARCH"]},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "application", "flag_sets": [["/c", "/O2", "/Gy"]],
                              "layout_flags": [], "unit_extensions": [".cpp"], "inputs": ["build/compile_commands.json"], "input_roots": []}}
    (root / "recovery/project.json").write_text(json.dumps(config))
    (root / "build/compile_commands.json").write_text(json.dumps([
        {"directory": str(root), "file": str(root / name),
         "arguments": [str(repository / "recovery/environment/vc6-cl"), "/nologo", "/c", str(root / name)]}
        for name in ("code/owner.cpp", "code/caller.cpp")]))
    def git(*args):
        return subprocess.run(["git", "-c", "user.name=Compiler fixture", "-c", "user.email=fixture@example.invalid", *args],
                              cwd=root, check=True, capture_output=True, text=True).stdout.strip()
    git("init", "-q", "--initial-branch=work")
    git("add", ".")
    git("commit", "-qm", "Synthetic VC6 source baseline")
    TaskStore(root).create(Task("owner", "synthetic", "coordinator", "Exercise preservation", git("rev-parse", "HEAD"),
                               (Origin("a" * 64, "pe:va", 0x401000),), ("code/owner.cpp", "code/model.h"), ("synthetic source",)))
    git("add", ".")
    git("commit", "-qm", "Authorize fixture task")
    path = root / "code/owner.cpp"
    path.write_text(path.read_text().replace("return m->value;", "return m->value + 1;"))
    before = git("diff")
    preserved = check_preservation(root, "owner")
    assert preserved["result"] == "pass", preserved
    assert preserved["controls"][0]["files"][0]["result"] == "pass"
    assert git("diff") == before
    (root / "code/model.h").rename(root / "code/renamed.h")
    for name in ("owner", "caller"):
        path = root / f"code/{name}.cpp"
        path.write_text(path.read_text().replace('"model.h"', '"renamed.h"'))
    before = git("diff")
    rehomed = check_preservation(root, "owner")
    assert rehomed["result"] == "pass", rehomed
    assert rehomed["materialization"] == "complete-tree-v5"
    assert all(row["result"] == "pass" for row in rehomed["controls"][0]["files"])
    assert git("diff") == before and not (root / "code/model.h").exists()
    (root / "code/renamed.h").write_text("struct Model { int padding; int value; };\n")
    before = git("diff")
    damaged = check_preservation(root, "owner")
    assert damaged["result"] == "fail", damaged
    assert next(row for row in damaged["units"] if row["unit"] == "code/caller.cpp")["result"] == "fail"
    assert git("diff") == before
    assert not list((root / "build/recovery/variants").iterdir())

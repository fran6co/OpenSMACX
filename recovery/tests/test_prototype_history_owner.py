"""Exercise actual typed history storage/consumers, not copied recovery drafts."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from recovery.tests.test_console_give_hints_host import _definition

ROOT = Path(__file__).resolve().parents[2]


def actual_source():
    path = ROOT / "src/veh.cpp"
    source = path.read_text()
    records = read_file(path)
    for address in (0x57DF00, 0x57DF30, 0x57DFC0):
        assert len([r for r in records if r.address == address]) == 1
    start = source.index("struct ProtoBung {")
    end = source.index("// The battle-resolution messages.", start)
    storage = re.sub(r"static_assert\(.*?\);", "", source[start:end], flags=re.S)
    bodies = [
        _definition(source, name, signature)
        for name, signature in (
            ("is_bunged", r"bool\s+__cdecl\s+is_bunged\s*\([^;]*?\)"),
            ("clear_bunglist", r"void\s+__cdecl\s+clear_bunglist\s*\([^;]*?\)"),
            ("proto_bung_put", r"void\s+__cdecl\s+proto_bung_put\s*\([^;]*?\)"),
        )
    ]
    return storage + "\n" + "\n".join(bodies)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "underwrite", "cursor"])
def test_actual_history_owner(compiler, damage, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip("host compiler unavailable")
    actual = actual_source()
    if damage == "underwrite":
        old = "ProtoBung *entry = &BungList[faction_id][0];"
        assert actual.count(old) == 2  # lookup and clear
        start = actual.index("void __cdecl clear_bunglist(")
        actual = actual[:start] + actual[start:].replace(
            old, "ProtoBung *entry = &BungList[faction_id - 1][0];", 1)
    elif damage == "cursor":
        old = "cursor = (slot + 1) % 64;"
        assert actual.count(old) == 1
        actual = actual.replace(old, "cursor = (slot + 2) % 64;")
    harness = Path(__file__).with_name("prototype_history_owner_harness.cpp").read_text()
    assert harness.count("/* ACTUAL_PROTOTYPE_HISTORY */") == 1
    cpp = tmp_path / "history.cpp"
    cpp.write_text(harness.replace("/* ACTUAL_PROTOTYPE_HISTORY */", actual))
    binary = tmp_path / "history"
    build = subprocess.run([executable, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", "-fno-pie", "-no-pie",
                            str(cpp), "-o", str(binary)], capture_output=True, text=True)
    assert build.returncode == 0, build.stdout + build.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    if damage is None:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "damaged recovery passed"

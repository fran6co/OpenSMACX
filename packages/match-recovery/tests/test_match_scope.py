"""Scoped match checks select every TU a change can reach, conservatively."""

import json
from pathlib import Path
from types import SimpleNamespace

from match_recovery.match_scope import affected_units


def _settings():
    return SimpleNamespace(source=SimpleNamespace(roots=("src",), image_pin="src/IMAGE_PIN"),
                           input_roots=(), inputs=("recovery/project.json",))


def _project(tmp_path, units):
    (tmp_path / "src").mkdir()
    rows = []
    for name, text in units.items():
        (tmp_path / name).write_text(text)
        if name.endswith(".cpp"):
            rows.append({"directory": str(tmp_path), "file": str(tmp_path / name),
                         "command": f"cl /FI{tmp_path / 'src/compat.h'} -c {tmp_path / name}"})
    (tmp_path / "src/compat.h").write_text("// forced\n")
    return json.dumps(rows).encode()


def test_header_change_selects_includers_and_forced_include_selects_all(tmp_path):
    database = _project(tmp_path, {
        "src/a.cpp": '#include "shared.h"\nint a;\n',
        "src/b.cpp": '#include "other.h"\nint b;\n',
        "src/shared.h": '#include "deep.h"\n',
        "src/deep.h": "int deep;\n",
        "src/other.h": "int other;\n"})
    assert affected_units(tmp_path, _settings(), database, ["src/deep.h"])["units"] == {"src/a.cpp"}
    assert affected_units(tmp_path, _settings(), database, ["src/b.cpp"])["units"] == {"src/b.cpp"}
    assert affected_units(tmp_path, _settings(), database, ["src/compat.h"])["units"] == {"src/a.cpp", "src/b.cpp"}
    assert affected_units(tmp_path, _settings(), database, ["docs/notes.md"])["units"] == set()


def test_unknown_include_closure_is_selected_and_policy_changes_widen(tmp_path):
    database = _project(tmp_path, {
        "src/a.cpp": '#define HEADER "x.h"\n#include HEADER\n',
        "src/b.cpp": "int b;\n",
        "src/x.h": "int x;\n"})
    result = affected_units(tmp_path, _settings(), database, ["src/x.h"])
    assert result["units"] == {"src/a.cpp"} and result["unknown_closure"] == ["src/a.cpp"]
    for name in ("CMakeLists.txt", "recovery/project.json", "recovery/environment/vc6-cl",
                 "packages/match-recovery/src/match_recovery/asm.py"):
        widened = affected_units(tmp_path, _settings(), database, [name])
        assert widened["units"] is None and name in widened["widened"]

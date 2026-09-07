"""Fresh recipe generation, retained include context, and guarded publication."""

import json
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.cli import main
from match_recovery.preparation import prepare_project
from test_build_check import project, command
from test_task_source import git


def policy(root, **changes):
    path = root / "recovery/prepare.json"
    value = json.loads(path.read_text())
    value.update(changes)
    path.write_text(json.dumps(value))


@pytest.fixture
def prepared(project):
    path = project / "recovery/project.json"
    value = json.loads(path.read_text())
    value.update(preparation="recovery/prepare.json", mappings={"compile_commands": "build/recipes.json"})
    path.write_text(json.dumps(value))
    value = json.loads((project / "recovery/build.json").read_text())
    value.update(artifacts="build/preparation", outputs=[{"path": "commands.json", "format": "compilation-database"}],
        commands=[command("from pathlib import Path; import json, sys; p=Path(sys.argv[1]); "
                         "assert not list(p.iterdir()); (p/'generated.h').write_text('#define GENERATED 42'); "
                         "(p/'commands.json').write_text(json.dumps([{'directory': '.', 'file': sys.argv[2]+'/code/main.c', "
                         "'arguments': ['compiler', '-DSPACE=one two', '-include', 'generated.h']}]))", "{build}", "{root}")])
    (project / "recovery/prepare.json").write_text(json.dumps(value))
    return project


def test_cli_generates_without_original_image_or_existing_database(prepared, capsys):
    before = git(prepared, "status", "--porcelain")
    assert main(["prepare", "--root", str(prepared), "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "pass" and report["published"] and report["entries"] == 1
    assert not report["ready_for_collection"] and not report["persisted"]
    assert "stdout" not in report["commands"][0]
    row, = json.loads((prepared / "build/recipes.json").read_text())
    directory = prepared / report["directory"]
    assert row["directory"] == str(directory) and row["file"] == str(prepared / "code/main.c")
    assert row["arguments"] == ["compiler", "-DSPACE=one two", "-include", "generated.h"]
    assert (directory / "generated.h").read_text() == "#define GENERATED 42"
    assert git(prepared, "status", "--porcelain") == before


def test_repeated_preparation_does_not_reuse_generated_context(prepared):
    first = prepare_project(prepared)
    (prepared / first["directory"] / "generated.h").write_text("damaged old generated header")
    previous = (prepared / "build/recipes.json").read_bytes()
    second = prepare_project(prepared)
    assert first["directory"] != second["directory"]
    assert first["database_sha256"] != second["database_sha256"]
    assert (prepared / second["directory"] / "generated.h").read_text() == "#define GENERATED 42"
    assert (prepared / second["directory"] / "previous-database.json").read_bytes() == previous


@pytest.mark.parametrize("failure", ["no-output", "empty", "invalid", "exit", "timeout", "missing-command"])
def test_bad_generation_never_publishes_or_counts_an_existing_database(prepared, failure):
    prepare_project(prepared)
    previous = (prepared / "build/recipes.json").read_bytes()
    action = {"no-output": "pass", "empty": "(p/'commands.json').write_text('')",
              "invalid": "(p/'commands.json').write_text('[]')", "exit": "raise SystemExit(1)",
              "timeout": "import time; time.sleep(10)", "missing-command": "pass"}[failure]
    invocation = command("from pathlib import Path; import sys; p=Path(sys.argv[1]); " + action, "{build}", timeout=1)
    if failure == "missing-command":
        invocation["argv"] = ["missing-prepare-command-0749", "{build}"]
    policy(prepared, commands=[invocation])
    report = prepare_project(prepared)
    assert report["result"] == ("fail" if failure == "exit" else "unknown")
    assert not report["published"] and (prepared / "build/recipes.json").read_bytes() == previous


@pytest.mark.parametrize("damage", ["source", "new-file", "index", "destination"])
def test_concurrent_changes_are_preserved_not_published_over(prepared, damage):
    prepare_project(prepared)
    original = (prepared / "build/recipes.json").read_bytes()
    value = json.loads((prepared / "recovery/prepare.json").read_text())
    action = {"source": "from pathlib import Path; Path('code/values.h').write_text('changed')",
              "new-file": "from pathlib import Path; Path('new-note').write_text('changed')",
              "index": "import subprocess; subprocess.run(['git','add','.'],check=True)",
              "destination": "from pathlib import Path; Path('build/recipes.json').write_text('concurrent output')"}[damage]
    policy(prepared, commands=[*value["commands"], command(action)])
    with pytest.raises(ValueError, match="changed"):
        prepare_project(prepared)
    if damage == "destination":
        assert (prepared / "build/recipes.json").read_text() == "concurrent output"
    else:
        assert (prepared / "build/recipes.json").read_bytes() == original
        if damage == "source":
            assert (prepared / "code/values.h").read_text() == "changed"
        elif damage == "new-file":
            assert (prepared / "new-note").read_text() == "changed"


@pytest.mark.parametrize("damage", ["symlink", "hardlink", "directory", "tracked", "parent-symlink", "unignored"])
def test_unsafe_publication_destinations_are_refused_before_generator(prepared, damage):
    (prepared / "build").mkdir(exist_ok=True)
    path = prepared / "build/recipes.json"
    if damage == "symlink":
        path.symlink_to(prepared / "code/main.c")
    elif damage == "hardlink":
        os.link(prepared / "code/main.c", path)
    elif damage == "directory":
        path.mkdir()
    elif damage == "tracked":
        path.write_text("user data")
        git(prepared, "add", "-f", "build/recipes.json")
    elif damage == "parent-symlink":
        (prepared / "build").rmdir()
        (prepared / "build").symlink_to(prepared / "code", target_is_directory=True)
    else:
        (prepared / ".gitignore").write_text("")
    with pytest.raises(ValueError):
        prepare_project(prepared)
    assert (prepared / "code/main.c").read_text().startswith('#include "values.h"')
    assert not list((prepared / "build/preparation").glob("build-*"))


def test_real_cmake_generation_sees_changed_defines_without_mtime_heuristics(prepared):
    cmake = shutil.which("cmake")
    cc = shutil.which("cc")
    if cmake is None or cc is None:
        pytest.skip("CMake and host C compiler needed")
    cmakelists = prepared / "CMakeLists.txt"
    text = 'cmake_minimum_required(VERSION 3.20)\nproject(RecipeControl C)\nadd_executable(sample code/main.c)\n'
    cmakelists.write_text(text + 'target_compile_definitions(sample PRIVATE VALUE_FROM_RECIPE=1)\n')
    policy(prepared, inputs=["CMakeLists.txt"],
           commands=[{"argv": [cmake, "-S", "{root}", "-B", "{build}", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"], "timeout": 30}],
           outputs=[{"path": "compile_commands.json", "format": "compilation-database"}])
    first = prepare_project(prepared)
    assert first["result"] == "pass"
    assert "VALUE_FROM_RECIPE=1" in (prepared / "build/recipes.json").read_text()
    stamp = cmakelists.stat()
    cmakelists.write_text(text + 'target_compile_definitions(sample PRIVATE VALUE_FROM_RECIPE=2)\n')
    os.utime(cmakelists, ns=(stamp.st_atime_ns, stamp.st_mtime_ns))
    second = prepare_project(prepared)
    assert second["result"] == "pass" and first["input_fingerprint"] != second["input_fingerprint"]
    row, = json.loads((prepared / "build/recipes.json").read_text())
    assert "VALUE_FROM_RECIPE=2" in row["command"] and Path(row["directory"]).is_dir()
    # The actual generated build directory remains usable, not just a parseable
    # database referencing deleted temporary CMake support files.
    subprocess.run([cmake, "--build", row["directory"]], check=True, capture_output=True, timeout=30)

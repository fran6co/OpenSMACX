"""Shared compiler failure controls and project-owned VC6 option semantics."""

import json
from pathlib import Path
import shutil
import subprocess
import sys

import pytest

from match_recovery.compilation_database import entries
from match_recovery.processes import run_bounded
from match_recovery.source import read_file
from match_recovery.source.asm import compiled_asm, CompileFailed
from recovery.environment import compile as environment
from recovery.environment.recipe import invocation, build_command, MissingRecipe, flag_dependent


def recipe(tmp_path, args):
    entry, = entries(json.dumps([{"file": "unit.cpp", "directory": str(tmp_path),
                                  "arguments": args}]).encode(), tmp_path)
    return invocation(entry)


def test_recipe_preserves_argument_boundaries_and_resolves_both_include_forms(tmp_path):
    args = ["./vc6-cl", "/I", "headers with spaces", "-I../shared", "/FI", "config.h",
            '-DMESSAGE="hello world"', "/c", "/O2", "/Oy-", "/Z7", "/Foout.obj", "unit.cpp"]
    assert recipe(tmp_path, args) == [str(tmp_path / "vc6-cl"), "/nologo",
        "/I" + str(tmp_path / "headers with spaces"), "/I" + str(tmp_path.parent / "shared"),
        "/FI" + str(tmp_path / "config.h"), '/DMESSAGE="hello world"']


def test_pch_filter_does_not_delete_an_unrelated_define(tmp_path):
    assert recipe(tmp_path, ["cl", "/FI", "cmake_pch.hxx", "/Yucmake_pch.hxx", "/Fp", "pch.bin",
                             "/DRESOURCE=cmake_pch", "unit.cpp"]) == ["cl", "/nologo", "/DRESOURCE=cmake_pch"]


@pytest.mark.parametrize("prefix", ["/", "-"])
@pytest.mark.parametrize("runtime", ["MD", "MDd", "MT", "MTd", "ML", "MLd"])
def test_recipe_preserves_runtime_predefines_and_link_policy(tmp_path, prefix, runtime):
    assert recipe(tmp_path, ["cl", prefix + runtime, "/O2", "/Z7", "unit.cpp"]) == [
        "cl", "/nologo", "/" + runtime]


def test_runtime_switch_order_is_not_reinterpreted_by_recipe_extraction(tmp_path):
    assert recipe(tmp_path, ["cl", "-MD", "/MT", "unit.cpp"]) == ["cl", "/nologo", "/MD", "/MT"]


def test_different_runtime_recipes_are_not_mistaken_for_duplicates(tmp_path):
    database = tmp_path / "compilation.json"
    rows = [{"file": "unit.cpp", "directory": str(tmp_path), "arguments": ["cl", flag, "unit.cpp"]}
            for flag in ("/MD", "-MT")]
    database.write_text(json.dumps(rows))
    with pytest.raises(ValueError, match="conflicting compilation recipes"):
        build_command(database, tmp_path / "unit.cpp")


def test_prepared_project_and_matching_compiler_use_static_multithreaded_runtime(tmp_path):
    from match_recovery.compilation_database import read_database
    from match_recovery.pe_coff import Coff
    from recovery.environment.wine import VC6_ROOT
    root = Path(__file__).resolve().parents[2]
    database = root / "build/compile_commands.json"
    if not database.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("prepared project and VC6/Wine are required")
    commands = [invocation(entry) for entry in read_database(database)]
    assert commands
    for command in commands:
        assert [word for word in command if word in ("/MD", "/MDd", "/MT", "/MTd", "/ML", "/MLd")] == ["/MT"]
    source = tmp_path / "runtime.cpp"
    source.write_text("#if !defined(_MT) || defined(_DLL) || defined(_DEBUG)\n"
                      "#error matching runtime policy is not static multithreaded\n#endif\n"
                      "int runtime_mode_control() { return 0; }\n")
    command = build_command(database, root / "src/alphanet.cpp")
    obj = Coff(environment.compile_object(source, tuple(command), ("/c", "/O2", "/Gy")))
    obj.function("?runtime_mode_control@@YAHXZ")
    # Check the actual compiler's predefines, not just spellings in a database.
    for wrong in ("/MD", "/ML", "/MTd"):
        changed = tuple(wrong if word == "/MT" else word for word in command)
        with pytest.raises(ValueError, match="compilation failed"):
            environment.compile_object(source, changed, ("/c", "/O2", "/Gy"))


def test_release_link_preserves_the_selected_runtime_code_and_relocations():
    """A bounded linked-runtime control, not full original-game acceptance."""
    from collections import defaultdict
    import hashlib
    from match_recovery.binary import compare_bound_code
    from match_recovery.build_check import check_build
    from match_recovery.coff_archive import CoffArchive
    from match_recovery.component_source import parse_component
    from match_recovery.debug_sidecar import read_map
    from match_recovery.import_source import read_imports, import_targets
    from match_recovery.model import Span
    from match_recovery.pe_coff import Coff, PeImage
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for the actual project link")
    root = Path(__file__).resolve().parents[2]
    manifest = parse_component((root / "src/vendor/vc6-runtime.recovery.h").read_text())
    raw = (VC6_ROOT / "LIB/LIBCMT.LIB").read_bytes()
    assert hashlib.sha256(raw).hexdigest() == manifest.archive_sha256
    binding, = (item for item in manifest.bindings if item.symbol == "_free")
    obj = Coff(CoffArchive(raw).member(binding.unit))
    code, relocations, alignment = obj.subject(obj.function(binding.symbol))
    report = check_build(root, retain=True)
    assert report["result"] == "pass", report
    directory = root / report["directory"]
    output, = report["outputs"]
    image = PeImage((directory / output["path"]).read_bytes(), output["sha256"])
    assert not any(item.module.lower() in ("msvcrt.dll", "msvcrtd.dll") for item in image.imports())
    publics, _ = read_map(directory / "OpenSMACX.map")
    addresses = defaultdict(set)
    for address, name, _, _ in publics:
        addresses[name].add(address)
    start, = addresses[binding.symbol]
    imports, _ = import_targets(read_imports(root), {binding.unit: obj}, image)
    targets = {str(index): next(iter(addresses[symbol.name])) for index, symbol in obj.symbols.items()
               if len(addresses[symbol.name]) == 1}
    targets.update({str(index): imports[symbol.name] for index, symbol in obj.symbols.items()
                    if symbol.name in imports})
    span = Span(start, start + len(code))
    proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                               original_absolute_sites=image.absolute_sites(span), alignment=alignment)
    assert proof["result"] == "pass", proof
    # The map supplies independent linked addresses, not original placement or
    # complete component admission. A fixed Release image failed this control.
    assert image.absolute_sites(span)


@pytest.mark.parametrize("args", [["compiler", "@options"], ["compiler", "/I"],
    ["compiler", "/FI"], ["compiler", "/Fo"], ["python", "wrapper.py", "unit.cpp"]])
def test_unsupported_recipe_is_not_silently_changed(tmp_path, args):
    with pytest.raises(ValueError):
        recipe(tmp_path, args)


def test_missing_recipe_is_distinct_from_conflicting_or_malformed_recipe(tmp_path):
    database = tmp_path / "compilation.json"
    rows = [{"file": "unit.cpp", "directory": str(tmp_path), "arguments": ["cl", flag, "unit.cpp"]}
            for flag in ("/DVALUE=1", "/DVALUE=2")]
    database.write_text(json.dumps(rows))
    with pytest.raises(ValueError, match="conflicting") as raised:
        build_command(database, tmp_path / "unit.cpp")
    assert not isinstance(raised.value, MissingRecipe)
    with pytest.raises(MissingRecipe):
        build_command(database, tmp_path / "seed.cpp")
    database.write_text("invalid")
    with pytest.raises(ValueError) as raised:
        build_command(database, tmp_path / "unit.cpp")
    assert not isinstance(raised.value, MissingRecipe)


@pytest.mark.parametrize("code,body,accepted", [(0, b"object", True), (1, b"object", False),
                                               (0, b"", False), (0, None, False), (7, None, False)])
def test_compiler_exit_and_object_are_both_required(tmp_path, monkeypatch, code, body, accepted):
    seen = []
    def run(argv, *, cwd, timeout):
        seen.append(cwd)
        assert timeout == 300 and argv[-2] == "/Founit.obj"
        assert '/DMESSAGE=hello world' in argv
        if body is not None:
            (cwd / "unit.obj").write_bytes(body)
        return subprocess.CompletedProcess(argv, code, b"compiler diagnostic", b"")
    monkeypatch.setattr(environment, "run_bounded", run)
    source = tmp_path / "unit.cpp"
    if accepted:
        assert environment.compile_object(source, ("compiler",), '/c "/DMESSAGE=hello world"') == body
    else:
        with pytest.raises(ValueError):
            environment.compile_object(source, ("compiler",), '/c "/DMESSAGE=hello world"')
    assert seen and not seen[0].exists()


def test_timeout_is_a_comparison_failure_and_temporary_directory_is_removed(tmp_path, monkeypatch):
    seen = []
    def stall(argv, *, cwd, timeout):
        seen.append(cwd)
        raise subprocess.TimeoutExpired(argv, timeout)
    monkeypatch.setattr(environment, "run_bounded", stall)
    source = tmp_path / "unit.cpp"
    source.write_text("// ORIGINAL: 0x00401000 f 0x00401000-0x00401008\nvoid f() {}\n")
    with pytest.raises(CompileFailed, match="exceeded 300 seconds"):
        compiled_asm(read_file(source)[0], ["compiler"], "/c", compiler=environment.compile_object)
    assert seen and not seen[0].exists()


def test_real_failed_process_cannot_supply_an_object_to_the_comparer(tmp_path):
    # A synthetic failing tool actually creates an object before exiting. This
    # reproduced the old subprocess path's mistake without proprietary fixtures.
    script = "from pathlib import Path; Path('unit.obj').write_bytes(b'object'); raise SystemExit(7)"
    with pytest.raises(ValueError, match="compilation failed"):
        environment.compile_object(tmp_path / "unit.cpp", (sys.executable, "-c", script), ())


def test_vc6_retry_policy_remains_in_the_environment():
    assert not flag_dependent("error C2065: identifier not declared")
    assert flag_dependent("fatal error C1001: internal compiler error")
    assert flag_dependent("fatal error C1060: out of heap space")


def test_unidentified_compilers_cannot_generate_matching_recipes(tmp_path):
    cmake, ninja, failed_compiler = (shutil.which(name) for name in ("cmake", "ninja", "false"))
    if not all((cmake, ninja, failed_compiler)):
        pytest.skip("CMake, Ninja and a failing POSIX command are required")
    root = Path(__file__).resolve().parents[2]
    # No installed compiler or Wine is needed. The forced toolchain used to
    # report success and emit unusable generic recipes even for /bin/false.
    install = tmp_path / "vc6"
    (install / "BIN").mkdir(parents=True)
    (install / "INCLUDE").mkdir()
    build = tmp_path / "build"
    completed = run_bounded([
        cmake, "-S", str(root), "-B", str(build), "-G", "Ninja",
        "-DVC6_ROOT=" + str(install), "-DCMAKE_MAKE_PROGRAM=" + ninja,
        "-DCMAKE_CXX_COMPILER=" + failed_compiler,
        "-DCMAKE_C_COMPILER=" + failed_compiler,
    ], cwd=tmp_path, timeout=60)
    assert completed.returncode != 0
    assert b"Compiler identification failed" in completed.stderr
    assert not (build / "compile_commands.json").exists()

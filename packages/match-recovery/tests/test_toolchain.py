"""External input damage must invalidate evidence, including same-path upgrades."""

import json
import os
from pathlib import Path
import sys
from concurrent.futures import ThreadPoolExecutor

import pytest

from match_recovery.build_check import Build, check_build
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.toolchain import snapshot
import match_recovery.toolchain as implementation
from test_build_check import project as build_project, write_config, command
from test_compiler import compiler_project
from test_recording import project as measurement_project


@pytest.fixture(autouse=True, params=["on", "off"])
def cache_mode(monkeypatch, request):
    monkeypatch.setenv("MATCH_RECOVERY_TOOLCHAIN_CACHE", request.param)
    return request.param


def configure(root, external):
    description = {"schema_version": 1, "files": {"compiler": str(external / "compiler")},
                   "directories": {"sdk": str(external / "sdk")}}
    adapter = root / "recovery/toolchain.py"
    adapter.write_text("import json,sys\nassert json.load(sys.stdin) == "
                       "{'schema_version':1,'operation':'toolchain_inputs'}\n"
                       f"print(json.dumps({description!r}))\n")
    path = root / "recovery/project.json"
    value = json.loads(path.read_text())
    value["toolchain"] = {"command": {"argv": [sys.executable, "recovery/toolchain.py"], "timeout": 5},
                          "inputs": ["recovery/toolchain.py"], "environment": ["TEST_SDK_OPTION"]}
    path.write_text(json.dumps(value))
    return description


@pytest.fixture
def external(tmp_path_factory):
    directory = tmp_path_factory.mktemp("external-sdk")
    (directory / "compiler").write_bytes(b"compiler-A")
    (directory / "sdk").mkdir()
    (directory / "sdk/type.h").write_bytes(b"layout-A")
    (directory / "sdk/runtime.lib").write_bytes(b"library-A")
    return directory


def test_compiler_without_external_policy_remains_explicitly_unobserved(compiler_project):
    assert snapshot(compiler_project) is None


@pytest.mark.parametrize("damage", ["compiler", "header", "library", "shadowing", "remove", "environment", "mode"])
def test_external_changes_refuse_reuse_even_with_same_size_and_mtime(compiler_project, external, monkeypatch, damage):
    root = compiler_project
    configure(root, external)
    compiler = Compiler(root)
    if damage == "environment":
        monkeypatch.setenv("TEST_SDK_OPTION", "changed")
    elif damage == "shadowing":
        (external / "sdk/new-header.h").write_bytes(b"new")
    else:
        path = external / {"compiler": "compiler", "mode": "compiler", "header": "sdk/type.h",
                           "library": "sdk/runtime.lib", "remove": "compiler"}[damage]
        original = path.stat()
        if damage == "remove":
            path.unlink()
        elif damage == "mode":
            path.chmod(original.st_mode ^ 0o100)
        else:
            path.write_bytes(path.read_bytes()[:-1] + b"B")
            os.utime(path, ns=(original.st_atime_ns, original.st_mtime_ns))
    with pytest.raises(ValueError, match="changed|missing"):
        compiler(root / "code/unit.cc", ("compile",), ("-O2",))


def test_compiler_cannot_replace_sdk_and_return_an_accepted_object(compiler_project, external):
    configure(compiler_project, external)
    path = compiler_project / "recovery/compiler.py"
    path.write_text(f"from pathlib import Path\nPath({str(external / 'sdk/type.h')!r}).write_bytes(b'wrong')\n"
                    "print('{\"object_base64\":\"b2JqZWN0\"}')\n")
    compiler = Compiler(compiler_project)
    with pytest.raises(ValueError, match="changed during compilation"):
        compiler(compiler_project / "code/unit.cc", (), ())
    assert (external / "sdk/type.h").read_bytes() == b"wrong"  # Never restore over another writer.


def test_internal_alias_is_pinned_but_root_or_external_aliases_refuse(compiler_project, external):
    configure(compiler_project, external)
    alias = external / "sdk/TYPE.H"
    alias.symlink_to("type.h")
    before = snapshot(compiler_project)
    alias.unlink()
    alias.symlink_to("runtime.lib")
    assert snapshot(compiler_project) != before
    alias.unlink()
    alias.symlink_to("../compiler")
    with pytest.raises(ValueError, match="aliases"):
        snapshot(compiler_project)
    alias.unlink()
    alias.symlink_to(".", target_is_directory=True)
    with pytest.raises(ValueError, match="aliases"):
        snapshot(compiler_project)


def test_internal_directory_alias_uses_fresh_canonical_contents(compiler_project, external):
    configure(compiler_project, external)
    directory = external / "sdk/SYS"
    directory.mkdir()
    (directory / "types.h").write_bytes(b"types")
    (external / "sdk/sys").symlink_to("SYS", target_is_directory=True)
    before = snapshot(compiler_project)
    (directory / "types.h").write_bytes(b"changed types")
    assert snapshot(compiler_project) != before


@pytest.mark.parametrize("directory", ["sdk", "sdk/nested"])
def test_directory_enumeration_failures_cannot_omit_inputs(compiler_project, external, monkeypatch, directory):
    configure(compiler_project, external)
    inaccessible = external / directory
    inaccessible.mkdir(exist_ok=True)
    (inaccessible / "readable.h").write_bytes(b"compiler can open a known filename")
    original = os.scandir
    def denied(path):
        if Path(path) == inaccessible:
            raise PermissionError("cannot enumerate SDK directory")
        return original(path)
    monkeypatch.setattr(os, "scandir", denied)
    with pytest.raises(PermissionError, match="enumerate SDK"):
        snapshot(compiler_project)


@pytest.mark.parametrize("description", [None, {}, {"schema_version": True, "files": {}, "directories": {}},
    {"schema_version": 1, "files": {}, "directories": {}},
    {"schema_version": 1, "files": {"c": "relative"}, "directories": {}},
    {"schema_version": 1, "files": {"c": "/missing-test-tool"}, "directories": {"c": "/tmp"}},
    {"schema_version": 1, "files": {"../escape": "/tmp"}, "directories": {}}])
def test_bad_or_empty_external_inventory_cannot_claim_identity(compiler_project, external, description):
    configure(compiler_project, external)
    (compiler_project / "recovery/toolchain.py").write_text(f"import json\nprint(json.dumps({description!r}))\n")
    with pytest.raises(ValueError):
        snapshot(compiler_project)


def test_build_refuses_toolchain_mutation_after_successful_output(build_project, external):
    configure(build_project, external)
    write_config(build_project, commands=[command("from pathlib import Path; import sys; "
        f"Path({str(external / 'sdk/runtime.lib')!r}).write_bytes(b'changed-library'); "
        "(Path(sys.argv[1])/'app').write_bytes(b'produced')", "{build}")])
    with pytest.raises(ValueError, match="changed"):
        check_build(build_project)


def test_build_snapshot_includes_external_inputs(build_project, external):
    configure(build_project, external)
    first_build = Build.read(build_project).snapshot(build_project)
    (external / "sdk/type.h").write_bytes(b"changed layout")
    assert first_build != Build.read(build_project).snapshot(build_project)


def test_measurement_snapshot_includes_external_inputs(measurement_project, external):
    measurement_root = measurement_project
    configure(measurement_root, external)
    first_measurement = Measurement.read(measurement_root).snapshot(measurement_root, {})
    (external / "sdk/type.h").write_bytes(b"changed layout")
    assert first_measurement != Measurement.read(measurement_root).snapshot(measurement_root, {})


def test_external_locations_and_environment_values_are_not_printed(compiler_project, external, monkeypatch):
    configure(compiler_project, external)
    monkeypatch.setenv("TEST_SDK_OPTION", "sensitive-option")
    result = snapshot(compiler_project)
    assert str(external) not in json.dumps(result) and "sensitive-option" not in json.dumps(result)
    assert result["inputs"]["sdk"]["file_count"] == 2


def test_input_mutation_during_hashing_refuses_observation(compiler_project, external, monkeypatch):
    configure(compiler_project, external)
    original = implementation._content_digest
    def damaged(stream, buffer):
        hashed = original(stream, buffer)
        if Path(stream.name) == external / "compiler":
            (external / "compiler").write_bytes(b"changed while hashing")
        return hashed
    monkeypatch.setattr(implementation, "_content_digest", damaged)
    with pytest.raises(ValueError, match="changed while hashing"):
        snapshot(compiler_project)


@pytest.mark.parametrize("value", [None, {}, {"command": {}, "inputs": [], "environment": [None]}])
def test_malformed_toolchain_policy_is_not_unconfigured(compiler_project, value):
    path = compiler_project / "recovery/project.json"
    configuration = json.loads(path.read_text())
    configuration["toolchain"] = value
    path.write_text(json.dumps(configuration))
    with pytest.raises(ValueError):
        snapshot(compiler_project)


def test_content_cache_matches_uncached_reads_and_cannot_be_poisoned_by_callers(compiler_project, external, monkeypatch, cache_mode):
    configure(compiler_project, external)
    original, reads = implementation._content_digest, []
    def counted(stream, buffer):
        reads.append(stream.name)
        return original(stream, buffer)
    monkeypatch.setattr(implementation, "_content_digest", counted)
    first = snapshot(compiler_project)
    assert len(reads) == 3
    second = snapshot(compiler_project)
    assert second == first
    assert len(reads) == (3 if cache_mode == "on" and os.name == "posix" else 6)
    first["inputs"].clear()
    assert snapshot(compiler_project) == second
    monkeypatch.setenv("MATCH_RECOVERY_TOOLCHAIN_CACHE", "off")
    count = len(reads)
    assert snapshot(compiler_project) == second and len(reads) == count + 3


def test_changes_during_cached_observation_cannot_pass(compiler_project, external, monkeypatch):
    configure(compiler_project, external)
    snapshot(compiler_project)
    original, calls = implementation._inventory, 0
    def changing(description):
        nonlocal calls
        calls += 1
        if calls == 2:
            (external / "sdk/type.h").write_bytes(b"changed between inventories")
        return original(description)
    monkeypatch.setattr(implementation, "_inventory", changing)
    with pytest.raises(ValueError, match="changed"):
        snapshot(compiler_project)


def test_cache_is_bounded_and_disabling_it_never_populates_entries(compiler_project, external, monkeypatch, cache_mode):
    configure(compiler_project, external)
    monkeypatch.setattr(implementation, "_CACHE_LIMIT", 2)
    previous = dict(implementation._observations)
    for index in range(3):
        monkeypatch.setenv("TEST_SDK_OPTION", str(index))
        snapshot(compiler_project)
    if cache_mode == "on" and os.name == "posix":
        assert len(implementation._observations) == 2
    else:
        assert dict(implementation._observations) == previous


def test_parallel_readers_can_evict_cache_entries_without_changing_identity(compiler_project, external, monkeypatch):
    roots = []
    for index in range(3):
        child = compiler_project / str(index)
        (child / "recovery").mkdir(parents=True)
        (child / "recovery/project.json").write_text('{"schema_version":1}')
        configure(child, external)
        adapter = child / "recovery/toolchain.py"
        adapter.write_text(adapter.read_text() + f"# local input identity {index}\n")
        roots.append(child)
    monkeypatch.setattr(implementation, "_CACHE_LIMIT", 1)
    expected = {root: snapshot(root) for root in roots}
    selected = roots * 6
    with ThreadPoolExecutor(max_workers=4) as pool:
        results = list(pool.map(snapshot, selected))
    assert results == [expected[root] for root in selected]


@pytest.mark.skipif(not hasattr(os, "fork"), reason="POSIX fork cache ownership control")
def test_forked_child_does_not_inherit_locked_parent_cache(compiler_project, external):
    configure(compiler_project, external)
    snapshot(compiler_project)
    with implementation._cache_lock:
        child = os.fork()
        if child == 0:
            clean = not implementation._cache_lock.locked() and not implementation._observations
            os._exit(0 if clean else 1)
    _, status = os.waitpid(child, 0)
    assert os.waitstatus_to_exitcode(status) == 0

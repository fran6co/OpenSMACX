"""Runtime hash reuse preserves fresh content identity, never cached authority."""

from concurrent.futures import ThreadPoolExecutor
import hashlib
import os
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery import framework_runtime as runtime
from match_recovery.model import digest
from test_framework_runtime import installed, write_binding


@pytest.fixture(autouse=True, params=["on", "off"])
def mode(request, monkeypatch):
    monkeypatch.setenv("MATCH_RECOVERY_RUNTIME_CACHE", request.param)
    runtime._after_fork()
    return request.param


def test_hash_identity_is_unchanged_and_reads_are_reused_only_when_enabled(tmp_path, monkeypatch, mode):
    path = tmp_path / "input.py"
    path.write_bytes(b"original content")
    original, calls = hashlib.file_digest, []

    def observed(stream, algorithm):
        calls.append(stream.name)
        return original(stream, algorithm)

    monkeypatch.setattr(hashlib, "file_digest", observed)
    expected = hashlib.sha256(path.read_bytes()).hexdigest()
    assert runtime._file(path) == runtime._file(path) == expected
    assert len(calls) == (1 if mode == "on" and os.name == "posix" else 2)


def test_same_size_write_with_restored_mtime_invalidates_hash(tmp_path):
    path = tmp_path / "input.py"
    path.write_bytes(b"old-value")
    before = path.stat()
    original = runtime._file(path)
    path.write_bytes(b"new-value")
    os.utime(path, ns=(before.st_atime_ns, before.st_mtime_ns))
    assert path.stat().st_size == before.st_size and path.stat().st_mtime_ns == before.st_mtime_ns
    assert runtime._file(path) != original


def test_real_active_binding_rejects_restored_mtime_package_edit(tmp_path, installed):
    binding = write_binding(tmp_path, installed)
    path = installed[1] / "__init__.py"
    with pytest.raises(ValueError, match="runtime content changed"):
        with runtime.binding(binding):
            runtime.fingerprint()  # Warm every runtime content observation.
            before, contents = path.stat(), path.read_bytes()
            path.write_bytes(bytes([contents[0] ^ 1]) + contents[1:])
            os.utime(path, ns=(before.st_atime_ns, before.st_mtime_ns))
            runtime.fingerprint()


def test_installation_and_package_hashes_keep_existing_descriptor_semantics(tmp_path):
    purelib = tmp_path / "site-packages"
    package = purelib / "match_recovery"
    package.mkdir(parents=True)
    (package / "__init__.py").write_bytes(b"# package")
    (package / "nested").mkdir()
    (package / "nested/code.py").write_bytes(b"# nested")
    (purelib / "dependency.py").write_bytes(b"# dependency")

    def old_identity(path):
        return digest({item.relative_to(path).as_posix(): hashlib.sha256(item.read_bytes()).hexdigest()
                       for item in path.rglob("*") if item.is_file()})

    package_sha, environment_sha = runtime._installation(package, purelib)
    assert (package_sha, environment_sha) == (old_identity(package), old_identity(purelib))
    assert runtime._installation(package, purelib) == (package_sha, environment_sha)


def test_one_installation_observation_reads_each_shared_package_file_once(tmp_path, monkeypatch):
    purelib = tmp_path / "site-packages"
    package = purelib / "match_recovery"
    package.mkdir(parents=True)
    (package / "__init__.py").write_bytes(b"# package")
    (purelib / "dependency.py").write_bytes(b"# dependency")
    original, files = runtime._file, []

    def observed(path):
        files.append(path)
        return original(path)

    monkeypatch.setattr(runtime, "_file", observed)
    runtime._installation(package, purelib)
    assert set(files) == {package / "__init__.py", purelib / "dependency.py"}
    assert len(files) == 2


@pytest.mark.parametrize("change", ["add", "remove", "nested_add", "replace"])
def test_warm_tree_still_reenumerates_membership_and_replaced_inodes(tmp_path, change):
    path = tmp_path / "input.py"
    path.write_bytes(b"old")
    before = runtime._tree(tmp_path, installed=True)
    if change == "add":
        (tmp_path / "new.py").write_bytes(b"new")
    elif change == "remove":
        path.unlink()
    elif change == "nested_add":
        (tmp_path / "nested").mkdir()
        (tmp_path / "nested/new.py").write_bytes(b"new")
    else:
        previous = path.stat()
        replacement = tmp_path / "replacement"
        replacement.write_bytes(b"new")
        os.utime(replacement, ns=(previous.st_atime_ns, previous.st_mtime_ns))
        replacement.replace(path)
    assert runtime._tree(tmp_path, installed=True) != before


@pytest.mark.parametrize("change", ["file_alias", "directory_alias", "dangling_alias", "bytecode", "fifo"])
def test_warm_tree_cannot_hide_aliases_nonregular_files_or_bytecode(tmp_path, change):
    (tmp_path / "input.py").write_bytes(b"old")
    runtime._tree(tmp_path, installed=True)
    path = tmp_path / "new"
    if change == "file_alias":
        path.symlink_to("input.py")
    elif change == "directory_alias":
        path.symlink_to(".", target_is_directory=True)
    elif change == "dangling_alias":
        path.symlink_to("missing")
    elif change == "bytecode":
        path = tmp_path / "new.pyc"
        path.write_bytes(b"unapproved bytecode")
    else:
        if not hasattr(os, "mkfifo"):
            pytest.skip("requires POSIX FIFO")
        os.mkfifo(path)
    with pytest.raises(ValueError, match="aliases|nonregular|bytecode"):
        runtime._tree(tmp_path, installed=True)


def test_directory_enumeration_failure_refuses_even_after_warming(tmp_path, monkeypatch):
    directory = tmp_path / "nested"
    directory.mkdir()
    (directory / "module.py").write_text("value = 1\n")
    runtime._tree(tmp_path, installed=True)
    original = os.scandir

    def denied(path):
        if Path(path) == directory:
            raise PermissionError("cannot enumerate runtime directory")
        return original(path)

    monkeypatch.setattr(os, "scandir", denied)
    with pytest.raises(PermissionError, match="cannot enumerate"):
        runtime._tree(tmp_path, installed=True)


def test_mutation_during_hash_never_populates_a_reusable_observation(tmp_path, monkeypatch):
    path = tmp_path / "input.py"
    path.write_bytes(b"old")
    original = hashlib.file_digest

    def raced(stream, algorithm):
        value = original(stream, algorithm)
        path.write_bytes(b"new")
        return value

    monkeypatch.setattr(hashlib, "file_digest", raced)
    with pytest.raises(ValueError, match="changed during hashing"):
        runtime._file(path)
    assert path not in runtime._FILES
    monkeypatch.setattr(hashlib, "file_digest", original)
    assert runtime._file(path) == hashlib.sha256(b"new").hexdigest()


def test_tree_mutation_after_cached_file_read_refuses_whole_observation(tmp_path, monkeypatch):
    path = tmp_path / "input.py"
    path.write_bytes(b"old")
    runtime._tree(tmp_path)
    original = runtime._file

    def raced(path):
        result = original(path)
        (tmp_path / "unexpected.py").write_bytes(b"new")
        return result

    monkeypatch.setattr(runtime, "_file", raced)
    with pytest.raises(ValueError, match="installation changed"):
        runtime._tree(tmp_path)


def test_file_cache_is_bounded_and_threads_do_not_change_content_identity(tmp_path, monkeypatch, mode):
    monkeypatch.setattr(runtime, "_FILE_LIMIT", 3)
    paths = [tmp_path / f"module-{i}.py" for i in range(12)]
    for i, path in enumerate(paths):
        path.write_bytes(bytes([i]) * 1024)
    expected = [hashlib.sha256(path.read_bytes()).hexdigest() for path in paths]
    with ThreadPoolExecutor(max_workers=4) as pool:
        for _ in range(3):
            assert list(pool.map(runtime._file, paths)) == expected
    assert len(runtime._FILES) <= 3
    if mode == "off":
        assert not runtime._FILES


@pytest.mark.skipif(not hasattr(os, "fork"), reason="requires POSIX fork")
def test_fork_clears_observations_and_inherited_lock_without_deadlock(tmp_path):
    # Isolate fork from pytest's other threads. The subprocess deliberately
    # forks while holding the cache lock to exercise the registered callback.
    path = tmp_path / "input.py"
    path.write_bytes(b"content")
    code = """
import os, sys
from pathlib import Path
from match_recovery import framework_runtime as runtime
path = Path(sys.argv[1])
expected = runtime._file(path)
with runtime._FILE_LOCK:
    child = os.fork()
    if child == 0:
        assert not runtime._FILES
        assert runtime._file(path) == expected
        os._exit(0)
    _, status = os.waitpid(child, 0)
assert status == 0
print('fresh child observations')
"""
    result = subprocess.run([sys.executable, "-B", "-c", code, str(path)],
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stderr
    assert result.stdout.strip() == "fresh child observations"


def test_invalid_cache_switch_never_changes_runtime_activation(tmp_path, monkeypatch):
    path = tmp_path / "input.py"
    path.write_bytes(b"content")
    monkeypatch.setenv("MATCH_RECOVERY_RUNTIME_CACHE", "sometimes")
    with pytest.raises(ValueError, match="must be on or off"):
        runtime._file(path)
    assert runtime.fingerprint() is None

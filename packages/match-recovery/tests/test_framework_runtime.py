"""Explicit runtime bindings and opt-in Python hooks preserve command authority."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import venv

import pytest

import match_recovery
from match_recovery import framework_runtime as runtime


def install_copy(prefix):
    """A genuinely importable, non-editable install, without pip or a network."""
    venv.EnvBuilder(with_pip=False, symlinks=True).create(prefix)
    python = prefix / "bin/python"
    query = subprocess.run(
        [str(python), "-I", "-B", "-c", "import sysconfig; print(sysconfig.get_path('purelib'))"],
        check=True, capture_output=True, text=True,
    )
    site = Path(query.stdout.strip())
    package = site / "match_recovery"
    shutil.copytree(Path(match_recovery.__file__).parent, package,
                    ignore=shutil.ignore_patterns("__pycache__", "*.pyc", "*.pyo"))
    metadata = site / "match_recovery-0.1.0.dist-info"
    metadata.mkdir()
    (metadata / "METADATA").write_text("Metadata-Version: 2.1\nName: match-recovery\nVersion: 0.1.0\n")
    (metadata / "RECORD").write_text("")
    return python, package, metadata


@pytest.fixture
def installed(tmp_path, monkeypatch):
    # Component tests exercise pinning/routing from pytest's editable checkout.
    # Only that process-location boundary is substituted; hashes, interpreter
    # probes and all child processes use production checks. Real-child tests
    # below prove the selected installation is mandatory in production.
    monkeypatch.setattr(runtime, "_require_current", lambda descriptor: None)
    return install_copy(tmp_path / "runtime")


def write_binding(tmp_path, installed, *, name="binding.json", launchers=None):
    python, _, _ = installed
    path = tmp_path / name
    descriptor = runtime.inspect(python)
    path.write_text(json.dumps({"schema_version": 1, "runtime": descriptor,
                                "launchers": launchers or [["uv", "run", "recover"]]}))
    return path


def test_inactive_runtime_does_not_use_environment_or_rewrite_commands(tmp_path, monkeypatch):
    monkeypatch.setenv("MATCH_RECOVERY_RUNTIME", str(tmp_path / "untrusted.json"))
    monkeypatch.setenv("PYTHONPATH", str(tmp_path / "untrusted-modules"))
    for argv in (["uv", "run", "recover", "inventory"],
                 ["uv", "run", "python", "compiler.py"], ["provider", "exec"]):
        assert list(runtime.resolve(argv)) == argv
    assert runtime.fingerprint() is None


def test_binding_rewrites_only_exact_declared_launcher_preserving_suffix(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    python, _, _ = installed
    suffix = ["tasks", "check", "two words", "--root", "project with spaces", "--summary"]
    expected = [str(python), "-I", "-B", "-m", "match_recovery", "--runtime", str(path), *suffix]
    before = dict(os.environ)
    with runtime.binding(path):
        assert list(runtime.resolve(["uv", "run", "recover", *suffix])) == expected
        assert list(runtime.native_argv(suffix)) == expected
        digest = runtime.fingerprint()
        assert isinstance(digest, str) and len(digest) == 64
        assert runtime.fingerprint() == digest
        for other in (["provider", "exec", "--model", "worker"],
                      ["uv", "run", "python", "compiler.py"],
                      ["uv", "run", "--project", "old", "recover", "inventory"],
                      ["/usr/bin/uv", "run", "recover", "inventory"],
                      ["uv", "run", "recovery", "inventory"],
                      ["uv", "run"], ["recover", "inventory"]):
            assert list(runtime.resolve(other)) == other
        assert dict(os.environ) == before
    assert runtime.fingerprint() is None
    assert dict(os.environ) == before


def test_explicit_framework_python_without_binding_selects_current_interpreter(monkeypatch):
    monkeypatch.setenv("MATCH_RECOVERY_RUNTIME", "/untrusted/runtime.json")
    monkeypatch.setenv("PYTHON", "/untrusted/python")
    suffix = ["hook with spaces.py", "{framework_python}", "--literal={framework_python}"]
    before = dict(os.environ)
    assert runtime.resolve(["{framework_python}", *suffix]) == [sys.executable, *suffix]
    assert runtime.resolve(["{framework_python}"]) == [sys.executable]
    assert runtime.fingerprint() is None
    assert dict(os.environ) == before


def test_explicit_framework_python_selects_pinned_interpreter_without_injecting_flags(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    suffix = ["hook with spaces.py", "--root", "project with spaces", "{framework_python}"]
    with runtime.binding(path):
        assert runtime.resolve(["{framework_python}", *suffix]) == [str(installed[0]), *suffix]
        assert runtime.resolve(["{framework_python}"]) == [str(installed[0])]
        isolated = ["-I", "-B", *suffix]
        assert runtime.resolve(["{framework_python}", *isolated]) == [str(installed[0]), *isolated]
        for argv in (["prefix{framework_python}", "hook.py"],
                     ["{framework_python}/suffix", "hook.py"],
                     ["provider", "{framework_python}"],
                     ["uv", "run", "python", "hook.py"],
                     [sys.executable, "hook.py"]):
            assert runtime.resolve(argv) == argv


def test_explicit_framework_python_isolated_child_ignores_checkout_and_pythonpath(tmp_path, installed, monkeypatch):
    from match_recovery.processes import run_bounded

    path = write_binding(tmp_path, installed)
    checkout = tmp_path / "old checkout"
    checkout.mkdir()
    poisoned = checkout / "match_recovery"
    poisoned.mkdir()
    marker = checkout / "old-package-imported"
    (poisoned / "__init__.py").write_text(
        f"from pathlib import Path\nPath({str(marker)!r}).write_text('imported')\n"
        "raise RuntimeError('checkout package is not the selected installation')\n"
    )
    monkeypatch.setenv("PYTHONPATH", str(checkout))
    hook = checkout / "hook.py"
    hook.write_text(
        "import json, sys\n"
        "import match_recovery\n"
        "from match_recovery.framework_runtime import fingerprint\n"
        "print(json.dumps({'package': match_recovery.__file__, 'python': sys.executable,\n"
        "                  'isolated': sys.flags.isolated, 'no_bytecode': sys.dont_write_bytecode,\n"
        "                  'authority': fingerprint(), 'argv': sys.argv[1:]}))\n"
    )
    # Positive poison control: without the project's explicit isolation flags,
    # Python really can import the conflicting checkout package.
    poison_control = subprocess.run([str(installed[0]), "-B", str(hook)], cwd=checkout,
                                    capture_output=True, timeout=30)
    assert poison_control.returncode != 0 and marker.exists()
    marker.unlink()
    with runtime.binding(path):
        result = run_bounded(["{framework_python}", "-I", "-B", str(hook),
                              "two words", "{framework_python}"], cwd=checkout, timeout=30)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    observed = json.loads(result.stdout)
    assert Path(observed["package"]).parent == installed[1]
    assert observed["python"] == str(installed[0])
    assert observed["isolated"] == 1 and observed["no_bytecode"] is True
    assert observed["authority"] is None  # Interpreter selection does not grant runtime authority.
    assert observed["argv"] == ["two words", "{framework_python}"]
    assert not marker.exists()


def test_explicit_framework_python_refuses_changed_installation_before_execution(tmp_path, installed):
    from match_recovery.processes import run_bounded

    path = write_binding(tmp_path, installed)
    marker = tmp_path / "changed-runtime-hook-ran"
    with pytest.raises(ValueError, match="framework runtime content changed"):
        with runtime.binding(path):
            target = installed[1] / "__init__.py"
            target.write_text(target.read_text() + "\n# changed installation\n")
            run_bounded(["{framework_python}", "-I", "-B", "-c",
                         f"from pathlib import Path; Path({str(marker)!r}).write_text('ran')"],
                        cwd=tmp_path, timeout=30)
    assert not marker.exists()
    assert runtime.fingerprint() is None


def test_same_binding_nesting_restores_outer_context(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    other = write_binding(tmp_path, installed, name="other.json", launchers=[["recover"]])
    with runtime.binding(path):
        original = runtime.fingerprint()
        with runtime.binding(path):
            assert runtime.fingerprint() == original
        with pytest.raises(ValueError):
            with runtime.binding(other):
                pytest.fail("A nested context must not replace native runtime authority")
        assert runtime.fingerprint() == original
    assert runtime.fingerprint() is None


@pytest.mark.parametrize("damage", ["modify", "add", "remove"])
def test_changed_installed_package_is_refused(tmp_path, installed, damage):
    path = write_binding(tmp_path, installed)
    _, package, _ = installed
    if damage == "modify":
        target = package / "__init__.py"
        target.write_text(target.read_text() + "\n# changed runtime\n")
    elif damage == "add":
        (package / "unexpected.py").write_text("VALUE = 'unapproved'\n")
    else:
        (package / "inspection.py").unlink()
    with pytest.raises(ValueError):
        with runtime.binding(path):
            pytest.fail("Changed runtime reached activation")
    assert runtime.fingerprint() is None


def test_changed_interpreter_is_refused_without_running_replacement(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    python, _, _ = installed
    marker = tmp_path / "replacement-ran"
    python.unlink()  # Test-owned venv symlink, never its shared target.
    python.write_text(f"#!/bin/sh\ntouch '{marker}'\nexit 1\n")
    python.chmod(0o755)
    with pytest.raises(ValueError):
        with runtime.binding(path):
            pytest.fail("Replaced interpreter reached activation")
    assert not marker.exists()


def test_editable_install_is_not_a_pinnable_runtime(installed):
    python, _, metadata = installed
    (metadata / "direct_url.json").write_text(json.dumps({
        "url": "file:///untrusted/source", "dir_info": {"editable": True},
    }))
    with pytest.raises(ValueError):
        runtime.inspect(python)


def test_loaded_library_must_match_the_selected_runtime(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    old_python, old_package, _ = install_copy(tmp_path / "old-runtime")
    target = old_package / "inspection.py"
    target.write_text(target.read_text() + "\n# previous framework build\n")
    script = (
        "import sys\n"
        "from pathlib import Path\n"
        "from match_recovery.framework_runtime import binding\n"
        "try:\n"
        "    with binding(Path(sys.argv[1])):\n"
        "        raise RuntimeError('mismatched loaded framework admitted')\n"
        "except ValueError:\n"
        "    print('refused different loaded framework')\n"
    )
    result = subprocess.run([str(old_python), "-I", "-B", "-c", script, str(path)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    assert result.stdout.strip() == "refused different loaded framework"


def test_identical_code_in_another_installation_cannot_claim_selected_runtime(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    other_python, _, _ = install_copy(tmp_path / "identical-other-runtime")
    selected = runtime.inspect(installed[0])
    other = runtime.inspect(other_python)
    assert other["package_sha256"] == selected["package_sha256"]
    assert other["executable_sha256"] == selected["executable_sha256"]
    assert other["prefix"] != selected["prefix"]
    script = (
        "import sys\n"
        "from pathlib import Path\n"
        "from match_recovery.framework_runtime import binding, fingerprint\n"
        "try:\n"
        "    with binding(Path(sys.argv[1])):\n"
        "        assert fingerprint() is not None\n"
        "        print('selected installation active')\n"
        "except ValueError:\n"
        "    print('different installation refused')\n"
        "    raise SystemExit(3)\n"
    )
    rejected = subprocess.run([str(other_python), "-I", "-B", "-c", script, str(path)],
                              capture_output=True, text=True, timeout=30)
    assert rejected.returncode == 3, rejected.stderr
    assert rejected.stdout.strip() == "different installation refused"
    accepted = subprocess.run([str(installed[0]), "-I", "-B", "-c", script, str(path)],
                              capture_output=True, text=True, timeout=30)
    assert accepted.returncode == 0, accepted.stderr
    assert accepted.stdout.strip() == "selected installation active"
    command = [str(installed[0]), "-I", "-B", "-m", "match_recovery", "--runtime", str(path), "--help"]
    help_result = subprocess.run(command, capture_output=True, text=True, timeout=30)
    assert help_result.returncode == 0, help_result.stderr
    assert "campaign" in help_result.stdout and "inventory" in help_result.stdout


@pytest.mark.parametrize("action", ["fingerprint", "resolve", "native_argv"])
def test_binding_mutation_during_context_refuses_further_use(tmp_path, installed, action):
    path = write_binding(tmp_path, installed)
    with pytest.raises(ValueError):
        with runtime.binding(path):
            document = json.loads(path.read_text())
            document["launchers"] = [["provider"]]
            path.write_text(json.dumps(document))
            if action == "fingerprint":
                runtime.fingerprint()
            elif action == "resolve":
                runtime.resolve(["uv", "run", "recover", "inventory"])
            else:
                runtime.native_argv(["inventory"])
    assert runtime.fingerprint() is None


def test_runtime_mutation_during_context_refuses_further_use(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    _, package, _ = installed
    with pytest.raises(ValueError):
        with runtime.binding(path):
            target = package / "__init__.py"
            target.write_text(target.read_text() + "\n# changed after validation\n")
            runtime.resolve(["uv", "run", "recover", "inventory"])


def test_equal_content_environment_configuration_alias_is_refused(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    configuration = installed[0].parent.parent / "pyvenv.cfg"
    replacement = tmp_path / "aliased-pyvenv.cfg"
    replacement.write_bytes(configuration.read_bytes())
    with pytest.raises(ValueError):
        with runtime.binding(path):
            configuration.unlink()
            configuration.symlink_to(replacement)
            runtime.fingerprint()
    assert configuration.is_symlink()
    assert configuration.read_bytes() == replacement.read_bytes()


@pytest.mark.parametrize("field,value", [("schema_version", 2), ("unexpected", True),
                                         ("launchers", []), ("launchers", [["uv", "run"]])])
def test_invalid_binding_refuses(tmp_path, installed, field, value):
    path = write_binding(tmp_path, installed)
    document = json.loads(path.read_text())
    document[field] = value
    path.write_text(json.dumps(document))
    with pytest.raises(ValueError):
        with runtime.binding(path):
            pytest.fail("Malformed or overly broad launcher binding was accepted")


def test_bound_subprocess_uses_new_install_not_old_worker_launcher(tmp_path, installed, monkeypatch):
    path = write_binding(tmp_path, installed)
    old_worker = tmp_path / "old-worker"
    old_worker.mkdir()
    old_bin = old_worker / "bin"
    old_bin.mkdir()
    marker = old_worker / "old-launcher-ran"
    old_uv = old_bin / "uv"
    old_uv.write_text(f"#!/bin/sh\ntouch '{marker}'\nexit 77\n")
    old_uv.chmod(0o755)
    old_package = old_worker / "match_recovery"
    old_package.mkdir()
    (old_package / "__init__.py").write_text("raise RuntimeError('old checkout imported')\n")
    monkeypatch.setenv("PATH", str(old_bin) + os.pathsep + os.environ["PATH"])
    monkeypatch.setenv("PYTHONPATH", str(old_worker))
    unbound = subprocess.run(["uv", "run", "recover", "--help"], cwd=old_worker,
                             capture_output=True, timeout=30)
    assert unbound.returncode == 77 and marker.exists()
    marker.unlink()
    with runtime.binding(path):
        argv = runtime.resolve(["uv", "run", "recover", "--help"])
        result = subprocess.run(argv, cwd=old_worker, capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    assert "campaign" in result.stdout and "inventory" in result.stdout
    assert not marker.exists()

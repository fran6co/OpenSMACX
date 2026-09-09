"""Compiler diagnostics must not wait for unrelated Wine service pipe owners."""

import os
import signal
import subprocess
import sys

import pytest

from recovery.environment import wine


@pytest.mark.parametrize("code", [0, 1, 3])
def test_tool_preserves_status_and_both_diagnostic_streams(code):
    result = wine.run_tool([sys.executable, "-c",
                           "import os,sys; os.write(1,b'output\\r\\n'); "
                           "os.write(2,b'error\\xff\\r\\n'); sys.exit(int(sys.argv[1]))", str(code)])
    assert result.returncode == code
    assert result.stdout == "output\n"
    assert result.stderr == "error\ufffd\n"


@pytest.mark.skipif(os.name != "posix", reason="models inherited Wine descriptors using fork")
def test_tool_returns_while_background_output_owner_is_alive(tmp_path, monkeypatch):
    pid_file = tmp_path / "background.pid"
    original_run = subprocess.run

    def bounded(*args, **kwargs):
        # A PIPE-based implementation times out: the background process owns
        # both output descriptors until our finally clause terminates it.
        return original_run(*args, timeout=5, **kwargs)

    monkeypatch.setattr(wine.subprocess, "run", bounded)
    script = ("import os,signal,sys\n"
              "pid=os.fork()\n"
              "if pid == 0:\n"
              "    signal.pause()\n"
              "    os._exit(0)\n"
              "with open(sys.argv[1], 'w') as stream: stream.write(str(pid))\n"
              "os.write(1,b'compiler output\\n')\n"
              "os.write(2,b'compiler warning\\n')\n"
              "os._exit(7)\n")
    try:
        result = wine.run_tool([sys.executable, "-c", script, str(pid_file)])
        assert result.returncode == 7
        assert result.stdout == "compiler output\n"
        assert result.stderr == "compiler warning\n"
        os.kill(int(pid_file.read_text()), 0)
    finally:
        if pid_file.exists():
            os.kill(int(pid_file.read_text()), signal.SIGTERM)


def test_missing_tool_is_not_reported_as_success():
    with pytest.raises(FileNotFoundError):
        wine.run_tool(["/nonexistent/recovery-test-compiler"])


@pytest.mark.parametrize("name", ["LINK", "_LINK_", "link", "_link_", "LiNk"])
def test_implicit_linker_options_refuse_before_running_any_tool(monkeypatch, name):
    monkeypatch.setenv(name, "/ALTERNATENAME:_g=_other")
    def unexpected(*args, **kwargs):
        pytest.fail("compiler was invoked with unobserved linker options")
    monkeypatch.setattr(wine.subprocess, "run", unexpected)
    with pytest.raises(ValueError, match="implicit linker environment options"):
        wine.run_tool(["unused"])


def test_linker_option_environment_is_explicitly_empty_in_child(monkeypatch):
    for name in tuple(os.environ):
        if name.upper() in {"LINK", "_LINK_"}:
            monkeypatch.delenv(name)
    monkeypatch.setenv("link", "")
    result = wine.run_tool([sys.executable, "-c",
        "import os; assert os.environ['LINK'] == os.environ['_LINK_'] == ''; assert 'link' not in os.environ"])
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("tool,arguments", [
    ("cl", ["/VERBOSE", "--link-diagnostics=out.log"]),
    ("lib", ["/VERBOSE", "--link-diagnostics=out.log"]),
    ("link", ["/VERBOSE:LIB", "--link-diagnostics=out.log"]),
    ("link", ["/VERBOSE", "--link-diagnostics="]),
    ("link", ["/VERBOSE", "--link-diagnostics=out.log", "--link-diagnostics=second.log"])])
def test_ambiguous_or_wrong_tool_diagnostic_requests_refuse(tool, arguments):
    with pytest.raises(ValueError):
        wine.take_link_diagnostics(tool, arguments)


def test_link_diagnostics_preserve_argv_and_replace_only_derived_output(tmp_path):
    path = tmp_path / "app.link.log"
    args = ["/VERBOSE", "one.obj", "@inputs.rsp", "--link-diagnostics=" + str(path)]
    kept, output = wine.take_link_diagnostics("link", args)
    assert kept == args[:-1] and output == path
    wine.write_link_diagnostics(output, "first complete log\n")
    wine.write_link_diagnostics(output, "second complete log\n")
    assert output.read_text() == "second complete log\n"
    assert list(tmp_path.iterdir()) == [path]
    alias = tmp_path / "alias.log"
    alias.symlink_to(path)
    with pytest.raises(ValueError):
        wine.take_link_diagnostics("link", ["/VERBOSE", "--link-diagnostics=" + str(alias)])


@pytest.mark.parametrize("code", [0, 1])
def test_wrapper_captures_unfiltered_link_log_without_losing_exit_status(tmp_path, monkeypatch, capsys, code):
    (tmp_path / "BIN").mkdir()
    (tmp_path / "BIN/LINK.EXE").write_bytes(b"synthetic executable")
    monkeypatch.setattr(wine, "VC6_ROOT", tmp_path)
    log = "Start Pass1\nEnd Pass1\nStart Pass2\nsource.cpp.obj\nEnd Pass2\n"
    log += "LINK : warning LNK9999: synthetic warning\n"
    received = []
    def run(command):
        received.extend(command)
        return subprocess.CompletedProcess(command, code, log, "stderr preserved\n")
    monkeypatch.setattr(wine, "run_tool", run)
    path = tmp_path / "app.link.log"
    assert wine.main(["vc6-link", "/VERBOSE", "one.obj", "--link-diagnostics=" + str(path)]) == code
    assert path.read_text() == log and not any("link-diagnostics" in arg for arg in received)
    output = capsys.readouterr()
    assert "warning LNK9999" in output.out and output.err == "stderr preserved\n"
    assert ("Start Pass1" in output.out) == (code != 0)

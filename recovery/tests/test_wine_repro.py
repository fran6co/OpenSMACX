"""Compiler reproducibility is explicit and cannot silently rewrite macros."""
from pathlib import Path
import subprocess
import sys

import pytest

from recovery.environment.wine_repro import ReproducibleCompile, _preprocessed
from recovery.environment import wine


@pytest.fixture
def invocation(tmp_path, monkeypatch):
    monkeypatch.delenv("CL", raising=False)
    monkeypatch.delenv("_CL_", raising=False)
    (tmp_path / "src").mkdir()
    (tmp_path / "build").mkdir()
    source = tmp_path / "src/unit.cpp"
    source.write_text("int value() { return 7; }\n")
    args = ["--repro-root=" + str(tmp_path), "/nologo", "/c", "/O2", "/Gy", "/Z7",
            "/I../src", "/Founit.obj", str(source)]
    return tmp_path, source, args


def preprocessed(path, body=b"int value() { return 7; }\n"):
    return b'#line 1 "' + str(path).encode().replace(b"\\", b"\\\\") + b'"\n' + body


def test_explicit_identity_preserves_output_and_include_destinations(invocation):
    root, source, args = invocation
    policy = ReproducibleCompile.read("cl", args, root / "build")
    seen = []
    def preprocess(words, cwd):
        seen.append((words, cwd))
        return preprocessed(source if cwd == root / "build" else Path("src/unit.cpp"))
    result = policy.verify(preprocess)
    assert result[-1] == "/Brepro" and result[-2] == "src/unit.cpp"
    assert "/Fo" + str(root / "build/unit.obj") in result
    assert "/I" + str(root / "build/../src") in result
    assert [cwd for _, cwd in seen] == [root / "build", root, root]
    assert all("/E" in words and "/c" not in words for words, _ in seen)
    assert all(not any(word.startswith("/Fo") for word in words) for words, _ in seen)
    policy.check_inputs()


@pytest.mark.parametrize("phase", [1, 2])
@pytest.mark.parametrize("body", [b'const char* s = "different.cpp";\n', b'const char* s = "12:34:56";\n',
                                  b'const char* s = "Sep 10 2026";\n', b'const char s[] = "\xff";\n',
                                  b'#pragma comment(lib, "other")\n'])
def test_changed_code_literals_and_pragmas_refuse(invocation, phase, body):
    root, source, args = invocation
    policy = ReproducibleCompile.read("cl", args, root / "build")
    calls = []
    def preprocess(words, cwd):
        calls.append(words)
        return preprocessed(source, body if len(calls) - 1 == phase else b"unchanged\n")
    with pytest.raises(ValueError, match="changes preprocessing"):
        policy.verify(preprocess)


def test_new_conditional_header_refuses_even_if_expanded_text_agrees(invocation):
    root, source, args = invocation
    header = root / "src/other.h"
    header.write_text("// different include\n")
    policy = ReproducibleCompile.read("cl", args, root / "build")
    def preprocess(words, cwd):
        data = preprocessed(source)
        return data + (preprocessed(header, b"") if "/Brepro" in words else b"")
    with pytest.raises(ValueError, match="observed inputs"):
        policy.verify(preprocess)


def test_source_edit_between_guard_passes_refuses(invocation):
    root, source, args = invocation
    policy = ReproducibleCompile.read("cl", args, root / "build")
    def preprocess(words, cwd):
        if cwd == root:
            source.write_text("int changed;\n")
        return preprocessed(source)
    with pytest.raises(ValueError, match="inputs changed"):
        policy.verify(preprocess)


@pytest.mark.parametrize("flag", ["/Yupch.h", "/Ycpch.h", "/YX", "/Fppch.pch", "/Brepro", "/B1custom", "/B2custom", "/d1Brepro", "-d1Brepro", "/d1", "/d2", "/P", "/E", "/EP", "/Zi", "/Tpother.cpp", "@hidden.rsp", "/FRbrowse"])
def test_unsupported_hidden_or_output_inputs_refuse(invocation, flag):
    root, source, args = invocation
    with pytest.raises(ValueError, match="unsupported"):
        ReproducibleCompile.read("cl", [*args, flag], root / "build")


def test_policy_is_explicit_and_cl_only(invocation):
    root, _, args = invocation
    assert ReproducibleCompile.read("cl", args[1:], root / "build") is None
    with pytest.raises(ValueError, match="only for cl"):
        ReproducibleCompile.read("link", args, root / "build")
    with pytest.raises(ValueError, match="one explicit"):
        ReproducibleCompile.read("cl", [args[0], *args], root / "build")


@pytest.mark.parametrize("name", ["CL", "_CL_", "cl"])
def test_implicit_options_refuse(invocation, monkeypatch, name):
    root, _, args = invocation
    monkeypatch.setenv(name, "/DUNOBSERVED=1")
    with pytest.raises(ValueError, match="implicit CL"):
        ReproducibleCompile.read("cl", args, root / "build")


def test_preprocessor_metadata_is_not_a_broad_filename_rewriter(invocation):
    root, source, _ = invocation
    body = b'const char s[] = "#line 9 \\"other.cpp\\"";\n'
    text, paths = _preprocessed(preprocessed(source, body), root)
    assert text == body and paths == {source}
    assert _preprocessed(b" \t" + preprocessed(source, body), root) == (body, {source})
    with pytest.raises(ValueError, match="unsupported preprocessor"):
        _preprocessed(b'#line 1 "unterminated\n', root)
    with pytest.raises(ValueError, match="no observed input"):
        _preprocessed(b"int value;\n", root)


def test_binary_capture_preserves_non_ascii_and_requested_cwd(tmp_path):
    result = wine.run_tool([sys.executable, "-c", "import os; os.write(1,b'\\xff\\r\\n'); os.write(2,os.fsencode(os.getcwd()))"],
                           cwd=tmp_path, binary=True)
    assert result.returncode == 0 and result.stdout == b"\xff\r\n"
    assert result.stderr == str(tmp_path).encode()


def test_compiler_overlay_remaps_only_its_explicit_reproducible_root(tmp_path):
    from recovery.environment.compile import _variant_arguments
    assert _variant_arguments(("--repro-root=" + str(tmp_path),), tmp_path,
                              tmp_path / "replica", ("src",)) == ("--repro-root=" + str(tmp_path / "replica"),)
    with pytest.raises(ValueError, match="declared project root"):
        _variant_arguments(("--repro-root=/unrelated",), tmp_path, tmp_path / "replica", ("src",))


def test_recipe_preserves_explicit_compiler_policy(invocation):
    import json
    from match_recovery.compilation_database import entries
    from recovery.environment.recipe import invocation as recipe
    root, source, args = invocation
    entry, = entries(json.dumps([{"directory": str(root / "build"), "file": str(source),
                                  "arguments": ["cl", *args]}]).encode(), root)
    assert "--repro-root=" + str(root) in recipe(entry)


@pytest.mark.parametrize("macro", [None, "__FILE__", "__DATE__", "__TIME__", "__TIMESTAMP__"])
def test_real_vc6_guard_keeps_macros_visible_and_stable_names(invocation, macro):
    from recovery.environment.compile import compile_object
    from match_recovery.pe_coff import Coff
    from match_recovery.coff_replica import object_signature
    if not (wine.VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root, source, _ = invocation
    source.write_text('namespace { int local = 3; int helper(int n) { return n + local; } }\n'
                      'int value(int n) { return helper(n); }\n' +
                      ('const char *visible = ' + macro + ';\n' if macro else 'const char *visible = "unchanged";\n'))
    command = (sys.executable, str(Path(wine.__file__)), "--tool=cl", "--repro-root=" + str(root), "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Ob0")
    if macro:
        with pytest.raises(ValueError, match="changes preprocessing"):
            compile_object(source, command, flags)
    else:
        first = Coff(compile_object(source, command, flags))
        second = Coff(compile_object(source, command, flags))
        assert object_signature(first) == object_signature(second)
        assert any(symbol.name == "?local@?%src/unit.cpp@@3HA" for symbol in first.symbols.values())


@pytest.mark.parametrize("macro", ["__FILE__", "__DATE__", "__TIME__"])
def test_real_vc6_pragma_macro_is_not_hidden_by_preprocessing(invocation, macro):
    from recovery.environment.compile import compile_object
    if not (wine.VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root, source, _ = invocation
    source.write_text('#pragma comment(user, ' + macro + ')\nint value() { return 7; }\n')
    command = (sys.executable, str(Path(wine.__file__)), "--tool=cl", "--repro-root=" + str(root), "/nologo")
    with pytest.raises(ValueError, match="changes preprocessing"):
        compile_object(source, command, ("/c", "/O2", "/Gy"))


def test_explicit_compiler_environment_clears_windows_default_option_channels(monkeypatch):
    monkeypatch.setenv("cl", "hidden")
    result = wine.run_tool([sys.executable, "-c",
        "import os; assert os.environ['CL'] == os.environ['_CL_'] == ''; assert 'cl' not in os.environ"],
        explicit_compiler=True)
    assert result.returncode == 0, result.stderr


def test_depfile_escapes_observed_paths_and_keeps_original_output_directory(invocation):
    root, source, args = invocation
    extra = root / "src/a #$ file.h"
    extra.write_text("// dependency\n")
    policy = ReproducibleCompile.read("cl", args, root / "build")
    policy.verify(lambda words, cwd: preprocessed(source) + preprocessed(extra, b""))
    policy.write_depfile("unit.d", "unit file.obj")
    contents = (root / "build/unit.d").read_text()
    assert contents.startswith("unit\\ file.obj:")
    assert "a\\ \\#$$\\ file.h" in contents
    assert str(source) in contents

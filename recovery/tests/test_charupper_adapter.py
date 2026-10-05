"""Source/native controls proposed for the 0x58FB70 Win32 text adapter."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest

from match_recovery.inspection import inspect_source
from match_recovery.source import Mode, read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src").is_dir() and
            (parent / "recovery" / "tests").is_dir())
_spec = importlib.util.spec_from_file_location(
    "game_logic_seed_batch_behavior",
    ROOT / "recovery" / "tests" / "test_game_logic_seed_batch_behavior.py")
_helper = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_helper)
_definition = _helper._definition

IMAGE = ROOT / ".opensmacx" / "game" / "terranx_original.exe"
HARNESS = Path(__file__).with_name("charupper_adapter_harness.cpp")
ADDRESS = 0x0058FB70
ORIGINAL_NAME = "sub_58fb70"
SOURCE_SYMBOL = "?uppercase_text@@YAPADPADH@Z"
SIGNATURE = "char *__cdecl uppercase_text("
EXPECTED_SPANS = ((0x0058FB70, 0x0058FBB2),)


def _source_body():
    records = [row for row in read_file(ROOT / "src/stringstruct.cpp")
               if row.address == ADDRESS]
    assert len(records) == 1, "0x58FB70 must be homed once in stringstruct.cpp"
    record = records[0]
    assert record.mode == Mode.BODY
    assert record.name == ORIGINAL_NAME
    assert record.symbol == SOURCE_SYMBOL
    assert record.image_spans == EXPECTED_SPANS
    assert record.region and not record.extract_error
    body = _definition(record.region, SIGNATURE)
    assert "BODY GOES HERE." not in body
    return body


def _render(body):
    template = HARNESS.read_text()
    marker = "/*EXTRACTED_UPPERCASE_TEXT*/"
    assert template.count(marker) == 1
    return template.replace(marker, body)


def _run_host(tmp_path, body, compiler):
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "uppercase_adapter_host.cpp"
    executable = tmp_path / "uppercase_adapter_host"
    source.write_text(_render(body))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(source), "-o", str(executable)], text=True, capture_output=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          env={**os.environ, "ASAN_OPTIONS": "detect_leaks=0"})


def _replace_once(text, old, new):
    assert text.count(old) == 1, "mutation anchor drift: " + old
    return text.replace(old, new)


def test_pinned_native_api_edges_and_sdk_import_binding():
    # This is a native PE control, not just a source comment check. It requires
    # both original indirect calls to resolve to the pinned USER32 import.
    report = inspect_source(ROOT, ["0x0058FB70"], image=IMAGE)
    assert report["image_verified"] is True
    subjects = [row for row in report["subjects"]
                if row["source"]["path"] == "src/stringstruct.cpp"]
    assert len(subjects) == 1
    original = subjects[0]["original"]
    assert original["result"] == "pass"
    assert original["spans"][0]["decoded_bytes"] == 66
    imported = [edge for edge in original["calls"]
                if edge["import"] and edge["import"]["name"] == "CharUpperA"]
    assert [(edge["at"], edge["slot"], edge["import"]["module"])
            for edge in imported] == [
                (0x0058FB96, 0x0066931C, "USER32.dll"),
                (0x0058FBA7, 0x0066931C, "USER32.dll")]

    # Source and SDK declaration evidence accompany the PE edge: Windows
    # headers supply the real API declaration; no private-address call is used.
    game_header = (ROOT / "src/game.h").read_text()
    assert "RECOVERY_IMPORT: v1 __imp__CharUpperA@4 USER32.dll name:CharUpperA" in game_header
    sdk_header = Path("/home/fran6co/opt/vc6/INCLUDE/WINUSER.H")
    if sdk_header.exists():
        sdk = sdk_header.read_text(errors="replace")
        assert "CharUpperA(" in sdk and "LPSTR lpsz" in sdk


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_owner_source_adapter_branches_and_mutation_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable: " + compiler_name)
    body = _source_body()
    original = _run_host(tmp_path / "pristine", body, compiler)
    assert original.returncode == 0, original.stdout + original.stderr
    assert "PASS: extracted uppercase adapter host controls" in original.stdout

    mutations = [
        ("noop-branch", _replace_once(body, "if (mode == 0)", "if (mode != 0)")),
        ("first-character-branch", _replace_once(body, "if (mode == 1)", "if (mode == 3)")),
        ("one-byte-terminator", _replace_once(body, "first_character[1] = '\\0';",
                                               "first_character[1] = 'x';")),
        ("first-character-writeback", _replace_once(body, "        *text = first_character[0];\n", "")),
        ("full-string-call", _replace_once(body, "    CharUpperA(text);",
                                            "    CharUpperA(text + 1);")),
        ("return-value", _replace_once(body, "    return text;\n}", "    return 0;\n}")),
    ]
    for index, (label, changed) in enumerate(mutations):
        result = _run_host(tmp_path / ("mutation-" + str(index)), changed, compiler)
        assert result.returncode != 0, "mutation survived: " + label

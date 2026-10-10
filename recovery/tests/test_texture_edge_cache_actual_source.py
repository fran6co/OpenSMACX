"""Actual-source host controls for Texture's typed EdgeScan cache chain.

The tested helpers and TextureStore lifetime bodies are extracted from
src/texture.cpp and the record/cache declarations from src/texture.h. The host
fixed_div is a signed 16.16 service double. This checks source behavior and
mutant sensitivity, not VC6 code generation, native ABI/linking, or full
rasterization acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"
HEADER = ROOT / "src/texture.h"
HARNESS = Path(__file__).with_name("texture_edge_cache_owner_harness.cpp")


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )


def _definition(source, signature):
    view = _masked(source)
    starts = [match.start() for match in re.finditer(re.escape(signature), view)]
    definitions = []
    for start in starts:
        opening_paren = view.find("(", start + len(signature) - 1)
        depth = 0
        close = -1
        for index in range(opening_paren, len(view)):
            if view[index] == "(":
                depth += 1
            elif view[index] == ")":
                depth -= 1
                if depth == 0:
                    close = index
                    break
        if close < 0:
            continue
        token = close + 1
        while token < len(view) and view[token].isspace():
            token += 1
        if token >= len(view) or view[token] != "{":
            continue
        depth = 0
        for end in range(token, len(view)):
            if view[end] == "{":
                depth += 1
            elif view[end] == "}":
                depth -= 1
                if depth == 0:
                    definitions.append(source[start:end + 1])
                    break
    assert len(definitions) == 1, signature
    return definitions[0]


def _declaration(source, kind, name):
    view = _masked(source)
    match = re.search(r"\b" + re.escape(kind) + r"\s+" + re.escape(name) + r"\s*\{", view)
    assert match, f"actual {kind} {name} declaration absent"
    opening = view.find("{", match.start())
    depth = 0
    for index in range(opening, len(view)):
        if view[index] == "{":
            depth += 1
        elif view[index] == "}":
            depth -= 1
            if depth == 0:
                end = view.find(";", index)
                assert end >= 0
                return source[match.start():end + 1]
    raise AssertionError(f"unterminated {kind} {name}")


def _translation_unit(source=None, header=None):
    source = SOURCE.read_text(encoding="utf-8") if source is None else source
    header = HEADER.read_text(encoding="utf-8") if header is None else header
    harness = HARNESS.read_text(encoding="utf-8")
    actual = {
        "// ACTUAL_EDGESCAN_DECLARATION": _declaration(header, "struct", "EdgeScan"),
        "// ACTUAL_VERT_DECLARATION": _declaration((ROOT / "src/vert.h").read_text(), "struct", "Vert"),
        "// ACTUAL_TEXTURESTORE_DECLARATION": _declaration(header, "class", "TextureStore"),
        "// ACTUAL_TEXTURESTORE_LIFETIME": "\n\n".join((
            _definition(source, "TextureStore::TextureStore("),
            _definition(source, "TextureStore::~TextureStore("),
        )),
        "// ACTUAL_EDGE_CACHE_HELPERS": "\n\n".join((
            _definition(source, "int __cdecl sub_620100("),
            _definition(source, "int __cdecl sub_620260("),
            _definition(source, "int __cdecl sub_624eb0("),
        )),
        "// ACTUAL_SENTINEL_WRAPPER": _definition(
            source, "void Texture::sub_624ca0("),
    }
    for marker, body in actual.items():
        assert harness.count(marker) == 1
        harness = harness.replace(marker, body, 1)
    return harness


def _run(tmp_path, source=None, header=None):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / "texture_edge_cache_actual.cpp"
    binary = tmp_path / "texture_edge_cache_actual"
    cpp.write_text(_translation_unit(source, header), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         str(cpp), "-o", str(binary)],
        check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=30)
    return ran


def test_actual_edge_cache_helpers_and_sentinel_wrapper(tmp_path):
    result = _run(tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("mutation", (
    ("replay_geometry_channel", "param_1->deltaA = cached.deltaA;",
     "param_1->deltaA = cached.deltaB;"),
    ("cache_wrap", "param_3->next_record_ = 0;",
     "param_3->next_record_ = slotBase;"),
    ("sentinel", "if (a6 == -1) {\n        sub_620a10(",
     "if (a6 != -1) {\n        sub_620a10("),
))
def test_actual_source_mutants_break_cache_or_sentinel_controls(tmp_path, mutation):
    _name, old, new = mutation
    source = SOURCE.read_text(encoding="utf-8")
    assert old in source
    mutated = source.replace(old, new, 1)
    result = _run(tmp_path, source=mutated)
    assert result.returncode != 0, f"{_name} mutant escaped assertions"

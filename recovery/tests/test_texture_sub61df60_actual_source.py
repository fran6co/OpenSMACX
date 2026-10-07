from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"
FIXTURE = Path(__file__).parent / "fixtures/texture_sub61df60/host.cpp"


def definition(source: str, name: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    matches = []
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", masked):
        depth = 1
        end = match.end()
        while end < len(masked) and depth:
            if masked[end] == "(":
                depth += 1
            elif masked[end] == ")":
                depth -= 1
            end += 1
        while end < len(masked) and masked[end].isspace():
            end += 1
        if depth == 0 and end < len(masked) and masked[end] == "{":
            matches.append((match.start(), end))
    assert len(matches) == 1, (name, len(matches))
    start = masked.rfind("\n", 0, matches[0][0]) + 1
    depth = 0
    for end in range(matches[0][1], len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(name + " body was not closed")


def live_translation_unit() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    owner = definition(source, "Texture::sub_61df60")
    caller = definition(source, "Texture::sub_624630")
    page_address = definition(source, "texture_sub61df60_page_address")
    signature = re.compile(
        r"\bvoid\s+Texture::sub_61df60\s*\(\s*Buffer\s*\*\s*\w+"
        r"\s*,\s*Vert\s*\*\s*\w+\s*,\s*int\s*\*\s*\w+"
        r"\s*,\s*Texture\s*\*\s*\w+\s*,\s*unsigned\s+char\s*\*"
        r"\s*\w+\s*,\s*int\s+\w+\s*,\s*Vert\s*\*\s*\w+"
        r"\s*,\s*int\s+\w+\s*,\s*unsigned\s+char\s*\*\s*\w+\s*\)")
    assert signature.search(owner), "installed typed owner signature is absent"
    assert "BODY GOES HERE." not in owner
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count("/* ACTUAL_CANDIDATE */") == 1
    assert re.search(
        r"\bvoid\s+Texture::sub_624630\s*\(\s*Buffer\s*\*", caller), \
        "caller is not yet typed with the recovered argument closure"
    helpers = "\n\n".join(definition(source, name) for name in (
        "texture_difference", "texture_span_pixels", "texture_span_step",
        "texture_light_step", "texture_raster_gradient_span_admitted"))
    setup = definition(source, "Texture::setup_edge_gouraud")
    opening = setup.index("{") + 1
    setup = setup[:opening] + "\n  ++setup_calls;" + setup[opening:]
    return (fixture.replace("/* ACTUAL_HELPERS */", helpers, 1)
            .replace("/* ACTUAL_SETUP */", setup, 1)
            .replace("/* ACTUAL_CANDIDATE */",
                            page_address + "\n\n" + owner, 1)
            .replace("/* ACTUAL_CALLER */", caller, 1))


def test_live_owner_gcc_clang(tmp_path: Path) -> None:
    source = live_translation_unit()
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if compiler is None:
            continue
        cpp = tmp_path / (compiler_name + ".cpp")
        exe = tmp_path / compiler_name
        cpp.write_text(source, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
             "-no-pie", str(cpp), "-o", str(exe)], cwd=ROOT,
            capture_output=True, text=True, timeout=60)
        assert built.returncode == 0, built.stdout + built.stderr
        run = subprocess.run([str(exe)], cwd=ROOT, capture_output=True,
                             text=True, timeout=30)
        assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("old,new", [
    ("if (borrowed_ == 0) goto cleanup;", "if (false) goto cleanup;"),
    ("if (mask_texture->borrowed_ == 0) goto cleanup;", "if (false) goto cleanup;"),
    ("if (--right.counter == 0)", "if (false)"),
    ("du = pending_du;", "du = 0;"),
    ("*blit_span_right = clipped_left;", "(void)clipped_left;"),
    ("*blit_slot_498 = light >> 8;", "*blit_slot_498 = light;"),
    ("static_cast<unsigned short>(light_step)) * skip", "static_cast<short>(light_step)) * skip"),
    ("| out[0];", "| base_texel;"),
])
def test_actual_owner_mutants_are_rejected(old: str, new: str, tmp_path: Path) -> None:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("GCC host control unavailable")
    source = live_translation_unit()
    assert source.count(old) == 1
    cpp = tmp_path / "mutant.cpp"
    exe = tmp_path / "mutant"
    cpp.write_text(source.replace(old, new, 1), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-no-pie", str(cpp), "-o", str(exe)],
        cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(exe)], cwd=ROOT, capture_output=True, text=True, timeout=30)
    assert run.returncode != 0, "mutant escaped the actual-owner controls"

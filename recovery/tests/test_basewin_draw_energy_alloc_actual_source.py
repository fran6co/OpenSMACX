"""Host controls for the installed BaseWin draw_energy_alloc owner.

These compile the source-reader-selected actual owner body against a compact
host adapter. They test source behavior, not the target ABI or a running game.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).parent / "fixtures" / "basewin_draw_energy_alloc_host.cpp.in"
ADDRESS = 0x00411980
SIGNATURE = "void BaseWin::draw_energy_alloc("


def _masked_code(source):
    """Mask comments and literals while preserving offsets and braces."""
    out = list(source)
    index = 0
    state = "code"
    quote = ""
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "line"
                continue
            if char == "/" and nxt == "*":
                out[index] = out[index + 1] = " "
                index += 2
                state = "block"
                continue
            if char in ('"', "'"):
                quote = char
                out[index] = " "
                state = "quoted"
        elif state == "line":
            if char == "\n":
                state = "code"
            else:
                out[index] = " "
        elif state == "block":
            if char == "*" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                out[index] = " "
        else:
            if char == "\\":
                out[index] = " "
                if index + 1 < len(source) and source[index + 1] != "\n":
                    out[index + 1] = " "
                    index += 2
                    continue
            elif char == quote:
                out[index] = " "
                state = "code"
            elif char != "\n":
                out[index] = " "
        index += 1
    return "".join(out)


def _actual_owner():
    records = [row for row in read_file(ROOT / "src" / "basewin.cpp")
               if row.address == ADDRESS]
    assert len(records) == 1, "missing or duplicate original owner mapping"
    row = records[0]
    assert row.mode == Mode.FILE
    assert row.name == "?draw_energy_alloc@BaseWin@@QAEXH@Z"
    assert row.image_spans == ((0x411980, 0x412A96),
                               (0x651B4C, 0x651B61))
    assert row.region and SIGNATURE in row.region
    masked = _masked_code(row.region)
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                body = row.region[start:index + 1]
                break
    else:
        raise AssertionError("source-reader owner has no balanced function body")
    assert "BODY GOES HERE." not in body
    assert "(void)a1;" not in body
    assert "garrison_rect_" not in body
    return body


def _compile_run(directory, body, compiler):
    harness = FIXTURE.read_text()
    marker = "// ACTUAL_FUNCTION_INSERTION_POINT"
    assert harness.count(marker) == 1
    source = harness.replace(marker, body)
    cpp = directory / "basewin_draw_energy_alloc.cpp"
    exe = directory / "basewin_draw_energy_alloc"
    cpp.write_text(source)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
    )
    return subprocess.run([str(exe)], check=False, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True)


def test_installed_basewin_draw_energy_alloc_actual_owner_and_callback_reload():
    compilers = [compiler for compiler in ("g++", "clang++")
                 if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    body = _actual_owner()
    with tempfile.TemporaryDirectory(prefix="osmx-basewin-energy-host-") as temp:
        for compiler in compilers:
            result = _compile_run(Path(temp), body, compiler)
            assert result.returncode == 0, f"{compiler}: {result.stdout}"


def test_installed_basewin_draw_energy_alloc_actual_source_damage_controls():
    compilers = [compiler for compiler in ("g++", "clang++")
                 if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    body = _actual_owner()
    mutants = (
        ("TextColourTable[86]", "TextColourTable[87]"),
        ("psych_display_add(row_bounds.top, 0x2A)",
         "psych_display_add(row_bounds.top, 0x15)"),
        ("label_get(0x4F8 / 4)", "label_get(0x4F0 / 4)"),
        ("soft_update(&production_content_rect_)",
         "update(&production_content_rect_, 0)"),
        ("_itoa(saved_clip.top, number, 10)",
         "_itoa(psych_total, number, 10)"),
        ("Base *marker_base = BaseCurrent;",
         "Base *marker_base = totals_base;"),
    )
    with tempfile.TemporaryDirectory(prefix="osmx-basewin-energy-mutants-") as temp:
        root = Path(temp)
        for compiler in compilers:
            for index, (old, new) in enumerate(mutants):
                assert old in body, f"unanchored damage control {old!r}"
                case = root / f"{compiler.replace('+', 'p')}-{index}"
                case.mkdir()
                result = _compile_run(case, body.replace(old, new, 1), compiler)
                assert result.returncode != 0, (
                    f"{compiler} failed to detect damage control {index}"
                )

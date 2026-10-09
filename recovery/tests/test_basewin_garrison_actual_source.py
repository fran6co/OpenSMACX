from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/basewin.cpp").is_file())
FIXTURE = Path(__file__).with_name("basewin_garrison_host_fixture.cpp")
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "void BaseWin::draw_garrison(int update_mode)"


def _masked_cpp(text: str) -> str:
    """Blank comments and string/character literals without changing offsets."""
    out = list(text)
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        if state == "code":
            if text.startswith("//", index):
                out[index:index + 2] = "  "
                index += 2
                state = "line"
            elif text.startswith("/*", index):
                out[index:index + 2] = "  "
                index += 2
                state = "block"
            elif text[index] in "\"'":
                quote = text[index]
                out[index] = " "
                index += 1
                state = "quoted"
            else:
                index += 1
        elif state == "line":
            if text[index] == "\n":
                state = "code"
            else:
                out[index] = " "
            index += 1
        elif state == "block":
            if text.startswith("*/", index):
                out[index:index + 2] = "  "
                index += 2
                state = "code"
            else:
                if text[index] != "\n":
                    out[index] = " "
                index += 1
        else:
            if text[index] == "\\":
                out[index] = " "
                if index + 1 < len(text):
                    if text[index + 1] != "\n":
                        out[index + 1] = " "
                    index += 2
                else:
                    index += 1
            elif text[index] == quote:
                out[index] = " "
                index += 1
                state = "code"
            else:
                if text[index] != "\n":
                    out[index] = " "
                index += 1
    return "".join(out)


def extract_actual_function() -> str:
    source = (ROOT / "src/basewin.cpp").read_text(encoding="utf-8")
    masked = _masked_cpp(source)
    hits = [match.start() for match in re.finditer(re.escape(SIGNATURE), masked)]
    assert len(hits) == 1, f"expected one real definition, found {len(hits)}"
    start = hits[0]
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated draw_garrison definition")


def assert_original_contract(body: str) -> None:
    assert "Font font;" in body
    assert body.index("Font font;") < body.index("canvas->set_clip(&clip_rect);")
    assert body.index("canvas->set_clip(&clip_rect);") < body.index(
        "font.init(DefaultFontFace, 0xc, 0);")
    assert "if (MapWinCaptionScale > 0 && MapWinCaptionDivisor != 0)" not in body
    assert "if (MapWinCaptionScale > 0) {" in body
    assert "int capacity;" in body
    assert "capacity = (frame_rect.right - frame_rect.left) / (spacing + 5);" in body
    assert "spacing + 5 > 0" not in body
    assert "if (capacity > 0)" not in body
    assert "spacing = (frame_rect.right - frame_rect.left) / capacity - 5;" in body
    assert "int initial_vehicle = veh_at(BaseCurrent->x, BaseCurrent->y);" in body
    assert "vehicle = initial_vehicle;" in body
    assert body.index("\n    vehicle = initial_vehicle;") > body.index(
        "MainInterfaceGlobal.flat_button_[10].show(0);")
    assert "int text_spacing;" in body
    assert "text_spacing = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;" in body
    assert "frame_rect.bottom - line_height + 6, text_spacing," in body
    assert "canvas->restore_clip();" in body


def compile_run(tmp_path: Path, body: str, mode: int,
                compiler_name: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    fixture = FIXTURE.read_text(encoding="utf-8")
    marker = "/*__DRAW_GARRISON_BODY__*/"
    assert fixture.count(marker) == 1
    source = tmp_path / f"garrison_{compiler_name}.cpp"
    binary = tmp_path / f"garrison_{compiler_name}"
    source.write_text(fixture.replace(marker, body), encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source), "-o", str(binary)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(binary), str(mode)], capture_output=True,
                          text=True, timeout=30)


def test_pinned_idiv_fault_sites_are_not_guarded_or_executed():
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable")
    image = PeImage(image_path.read_bytes(), PIN)
    assert image.read(Span(0x004104F0, 0x004104F6)) == bytes.fromhex(
        "f7 3d 70 1e 69 00")  # IDIV [0x691E70]
    assert image.read(Span(0x0041056D, 0x00410573)) == bytes.fromhex(
        "99 83 c1 05 f7 f9")  # CDQ; ADD ECX,5; IDIV ECX
    assert image.read(Span(0x004105BD, 0x004105C2)) == bytes.fromhex(
        "2b c1 99 f7 ff")  # SUB EAX,ECX; CDQ; IDIV EDI
    assert_original_contract(extract_actual_function())


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mode", [0, 1, 2, 3, 4, 5, 6, 7])
def test_actual_function_host_paths(tmp_path, compiler_name, mode):
    body = extract_actual_function()
    assert_original_contract(body)
    result = compile_run(tmp_path, body, mode, compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_behavior_mutations_are_caught(tmp_path, compiler_name):
    body = extract_actual_function()
    assert_original_contract(body)
    mutants = (
        # Invent a scroll offset after callbacks: it skips original row zero.
        ("    int column = 0;",
         "    if (vehicle >= 0) vehicle = Vehs[vehicle].next_veh_id_stack;\n"
         "    int column = 0;", 0),
        # Keep first-walk head rather than rewalking the saved veh_at ID.
        ("    vehicle = initial_vehicle;\n"
         "    if (vehicle >= 0) {\n"
         "        while (Vehs[vehicle].prev_veh_id_stack >= 0) {\n"
         "            vehicle = Vehs[vehicle].prev_veh_id_stack;\n"
         "        }\n"
         "    }",
         "    // cached first head mutant", 6),
        # Reuse overflow-adjusted column spacing instead of second pitch read.
        ("        int text_spacing;\n"
         "        if (MapWinCaptionScale > 0) {\n"
         "            text_spacing = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;\n"
         "        } else {\n"
         "            text_spacing = 0x38;\n"
         "        }",
         "        int text_spacing = spacing;", 4),
        # Restore saved clip before publication instead of after.
        ("    canvas->restore_clip();\n"
         "    if (update_mode != 0) MainInterfaceGlobal.update(&garrison_rect_, nullptr);\n"
         "    else MainInterfaceGlobal.soft_update(&garrison_rect_);\n"
         "    canvas->set_clip(&saved_clip);",
         "    canvas->set_clip(&saved_clip);\n"
         "    if (update_mode != 0) MainInterfaceGlobal.update(&garrison_rect_, nullptr);\n"
         "    else MainInterfaceGlobal.soft_update(&garrison_rect_);\n"
         "    canvas->restore_clip();", 0),
        # Replace required live full-window clip with the panel rectangle.
        ("canvas->restore_clip();",
         "canvas->set_clip(&clip_rect);", 0),
        # Preserve separate appends for debug label formatting.
        ('strcat(StringTemp, " ");\n            strcat(StringTemp, "(");',
         'strcat(StringTemp, " (");', 7),
        # Construct Font after clip setup instead of before it.
        ("void BaseWin::draw_garrison(int update_mode) {\n"
         "    Font font;\n"
         "    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);",
         "void BaseWin::draw_garrison(int update_mode) {\n"
         "    Buffer *canvas = static_cast<Buffer *>(&MainInterfaceGlobal);", 0),
    )
    for index, (old, new, mode) in enumerate(mutants):
        assert body.count(old) == 1, f"mutant anchor {index} changed"
        mutant = body.replace(old, new, 1)
        if index == 6:
            anchor = "    canvas->set_clip(&clip_rect);\n"
            assert mutant.count(anchor) == 1
            mutant = mutant.replace(anchor, anchor + "    Font font;\n", 1)
        case = tmp_path / str(index)
        case.mkdir()
        result = compile_run(case, mutant, mode, compiler_name)
        assert result.returncode != 0, f"mutant {index} escaped"

"""Installed-source checks for the MapWin::UNK5 ownership transfer.

This file intentionally reads only the checkout's product sources. It does
not import or fall back to the ignored proposal projection.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.object_source import parse_objects
from match_recovery.source import Mode, read_text


ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/mapwin.cpp').is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ADDRESS = 0x0046FA20
SPAN = ((0x0046FA20, 0x0046FB10),)
DEFINITION = re.compile(r"\bvoid\s+MapWin::UNK5\s*\(\s*int\s+panel\s*\)\s*\{")

FIXTURE = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>

struct BITMAPINFOHEADER { int biWidth; int biHeight; };
struct Dib { BITMAPINFOHEADER bmiHeader; };
struct Win { int base_words[4]; };
struct Buffer { Dib dib_; };
struct GraphicWin : Win, Buffer {};
struct MapWin : virtual GraphicWin { void UNK5(int panel); };
struct Console : MapWin {};
struct MapPanelRect { int left, top, right, bottom; };

static Console ConsoleGlobal;
static MapPanelRect MapPanelRects[2];
static unsigned int MapPanelDirtyMask;
static int MapPanelGeometryWord008EA0B0;
static int MapPanelGeometryWord008EA0B8;
static int IsMultiplayerNet;

BODY

static void require(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(9); }
}
#define REQUIRE(x) require((x), __LINE__)

static void prepare(int width, int height, int extent_a, int extent_b, int net) {
    GraphicWin &graphic = static_cast<GraphicWin &>(ConsoleGlobal);
    graphic.Buffer::dib_.bmiHeader.biWidth = width;
    graphic.Buffer::dib_.bmiHeader.biHeight = height;
    MapPanelGeometryWord008EA0B0 = extent_a;
    MapPanelGeometryWord008EA0B8 = extent_b;
    IsMultiplayerNet = net;
    MapPanelDirtyMask = 0x10U;
    MapPanelRects[0] = MapPanelRect{-90, -91, -92, -93};
    MapPanelRects[1] = MapPanelRect{-80, -81, -82, -83};
}

int main() {
    prepare(101, -67, 500, 30, 0);
    ConsoleGlobal.UNK5(1);
    REQUIRE(MapPanelRects[1].left == 75);
    REQUIRE(MapPanelRects[1].top == 0);
    REQUIRE(MapPanelRects[1].right == 100);
    REQUIRE(MapPanelRects[1].bottom == 268); // signed division truncates toward zero
    REQUIRE(MapPanelRects[0].left == -90 && MapPanelRects[0].bottom == -93);
    REQUIRE(MapPanelDirtyMask == 0x12U);

    prepare(101, -67, 500, 30, 0);
    ConsoleGlobal.UNK5(0);
    REQUIRE(MapPanelRects[0].left == 0 && MapPanelRects[0].top == 0);
    REQUIRE(MapPanelRects[0].right == 101);
    REQUIRE(MapPanelRects[0].bottom == 537);
    REQUIRE(MapPanelDirtyMask == 0x11U);

    prepare(101, -67, 500, 30, 1);
    ConsoleGlobal.UNK5(0);
    REQUIRE(MapPanelRects[0].right == 75);
    REQUIRE(MapPanelRects[0].bottom == 537);
    REQUIRE(MapPanelDirtyMask == 0x11U);

    prepare(-5, 7, 40, 4, 1);
    ConsoleGlobal.UNK5(1);
    REQUIRE(MapPanelRects[1].left == -3);
    REQUIRE(MapPanelRects[1].right == -4);
    REQUIRE(MapPanelRects[1].bottom == 14);
    REQUIRE(MapPanelDirtyMask == 0x12U);
    return 0;
}
'''


def _blank_noncode(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), text
    )


def _definitions(text: str) -> list[int]:
    code = _blank_noncode(text)
    return [match.start() for match in DEFINITION.finditer(code)]


def _comment_out_definition(text: str) -> str:
    code = _blank_noncode(text)
    match, = list(DEFINITION.finditer(code))
    opening = code.find("{", match.start(), match.end())
    depth = 0
    closing = None
    for position in range(opening, len(code)):
        if code[position] == "{":
            depth += 1
        elif code[position] == "}":
            depth -= 1
            if depth == 0:
                closing = position + 1
                break
    assert closing is not None
    return text[:match.start()] + "/*" + text[match.start():closing] + "*/" + text[closing:]


def _product_record():
    path = ROOT / "src/mapwin.cpp"
    text = path.read_text(encoding="utf-8")
    records = read_text(text, path)
    matching = [record for record in records if record.address == ADDRESS]
    assert len(matching) == 1, "the original UNK5 mapping must have one current owner"
    return text, matching[0]


def _historical_record():
    result = subprocess.run(
        ["git", "show", "5fe79d193:src/leaf_recoveries.cpp"],
        cwd=ROOT, check=True, capture_output=True, text=True,
    )
    matches = [record for record in read_text(result.stdout, "src/leaf_recoveries.cpp")
               if record.address == ADDRESS]
    assert len(matches) == 1
    return matches[0]


def _compile_run(tmp_path: Path, mutation: tuple[str, str] | None = None):
    _, record = _product_record()
    assert record.mode is Mode.BODY
    body = record.region
    assert "void MapWin::UNK5(int panel)" in body
    assert "BODY GOES HERE." not in body
    if mutation is not None:
        old, new = mutation
        assert body.count(old) == 1, old
        body = body.replace(old, new, 1)
    source = FIXTURE.replace("BODY", body)
    cpp = tmp_path / "mapwin_unk5_installed_body.cpp"
    exe = tmp_path / "mapwin_unk5_installed_body"
    cpp.write_text(source, encoding="utf-8")
    compiler = shutil.which("g++")
    assert compiler, "host GCC is required for these source controls"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(exe)], capture_output=True, text=True)


def test_identity_claims_history_and_real_owner_transport():
    text, current = _product_record()
    original = _historical_record()
    assert current.name == original.name == "?UNK5@MapWin@@QAEXH@Z"
    assert current.image_spans == original.image_spans == SPAN
    assert current.kind == original.kind
    assert current.byte_exact == original.byte_exact
    assert current.semantic == original.semantic
    assert current.levers == original.levers
    assert current.ruled_out == original.ruled_out
    assert current.unrecoverable == original.unrecoverable
    assert current.deferred == original.deferred
    lesson_history = lambda record: tuple(
        (lesson.kind, lesson.key, lesson.prose,
         tuple(text for _, text in lesson.lines))
        for lesson in (record.origin or ())
    )
    assert lesson_history(current) == lesson_history(original)
    assert current.mode is Mode.BODY

    # The source reader's mapping is coupled to the live method definition,
    # not prose in its Purpose/history block or a second adapter definition.
    positions = _definitions(text)
    assert len(positions) == 1
    header = (ROOT / "src/mapwin.h").read_text(encoding="utf-8")
    assert re.search(r"\bvoid\s+UNK5\s*\(\s*int\s+panel\s*\)\s*;", _blank_noncode(header))
    init_records = [record for record in read_text(text, ROOT / "src/mapwin.cpp")
                    if record.address == 0x00470A90]
    assert len(init_records) == 1
    assert re.search(r"\b(?:this\s*->\s*)?UNK5\s*\(\s*index\s*\)",
                     _blank_noncode(init_records[0].region))
    assert not (ROOT / "src/leaf_recoveries.cpp").exists(), "retired leaf owner source reappeared"

    objects = parse_objects(text, "src/mapwin.cpp", PIN, address_space="pe:va")
    geometry = [row for row in objects if row.origin.address in (0x008EA0B0, 0x008EA0B8)]
    assert {row.origin.address for row in geometry} == {0x008EA0B0, 0x008EA0B8}
    assert all(row.span is None and row.evidence is None for row in geometry)


def test_installed_body_panel_geometry_and_dirty_latch(tmp_path):
    result = _compile_run(tmp_path)
    assert result.returncode == 0, result.stderr


def test_full_tu_lexical_mask_and_four_behavior_mutants(tmp_path):
    text, _ = _product_record()
    decoys = text + '\n// void MapWin::UNK5(int panel) { fake(); }\n' \
        'static const char *fake_definition = "void MapWin::UNK5(int panel) {";\n'
    assert len(_definitions(decoys)) == 1
    assert not _definitions(_comment_out_definition(text))

    mutants = (
        ("(width * 3) / 4", "(width * 2) / 4"),
        ("(MapPanelGeometryWord008EA0B0 - height - MapPanelGeometryWord008EA0B8) / 2",
         "MapPanelGeometryWord008EA0B0 - height - MapPanelGeometryWord008EA0B8"),
        ("if (IsMultiplayerNet != 0)", "if (IsMultiplayerNet == 0)"),
        ("MapPanelDirtyMask |= 1 << panel;", "MapPanelDirtyMask = 1 << panel;"),
    )
    for mutant in mutants:
        result = _compile_run(tmp_path, mutant)
        assert result.returncode != 0, mutant

"""Actual-source host controls for the three recovered paint owners.

Behavior controls only. Function bodies are read from the mapped product TUs;
the sibling C++ files provide bounded dependency doubles, never fallback bodies.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, State, read_file


ROOT = Path(os.environ.get("OPENSMACX_ROOT", Path(__file__).resolve().parents[2])).resolve()
SPECS = {
    "graphic": {
        "unit": "src/graphicwin.cpp", "address": 0x005D63F0,
        "end": 0x005D68B7,
        "original_name": "?on_redraw_nc@GraphicWin@@QAEXPAURECT@@H@Z",
        "symbol": "?on_redraw_nc@GraphicWin@@UAEXPAUtagRECT@@H@Z",
        "signature": "void GraphicWin::on_redraw_nc(RECT *area, int side)",
        "fixture": "graphicwin_paint_owner_control.cpp",
    },
    "newtech": {
        "unit": "src/newtechwin.cpp", "address": 0x00483950,
        "end": 0x00483E7F,
        "original_name": "?on_redraw@NewTechWin@@QAEXXZ",
        "symbol": "?on_redraw@NewTechWin@@UAEXXZ",
        "signature": "void NewTechWin::on_redraw()",
        "fixture": "newtechwin_paint_owner_control.cpp",
    },
    "pushbutton": {
        "unit": "src/gamma.cpp", "address": 0x0062C110,
        "end": 0x0062C82D,
        "original_name": "?on_redraw@PushButton@@QAEXXZ",
        "symbol": "?on_redraw@PushButton@@UAEXXZ",
        "signature": "void PushButton::on_redraw()",
        "fixture": "pushbutton_paint_owner_control.cpp",
    },
}


def _owner_body(spec):
    path = ROOT / spec["unit"]
    rows = [row for row in read_file(path) if row.address == spec["address"]]
    assert len(rows) == 1, (spec["unit"], hex(spec["address"]), len(rows))
    row, = rows
    assert row.image_spans == ((spec["address"], spec["end"]),)
    assert row.name == spec["original_name"]
    assert row.symbol == spec["symbol"]
    assert row.mode == Mode.BODY and row.state == State.IMPLEMENTED
    assert row.kind == "game" and row.region and not row.extract_error

    text = path.read_text(encoding="utf-8")
    original_marker = (
        f"// ORIGINAL: 0x{spec['address']:08X} {spec['original_name']} "
        f"0x{spec['address']:08X}-0x{spec['end']:08X}"
    )
    assert text.count(original_marker) == 1, "original name/span history was lost"
    return _definition(path, spec["signature"])


def _definition(path, signature):
    text = path.read_text(encoding="utf-8")
    assert text.count(signature) == 1, f"expected one actual definition: {signature}"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated mapped owner {spec['signature']}")


def _source(spec, body=None):
    fixture = (Path(__file__).with_name(spec["fixture"])).read_text(encoding="utf-8")
    marker = "/* ACTUAL OWNER BODY */"
    assert fixture.count(marker) == 1
    return fixture.replace(marker, _owner_body(spec) if body is None else body)


def _compile_and_run(tmp_path, spec, body=None):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / (spec["fixture"].replace(".cpp", "_actual.cpp"))
    binary = tmp_path / source.stem
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(_source(spec, body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-const-variable",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(source), "-o", str(binary)], capture_output=True, text=True,
        timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
               UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=15, env=env)


@pytest.mark.parametrize("name", tuple(SPECS))
def test_mapped_owner_body_runs_from_product_source(tmp_path, name):
    result = _compile_and_run(tmp_path, SPECS[name])
    assert result.returncode == 0, (name, result.stdout, result.stderr)


def test_graphicwin_caption_callback_uses_typed_storage_and_safe_raw_view():
    header = (ROOT / "src/graphicwin.h").read_text(encoding="utf-8")
    source = (ROOT / "src/graphicwin.cpp").read_text(encoding="utf-8")
    consumer = (ROOT / "src/basewin.cpp").read_text(encoding="utf-8")
    producer = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    assert "GraphicWinCaptionDraw field_A0C_;" in header
    assert "union GraphicWinCaptionAction" not in header
    accessor = _definition(ROOT / "src/graphicwin.cpp",
                           "uint32_t GraphicWin::field_A0C_value() const")
    assert "memcpy(&value, &field_A0C_, sizeof(value))" in accessor
    assert "menu.field_A0C_value()" in consumer
    assert "GraphicWinFieldA0CDefault = &def_caption_draw;" in producer
    assert "field_A0C_(this, caption, buffer)" in _owner_body(SPECS["graphic"])
    assert "call-list evidence in Git history" in source


def test_pushbutton_sprite_producers_and_all_consumers_are_typed():
    header = (ROOT / "src/pushbutton.h").read_text(encoding="utf-8")
    gamma = (ROOT / "src/gamma.cpp").read_text(encoding="utf-8")
    game = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    assert "Sprite *PushButtonDefaults[18];" in gamma
    for name in ("AB8", "ABC", "AC0", "AC4", "AC8", "ACC", "AD0",
                 "AD4", "AD8", "ADC", "AE0", "AE4", "AE8", "AEC",
                 "AF0", "AF4", "AF8", "AFC"):
        assert f"Sprite *field_{name}_;" in header
    for index, sprite in ((4,18),(13,24),(2,19),(11,25),(5,20),(14,26)):
        assert f"PushButtonDefaults[{index}] = &g_BASEWIN_SPRITES[{sprite}];" in game
    assert "const int push_zero_slots[12] = {6,15,7,16,3,12,0,9,1,10,8,17};" in game
    assert "reinterpret_cast<uint32_t>(&g_BASEWIN_SPRITES" not in game
    for index, name in enumerate(("AB8", "ABC", "AC0", "AC4", "AC8", "ACC",
                                  "AD0", "AD4", "AD8", "ADC", "AE0", "AE4",
                                  "AE8", "AEC", "AF0", "AF4", "AF8", "AFC")):
        assert f"field_{name}_ = PushButtonDefaults[{index}];" in gamma
    body = _owner_body(SPECS["pushbutton"])
    assert "reinterpret_cast" not in body
    assert "TWO parallel 9-draw sets" in gamma


def test_newtech_technology_and_interface_producer_closure_is_retained():
    header = (ROOT / "src/newtechwin.h").read_text(encoding="utf-8")
    source = _owner_body(SPECS["newtech"])
    assert "void on_redraw();" in header
    assert "g_TECH_ICON_SPRITES[technology_id]" in source
    assert "theory_of_everything" in source
    owner_file = (ROOT / "src/newtechwin.cpp").read_text(encoding="utf-8")
    assert "previous unit's NO_COMPILE finding" in owner_file


def test_graphicwin_mutant_inverting_custom_surface_gate_fails(tmp_path):
    spec = SPECS["graphic"]
    body = _owner_body(spec)
    anchor = "if ((iFlags_ & 0x10000000) == 0) {"
    assert body.count(anchor) == 1
    mutant = body.replace(anchor, "if ((iFlags_ & 0x10000000) != 0) {", 1)
    assert _compile_and_run(tmp_path, spec, mutant).returncode != 0


def test_graphicwin_mutant_removing_later_quadrants_fails(tmp_path):
    spec = SPECS["graphic"]
    body = _owner_body(spec)
    anchor = "if (buffer4_ != 0 && (side == -1 || side == 2)) {"
    assert body.count(anchor) == 1
    mutant = body.replace(anchor, "if (buffer4_ != 0 && side == 99) {", 1)
    assert _compile_and_run(tmp_path, spec, mutant).returncode != 0


def test_graphicwin_mutant_reversing_caption_callbacks_fails(tmp_path):
    spec = SPECS["graphic"]
    body = _owner_body(spec)
    anchor = ("                field_A0C_(this, caption, buffer);\n"
              "            }\n            on_draw_caption(caption, buffer);")
    assert body.count(anchor) == 1
    mutant = body.replace(anchor,
        "            on_draw_caption(caption, buffer);\n"
        "                field_A0C_(this, caption, buffer);\n            }", 1)
    assert _compile_and_run(tmp_path, spec, mutant).returncode != 0


def test_newtech_mutants_are_rejected(tmp_path):
    spec = SPECS["newtech"]
    body = _owner_body(spec)
    mutants = (
        body.replace("field_B28_) > 0 ? 542 : 541",
                     "field_B28_) > 0 ? 541 : 542", 1),
        body.replace("strcat(StringTemp, NewTechCentempSuffix);", "", 1),
    )
    for mutant in mutants:
        assert mutant != body
        assert _compile_and_run(tmp_path, spec, mutant).returncode != 0


def test_pushbutton_sprite_branch_mutant_is_rejected(tmp_path):
    spec = SPECS["pushbutton"]
    body = _owner_body(spec)
    original = "if (field_A9C_ == 0) {\n            top_left = field_AC8_;"
    assert body.count(original) == 1
    mutant = body.replace(original,
        "if (field_A9C_ != 0) {\n            top_left = field_AC8_;", 1)
    assert _compile_and_run(tmp_path, spec, mutant).returncode != 0

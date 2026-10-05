from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).parent
PROJECT = ROOT.resolve().parents[1]


def source_function(source):
    signature = "void BaseWin::draw_production("
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return start, index + 1, source[start:index + 1]
    raise AssertionError("draw_production definition is not brace-balanced")


def actual_source_body():
    actual = (PROJECT / "src/basewin.cpp").read_text()
    _, _, extracted = source_function(actual)
    assert "BODY GOES HERE." not in extracted
    return extracted


def compile_and_run(tmp_path, mutant=None):
    shutil.copyfile(ROOT / "basewin_production_source_control.cpp", tmp_path / "source_harness.cpp")
    body = actual_source_body()
    if mutant is not None:
        old, new = mutant
        assert old in body
        body = body.replace(old, new, 1)
    (tmp_path / "basewin_production_body.inc").write_text(body)
    executable = tmp_path / "source_harness"
    subprocess.run(
        ["g++", "-std=c++98", "-fpermissive", "source_harness.cpp", "-o", str(executable)],
        cwd=tmp_path,
        check=True,
        capture_output=True,
        text=True,
    )
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_extracted_body_vehicle_and_facility_paths(tmp_path):
    result = compile_and_run(tmp_path)
    assert result.returncode == 0, result.stderr


def test_extracted_body_control_catches_missing_nested_rows(tmp_path):
    result = compile_and_run(
        tmp_path,
        ("for (int row = 0; row < rows; ++row)",
         "for (int row = 0; row < 1; ++row)"),
    )
    assert result.returncode != 0


def test_extracted_body_controls_reject_native_detail_mutants(tmp_path):
    mutants = (
        ("display_prototype = veh_fake(item_id, BaseCurrent->faction_id_current);",
         "display_prototype = veh_fake(item_id, base->faction_id_current);"),
        ("breed_mod(BaseIDCurrentSelected,\n                          BaseCurrent->faction_id_current) + 1",
         "breed_mod(BaseIDCurrentSelected,\n                          base->faction_id_current) + 1"),
        ("content.top = MainInterfaceGlobal.wrap_cent(StringTemp,\n                panel_center_x, text_y, 0);",
         "text_y = MainInterfaceGlobal.wrap_cent(StringTemp,\n                panel_center_x, text_y, 0);"),
        ("Base *economy_base = BaseCurrent;",
         "Base *economy_base = base;"),
        ("const int current_minerals = economy_base->minerals_accumulated;\n    int item_turn_cost = cost_factor(economy_base->faction_id_current, 1, -1);",
         "int item_turn_cost = cost_factor(economy_base->faction_id_current, 1, -1);\n    const int current_minerals = economy_base->minerals_accumulated;"),
        ("const int mineral_surplus = BaseCurrent->mineral_surplus;",
         "const int mineral_surplus = base->mineral_surplus;"),
        ("MainInterfaceGlobal.wrap_cent(&detail_font, StringTemp,\n                                                    &content);",
         "MainInterfaceGlobal.wrap_cent(&unit_font, StringTemp,\n                                                    &content);"),
        ("TextColourTable[120]", "TextColourTable[121]"),
        ("TextColourTable[104]", "TextColourTable[105]"),
        ("if (gross_cost == 0) {", "if (gross_cost != 0) {"),
        ("Weapon[VehPrototypes[item_id].weapon_id].offense_rating < 0",
         "Weapon[VehPrototypes[item_id].weapon_id].offense_rating >= 0"),
        ("breed_mod(BaseIDCurrentSelected,\n                          BaseCurrent->faction_id_current) + 1",
         "breed_mod(BaseIDCurrentSelected,\n                          BaseCurrent->faction_id_current)"),
        ("BaseCurrent->mineral_surplus + remaining - 1",
         "divisor + remaining - 1"),
        ("content.bottom -= 2;\n        content.bottom -= 2 * unit_font.line_advance();",
         "content.bottom -= 2 * unit_font.line_advance();"),
        ("MainInterfaceGlobal.set_font(&detail_font, 0, 0, 0);",
         "MainInterfaceGlobal.set_font(&unit_font, 0, 0, 0);"),
        ("MainInterfaceGlobal.set_font(&unit_font, 0, 0, 0);",
         "MainInterfaceGlobal.set_font(&detail_font, 0, 0, 0);"),
        ("TextColourTable[82]", "TextColourTable[83]"),
        ("TextColourTable[118]", "TextColourTable[119]"),
        ("TextColourTable[116]", "TextColourTable[117]"),
        ("content.bottom -= 2 * unit_font.line_advance();",
         "content.bottom -= 2;"),
        ("content.top = MainInterfaceGlobal.wrap_cent(&detail_font, StringTemp,\n                                                    &content);",
         "MainInterfaceGlobal.set_font(&detail_font, 0, 0, 0);\n        content.top = MainInterfaceGlobal.wrap_cent(StringTemp, &content);"),
        ("0.000699999975040555f", "0.017444444820284843f"),
        ("++cell.left;", "cell.left += 0;"),
        ("current_minerals >= gross_cost ? 0x6C : 0xDF",
         "current_minerals >= gross_cost ? 0xDF : 0x6C"),
        ("cell_index + 1 > progress_boundary",
         "cell_index + 1 >= progress_boundary"),
        ("cell_index + 1 <= progress_boundary + mineral_surplus",
         "cell_index + 1 > progress_boundary + mineral_surplus"),
        ("signed_facility_id != -1 && signed_facility_id < 0x41",
         "signed_facility_id < 0x41"),
        ("MainInterfaceGlobal.fill(&cell, color)",
         "GraphicWin::fill(&cell, color)"),
        ("if (cell_index >= max_cell_index)",
         "if (cell_index > max_cell_index)"),
        ("if (drawn != 0) {",
         "MapWinCaptionScale = -1;\n        MapWinCaptionDivisor = -1;\n        if (drawn != 0) {"),
    )
    for index, mutant in enumerate(mutants):
        case_path = tmp_path / str(index)
        case_path.mkdir()
        result = compile_and_run(case_path, mutant)
        assert result.returncode != 0

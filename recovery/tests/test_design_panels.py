"""Actual owner-body controls, not matching or application acceptance."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


def _exact_definition(source, signature):
    # Helpers have forward declarations before draw_chassis and definitions
    # afterward. Select the final exact spelling so extraction starts at the
    # definition, not at the next function body.
    start = source.rindex(signature)
    return definition(source[start:], re.escape(signature))


def actual_control():
    source = (ROOT / "src/designwin.cpp").read_text()
    names = ["designwin_panel_signed", "designwin_panel_add",
             "designwin_panel_subtract", "designwin_panel_font_height",
             "designwin_panel_append_number", "designwin_panel_append_label"]
    blocks = [_exact_definition(source, {
        "designwin_panel_signed": "static int designwin_panel_signed(uint32_t bits)",
        "designwin_panel_add": "static int designwin_panel_add(int left, int right)",
        "designwin_panel_subtract": "static int designwin_panel_subtract(int left, int right)",
        "designwin_panel_font_height": "static int designwin_panel_font_height(int extra, int line_height, int height)",
        "designwin_panel_append_number": "static void designwin_panel_append_number(char *output, int value)",
        "designwin_panel_append_label": "static void designwin_panel_append_label(char *output, int label_id)",
    }[name]) for name in names]
    for name in ("weapon", "armor", "reactor"):
        blocks.append(definition(source, re.escape("void DesignWin::draw_" + name + "(")))
    fixture = Path(__file__).with_name("design_panels_control.cpp").read_text()
    token = "// @@ACTUAL_DESIGN_WEAPON@@"
    assert fixture.count(token) == 1
    return fixture.replace(token, "\n\n".join(blocks))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_component_panels_and_damage(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    actual = actual_control()
    variants = {
        "actual": actual,
        "literal_lookup": actual.replace('strcat(StringTemp, "*");',
                                         "designwin_panel_append_label(StringTemp, 0x683550);", 1),
        "local_model": actual.replace("caviar_.add_object(&g_VW00_CAVIARDATA, 0);",
                                     "CaviarData local; caviar_.add_object(&local, 0);", 1),
        "cached_selection": actual.replace("const int mode = Weapon[protoWeaponType_].mode;",
                                          "const int mode = Weapon[weapon_id].mode;", 1),
        "wrong_center": actual.replace("designwin_panel_add(weapon_panel.left, weapon_panel.right)",
                                      "designwin_panel_add(area.left, area.right)", 1),
        "unsigned_center": actual.replace("designwin_panel_add(weapon_panel.left, weapon_panel.right) / 2",
                                         "static_cast<unsigned>(designwin_panel_add(weapon_panel.left, weapon_panel.right)) / 2", 1),
        "wrong_chassis": actual.replace("? -1 : static_cast<int>(protoChassisType_);",
                                       "? 0 : static_cast<int>(protoChassisType_);", 1),
        "uppercase_detail": actual.replace("canvas->write_cent_l(StringTemp, &detail,",
                                          "CharUpperA(StringTemp); canvas->write_cent_l(StringTemp, &detail,", 1),
        "armor_cached_render": actual.replace("if (protoArmorType_ == 5)", "if (false)", 1),
        "armor_unsigned_defense": actual.replace("const int defense = Armor[protoArmorType_].defense_rating;",
                                                "const int defense = static_cast<unsigned char>(Armor[protoArmorType_].defense_rating);", 1),
        "reactor_wrong_index": actual.replace("Reactor[protoReactorType_ - 1].name", "Reactor[protoReactorType_].name", 1),
        "reactor_no_ability": actual.replace("if ((protoAbilityFlags_ & 0x8000u) != 0)", "if (false)", 1),
        "reactor_wrong_rating_top": actual.replace("area.top = canvas->wrap_cent(StringTemp, &area);",
                                                   "area.top = canvas->wrap_cent(StringTemp, &area) + font1_.height_;", 1),
        "reactor_fixed_selection": actual.replace("static_cast<int>(protoReactorType_), protoAbilityFlags_,",
                                                  "2, protoAbilityFlags_,", 1),
        "wrong_rotation": actual.replace("0.5233333706855774f, 0.0f, 0.0f", "0.5323889f, 0.0f, 0.0f", 1),
    }
    with tempfile.TemporaryDirectory(prefix="design-weapon-actual-") as temporary:
        directory = Path(temporary)
        for name, text in variants.items():
            if name != "actual":
                assert text != actual, name
            cpp, binary = directory / (name + ".cpp"), directory / name
            cpp.write_text(text)
            build = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                    "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                                    str(cpp), "-o", str(binary)],
                                   capture_output=True, text=True, timeout=45)
            assert build.returncode == 0, build.stdout + build.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
            assert (run.returncode == 0) == (name == "actual"), name + run.stdout + run.stderr

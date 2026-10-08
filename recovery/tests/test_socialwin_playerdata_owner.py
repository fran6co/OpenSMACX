"""Actual-body host control for the SocialWin -> PlayerData owner migration.

This imports the existing SocialWin actual-source
fixture, extracts both unchanged bodies from src/socialwin.cpp, and supplies
only host doubles for game services/storage. It is not a VC6 ABI proof.
"""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/socialwin.cpp").is_file())
BASE_TEST = ROOT / "recovery/tests/test_socialwin_typed_owner_actual_source.py"
spec = importlib.util.spec_from_file_location("socialwin_base_test", BASE_TEST)
base = importlib.util.module_from_spec(spec)
spec.loader.exec_module(base)


def actual_parts():
    source = (ROOT / "src/socialwin.cpp").read_text()
    return {**base.actual_parts(),
        "BODY_UNK1": base.body(source, "void SocialWin::UNK1("),
        "BODY_PAINT": base.body(source, "int SocialWin::on_button_text_draw("),
    }


def harness():
    text = base.HARNESS
    text = text.replace("struct PendingPlayerEffect { int values_[11]; };\n"
                        "struct PlayerData { PendingPlayerEffect soc_effect_pending; };\n"
                        "static PlayerData PlayersData[8];", "")
    text = text.replace("static int current_visible, iface_set_count, iface_release_count;",
                        "static int current_visible, iface_set_count, iface_release_count, last_text_color;")
    text = text.replace(
        "struct SocialWin : GraphicWin, SubInterface {",
        "struct SocialCategory { enum ModelIndex { POLITICS, ECONOMICS, VALUES, FUTURE }; uint32_t models_[4]; };\n"
        "struct SocialEffect { int values_[11]; };\n"
        "struct PlayerData { SocialCategory soc_category_pending; "
        "SocialCategory soc_category_active; int soc_upheaval_cost_paid; "
        "SocialEffect soc_effect_pending; };\n"
        "static PlayerData PlayersData[8];\n"
        "static int BreakthroughTurnsGlobal, NetIncomeGlobal;\n"
        "static int calc_calls, energy_calls, draw_calls;\n"
        "static int timeline[8], timeline_size;\n"
        "static uint32_t *callback_faction;\n"
        "static SocialCategory *calc_categories[3];\n"
        "static SocialEffect *calc_effects[3];\n"
        "static int calc_factions[3];\n"
        "static void social_calc(SocialCategory *c, SocialEffect *e, int f, int, int) {"
        " if (calc_calls < 3) { calc_categories[calc_calls]=c; calc_effects[calc_calls]=e; "
        "calc_factions[calc_calls]=f; } timeline[timeline_size++]=10+calc_calls; ++calc_calls; }\n"
        "static void energy_compute(int, int) { ++energy_calls; timeline[timeline_size++]=20; "
        "BreakthroughTurnsGlobal=42; NetIncomeGlobal=17; "
        "if (callback_faction) *callback_faction=4; }\n"
        "static void set_social_category_value(SocialCategory &c, int i, int v) { c.models_[i]=v; }\n"
        "struct SocialWin : GraphicWin, SubInterface { uint32_t field_A1C_; ")
    text = text.replace("unsigned char pad_to_faction[0xCF4-0xA1C]; uint32_t factionID_;",
                        "unsigned char pad_to_faction[0xCF4-0xA20]; uint32_t factionID_;")
    text = text.replace(
        "unsigned char pad_to_effect[0xD08-0xCF8]; int socEffect_[11];\n"
        "    unsigned char pad_to_locks[0xD6C-0xD34]; CheckButton energyLockButtons_[3];",
        "SocialCategory socCategory_; SocialEffect socEffect_; SocialEffect socEffectOrig_;\n"
        "    unsigned char pad_to_locks[0xD6C-0xD60]; CheckButton energyLockButtons_[3];")
    text = text.replace(
        "int on_button_text_draw(GraphicWin *, char *, int);",
        "int on_button_text_draw(GraphicWin *, char *, int);\n"
        "    int breakthroughTurns_, netIncome_;\n"
        "    void draw_social(int) { ++draw_calls; timeline[timeline_size++]=1; }\n"
        "    void UNK1(int, int);")
    text = text.replace("int set_text_color(int,int,int,int) { return 0; }",
                        "int set_text_color(int a,int,int,int) { last_text_color=a; return 0; }")
    text = text.replace("BODY_PAINT\n", "BODY_PAINT\nBODY_UNK1\n")
    text = text.replace(
        "    // Guard remains before all receiver/window use for out-of-domain ids.",
        """    // UNK1 must reread the selected player row at each original call site.
    std::memset(PlayersData, 0, sizeof(PlayersData));
    panel.factionID_=3; calc_calls=energy_calls=draw_calls=timeline_size=0;
    BreakthroughTurnsGlobal=1; NetIncomeGlobal=2;
    PlayersData[3].soc_category_pending.models_[2]=77;
    PlayersData[3].soc_effect_pending.values_[5]=125;
    PlayersData[4].soc_category_pending.models_[2]=99;
    callback_faction=&panel.factionID_;
    panel.UNK1(2,77);
    if (draw_calls!=1 || calc_calls!=3 || energy_calls!=1) fail("UNK1 call counts/order");
    if (timeline_size!=5 || timeline[0]!=1 || timeline[1]!=10 || timeline[2]!=11 ||
        timeline[3]!=20 || timeline[4]!=12) fail("social_calc/energy callback order");
    if (panel.breakthroughTurns_!=42 || panel.netIncome_!=17)
        fail("global reads remain after energy callback");
    if (calc_categories[0]!=&panel.socCategory_ || calc_effects[0]!=&panel.socEffect_ ||
        calc_factions[0]!=3) fail("working social_calc inputs");
    if (calc_effects[1]!=&PlayersData[3].soc_effect_pending ||
        calc_categories[2]!=&PlayersData[4].soc_category_pending ||
        calc_effects[2]!=&PlayersData[4].soc_effect_pending ||
        calc_factions[2]!=4) fail("PlayerData social row is reread after callback");
    if (calc_categories[2]->models_[2]!=99) fail("pending category value");

    // Paint comparison must use this faction's pending effect, not row zero.
    panel.factionID_=3; panel.socEffect_.values_[6]=50;
    PlayersData[0].soc_effect_pending.values_[6]=1;
    PlayersData[3].soc_effect_pending.values_[6]=100;
    unsigned char *paint_button=reinterpret_cast<unsigned char *>(panel.flatButtons_)+0x16*0xB4C;
    *reinterpret_cast<int *>(paint_button+0xAB8)=0;
    *reinterpret_cast<int *>(paint_button+0xA18)=0;
    *reinterpret_cast<int *>(paint_button+0xA74)=1;
    char paint_label[]="label"; last_text_color=0;
    panel.on_button_text_draw(&window,paint_label,0x16);
    if(last_text_color!=0xf9) fail("paint comparison uses selected faction's pending value");

    // Guard remains before all receiver/window use for out-of-domain ids.""")
    return text


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_socialwin_playerdata_bodies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    source = harness()
    for marker, body in actual_parts().items():
        source = source.replace(marker, body)
    path = tmp_path / "socialwin-playerdata.cpp"
    binary = tmp_path / "socialwin-playerdata"
    path.write_text(source)
    pointer_warning = ("-Wno-tautological-pointer-compare" if "clang" in compiler_name
                      else "-Wno-address")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-invalid-offsetof", "-Wno-unused-parameter", pointer_warning,
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(path), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                         env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    assert run.returncode == 0, run.stderr


def test_source_has_no_fixed_social_block_accesses():
    source = (ROOT / "src/socialwin.cpp").read_text()
    assert "PlayerSocialCategoryBlock" not in source
    assert "PlayerSocialEffectBlock" not in source
    assert "&PlayersData[factionID_].soc_category_pending" in source
    assert "&PlayersData[factionID_].soc_effect_pending" in source
    assert "PlayersData[factionID_].soc_effect_pending.values_[effect]" in source


def test_owner_address_relationship():
    # Arithmetic control, not a substitute for pinned instruction evidence.
    source = (ROOT / "src/socialwin.cpp").read_text()
    assert "// ORIGINAL: 0x004AEBF0" in source
    assert "// ORIGINAL: 0x004B17A0" in source
    assert 0x96C9E0 + 0x228 == 0x96CC08
    assert 0x96C9E0 + 0x24C == 0x96CC2C
    assert 0x20CC // 4 == 2099

def test_pinned_final_social_calc_has_all_five_arguments():
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned executable unavailable")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image = PeImage(image_path.read_bytes(),
        "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    decoded = {i.address: i for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x004AEBF0, 0x004AECD2)), 0x004AEBF0)}
    assert [(decoded[a].mnemonic, decoded[a].op_str) for a in (
        0x004AEC86, 0x004AECA2, 0x004AECA9, 0x004AECC2, 0x004AECC3)] == [
        ("push", "0"), ("push", "0"), ("push", "esi"),
        ("push", "edx"), ("push", "eax")]
    assert (decoded[0x004AECC4].mnemonic, decoded[0x004AECC4].op_str) == (
        "call", "0x5b4210")
    assert (decoded[0x004AECC9].mnemonic, decoded[0x004AECC9].op_str) == (
        "add", "esp, 0x44")

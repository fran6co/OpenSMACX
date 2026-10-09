"""Actual-source controls for SocialWin's remaining typed storage projections.

The host executable compiles selected live expressions and the existing
SocialWin::immune body against small owner fixtures. It checks data routing,
not VC6 ABI or runtime acceptance.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def braced_block(text: str, anchor: str) -> str:
    at = text.index(anchor)
    opening = text.rfind("{", 0, at)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[opening:end + 1]
    raise AssertionError(f"unterminated block containing: {anchor}")


def live_parts():
    text = SOURCE.read_text()
    init = body(text, "void SocialWin::init(")
    info = body(text, "void SocialWin::draw_info()")
    draw_social = body(text, "void SocialWin::draw_social(")
    immune = body(text, "int SocialWin::immune(")
    layout = braced_block(init, "int left = MainInterfaceGlobal.main_frame_rect().left + 7;")
    allocations = braced_block(
        init, "int net_min = 10 - PlayersData[factionID_].se_alloc_labs")
    registry = "InterfaceRegistry[2] = static_cast<SubInterface *>(this);"
    tutorial = (
        "TutorialWindow.set_tour_stage(5);",
        "TutorialWindow.soc_rect(&TutWinSocWindow.tooltipSocEng_, &out_a, &out_b);",
    )
    assert registry in init
    assert all(statement in init for statement in tutorial)
    assert "PlayersData[factionID_].se_alloc_psych" in info
    assert "PlayersData[factionID_].se_alloc_labs" in info
    assert "SocialCategories[3].model_effect[1].values_" in draw_social
    assert "while (p != social_effect_row + 3)" in draw_social
    assert "while (p2 != social_effect_row + 3)" in draw_social
    assert "if (p2 > social_effect_row + 10)" in draw_social
    code = "\n".join(line for line in text.splitlines()
                       if not line.lstrip().startswith(("//", "*")))
    for old in ("SocialEffectTable[", "SocialEffectRow[", "SocialAllocTable[",
                "*SocialSubInterfaceSlot", "*MainPanelLeft", "*MainPanelTop",
                "*MainPanelRight", "*SocialColumnTop", "*SocialColumnRight",
                "*SocialColumnBottom", "*TutorialStage", "TutorialSocRect"):
        assert old not in code
    return immune, layout, allocations, registry, tutorial


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstdlib>
struct RECT { int left, top, right, bottom; };
struct SocialEffect { int values_[11]; };
struct RulesSocialCategory {
    uint32_t type; int preq_tech[4]; uint32_t name[4];
    SocialEffect model_effect[4];
};
static_assert(sizeof(RulesSocialCategory) == 0xd4, "native category stride");
static RulesSocialCategory SocialCategories[4];
struct Base { unsigned char faction_id_current; };
static Base Bases[8];
struct SecretProject {
    int cloning, backbone;
    int cloning_vats() const { return cloning; }
    int network_backbone() const { return backbone; }
};
struct GameData { struct { SecretProject secret_project_; } fields_; };
static GameData GameDataStateStorage;
struct PlayerData { int se_alloc_psych, se_alloc_labs; };
static PlayerData PlayersData[8];
struct SubInterface {};
static SubInterface *InterfaceRegistry[12];
struct MainInterface {
    RECT main, right, lower, info;
    const RECT &main_frame_rect() const { return main; }
    const RECT &right_frame_rect() const { return right; }
    const RECT &lower_frame_rect() const { return lower; }
    const RECT &info_panel_frame() const { return info; }
} MainInterfaceGlobal;
struct Tutorial {
    uint32_t field_53C0_ = 0;
    RECT *seen = nullptr;
    int set_calls = 0, rect_calls = 0;
    void set_tour_stage(int stage) { field_53C0_ = static_cast<uint32_t>(stage); ++set_calls; }
    void soc_rect(RECT *rect, int *x, int *y) {
        seen = rect; ++rect_calls;
        *x = rect->left + (rect->right - rect->left) / 2;
        *y = rect->top + (rect->bottom - rect->top) / 2;
    }
} TutorialWindow;
struct SocialWinFixture : SubInterface {
    int factionID_ = 0;
    RECT tooltipSocEng_{}, tooltipEconomy_{}, tooltipPsych_{}, tooltipLabs_;
    RECT tooltipEffects_{}, tutRect9_{};
    uint32_t economyAllocOrig_{}, psychAllocOrig_{}, labsAllocOrig_{};
    uint32_t economyAlloc3_{}, psychAlloc3_{}, labsAlloc3_{}, pendingAlloc_[3]{};
    int immune(int, int, int);
    void run_layout();
    void run_allocations();
    void run_init_storage();
} TutWinSocWindow;
__IMMUNE__
void SocialWinFixture::run_layout() __LAYOUT__
void SocialWinFixture::run_allocations() __ALLOCATIONS__
void SocialWinFixture::run_init_storage() {
    __REGISTRY__
    __TUTORIAL_STAGE__
    int out_a = 0, out_b = 0;
    __TUTORIAL_RECT__
}
static void check(bool okay) { if (!okay) std::abort(); }
int main() {
    SocialWinFixture other;
    other.factionID_ = 2;
    GameDataStateStorage.fields_.secret_project_ = {1, 3};
    Bases[1].faction_id_current = 2;
    Bases[3].faction_id_current = 2;
    SocialCategories[2].model_effect[1].values_[5] = -2;
    check(other.immune(2, 1, 5) == 1);
    SocialCategories[2].model_effect[1].values_[5] = 2;
    check(other.immune(2, 1, 5) == 0);
    SocialCategories[3].model_effect[1].values_[10] = -1;
    check(other.immune(3, 1, 10) == 1);
    Bases[3].faction_id_current = 1;
    check(other.immune(3, 1, 10) == 0);

    PlayersData[2] = {4, 2};
    other.factionID_ = 2;
    other.run_allocations();
    check(other.economyAllocOrig_ == 4 && other.pendingAlloc_[0] == 4);
    check(other.psychAllocOrig_ == 4 && other.pendingAlloc_[1] == 4);
    check(other.labsAllocOrig_ == 2 && other.pendingAlloc_[2] == 2);
    PlayersData[2] = {6, 1};
    other.run_allocations();
    check(other.economyAllocOrig_ == 3 && other.psychAllocOrig_ == 6 &&
          other.labsAllocOrig_ == 1);

    MainInterfaceGlobal.main = {10, 20, 200, 100};
    MainInterfaceGlobal.lower = {30, 40, 330, 140};
    MainInterfaceGlobal.right = {1, 2, 101, 102};
    other.run_layout();
    check(other.tooltipEconomy_.left == 17 && other.tooltipEconomy_.top == 47 &&
          other.tooltipEconomy_.right == 117 && other.tooltipEconomy_.bottom == 133);
    check(other.tooltipPsych_.left == 120 && other.tooltipLabs_.left == 223);
    check(other.tutRect9_.left == 14 && other.tutRect9_.right == 196);
    check(other.tooltipEffects_.left == 5 && other.tooltipEffects_.right == 97);
    MainInterfaceGlobal.right = {3, 5, 103, 105};
    other.run_layout();
    check(other.tooltipEffects_.left == 7 && other.tooltipEffects_.right == 99);

    other.tooltipSocEng_ = {100, 200, 300, 400};
    TutWinSocWindow.tooltipSocEng_ = {11, 21, 31, 41};
    other.run_init_storage();
    check(InterfaceRegistry[2] == static_cast<SubInterface *>(&other));
    check(TutorialWindow.field_53C0_ == 5 && TutorialWindow.set_calls == 1);
    check(TutorialWindow.seen == &TutWinSocWindow.tooltipSocEng_ &&
          TutorialWindow.seen != &other.tooltipSocEng_);
    check(TutorialWindow.rect_calls == 1 && TutorialWindow.seen != nullptr);
    return 0;
}
'''


def host_source(mutant=None):
    immune, layout, allocations, registry, tutorial = live_parts()
    immune = immune.replace("int SocialWin::immune(",
                            "int SocialWinFixture::immune(")
    stage, rect = tutorial
    if mutant == "wrong_social_row":
        immune = immune.replace(
            "SocialCategories[3].model_effect[1].values_[effect_id]",
            "SocialCategories[3].model_effect[0].values_[effect_id]")
    elif mutant == "wrong_allocation_member":
        allocations = allocations.replace(
            "PlayersData[factionID_].se_alloc_psych",
            "PlayersData[factionID_].se_alloc_labs")
    elif mutant == "wrong_frame_owner":
        layout = layout.replace("MainInterfaceGlobal.right_frame_rect()",
                                "MainInterfaceGlobal.info_panel_frame()")
    elif mutant == "receiver_relative_tutorial_rect":
        rect = rect.replace("TutWinSocWindow.tooltipSocEng_", "tooltipSocEng_")

    return (HARNESS.replace("__IMMUNE__", immune)
            .replace("__LAYOUT__", layout)
            .replace("__ALLOCATIONS__", allocations)
            .replace("__REGISTRY__", registry)
            .replace("__TUTORIAL_STAGE__", stage)
            .replace("__TUTORIAL_RECT__", rect))


COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


def run_host(tmp_path, compiler, mutant=None):
    source = tmp_path / "socialwin-storage.cpp"
    executable = tmp_path / "socialwin-storage"
    source.write_text(host_source(mutant))
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    if build.returncode != 0:
        return build, None
    return build, subprocess.run([str(executable)], capture_output=True,
                                 text=True, timeout=10)


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_socialwin_storage_projections(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    build, result = run_host(tmp_path, compiler)
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode == 0, result.stderr if result else "no executable"


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize("mutant", (
    "wrong_social_row", "wrong_allocation_member", "wrong_frame_owner",
    "receiver_relative_tutorial_rect"))
def test_storage_owner_damage_controls(tmp_path, compiler, mutant):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    build, result = run_host(tmp_path, compiler, mutant)
    assert build.returncode == 0, build.stderr
    assert result is not None and result.returncode != 0

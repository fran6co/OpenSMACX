"""Actual-source behavioral controls for prefs_save.

This bounded host model compiles the complete definition extracted from
alpha.cpp against observable preference and INI boundaries. It checks saved
values and order; it is not native matching or full-game proof.
"""

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
ALPHA_CPP = ROOT / "src" / "alpha.cpp"
PREFS_SAVE_ADDRESS = 0x0059E5D0
PREFS_SAVE_SIGNATURE = "void __cdecl prefs_save("


def actual_prefs_save() -> str:
    records = [r for r in read_file(ALPHA_CPP) if r.address == PREFS_SAVE_ADDRESS]
    assert len(records) == 1
    record = records[0]
    assert record.mode is Mode.BODY and record.name == "?prefs_save@@YAXH@Z"
    assert record.region and "BODY GOES HERE." not in record.region
    return _definition(record.region, PREFS_SAVE_SIGNATURE)


HOST_PREFIX = r"""
#define __cdecl
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
using BOOL = int;
using LPCSTR = const char *;
using uint32_t = std::uint32_t;
constexpr int MaxPlayerNum = 8;
static const char PrefsSection[] = "prefs";
static const char PrefsFile[] = "prefs.ini";
struct DefaultPrefsType { int difficulty, map_type, top_menu, faction_id; } DefaultPrefs;
struct AlphaIniPrefsType {
    int preferences, more_preferences, semaphore, announce, rules, customize;
    unsigned int time_controls;
    unsigned int custom_world[7];
} AlphaIniPrefs;
struct PlayerType { const char *filename; } Players[MaxPlayerNum];
int ExpansionEnabled;
char StringTemp[4096];
struct IniWrite {
    std::string section, key, value, file;
};
static std::vector<IniWrite> writes;
static std::string disable_expansion_after_key;

static BOOL WritePrivateProfileStringA(LPCSTR section, LPCSTR key,
                                       LPCSTR value, LPCSTR file) {
    writes.push_back({section, key, value ? value : "<null>", file});
    if (!disable_expansion_after_key.empty() && key == disable_expansion_after_key)
        ExpansionEnabled = 0;
    return 1;
}
static void say_num(int value) {
    char text[80];
    std::snprintf(text, sizeof(text), "%d", value);
    std::strcat(StringTemp, text);
}
static void prefs_put(LPCSTR key, LPCSTR value) {
    WritePrivateProfileStringA(PrefsSection, key, value, PrefsFile);
}
static void prefs_put(LPCSTR key, int value, BOOL binary) {
    StringTemp[0] = 0;
    if (binary) {
        const uint32_t bits = static_cast<uint32_t>(value);
        bool non_pad = false;
        for (int shift = 31; shift >= 0; --shift) {
            if ((uint32_t(1) << shift) & bits) {
                non_pad = true;
                std::strcat(StringTemp, "1");
            } else if (non_pad || shift < 8) {
                std::strcat(StringTemp, "0");
            }
        }
    } else {
        char number[80];
        std::snprintf(number, sizeof(number), "%d", value);
        std::strcat(StringTemp, number);
    }
    WritePrivateProfileStringA(PrefsSection, key, StringTemp, PrefsFile);
}
static void reset_state() {
    writes.clear();
    disable_expansion_after_key.clear();
    StringTemp[0] = 0;
    ExpansionEnabled = 1;
    DefaultPrefs = {3, 4, 5, 6};
    AlphaIniPrefs = {-1, 0x40000001, 2, 3, 0x12345678, 0, 9,
                     {2, 0, 10, 3, 4, 5, 6}};
    static const char *filenames[MaxPlayerNum] = {
        "unused0.sav", "player1.sav", "player2.sav", "player3.sav",
        "player4.sav", "player5.sav", "player6.sav", "player7.sav"
    };
    for (int i = 0; i < MaxPlayerNum; ++i) Players[i].filename = filenames[i];
}
static void expect_write(size_t at, const char *key, const char *value) {
    assert(at < writes.size());
    const IniWrite &w = writes[at];
    assert(w.section == PrefsSection && w.file == PrefsFile);
    assert(w.key == key && w.value == value);
}
static void expect_standard_writes() {
    expect_write(0, "Prefs Format", "12");
    expect_write(1, "Difficulty", "3");
    expect_write(2, "Map Type", "4");
    expect_write(3, "Top Menu", "5");
    expect_write(4, "Faction", "6");
    expect_write(5, "Preferences", "11111111111111111111111111111111");
    expect_write(6, "More Preferences", "1000000000000000000000000000001");
    expect_write(7, "Semaphore", "00000010");
    expect_write(8, "Announce", "00000011");
    expect_write(9, "Rules", "10010001101000101011001111000");
    expect_write(10, "Customize", "0");
    expect_write(11, "Custom World", "2, 0, 10, 3, 4, 5, 6, ");
    expect_write(12, "Time Controls", "9");
}
"""


def host_program(body: str) -> str:
    return HOST_PREFIX + "\n" + body + r"""
int main() {
    reset_state();
    prefs_save(1);
    expect_standard_writes();
    assert(writes.size() == 20);
    for (int i = 1; i < MaxPlayerNum; ++i) {
        char key[32];
        std::snprintf(key, sizeof(key), "Faction %d", i);
        expect_write(12 + i, key, Players[i].filename);
    }

    reset_state();
    prefs_save(-1);
    expect_standard_writes();
    assert(writes.size() == 20);
    for (int i = 1; i < MaxPlayerNum; ++i) {
        char key[32];
        std::snprintf(key, sizeof(key), "Faction %d", i);
        expect_write(12 + i, key, Players[i].filename);
    }

    reset_state();
    prefs_save(0);
    expect_standard_writes();
    assert(writes.size() == 13);

    reset_state();
    ExpansionEnabled = 0;
    prefs_save(1);
    expect_standard_writes();
    assert(writes.size() == 13);

    reset_state();
    disable_expansion_after_key = "Faction 1";
    prefs_save(1);
    expect_standard_writes();
    assert(writes.size() == 14);
    expect_write(13, "Faction 1", Players[1].filename);
    assert(ExpansionEnabled == 0);
}
"""


def compile_and_run(tmp_path: Path, compiler: str, body: str) -> subprocess.CompletedProcess[str]:
    source = tmp_path / "prefs_save_actual.cpp"
    executable = tmp_path / "prefs_save_actual"
    source.write_text(host_program(body))
    built = subprocess.run(
        [compiler, "-std=c++11", "-O2", str(source), "-o", str(executable)],
        text=True, capture_output=True, check=False
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True, check=False)


@pytest.fixture(scope="module")
def body() -> str:
    return actual_prefs_save()


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_prefs_save_ini_writes(tmp_path: Path, compiler_name: str, body: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_and_run(tmp_path, compiler, body)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["old_delimiter", "hoisted_expansion"])
def test_behavior_mutants_compile_and_fail(
    tmp_path: Path, compiler_name: str, damage: str, body: str
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    if damage == "old_delimiter":
        correct = (
            '        say_num(AlphaIniPrefs.custom_world[i]);\n'
            '        strcat(StringTemp, ", ");'
        )
        old = (
            '        if (i != 0) {\n'
            '            strcat(StringTemp, ", ");\n'
            '        }\n'
            '        say_num(AlphaIniPrefs.custom_world[i]);'
        )
        assert body.count(correct) == 1, "custom-world delimiter anchor drifted"
        mutant = body.replace(correct, old, 1)
    else:
        correct = (
            "if (save_factions) {\n"
            "        for (uint32_t i = 1; i < MaxPlayerNum; i++) {\n"
            "            if (ExpansionEnabled) {"
        )
        old = (
            "if (save_factions && ExpansionEnabled) {\n"
            "        for (uint32_t i = 1; i < MaxPlayerNum; i++) {\n"
            "            if (true) {"
        )
        assert body.count(correct) == 1, "per-iteration flag anchor drifted"
        mutant = body.replace(correct, old, 1)
    assert mutant != body
    result = compile_and_run(tmp_path, compiler, mutant)
    assert result.returncode != 0, f"{damage} mutation escaped the INI write oracle"

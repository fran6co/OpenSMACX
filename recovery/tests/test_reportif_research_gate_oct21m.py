"""Narrow extracted-source test for ReportIf's research-selection gate."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_typed_font_consumers_oct10 import ROOT, extract_definition


SOURCE = ROOT / "src/reportif.cpp"
IDENTITY = ("0x004A41D0", "void ReportIf::on_status(int mode)")
START = "    if ((GameDataStateStorage.fields_.game_state_ & 0x80)"
END = "    for (int topic = 0; topic < 7; ++topic) {"

FIXTURE = r'''#include <assert.h>

struct PlayerData {
    int tech_id_researching;
};

struct PlayerDataTable {
    PlayerData rows[8];
    int reads;
    int last_index;
    PlayerData &operator[](int index) {
        ++reads;
        last_index = index;
        return rows[index];
    }
};

struct GameDataFields {
    unsigned int game_state_;
    unsigned int game_rules_;
};

struct GameDataStorage {
    GameDataFields fields_;
};

struct ReportWin;
struct ResearchFocusRef {
    ReportWin *owner;
    ResearchFocusRef &operator=(int value);
};

struct ReportWin {
    int shared_focus;
    int stores[8];
    int store_count;
    ResearchFocusRef shared_research_focus() {
        ResearchFocusRef result;
        result.owner = this;
        return result;
    }
};

ResearchFocusRef &ResearchFocusRef::operator=(int value) {
    owner->shared_focus = value;
    owner->stores[owner->store_count++] = value;
    return *this;
}

static PlayerDataTable PlayersData;
static GameDataStorage GameDataStateStorage;
ReportWin ReportWinGlobal;

struct ReportIf {
    int report_faction_id_;
    void initialize_research();
};
'''

MAIN = r'''
static void reset_case(ReportIf &report, unsigned int state, unsigned int rules,
                       int faction, int research) {
    GameDataStateStorage.fields_.game_state_ = state;
    GameDataStateStorage.fields_.game_rules_ = rules;
    report.report_faction_id_ = faction;
    PlayersData.reads = 0;
    PlayersData.last_index = -1;
    PlayersData.rows[faction].tech_id_researching = research;
    ReportWinGlobal.shared_focus = 77;
    ReportWinGlobal.store_count = 0;
}

static void selected_case(ReportIf &report, unsigned int state,
                          unsigned int rules, int research,
                          int expected_reads, int expected_focus,
                          int expected_stores, int first_store,
                          int second_store) {
    reset_case(report, state, rules, 3, research);
    report.initialize_research();
    assert(PlayersData.reads == expected_reads);
    if (expected_reads) assert(PlayersData.last_index == 3);
    assert(ReportWinGlobal.shared_focus == expected_focus);
    assert(ReportWinGlobal.store_count == expected_stores);
    assert(ReportWinGlobal.stores[0] == first_store);
    if (expected_stores > 1)
        assert(ReportWinGlobal.stores[1] == second_store);
}

int main() {
    ReportIf report;

    // Original gate is state bit set OR rules bit clear.
    selected_case(report, 0x80, 0x200, 12, 1, 12, 1, 12, 0);
    selected_case(report, 0x80, 0,     12, 1, 12, 1, 12, 0);
    selected_case(report, 0,    0,     12, 1, 12, 1, 12, 0);

    // When the state bit is clear and the rules bit is set, original code
    // writes zero without indexing PlayersData.
    selected_case(report, 0, 0x200, 12, 0, 0, 1, 0, 0);

    // On the selected branch, original stores the raw value before clamping.
    selected_case(report, 0x80, 0x200, -1, 1, 0, 2, -1, 0);
    selected_case(report, 0x80, 0x200,  0, 1, 0, 1,  0, 0);
    selected_case(report, 0x80, 0x200, 89, 1, 89, 1, 89, 0);
    selected_case(report, 0x80, 0x200, 90, 1, 0, 2, 90, 0);
    return 0;
}
'''


OLD_UNGUARDED = r'''    int research = PlayersData[report_faction_id_].tech_id_researching;
    if (!((GameDataStateStorage.fields_.game_state_ & 0x80) || !(GameDataStateStorage.fields_.game_rules_ & 0x200)) || research < 0 || research > 89)
        research = 0;
    window.shared_research_focus() = research;'''


def extracted_research_block() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_definition(source, *IDENTITY)
    start = body.find(START)
    assert start >= 0 and body.count(START) == 1
    end = body.find(END, start)
    assert end > start and body.count(END) == 1
    return body[start:end].rstrip()


def source_program(mutation: str | None = None) -> str:
    block = extracted_research_block()
    if mutation == "unguarded_read":
        block = OLD_UNGUARDED
    elif mutation == "wrong_gate":
        old = "(GameDataStateStorage.fields_.game_state_ & 0x80) || !(GameDataStateStorage.fields_.game_rules_ & 0x200)"
        assert block.count(old) == 1
        block = block.replace(old,
            "(GameDataStateStorage.fields_.game_state_ & 0x80) && !(GameDataStateStorage.fields_.game_rules_ & 0x200)",
            1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    method = ("\nvoid ReportIf::initialize_research() {\n"
              "    ReportWin &window = ReportWinGlobal;\n" + block + "\n}\n")
    return FIXTURE + method + MAIN


def compile_program(compiler: str, program: str, tmp_path: Path,
                    label: str) -> tuple[subprocess.CompletedProcess[str], Path]:
    cpp = tmp_path / f"reportif_research_{Path(compiler).name}_{label}.cpp"
    exe = tmp_path / f"reportif_research_{Path(compiler).name}_{label}"
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    return built, exe


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_extracted_on_status_research_gate_and_store_order(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    built, exe = compile_program(compiler, source_program(), tmp_path, "actual")
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("unguarded_read", "wrong_gate"))
def test_successful_research_gate_mutants_fail_runtime(
        compiler_name: str, mutation: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    built, exe = compile_program(compiler, source_program(mutation), tmp_path, mutation)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode != 0, f"{mutation} mutant unexpectedly passed"

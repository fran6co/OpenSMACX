"""Execute ReportIf::close_projects from src against focused owner doubles."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_typed_font_consumers_oct10 import ROOT, extract_definition


SOURCE = ROOT / "src/reportif.cpp"
IDENTITY = ("0x004AA930", "void ReportIf::close_projects()")

FIXTURE = r'''#include <assert.h>

static int events[8];
static int event_count;

struct FlatButton {
    int id;
    void hide() { events[event_count++] = id; }
};

struct MainInterface {
    FlatButton flat_button_[8];
    MainInterface() {
        for (int i = 0; i < 8; ++i) flat_button_[i].id = i;
    }
};

struct ClimateWindow {
    void hide_all() { events[event_count++] = -1; }
};

struct ReportIf {
    int field_8_;
    void close_projects();
};

static MainInterface MainInterfaceGlobal;
static ClimateWindow WorldClimateWorldWin;
'''

MAIN = r'''
static void reset_events() {
    event_count = 0;
    for (int i = 0; i < 8; ++i) events[i] = -99;
}

int main() {
    ReportIf report;
    report.field_8_ = 3;
    reset_events();
    report.close_projects();
    assert(event_count == 3);
    assert(events[0] == -1 && events[1] == 6 && events[2] == 7);

    report.field_8_ = 4;
    reset_events();
    report.close_projects();
    assert(event_count == 0);

    report.field_8_ = 5;
    reset_events();
    report.close_projects();
    assert(event_count == 0);
    return 0;
}
'''


def source_program(mutation: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    body = extract_definition(source, *IDENTITY)
    if mutation == "wrong_range":
        old = "index = 6; index <= 7"
        assert old in body
        body = body.replace(old, "index = 5; index <= 7", 1)
    elif mutation == "reversed_calls":
        body = body.replace(
            "int index = 6; index <= 7; ++index",
            "int index = 7; index >= 6; --index", 1)
        assert "index = 7; index >= 6; --index" in body
    elif mutation == "wrong_guard":
        old = "state != 4 && state != 5"
        assert old in body
        body = body.replace(old, "state != 4", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")
    return FIXTURE + "\n" + body + "\n" + MAIN


def compile_and_run(compiler: str, program: str,
                    tmp_path: Path, label: str) -> subprocess.CompletedProcess[str]:
    source = tmp_path / f"reportif_projects_{Path(compiler).name}_{label}.cpp"
    executable = tmp_path / f"reportif_projects_{Path(compiler).name}_{label}"
    source.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source),
         "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    if built.returncode != 0:
        return built
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_close_projects_hides_exact_buttons_after_overlay_and_honors_guards(
        compiler_name: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(compiler, source_program(), tmp_path, "actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["wrong_range", "reversed_calls", "wrong_guard"])
def test_close_projects_controls_reject_range_order_and_guard_mutants(
        compiler_name: str, mutation: str, tmp_path: Path) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_and_run(
        compiler, source_program(mutation), tmp_path, mutation)
    assert result.returncode != 0, f"{compiler_name} accepted {mutation} mutation"

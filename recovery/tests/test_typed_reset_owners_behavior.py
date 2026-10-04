"""Compile the current source reset bodies in C++98 host-footprint controls."""
import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/game.h").is_file()
)
CONTROL = Path(__file__).with_name("typed_reset_owners_control.cpp")


def _block(text, marker):
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
    raise AssertionError("unterminated source block: " + marker)


def _source_material():
    header = (ROOT / "src/game.h").read_text()
    source = (ROOT / "src/game.cpp").read_text()
    structs = "\n\n".join(
        _block(header, "struct " + name) + ";"
        for name in ("OfferResetCell", "TamperRecord")
    )
    bodies = {
        name: _block(source, "void __cdecl " + name + "(")
        for name in ("clear_offer_pair", "clear_tamper_registry")
    }
    return structs, bodies


def _prefix(structs, proxy=False):
    includes = """#include <stdio.h>
#include <string.h>
#if !defined(_MSC_VER) && !defined(__cdecl)
#define __cdecl
#endif
"""
    if proxy:
        includes += """static int write_count;
struct TrackedInt;
static TrackedInt *write_log[400];
struct TrackedInt {
    int value;
    bool logged;
    TrackedInt() : value(0), logged(false) {}
    TrackedInt &operator=(int next) {
        if (logged) write_log[write_count++] = this;
        value = next;
        return *this;
    }
    operator int() const { return value; }
};
"""
        structs = re.sub(r"\bint\b", "TrackedInt", structs)
    else:
        includes += "static const int MaxPlayerNum = 8;\n"
    size_checks = ""
    if not proxy:
        size_checks = """\ntypedef char OfferCellSizeCheck[sizeof(OfferResetCell) == 0x90 ? 1 : -1];
typedef char TamperRecordSizeCheck[sizeof(TamperRecord) == 0x18 ? 1 : -1];
"""
    globals_text = """
OfferResetCell OfferResetCells[8][8];
struct TamperHostBacking {
    TamperRecord records[50];
""" + ("    TrackedInt trailing_guard;\n    TrackedInt cursor;\n" if proxy else "    int trailing_guard;\n    int cursor;\n") + """};
static TamperHostBacking TamperBacking;
TamperRecord (&TamperRegistryRecords)[50] = TamperBacking.records;
""" + ("TrackedInt &TamperRegistryCursor = TamperBacking.cursor;\n" if proxy
         else "int &TamperRegistryCursor = TamperBacking.cursor;\n")
    return includes + structs + size_checks + globals_text + "\n"


def _program(structs, bodies, harness, proxy=False):
    return (
        _prefix(structs, proxy)
        + bodies["clear_offer_pair"]
        + "\n\n"
        + bodies["clear_tamper_registry"]
        + "\n\n"
        + harness
    )


def _compile_and_run(compiler, text, tmp_path, stem, expected_success):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " is unavailable")
    source_path = tmp_path / (stem + ".cpp")
    executable = tmp_path / stem
    source_path.write_text(text)
    subprocess.run(
        [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
         str(source_path), "-o", str(executable)],
        check=True,
    )
    result = subprocess.run([str(executable)], check=False)
    assert (result.returncode == 0) == expected_success, stem


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_controls_compile_current_source_body_and_types(compiler, tmp_path):
    structs, bodies = _source_material()
    harness = CONTROL.read_text()
    _compile_and_run(
        compiler, _program(structs, bodies, harness), tmp_path,
        "actual-source-" + compiler, True,
    )


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_source_controls_detect_footprint_mutations(compiler, tmp_path):
    structs, bodies = _source_material()
    harness = CONTROL.read_text()
    mutants = []

    body = bodies["clear_offer_pair"]
    mutants.append((
        "skip-transposed-count",
        {**bodies, "clear_offer_pair": body.replace(
            "    OfferResetCells[faction_b][faction_a].count_[0] = 0;\n", "", 1
        )},
    ))
    mutants.append((
        "touch-offer-payload",
        {**bodies, "clear_offer_pair": body.rsplit("}", 1)[0]
            + "    OfferResetCells[faction_a][faction_b].type_[0][0] = 0;\n}"},
    ))
    body = bodies["clear_tamper_registry"]
    mutants.append((
        "skip-last-field",
        {**bodies, "clear_tamper_registry": body.replace(
            "        record.field_14_ = 0;\n", "", 1
        )},
    ))
    mutants.append((
        "cross-source-array-end",
        {**bodies, "clear_tamper_registry": body.replace(
            "index < 50", "index < 51", 1
        )},
    ))

    for label, mutated in mutants:
        _compile_and_run(
            compiler, _program(structs, mutated, harness), tmp_path,
            compiler + "-" + label, False,
        )


ORDER_HARNESS = r"""
static int fail(const char *message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}
static void track(TrackedInt &word) { word.logged = true; }
static void track_tamper(TamperRecord &record) {
    track(record.field_0_); track(record.field_4_); track(record.field_8_);
    track(record.field_C_); track(record.count_); track(record.field_14_);
}
static int verify_order() {
    for (int row = 0; row < 8; ++row) {
        for (int column = 0; column < 8; ++column) {
            OfferResetCell &cell = OfferResetCells[row][column];
            for (int i = 0; i < 2; ++i) track(cell.count_[i]);
            for (int i = 0; i < 2; ++i) track(cell.state_[i]);
            for (int side = 0; side < 2; ++side) {
                for (int i = 0; i < 8; ++i) {
                    track(cell.type_[side][i]); track(cell.value_[side][i]);
                }
            }
        }
    }
    write_count = 0;
    clear_offer_pair(2, 5);
    TrackedInt *offer_expected[8] = {
        &OfferResetCells[2][5].count_[0], &OfferResetCells[2][5].count_[1],
        &OfferResetCells[5][2].count_[0], &OfferResetCells[5][2].count_[1],
        &OfferResetCells[2][5].state_[0], &OfferResetCells[2][5].state_[1],
        &OfferResetCells[5][2].state_[0], &OfferResetCells[5][2].state_[1]
    };
    if (write_count != 8) return fail("offer store count changed");
    for (int i = 0; i < 8; ++i)
        if (write_log[i] != offer_expected[i]) return fail("offer store order changed");

    track(TamperRegistryCursor);
    for (int index = 0; index < 50; ++index)
        track_tamper(TamperRegistryRecords[index]);
    write_count = 0;
    clear_tamper_registry();
    if (write_count != 301) return fail("tamper store count changed");
    if (write_log[0] != &TamperRegistryCursor) return fail("cursor is not first store");
    int at = 1;
    for (int index = 0; index < 50; ++index) {
        TamperRecord &record = TamperRegistryRecords[index];
        TrackedInt *expected[6] = {
            &record.field_0_, &record.field_4_, &record.field_8_,
            &record.count_, &record.field_C_, &record.field_14_
        };
        for (int word = 0; word < 6; ++word, ++at)
            if (write_log[at] != expected[word])
                return fail("tamper record store order changed");
    }
    puts("PASS: native typed store order");
    return 0;
}
int main() { return verify_order(); }
"""


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_assignment_proxy_checks_actual_native_store_order(compiler, tmp_path):
    structs, bodies = _source_material()
    _compile_and_run(
        compiler, _program(structs, bodies, ORDER_HARNESS, proxy=True),
        tmp_path, "order-" + compiler, True,
    )


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_order_control_detects_same_footprint_reordering(compiler, tmp_path):
    structs, bodies = _source_material()
    body = bodies["clear_offer_pair"]
    first = "    OfferResetCells[faction_a][faction_b].count_[0] = 0;\n"
    second = "    OfferResetCells[faction_a][faction_b].count_[1] = 0;\n"
    reordered = body.replace(first + second, second + first, 1)
    mutated = {**bodies, "clear_offer_pair": reordered}
    _compile_and_run(
        compiler, _program(structs, mutated, ORDER_HARNESS, proxy=True),
        tmp_path, "order-mutant-" + compiler, False,
    )

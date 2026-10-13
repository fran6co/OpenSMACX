"""Typed checksum-report contract and consumer controls (not game acceptance)."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ROOT = Path(__file__).resolve().parents[2]
BASE = "e05d10549f0f3149d17e1a6a83ecaa80d9da03c4"
OLD_SIGNATURE = "void NetDaemon::game_checksum(int *results)"
SIGNATURE = "void NetDaemon::game_checksum(NetChecksumReport *results)"
REPORT = ROOT / "src/net_checksum_report.h"


def report_header() -> str:
    return REPORT.read_text(encoding="utf-8").replace("#pragma once\n", "", 1)


def report_declaration() -> str:
    return extract_definition(report_header(), "struct NetChecksumReport ") + ";"


def restore_report_migration(source: str) -> str:
    """Reverse only the typed output view for preceding checksum source controls."""
    old = subprocess.check_output(
        ["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT
    ).decode()
    current = extract_definition(source, SIGNATURE)
    restored = current
    replacements = (
        ("results->common[0] = cs;", "*out++ = cs;"),
        ("results->common[1] = cs;", "*out++ = cs;"),
        ("results->common[2] = cs;", "*out++ = cs;"),
        ("results->common[3] = cs;", "*out++ = cs;"),
        ("results->common[4] = cs;", "*out++ = cs;"),
        ("results->map_sections[k] = cs;", "*out++ = cs;"),
        ("results->diagnostic_aggregates[0] = checksum(\n            (char *)&GameDataStateStorage.fields_.lock_masks_.enable_mask_, 2, cs);", "*out++ = checksum((char *)&GameDataStateStorage.fields_.lock_masks_.enable_mask_, 2, cs);"),
        ("results->diagnostic_aggregates[1] = checksum(\n            (char *)&GameDataStateStorage.fields_.secret_project_, 0x100, 0);", "*out++ = checksum((char *)&GameDataStateStorage.fields_.secret_project_, 0x100, 0);"),
        ("results->diagnostic_aggregates[2] = cs;", "*out++ = cs;"),
        ("results->faction_values[faction - 1] = cs;", "*out++ = cs;"),
        ("results->map_values[0] = cs;", "*out++ = cs;"),
        ("results->map_values[1] = cs;", "*out++ = cs;"),
        ("results->map_values[2] = cs;", "*out++ = cs;"),
        ("results->map_values[3] = cs;", "*out++ = cs;"),
        ("results->map_values[4] = cs;", "*out++ = cs;"),
        ("results->map_values[5] = cs;", "*out++ = cs;"),
        ("results->map_values[6] = checksum((char *)&MapSizePlanet, 0x1c, 0);", "*out++ = checksum((char *)&MapSizePlanet, 0x1c, 0);"),
        ("results->vehicles_by_faction[idx2b] = cs;", "*out++ = cs;"),
        ("results->base_value = cs;", "*out++ = cs;"),
    )
    for typed, previous in replacements:
        assert restored.count(typed) == 1, typed
        restored = restored.replace(typed, previous, 1)
    restored = restored.replace(
        "void NetDaemon::game_checksum(NetChecksumReport *results) {\n    unsigned char cs;",
        "void NetDaemon::game_checksum(int *results) {\n    unsigned char cs;\n    int *out = results;",
        1,
    )
    source = source.replace(current, restored, 1)
    marker = "// ORIGINAL: 0x0052F8E0 ?game_checksum@NetDaemon@@QAEXPAH@Z"
    marker_at = source.index(marker)
    region_end = source.index("// size      ", marker_at)
    region = source[marker_at:region_end]
    region = re.sub(r"(?m)^// symbol\s+.*\n", "", region)
    region = re.sub(r"(?m)^// RECOVERY_BINDING: v1 .*\n", "", region)
    source = source[:marker_at] + region + source[region_end:]
    report_include = '#include "net_checksum_report.h"\n'
    assert source.count(report_include) == 1
    return source.replace(report_include, "", 1)


def test_report_declaration_groups_and_physical_layout_are_explicit():
    header = report_header()
    for member in (
        "int common[5];",
        "int map_sections[16];",
        "int diagnostic_aggregates[3];",
        "int faction_values[7];",
        "int map_values[7];",
        "int vehicles_by_faction[8];",
        "int base_value;",
    ):
        assert member in header
    for assertion in (
        "offsetof(NetChecksumReport, common) == 0x00",
        "offsetof(NetChecksumReport, map_sections) == 0x14",
        "offsetof(NetChecksumReport, diagnostic_aggregates) == 0x54",
        "offsetof(NetChecksumReport, faction_values) == 0x60",
        "offsetof(NetChecksumReport, map_values) == 0x7C",
        "offsetof(NetChecksumReport, vehicles_by_faction) == 0x98",
        "offsetof(NetChecksumReport, base_value) == 0xB8",
        "sizeof(NetChecksumReport) == 0xBC",
    ):
        assert assertion in header


def test_producer_consumer_use_one_typed_report_and_preserve_original_identity():
    source = (ROOT / "src/netdaemon.cpp").read_text(encoding="utf-8")
    consumer = (ROOT / "src/map.cpp").read_text(encoding="utf-8")
    owner_header = (ROOT / "src/netdaemon.h").read_text(encoding="utf-8")
    assert "struct NetChecksumReport;" in owner_header
    assert '#include "net_checksum_report.h"' not in owner_header
    assert source.count('#include "net_checksum_report.h"') == 1
    assert consumer.count('#include "net_checksum_report.h"') == 1
    include_owners = {
        path.relative_to(ROOT).as_posix()
        for suffix in ("*.cpp", "*.h")
        for path in (ROOT / "src").rglob(suffix)
        if '#include "net_checksum_report.h"' in path.read_text(encoding="utf-8")
    }
    assert include_owners == {"src/netdaemon.cpp", "src/map.cpp"}
    assert restore_report_migration(source) == subprocess.check_output(
        ["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT
    ).decode()
    body = extract_definition(source, SIGNATURE)
    consumer_body = extract_definition(
        consumer, "void __cdecl do_checksums(int type)"
    )
    assert "void game_checksum(NetChecksumReport *results);" in owner_header
    assert "NetChecksumReport checksums;" in consumer_body
    assert "NetDaemonNet.game_checksum(&checksums);" in consumer_body
    assert "memcpy(NetTurnChecksums, &checksums, 0x54);" in consumer_body
    assert "int checksums[47]" not in consumer_body
    assert "results->common[0] = cs;" in body
    assert "results->map_sections[k] = cs;" in body
    assert "results->diagnostic_aggregates[0]" in body
    assert "results->faction_values[faction - 1] = cs;" in body
    assert "results->map_values[6] = checksum((char *)&MapSizePlanet, 0x1c, 0);" in body
    assert "results->vehicles_by_faction[idx2b] = cs;" in body
    assert "results->base_value = cs;" in body
    assert "if (BaseAtDebugPopupFlag != 0)" in body
    assert "// ORIGINAL: 0x0052F8E0 ?game_checksum@NetDaemon@@QAEXPAH@Z" in source
    assert "// symbol    ?game_checksum@NetDaemon@@QAEXPAUNetChecksumReport@@@Z" in source
    assert "NetTurnChecksumWords = 21" in owner_header
    assert "extern int NetTurnChecksums[NetTurnChecksumWords];" in owner_header


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_typed_prefix_copy_and_caller_poison_preserve_unwritten_tail(tmp_path, compiler):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " unavailable")
    source = r'''#include <cassert>
#include <cstring>
REPORT_HEADER
static int wire[21];
static void producer(NetChecksumReport *report, bool debug) {
    for (int i=0;i<5;++i) report->common[i]=0x100+i;
    for (int i=0;i<16;++i) report->map_sections[i]=0x105+i;
    if (debug) {
        for (int i=0;i<3;++i) report->diagnostic_aggregates[i]=0x115+i;
        for (int i=0;i<7;++i) report->faction_values[i]=0x118+i;
        for (int i=0;i<7;++i) report->map_values[i]=0x11f+i;
        for (int i=0;i<8;++i) report->vehicles_by_faction[i]=0x126+i;
        report->base_value=0x12e;
    }
}
int main() {
    NetChecksumReport report;
    std::memset(&report, 0x5a, sizeof(report));
    producer(&report, false);
    for (int i=0;i<21;++i) wire[i]=-1;
    std::memcpy(wire, &report, 0x54);
    for (int i=0;i<5;++i) assert(wire[i]==0x100+i);
    for (int i=0;i<16;++i) assert(wire[i+5]==0x105+i);
    const unsigned char *bytes=reinterpret_cast<const unsigned char *>(&report);
    for (int i=0x54;i<0xBC;++i) assert(bytes[i]==0x5a);
    producer(&report, true);
    assert(report.diagnostic_aggregates[0]==0x115);
    assert(report.faction_values[0]==0x118);
    assert(report.map_values[0]==0x11f);
    assert(report.vehicles_by_faction[0]==0x126);
    assert(report.base_value==0x12e);
}
'''.replace("REPORT_HEADER", report_header())
    cpp = tmp_path / "report.cpp"
    binary = tmp_path / "report"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr

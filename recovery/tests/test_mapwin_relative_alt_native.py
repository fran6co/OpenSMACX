"""VC6/Wine controls for the recovered MapWin relative-altitude reader.

The unchanged portable UBSan test exposes the known oversized-shift defect,
including ordinary positive detail values. This exact source spelling relies
on the measured VC6/IA-32 shift-count masking. This test compiles the actual C++98
body and checks that target behavior directly.
"""
from pathlib import Path
import os
import shutil
import subprocess
import sys

import pytest
from match_recovery.measurement import Measurement
from match_recovery.processes import run_bounded
from recovery.environment.compile import compile_object
from recovery.environment.wine import VC6_ROOT, environment


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated product definition")


DECLARATIONS = r'''#include <stddef.h>
typedef unsigned int uint32_t;
typedef int int32_t;

struct MapMeta { int longitude_bounds_; };
struct MapStorageWindow { MapMeta metadata_; };
extern MapStorageWindow MapStorage;
#define MapLongitudeBounds (MapStorage.metadata_.longitude_bounds_)

ACTUAL_RECORD

struct MapWin {
    TerrainCacheEntry *owned_;
    int get_alt(int, int, int);
    int get_relative_alt(int, int, int);
};
'''

BODY_UNIT = r'''#include "fixture.h"
#include <string.h>
ACTUAL_BODY
'''

HARNESS = r'''#include "fixture.h"
#include <assert.h>
#include <limits.h>

MapStorageWindow MapStorage;
static TerrainCacheEntry records[400];
static TerrainCacheEntry other_records[400];
static int calls, last_x, last_y, last_detail, altitude_return;
static TerrainCacheEntry *owner_after_get;
static int width_after_get;

int MapWin::get_alt(int x, int y, int detail) {
    ++calls;
    last_x = x; last_y = y; last_detail = detail;
    owned_ = owner_after_get;
    MapLongitudeBounds = width_after_get;
    return altitude_return;
}

static void prepare(MapWin &window, TerrainCacheEntry *before,
                    TerrainCacheEntry *after, int width) {
    window.owned_ = before;
    owner_after_get = after;
    width_after_get = width;
    MapLongitudeBounds = 16;
    calls = last_x = last_y = last_detail = 0;
    altitude_return = -77;
}

int main() {
    typedef char record_is_twelve_bytes[(sizeof(TerrainCacheEntry) == 12) ? 1 : -1];
    typedef char detail_is_at_ten[(offsetof(TerrainCacheEntry, altitude_detail) == 10) ? 1 : -1];
    (void)sizeof(record_is_twelve_bytes);
    (void)sizeof(detail_is_at_ten);

    MapWin window;
    prepare(window, records, 0, 16);
    records[18].altitude_detail = 0x4321;
    assert(window.get_relative_alt(4, 2, 1) == 0);
    assert(calls == 1 && last_x == 4 && last_y == 2 && last_detail == 1);

    prepare(window, records, records, 16);
    assert(window.get_relative_alt(4, 2, 0) == 4);
    assert(calls == 1 && last_detail == 0);

    records[18].altitude_detail = 0x4321;
    for (int detail = 1; detail <= 4; ++detail) {
        prepare(window, records, records, 16);
        assert(window.get_relative_alt(4, 2, detail) == detail);
    }

    // The original has no detail-range guard: the x86 shift masks to five
    // bits, including for negative and extreme detail values.
    const int details[] = {-1, -8, 5, 8, 9, 10, INT_MIN, INT_MAX};
    for (unsigned int i = 0; i < sizeof(details) / sizeof(details[0]); ++i) {
        const int detail = details[i];
        unsigned int shift =
            ((static_cast<uint32_t>(detail) - 1U) * 4U) & 31U;
        prepare(window, records, records, 16);
        assert(window.get_relative_alt(4, 2, detail) ==
               static_cast<int>((static_cast<uint32_t>(0x4321) >> shift) & 0xfU));
    }

    // get_alt runs before owner and width are read. Its side effects select
    // record 14 using the new full width 12, not the old width 16.
    other_records[14].altitude_detail = 0x0006;
    prepare(window, records, other_records, 12);
    assert(window.get_relative_alt(4, 2, 1) == 6);
    assert(calls == 1 && window.owned_ == other_records && MapLongitudeBounds == 12);

    // Negative coordinates preserve signed row indexing. A centered owner
    // keeps the original unchecked row access inside allocated storage.
    records[11].altitude_detail = 0xabcd;
    prepare(window, records + 20, records + 20, 16);
    assert(window.get_relative_alt(-1, -1, 1) == 13);

    // 32-bit row arithmetic wraps to -1. The owner offset makes that access
    // valid while checking the original wraparound computation.
    records[0].altitude_detail = 0x5678;
    prepare(window, records + 1, records + 1, 0x1fffffff);
    assert(window.get_relative_alt(30, 16, 1) == 8);
    return 0;
}
'''


def current_body():
    return braced((ROOT / "src/mapwin.cpp").read_text(),
                  "int MapWin::get_relative_alt(int x, int y, int detail)")


def damage_mutants(body):
    shift = ("const unsigned int shift =\n"
             "        (static_cast<uint32_t>(detail) + 0x3fffU) << 2;")
    stride = "static_cast<uint32_t>(MapLongitudeBounds >> 1) *"
    row = "owned_[row].altitude_detail"
    callback = "get_alt(x, y, detail);"
    sentinel = "return 4;"
    assert body.count(shift) == 1
    assert body.count(stride) == 1
    assert body.count(row) == 1
    assert body.count(callback) == 1
    assert body.count(sentinel) == 1
    return {
        "wrong shift modulo 16": body.replace(
            shift, "const unsigned int shift =\n"
                   "        ((static_cast<uint32_t>(detail) + 0x3fffU) << 2) & 15U;", 1),
        "wrong full-width row stride": body.replace(
            stride, "static_cast<uint32_t>(MapLongitudeBounds) *", 1),
        "off-by-one row access": body.replace(
            row, "owned_[row + 1].altitude_detail", 1),
        "missing get_alt callback": body.replace(
            callback, "/* callback removed */", 1),
        "wrong zero-detail sentinel": body.replace(sentinel, "return 0;", 1),
    }


def native_runner(tmp_path):
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native MapWin controls require the coordinator's existing compiler lease")
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or not shutil.which("wine32"):
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")

    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    # This is the reproduced mapwin.cpp application recipe, including the
    # current Ob2 and frame-pointer settings used by the byte-exact probe.
    flags = ("/TP", "/Zm200", "/GX", "/O2", "/Ob2", "/W4", "/Oy-", "/c") + settings.layout_flags
    record = braced((ROOT / "src/mapwin.h").read_text(), "struct TerrainCacheEntry") + ";"
    body = current_body()

    def build(name, selected_body):
        declarations = tmp_path / "fixture.h"
        body_source = tmp_path / (name + "-body.cpp")
        main_source = tmp_path / (name + "-main.cpp")
        body_obj = tmp_path / (name + "-body.obj")
        main_obj = tmp_path / (name + "-main.obj")
        exe = tmp_path / (name + ".exe")
        declarations.write_text(DECLARATIONS.replace("ACTUAL_RECORD", record), encoding="utf-8")
        body_source.write_text(BODY_UNIT.replace("ACTUAL_BODY", selected_body), encoding="utf-8")
        main_source.write_text(HARNESS, encoding="utf-8")
        body_obj.write_bytes(compile_object(body_source, command, flags))
        main_obj.write_bytes(compile_object(main_source, command, flags))
        linked = run_bounded(
            [sys.executable, str(wrapper), "--tool=link", "/nologo",
             "/subsystem:console", "/incremental:no", "/out:" + str(exe),
             str(body_obj), str(main_obj), "kernel32.lib"], cwd=tmp_path, timeout=90)
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        return exe

    def execute(exe):
        return subprocess.run(["wine32", str(exe)], env=environment(),
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=30)

    return build, execute, body


def test_vc6_application_profile_actual_body_and_five_damage_controls(tmp_path):
    build, execute, body = native_runner(tmp_path)
    good = build("relative-alt-good", body)
    passed = execute(good)
    assert passed.returncode == 0, passed.stdout.decode(errors="replace")

    mutants = damage_mutants(body)
    for index, (label, mutant) in enumerate(mutants.items()):
        assert mutant != body, label
        damaged = build("relative-alt-bad-%d" % index, mutant)
        failed = execute(damaged)
        assert failed.returncode != 0, f"{label} mutant survived the native controls"

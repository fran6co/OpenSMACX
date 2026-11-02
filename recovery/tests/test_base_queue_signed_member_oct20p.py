"""Actual-source checks for the original signed Base queue-count semantics."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


TEST_ROOT = Path(__file__).resolve().parent
PROJECT = TEST_ROOT.parents[1]
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE = next(
    (parent / ".opensmacx/game/terranx_original.exe"
     for parent in (PROJECT, *PROJECT.parents)
     if (parent / ".opensmacx/game/terranx_original.exe").is_file()),
    PROJECT / ".opensmacx/game/terranx_original.exe",
)


def _masked_cpp(text: str) -> str:
    """Mask comments and literals so braces inside them do not bound a body."""
    out = list(text)
    index = 0
    state = "code"
    quote = ""
    raw_end = ""
    while index < len(text):
        if state == "code":
            if text.startswith("//", index):
                out[index:index + 2] = "  "
                index += 2
                state = "line"
            elif text.startswith("/*", index):
                out[index:index + 2] = "  "
                index += 2
                state = "block"
            elif text.startswith('R"', index):
                delimiter_end = text.find("(", index + 2, index + 20)
                if delimiter_end >= 0:
                    delimiter = text[index + 2:delimiter_end]
                    raw_end = ")" + delimiter + '"'
                    for pos in range(index, delimiter_end + 1):
                        if text[pos] != "\n":
                            out[pos] = " "
                    index = delimiter_end + 1
                    state = "raw"
                else:
                    index += 1
            elif text[index] in "\"'":
                quote = text[index]
                out[index] = " "
                index += 1
                state = "quoted"
            else:
                index += 1
        elif state == "line":
            if text[index] == "\n":
                state = "code"
            else:
                out[index] = " "
            index += 1
        elif state == "block":
            if text.startswith("*/", index):
                out[index:index + 2] = "  "
                index += 2
                state = "code"
            else:
                if text[index] != "\n":
                    out[index] = " "
                index += 1
        elif state == "raw":
            if text.startswith(raw_end, index):
                for pos in range(index, index + len(raw_end)):
                    if text[pos] != "\n":
                        out[pos] = " "
                index += len(raw_end)
                state = "code"
            else:
                if text[index] != "\n":
                    out[index] = " "
                index += 1
        else:
            if text[index] == "\\":
                out[index] = " "
                if index + 1 < len(text):
                    if text[index + 1] != "\n":
                        out[index + 1] = " "
                    index += 2
                else:
                    index += 1
            elif text[index] == quote:
                out[index] = " "
                index += 1
                state = "code"
            else:
                if text[index] != "\n":
                    out[index] = " "
                index += 1
    return "".join(out)


def _extract_definition(path: Path, signature: str) -> str:
    source = path.read_text(encoding="utf-8")
    masked = _masked_cpp(source)
    starts = [match.start() for match in re.finditer(re.escape(signature), masked)]
    assert len(starts) == 1, f"expected one {signature!r}, found {len(starts)}"
    start = starts[0]
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                body = source[start:index + 1]
                assert "BODY GOES HERE." not in body
                return body
    raise AssertionError(f"unterminated definition {signature!r}")


def _extract_ai_queue_loop() -> str:
    source = (PROJECT / "src/faction_strategy.cpp").read_text(encoding="utf-8")
    signature = "for (int queue_index = 0; queue_index <= base.queue_size; ++queue_index)"
    masked = _masked_cpp(source)
    starts = [match.start() for match in re.finditer(re.escape(signature), masked)]
    assert len(starts) == 1, f"expected one AI queue loop, found {len(starts)}"
    start = starts[0]
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated enemy_strategy queue loop")


def _queue_fixture(base_queue_body: str, member_type: str = "int") -> str:
    return r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct AccessFault {};
struct QueueItems {
    int values[10];
    int reads[10];
    int writes[10];
    QueueItems() { memset(values, 0, sizeof(values)); memset(reads, 0, sizeof(reads)); memset(writes, 0, sizeof(writes)); }
    int get(int index) {
        if (index < 0 || index >= 10) throw AccessFault();
        ++reads[index];
        return values[index];
    }
    void set(int index, int value) {
        if (index < 0 || index >= 10) throw AccessFault();
        ++writes[index];
        values[index] = value;
    }
    struct Reference {
        QueueItems *owner;
        int index;
        operator int() const { return owner->get(index); }
        Reference &operator=(int value) { owner->set(index, value); return *this; }
        Reference &operator=(const Reference &value) { owner->set(index, static_cast<int>(value)); return *this; }
    };
    Reference operator[](int index) { Reference result = { this, index }; return result; }
    void clear_accesses() { memset(reads, 0, sizeof(reads)); memset(writes, 0, sizeof(writes)); }
};
struct Base {
    MEMBER_TYPE queue_size;
    unsigned int faction_id_current;
    int x;
    int y;
    QueueItems queue_production_id;
};
Base Bases[1];
struct PlayerData { int proto_id_queue[512]; };
PlayerData PlayersData[8];
typedef int BOOL;
#define __cdecl
const int FacilitySPStart = 64;
int vehicle_script[4], facility_script[4];
int vehicle_calls, facility_calls, radius_calls, wave_calls;
int vehicle_ids[4], facility_ids[4];
int callback_mutates_queue, callback_queue_value;
int veh_avail(int id, unsigned int, int) {
    vehicle_ids[vehicle_calls] = id;
    if (callback_mutates_queue) Bases[0].queue_production_id.values[0] = callback_queue_value;
    return vehicle_script[vehicle_calls++];
}
int facility_avail(int id, unsigned int, int, int) {
    facility_ids[facility_calls] = id;
    if (callback_mutates_queue) Bases[0].queue_production_id.values[0] = callback_queue_value;
    return facility_script[facility_calls++];
}
void wave_it(int) { ++wave_calls; }
void draw_radius(int, int, int, int) { ++radius_calls; }
/*__BASE_QUEUE__*/
int main(int argc, char **argv) {
    if (argc != 2) return 80;
    const char *scenario = argv[1];
    if (!strcmp(scenario, "positive")) {
        Bases[0].queue_size = 2;
        Bases[0].queue_production_id[0] = 5;
        Bases[0].queue_production_id[1] = 6;
        Bases[0].queue_production_id[2] = 7;
        Bases[0].queue_production_id.clear_accesses();
        PlayersData[0].proto_id_queue[5] = 4;
        PlayersData[0].proto_id_queue[6] = 3;
        vehicle_script[0] = 0;
        vehicle_script[1] = 1;
        if (!base_queue(0)) return 1;
        if (Bases[0].queue_size != 0) return 2;
        if (Bases[0].queue_production_id.values[0] != 7) return 3;
        if (PlayersData[0].proto_id_queue[5] != 3 || PlayersData[0].proto_id_queue[6] != 2) return 4;
        if (vehicle_calls != 2 || vehicle_ids[0] != 6 || vehicle_ids[1] != 7) return 5;
        if (radius_calls != 1 || wave_calls != 0) return 6;
        return 0;
    }
    if (!strcmp(scenario, "negative69")) {
        Bases[0].queue_size = -69;
        Bases[0].queue_production_id[0] = 5;
        Bases[0].queue_production_id[1] = 6;
        Bases[0].queue_production_id.clear_accesses();
        PlayersData[0].proto_id_queue[5] = 4;
        vehicle_script[0] = 1;
        try {
            if (!base_queue(0)) return 7;
        } catch (AccessFault) { return 90; }
        if (Bases[0].queue_size != -70) return 8;
        if (Bases[0].queue_production_id.values[0] != 5 || Bases[0].queue_production_id.writes[0] != 0) return 9;
        if (Bases[0].queue_production_id.reads[1] != 0 || Bases[0].queue_production_id.writes[1] != 0) return 10;
        if (PlayersData[0].proto_id_queue[5] != 3 || vehicle_calls != 1 || radius_calls != 1) return 11;
        return 0;
    }
    if (!strcmp(scenario, "negative1")) {
        Bases[0].queue_size = -1;
        Bases[0].queue_production_id[0] = -69;
        Bases[0].queue_production_id.clear_accesses();
        facility_script[0] = 1;
        try {
            if (!base_queue(0)) return 12;
        } catch (AccessFault) { return 91; }
        if (Bases[0].queue_size != -2) return 13;
        if (Bases[0].queue_production_id.values[0] != -69 || Bases[0].queue_production_id.writes[0] != 0) return 14;
        if (Bases[0].queue_production_id.reads[1] != 0 || facility_calls != 1 || facility_ids[0] != 69) return 15;
        return 0;
    }
    if (!strcmp(scenario, "callback_to_project") || !strcmp(scenario, "callback_to_vehicle")) {
        const bool to_project = !strcmp(scenario, "callback_to_project");
        Bases[0].queue_size = 1;
        Bases[0].queue_production_id[0] = 5;
        Bases[0].queue_production_id[1] = to_project ? 6 : -FacilitySPStart;
        vehicle_script[0] = facility_script[0] = 1;
        callback_mutates_queue = 1;
        callback_queue_value = to_project ? -FacilitySPStart : 6;
        if (!base_queue(0)) return 16;
        if (radius_calls != 1 || Bases[0].queue_production_id.values[0] != callback_queue_value) return 17;
        if (wave_calls != (to_project ? 1 : 0)) return 18;
        if (vehicle_calls != (to_project ? 1 : 0) || facility_calls != (to_project ? 0 : 1)) return 19;
        return 0;
    }
    return 81;
}
'''.replace("MEMBER_TYPE", member_type).replace("/*__BASE_QUEUE__*/", base_queue_body)


def _basewin_fixture(add_queue_body: str, member_type: str = "int") -> str:
    return r'''#include <stdint.h>
#include <string.h>
struct QueueItems {
    int values[10];
    QueueItems() { memset(values, 0, sizeof(values)); }
    int &operator[](int index) { return values[index]; }
};
struct Base { MEMBER_TYPE queue_size; QueueItems queue_production_id; };
Base base_storage;
Base *BaseCurrent = &base_storage;
struct BaseWin {
    int selected_queue_slot_;
    int field_40B30_;
    int queue_draws;
    int production_draws;
    void draw_queue(int) { ++queue_draws; }
    void draw_production(int) { ++production_draws; }
    void add_queue(int, int);
};
/*__ADD_QUEUE__*/
int main() {
    BaseWin window;
    memset(&window, 0, sizeof(window));
    BaseCurrent->queue_size = -69;
    window.selected_queue_slot_ = 1;
    try {
        window.add_queue(1, 5);
    } catch (...) { return 90; }
    if (static_cast<int32_t>(BaseCurrent->queue_size) != -67) return 1;
    if (BaseCurrent->queue_production_id.values[1] != 5) return 2;
    if (window.queue_draws != 1) return 3;
    return 0;
}
'''.replace("MEMBER_TYPE", member_type).replace("/*__ADD_QUEUE__*/", add_queue_body)


def _ai_fixture(queue_loop: str, member_type: str = "int") -> str:
    return r'''#include <stdint.h>
#include <string.h>
struct AccessFault {};
struct QueueItems {
    int values[10];
    int reads[10];
    QueueItems() { memset(values, 0, sizeof(values)); memset(reads, 0, sizeof(reads)); }
    int get(int index) {
        if (index < 0 || index >= 10) throw AccessFault();
        ++reads[index]; return values[index];
    }
    struct Reference {
        QueueItems *owner; int index;
        operator int() const { return owner->get(index); }
    };
    Reference operator[](int index) { Reference result = { this, index }; return result; }
};
struct Base { MEMBER_TYPE queue_size; QueueItems queue_production_id; };
struct PlayerData { int proto_id_queue[512]; };
/*__AI_LOOP__*/
int main(int argc, char **argv) {
    if (argc != 2) return 80;
    Base base;
    PlayerData faction_data;
    memset(&faction_data, 0, sizeof(faction_data));
    if (!strcmp(argv[1], "negative")) {
        base.queue_size = -1;
        base.queue_production_id.values[0] = 7;
        try { RUN_LOOP; } catch (AccessFault) { return 90; }
        if (faction_data.proto_id_queue[7] != 0 || base.queue_production_id.reads[0] != 0) return 1;
        return 0;
    }
    if (!strcmp(argv[1], "positive")) {
        base.queue_size = 2;
        base.queue_production_id.values[0] = 5;
        base.queue_production_id.values[1] = -69;
        base.queue_production_id.values[2] = 7;
        try { RUN_LOOP; } catch (AccessFault) { return 91; }
        if (faction_data.proto_id_queue[5] != 1 || faction_data.proto_id_queue[7] != 1) return 2;
        if (base.queue_production_id.reads[0] != 1 || base.queue_production_id.reads[1] != 1 || base.queue_production_id.reads[2] != 1) return 3;
        return 0;
    }
    return 81;
}
'''.replace("MEMBER_TYPE", member_type).replace("/*__AI_LOOP__*/", "int main_loop_anchor;").replace("RUN_LOOP", queue_loop)


def _compile_run(tmp_path: Path, compiler: str, source: str, *args: str) -> subprocess.CompletedProcess[str]:
    tmp_path.mkdir(parents=True, exist_ok=True)
    source_path = tmp_path / "candidate.cpp"
    executable = tmp_path / "candidate"
    source_path.write_text(source, encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Wno-sign-compare", str(source_path), "-o", str(executable)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(executable), *args], capture_output=True, text=True, timeout=10)


def _compiler(name: str) -> str:
    value = shutil.which(name)
    if value is None:
        pytest.skip(f"{name} unavailable")
    return value


def _base_queue_body() -> str:
    return _extract_definition(PROJECT / "src/base.cpp", "BOOL __cdecl base_queue(int base_id)")


def _basewin_add_queue_body() -> str:
    return _extract_definition(PROJECT / "src/basewin.cpp", "void BaseWin::add_queue(int insert, int production_id)")


def _ai_loop() -> str:
    return _extract_ai_queue_loop()


def test_pinned_original_queue_count_branches_are_signed() -> None:
    assert IMAGE.is_file(), "pinned original image is required for this regression"
    image = PeImage(IMAGE.read_bytes(), PIN_SHA256)
    # base_queue: signed nonpositive count bypasses shifting, while its outer
    # loop still repeats on any nonzero count.
    assert image.read(Span(0x004F0732, 0x004F0736)) == bytes.fromhex("85 c0 7e 19")
    assert image.read(Span(0x004F0790, 0x004F0798)) == bytes.fromhex("85 c0 0f 85 75 ff ff ff")
    # enemy_strategy's inclusive queue walk is skipped for a signed negative.
    assert image.read(Span(0x005614F7, 0x00561501)) == bytes.fromhex(
        "8b 86 8c d0 97 00 85 c0 7c 26")
    # BaseWin::add_queue increments only when signed count is below 9.
    assert image.read(Span(0x00416EC1, 0x00416EC9)) == bytes.fromhex(
        "8b 41 4c 83 f8 09 7d 0a")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("scenario", ["positive", "negative69", "negative1"])
def test_actual_base_queue_body_signed_member_and_loop(tmp_path: Path, compiler_name: str, scenario: str) -> None:
    body = _base_queue_body()
    assert "for (int i = 0; i < base->queue_size; i++)" in body
    result = _compile_run(tmp_path, _compiler(compiler_name), _queue_fixture(body), scenario)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("scenario", ["callback_to_project", "callback_to_vehicle"])
def test_actual_base_queue_reloads_head_after_availability(tmp_path, compiler_name, scenario):
    body = _base_queue_body()
    result = _compile_run(tmp_path / "actual", _compiler(compiler_name), _queue_fixture(body), scenario)
    assert result.returncode == 0, result.stdout + result.stderr
    cached = body.replace("base->queue_production_id[0] <= -FacilitySPStart",
                          "queue_prod_id <= -FacilitySPStart", 1)
    assert cached != body
    damaged = _compile_run(tmp_path / "cached", _compiler(compiler_name), _queue_fixture(cached), scenario)
    assert damaged.returncode == 18, damaged.stdout + damaged.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_base_queue_body_rejects_unsigned_member_and_counter_mutants(
    tmp_path: Path, compiler_name: str,
) -> None:
    body = _base_queue_body()
    negative = "negative69"
    unsigned_member = _compile_run(
        tmp_path / "member", _compiler(compiler_name),
        _queue_fixture(body, "uint32_t"), negative,
    )
    assert unsigned_member.returncode != 0

    unsigned_counter_body = body.replace(
        "for (int i = 0; i < base->queue_size; i++)",
        "for (uint32_t i = 0; i < base->queue_size; i++)", 1,
    )
    assert unsigned_counter_body != body
    unsigned_counter = _compile_run(
        tmp_path / "counter", _compiler(compiler_name),
        _queue_fixture(unsigned_counter_body), negative,
    )
    assert unsigned_counter.returncode != 0


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_basewin_add_queue_keeps_signed_below_nine_behavior(
    tmp_path: Path, compiler_name: str,
) -> None:
    body = _basewin_add_queue_body()
    assert "if (BaseCurrent->queue_size < 9)" in body
    result = _compile_run(tmp_path, _compiler(compiler_name), _basewin_fixture(body))
    assert result.returncode == 0, result.stdout + result.stderr

    unsigned_member = _compile_run(
        tmp_path / "unsigned-member", _compiler(compiler_name),
        _basewin_fixture(body, "uint32_t"),
    )
    assert unsigned_member.returncode != 0


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_enemy_strategy_queue_loop_is_inclusive_and_signed(
    tmp_path: Path, compiler_name: str,
) -> None:
    loop = _ai_loop()
    assert "queue_index <= base.queue_size" in loop
    signed = _compile_run(tmp_path, _compiler(compiler_name), _ai_fixture(loop), "positive")
    assert signed.returncode == 0, signed.stdout + signed.stderr
    negative = _compile_run(tmp_path / "negative", _compiler(compiler_name), _ai_fixture(loop), "negative")
    assert negative.returncode == 0, negative.stdout + negative.stderr

    unsigned_member = _compile_run(
        tmp_path / "unsigned-member", _compiler(compiler_name),
        _ai_fixture(loop, "uint32_t"), "negative",
    )
    assert unsigned_member.returncode != 0

    unsigned_counter_loop = loop.replace(
        "for (int queue_index = 0;",
        "for (uint32_t queue_index = 0;", 1,
    )
    assert unsigned_counter_loop != loop
    unsigned_counter = _compile_run(
        tmp_path / "unsigned-counter", _compiler(compiler_name),
        _ai_fixture(unsigned_counter_loop), "negative",
    )
    assert unsigned_counter.returncode != 0


def test_member_layout_and_consumer_inventory_are_retained() -> None:
    header = (PROJECT / "src/base.h").read_text(encoding="utf-8")
    assert re.search(r"\bint\s+queue_size\s*;", header)
    assert "int queue_production_id[10];" in header
    assert "int eco_damage;\n    int queue_size;" in header
    assert "RECOVERY_MEMBER: v1 Base queue_production_id 0x50" in header

    # These consumers intentionally reinterpret the signed count before an
    # inclusive walk; a signed member preserves that behavior without cleanup.
    for relative, fragment in (
        ("src/faction.cpp", "static_cast<int32_t>(base.queue_size)"),
        ("src/veh.cpp", "static_cast<int32_t>(base.queue_size)"),
        ("src/game.cpp", "static_cast<int>(base.queue_size)"),
    ):
        assert fragment in (PROJECT / relative).read_text(encoding="utf-8")

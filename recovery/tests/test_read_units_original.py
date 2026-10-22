"""Actual-source controls for the pinned #UNITS parser contract."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


def _root():
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/alpha.cpp").is_file():
            return parent
    raise AssertionError("repository root not found")


ROOT = _root()
SOURCE = ROOT / "src/alpha.cpp"


def _without_comments(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def _extract_read_units(text=None):
    source = _without_comments(SOURCE.read_text() if text is None else text)
    signature = "BOOL __cdecl read_units()"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated read_units body")


HARNESS_PREFIX = r"""
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <string>
#include <vector>

#define __cdecl
typedef int BOOL;
typedef char *LPSTR;
typedef const char *LPCSTR;

static const int MaxVehProtoFactionNum = 20;
enum { BSC_BATTLE_OGRE_MK1 = 16, BSC_BATTLE_OGRE_MK2 = 17,
       BSC_BATTLE_OGRE_MK3 = 18 };
enum { RECT_FISSION = 1, RECT_FUSION = 2, RECT_QUANTUM = 3 };

std::vector<std::string> events;
std::vector<std::vector<std::string> > rows;
int line_index = -1;
size_t item_index = 0;
bool missing_units = false;
LPCSTR AlphaxFileID = "ALPHAX";

void check(bool condition) { if (!condition) std::exit(71); }
std::string number_text(int value) {
    char buffer[32];
    sprintf(buffer, "%d", value);
    return buffer;
}
void add_event(const char *kind, const std::string &value) {
    events.push_back(std::string(kind) + ":" + value);
}
int text_open(LPCSTR, LPCSTR section) {
    events.push_back(std::string("open:") + section);
    return missing_units ? 1 : 0;
}
void text_get() {
    events.push_back("get");
    ++line_index;
    item_index = 0;
    check(line_index >= 0 && line_index < static_cast<int>(rows.size()));
}
std::string consume(const char *kind) {
    check(line_index >= 0 && line_index < static_cast<int>(rows.size()));
    check(item_index < rows[line_index].size());
    const std::string value = rows[line_index][item_index++];
    add_event(kind, value);
    return value;
}
LPSTR text_item() {
    check(line_index >= 0 && line_index < static_cast<int>(rows.size()));
    check(item_index < rows[line_index].size());
    std::string &value = rows[line_index][item_index++];
    add_event("item", value);
    return const_cast<char *>(value.c_str());
}
int text_item_number() { return std::atoi(consume("number").c_str()); }
int text_item_binary() { return std::atoi(consume("binary").c_str()); }
int range(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}
int chas_name(LPSTR value) {
    events.push_back("chas"); return std::atoi(value);
}
int weap_name(LPSTR value) {
    events.push_back("weapon"); return std::atoi(value);
}
int arm_name(LPSTR value) {
    events.push_back("armor"); return std::atoi(value);
}
int tech_name(LPSTR value) {
    events.push_back("tech"); return std::atoi(value);
}

template <typename T> struct Tracked {
    T value;
    int prototype;
    const char *field;
    Tracked() : value(), prototype(-1), field("") {}
    void configure(int id, const char *name) {
        prototype = id;
        field = name;
    }
    Tracked &operator=(T next) {
        value = next;
        char buffer[64];
        sprintf(buffer, "store:%s:%d", field, prototype);
        events.push_back(buffer);
        return *this;
    }
    operator T() const { return value; }
};
struct VehPrototype {
    char veh_name[32];
    Tracked<int16_t> preq_tech;
    Tracked<uint8_t> plan;
    Tracked<uint8_t> cost;
    Tracked<uint8_t> carry_capacity;
    Tracked<int8_t> icon_offset;
    VehPrototype() { veh_name[0] = 0; }
    void configure(int id) {
        preq_tech.configure(id, "preq");
        plan.configure(id, "plan");
        cost.configure(id, "cost");
        carry_capacity.configure(id, "carry");
        icon_offset.configure(id, "icon");
        preq_tech.value = -99;
        plan.value = 0x55;
        cost.value = 0x55;
        carry_capacity.value = 0x55;
        icon_offset.value = 0x55;
    }
} VehPrototypes[MaxVehProtoFactionNum];

struct ProtoCall {
    int proto, chassis, weapon, armor, ability, reactor;
};
std::vector<ProtoCall> proto_calls;
void make_proto(int proto, int chassis, int weapon, int armor,
                int ability, int reactor) {
    ProtoCall call = { proto, chassis, weapon, armor, ability, reactor };
    proto_calls.push_back(call);
    events.push_back(std::string("make:") + number_text(proto));
}

"""


HARNESS_SUFFIX = r"""
void reset_fixture() {
    events.clear();
    proto_calls.clear();
    rows.clear();
    line_index = -1;
    item_index = 0;
    missing_units = false;
    for (int i = 0; i < MaxVehProtoFactionNum; ++i)
        VehPrototypes[i].configure(i);
}

std::string event_text(const char *prefix, const std::string &value) {
    return std::string(prefix) + ":" + value;
}
int plan_value(int id) {
    if (id == 0) return -1;
    if (id == 1) return -2;
    if (id == 2) return INT_MIN;
    if (id == 3) return 0;
    if (id == 4) return 258;
    return -1;
}
int cost_value(int id) { return id == 3 ? 257 : 0; }
int carry_value(int id) { return id == 3 ? 258 : 0; }
int icon_value(int id) { return id == 3 ? 257 : -2; }
int reactor_value(int id) {
    if (id == BSC_BATTLE_OGRE_MK2) return RECT_FUSION;
    if (id == BSC_BATTLE_OGRE_MK3) return RECT_QUANTUM;
    return RECT_FISSION;
}
void push_row(int id) {
    std::vector<std::string> row;
    row.push_back(std::string("unit") + number_text(id));
    row.push_back(number_text(100 + id));
    row.push_back(number_text(200 + id));
    row.push_back(number_text(300 + id));
    row.push_back(number_text(plan_value(id)));
    row.push_back(number_text(cost_value(id)));
    row.push_back(number_text(carry_value(id)));
    row.push_back(number_text(50000 + id));
    row.push_back(number_text(icon_value(id)));
    row.push_back(number_text(700 + id));
    rows.push_back(row);
}
void expect_token(std::vector<std::string> &expected, const char *kind,
                  const std::string &value) {
    expected.push_back(event_text(kind, value));
}
void check_full_parse_and_effects() {
    reset_fixture();
    rows.push_back(std::vector<std::string>(1, "25"));
    for (int id = 0; id < MaxVehProtoFactionNum; ++id) push_row(id);

    const BOOL result = read_units();
    check(result == 0);
    check(static_cast<int>(proto_calls.size()) == MaxVehProtoFactionNum);

    std::vector<std::string> expected;
    expected.push_back("open:UNITS");
    expected.push_back("get");
    expect_token(expected, "number", "25");
    for (int id = 0; id < MaxVehProtoFactionNum; ++id) {
        const std::string name = std::string("unit") + number_text(id);
        expected.push_back("get");
        expect_token(expected, "item", name);
        expect_token(expected, "item", number_text(100 + id));
        expected.push_back("chas");
        expect_token(expected, "item", number_text(200 + id));
        expected.push_back("weapon");
        expect_token(expected, "item", number_text(300 + id));
        expected.push_back("armor");
        expect_token(expected, "number", number_text(plan_value(id)));
        expect_token(expected, "number", number_text(cost_value(id)));
        expect_token(expected, "number", number_text(carry_value(id)));
        expect_token(expected, "item", number_text(50000 + id));
        expected.push_back("tech");
        expected.push_back(std::string("store:preq:") + number_text(id));
        expect_token(expected, "number", number_text(icon_value(id)));
        expect_token(expected, "binary", number_text(700 + id));
        expected.push_back(std::string("make:") + number_text(id));
        if (plan_value(id) >= 0)
            expected.push_back(std::string("store:plan:") + number_text(id));
        if (cost_value(id) != 0)
            expected.push_back(std::string("store:cost:") + number_text(id));
        if (carry_value(id) != 0)
            expected.push_back(std::string("store:carry:") + number_text(id));
        expected.push_back(std::string("store:icon:") + number_text(id));

        const ProtoCall &call = proto_calls[id];
        check(call.proto == id && call.chassis == 100 + id &&
              call.weapon == 200 + id && call.armor == 300 + id &&
              call.ability == 700 + id && call.reactor == reactor_value(id));
        check(strcmp(VehPrototypes[id].veh_name, name.c_str()) == 0);
        check(VehPrototypes[id].preq_tech.value ==
              static_cast<int16_t>(50000 + id));
        const uint8_t expected_plan = plan_value(id) >= 0
            ? static_cast<uint8_t>(plan_value(id)) : 0x55;
        const uint8_t expected_cost = cost_value(id) != 0
            ? static_cast<uint8_t>(cost_value(id)) : 0x55;
        const uint8_t expected_carry = carry_value(id) != 0
            ? static_cast<uint8_t>(carry_value(id)) : 0x55;
        check(VehPrototypes[id].plan.value == expected_plan);
        check(VehPrototypes[id].cost.value == expected_cost);
        check(VehPrototypes[id].carry_capacity.value == expected_carry);
        check(VehPrototypes[id].icon_offset.value ==
              static_cast<int8_t>(icon_value(id)));
    }
    check(events == expected);
    check(line_index == MaxVehProtoFactionNum);
    check(item_index == rows[line_index].size());
}

void check_missing_section_short_circuits() {
    reset_fixture();
    missing_units = true;
    rows.push_back(std::vector<std::string>(1, "25"));
    check(read_units() == 1);
    check(events.size() == 1 && events[0] == "open:UNITS");
    check(proto_calls.empty());
}

void check_empty_unit_count(int count) {
    reset_fixture();
    rows.push_back(std::vector<std::string>(1, number_text(count)));
    check(read_units() == 0);
    check(proto_calls.empty());
    check(events.size() == 3);
    check(events[0] == "open:UNITS");
    check(events[1] == "get");
    check(events[2] == event_text("number", number_text(count)));
    check(line_index == 0 && item_index == 1);
}

int main() {
    check_full_parse_and_effects();
    check_missing_section_short_circuits();
    check_empty_unit_count(0);
    check_empty_unit_count(-1);
    check_empty_unit_count(INT_MIN);
    return 0;
}
"""


def _mutants(body):
    extra_token = body.replace(
        "int ability = text_item_binary();",
        "int ability = text_item_binary();\n        (void)text_item_number();",
        1,
    )
    plan_sentinel = body.replace("if (plan >= 0)", "if (plan != -1)", 1)
    wrong_mk1 = body.replace(
        "case BSC_BATTLE_OGRE_MK1:\n            reactor_id = RECT_FISSION;",
        "case BSC_BATTLE_OGRE_MK1:\n            reactor_id = RECT_FUSION;",
        1,
    )
    plan_store = "VehPrototypes[proto_id].plan = (uint8_t)plan;"
    cost_store = "VehPrototypes[proto_id].cost = (uint8_t)cost;"
    store_order = body.replace(plan_store, "STORE_PLAN", 1)
    store_order = store_order.replace(cost_store, "STORE_COST", 1)
    store_order = store_order.replace("STORE_PLAN", cost_store, 1)
    store_order = store_order.replace("STORE_COST", plan_store, 1)
    missing_store = body.replace(plan_store, "(void)plan;", 1)
    assert all(candidate != body for candidate in
               (extra_token, plan_sentinel, wrong_mk1, store_order, missing_store)), [
                   name for name, candidate in (
                       ("extra", extra_token), ("sentinel", plan_sentinel),
                       ("mk1", wrong_mk1), ("order", store_order),
                       ("missing", missing_store)) if candidate == body]
    assert "int ability = text_item_binary();\n        (void)text_item_number();" in extra_token
    assert "if (plan != -1)" in plan_sentinel
    assert "reactor_id = RECT_FUSION;" in wrong_mk1
    return {
        "extra-reactor-token": extra_token,
        "plan-sentinel": plan_sentinel,
        "wrong-mk1-reactor": wrong_mk1,
        "reordered-stores": store_order,
        "missing-plan-store": missing_store,
    }


def _compile_run(compiler, body, tmp_path, name):
    source_path = tmp_path / (name + ".cpp")
    executable = tmp_path / name
    source_path.write_text(HARNESS_PREFIX + body + HARNESS_SUFFIX)
    compile_result = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(source_path), "-o", str(executable)],
        text=True, capture_output=True, timeout=30,
    )
    assert compile_result.returncode == 0, compile_result.stderr
    return subprocess.run(
        [str(executable)], text=True, capture_output=True, timeout=10,
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_read_units_body_and_compile_success_mutants(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    body = _extract_read_units()
    assert "int ability = text_item_binary();" in body
    assert "case BSC_BATTLE_OGRE_MK1:" in body
    assert "if (plan >= 0)" in body
    assert "int reactor_id = text_item_number()" not in body

    baseline = _compile_run(compiler, body, tmp_path, "actual-read-units")
    assert baseline.returncode == 0, baseline.stderr
    for name, mutant in _mutants(body).items():
        result = _compile_run(compiler, mutant, tmp_path, name)
        assert result.returncode != 0, name + " survived actual-source controls"

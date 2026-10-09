"""Actual-source host controls for the pointer-typed string core.

These controls exercise extracted production bodies with a 32-bit address
window model. They do not establish native matching or game acceptance.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

ROOT = Path(__file__).resolve().parents[2]


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    state = "code"
    i = brace
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "*": state = "block"; i += 2; continue
            if c == "/" and n == "/": state = "line"; i += 2; continue
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        elif state == "block":
            if c == "*" and n == "/": state = "code"; i += 2; continue
        elif state == "line":
            if c == "\n": state = "code"
        else:
            if c == "\\": i += 2; continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed actual body: {signature}")


def source_parts() -> tuple[str, str, str, str]:
    strings = (ROOT / "src/strings.cpp").read_text()
    alpha = (ROOT / "src/alpha.cpp").read_text()
    alpha_h = (ROOT / "src/alpha.h").read_text()
    general = (ROOT / "src/general.cpp").read_text()
    strings_h = (ROOT / "src/strings.h").read_text()
    assert "LPSTR get(LPSTR address);" in strings_h
    assert "LPSTR *strings_ptr;" in alpha_h
    assert "get(int address)" not in strings
    assert "StringTable.get((int)*((LPSTR *)Labels.strings_ptr" not in alpha_h
    return (
        extract_body(strings, "LPSTR Strings::get(LPSTR address)"),
        extract_body(alpha, "BOOL __cdecl labels_init()"),
        extract_body(alpha_h, "__forceinline LPSTR __cdecl label_get(int label_offset)"),
        extract_body(general, "int __cdecl parse_say(int id, LPSTR input, int gender, int pluralality)"),
    )


def terraform_loops() -> tuple[str, str]:
    source = (ROOT / "src/alpha.cpp").read_text()
    tf_header = (ROOT / "src/terraforming.h").read_text()
    veh_header = (ROOT / "src/veh.h").read_text()
    assert "LPSTR name;\n    LPSTR name_sea;\n    int preq_tech;\n    int preq_tech_sea;\n    uint32_t bit;\n    uint32_t bit_incompatible;" in tf_header
    assert "struct RulesOrder {\n    LPSTR order;\n    LPSTR order_sea;\n    LPSTR letter;\n};" in veh_header
    names = extract_body(source[source.index("// Land + sea terraforming"):],
                         "for (int j = 0; j < 2; j++)")
    orders = extract_body(source[source.index("// Land + sea orders"):],
                          "for (j = 0; j < 2; j++)")
    for field in ("name", "name_sea", "preq_tech", "preq_tech_sea",
                  "bit", "bit_incompatible", "TerraformingBits[i][j]"):
        assert field in names
    for field in ("Terraforming[i].name", "Terraforming[i].name_sea",
                  "Order[i + 4].order", "Order[i + 4].order_sea"):
        assert field in orders
    return names, orders


PRELUDE = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
#define __forceinline inline
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef void *LPVOID;
typedef int BOOL;
static const BOOL FALSE = 0;
static std::vector<std::string> events;
struct Heap {
  uint32_t base32 = 0;
  uint32_t size32 = 0;
  LPVOID base_raw = 0;
  LPVOID get_base() { return base_raw; }
  size_t get_base_size() { return size32; }
};
struct Strings : Heap {
  LPSTR get(LPSTR address);
};
struct Label { LPSTR *strings_ptr; int count; };
static Label Labels = {0, 0};
static Strings StringTable;
static struct { char str[128]; } ParseStrBuffer[10];
static int ParseStrGender[10], ParseStrPlurality[10];
static int GenderDefault = 7, PluralityDefault = 3;
static char label_text_storage[] = "zero\0one\0two\0guard\0";
static char *label_text_0 = label_text_storage;
static char *label_text_1 = label_text_storage + 5;
static char *label_text_2 = label_text_storage + 9;
static char *label_guard = label_text_storage + 13;
static char *label_texts[] = {label_text_0, label_text_1, label_text_2};
static int next_label_text;
static size_t requested_label_bytes;
static void *label_allocation;
static void labels_shutdown() { Labels.strings_ptr = 0; Labels.count = 0; }
static int text_open(const char *, const char *) { events.push_back("open"); return 0; }
static void text_get() { events.push_back("text_get"); }
static int text_item_number() { events.push_back("count"); return 3; }
static LPSTR text_string() { events.push_back("text_string"); return label_texts[next_label_text++]; }
static void text_close() { events.push_back("close"); }
static void *mem_get(size_t bytes) {
  requested_label_bytes = bytes;
  // Target labels use 4-byte slots. Allocate enough host pointer slots for
  // the target table plus a defined guard region that catches bad-stride
  // mutations without host OOB; this is not native allocation-extent evidence.
  const size_t target_slots = bytes / 4;
  const size_t host_guard_slots = target_slots * 4 + 1;
  LPSTR *slots = new LPSTR[host_guard_slots];
  for (size_t i = 0; i < host_guard_slots; ++i) slots[i] = label_guard;
  label_allocation = slots;
  return label_allocation;
}
'''


def cpp_source(parts: tuple[str, str, str, str], mutation: str | None) -> str:
    get_body, labels_body, label_get_body, parse_body = parts
    if mutation == "exclusive_end":
        get_body = get_body.replace("address32 <= end32", "address32 < end32", 1)
    elif mutation == "nonwrapping_end":
        get_body = get_body.replace(
            "const uint32_t end32 = base32 + static_cast<uint32_t>(get_base_size());",
            "const uint64_t end32 = static_cast<uint64_t>(base32) + get_base_size();", 1)
    elif mutation == "wrong_low_fallback":
        get_body = get_body.replace("return static_cast<LPSTR>(get_base());", "return address;", 1)
    elif mutation == "label_byte_stride":
        label_get_body = label_get_body.replace("strings_ptr[label_offset]", "strings_ptr[label_offset * 4]", 1)
    elif mutation == "parse_before_clear":
        parse_body = parse_body.replace(
            "ParseStrBuffer[id].str[0] = 0;\n    strcat(ParseStrBuffer[id].str, StringTable.get(input));",
            "strcat(ParseStrBuffer[id].str, StringTable.get(input));\n    ParseStrBuffer[id].str[0] = 0;", 1)
    return PRELUDE + "\n".join((get_body, labels_body, label_get_body, parse_body)) + r'''
int main() {
  // Ordinary range: low/equal base fallback, interior and inclusive end,
  // then one byte above the end.
  StringTable.base32 = 0x10000000u;
  StringTable.size32 = 0x20u;
  StringTable.base_raw = reinterpret_cast<LPVOID>(uintptr_t(0x10000000u));
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0x0FFFFFFFu))) ==
         reinterpret_cast<LPSTR>(uintptr_t(0x10000000u)));
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0x10000000u))) ==
         reinterpret_cast<LPSTR>(uintptr_t(0x10000000u)));
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0x10000001u))) ==
         reinterpret_cast<LPSTR>(uintptr_t(0x10000001u)));
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0x10000020u))) ==
         reinterpret_cast<LPSTR>(uintptr_t(0x10000020u)));
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0x10000021u))) == 0);

  // The original end addition is a target32 unsigned wrap, not a host-width
  // pointer addition. The high argument must fail against the wrapped end.
  StringTable.base32 = 0xFFFFFFF0u;
  StringTable.size32 = 0x30u;
  StringTable.base_raw = reinterpret_cast<LPVOID>(uintptr_t(0xFFFFFFF0u));
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0xFFFFFFF5u))) == 0);
  assert(StringTable.get(reinterpret_cast<LPSTR>(uintptr_t(0x00000010u))) ==
         reinterpret_cast<LPSTR>(uintptr_t(0xFFFFFFF0u)));

  // Extracted labels_init allocates a pointer table and fills 4-byte target
  // slots in order; label_get retains its integer element index.
  next_label_text = 0;
  events.clear();
  assert(labels_init() == FALSE);
  assert(Labels.count == 3 && requested_label_bytes == 12);
  assert(Labels.strings_ptr[0] == label_text_0);
  assert(Labels.strings_ptr[1] == label_text_1);
  assert(Labels.strings_ptr[2] == label_text_2);
  assert(events.size() == 7 && events[0] == "open" &&
         events[1] == "text_get" && events[2] == "count" &&
         events[3] == "text_string" && events[4] == "text_string" &&
         events[5] == "text_string" && events[6] == "close");
  StringTable.base32 = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(label_text_0));
  StringTable.size32 = 12;
  StringTable.base_raw = label_text_0;
  assert(label_get(2) == label_text_2);

  // parse_say keeps its int status, guard/default stores, clear-before-get,
  // and appends the pointer input without an integer compatibility path.
  StringTable.base32 = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(label_text_0));
  StringTable.size32 = 12;
  StringTable.base_raw = label_text_0;
  std::strcpy(ParseStrBuffer[0].str, "stale");
  events.clear();
  GenderDefault = 8; PluralityDefault = 4;
  assert(parse_say(0, label_text_1, -1, -1) == 0);
  assert(std::strcmp(ParseStrBuffer[0].str, "one") == 0);
  assert(ParseStrGender[0] == 8 && ParseStrPlurality[0] == 4);
  assert(parse_say(10, label_text_0, 1, 1) == 3);
  assert(std::strcmp(ParseStrBuffer[0].str, "one") == 0);
  puts("actual source Strings/Labels/parse_say controls passed");
}
'''


def terraform_cpp_source(loops: tuple[str, str], mutation: str | None) -> str:
    names, orders = loops
    if mutation == "sea_uses_land_name":
        orders = orders.replace(": Terraforming[i].name_sea",
                                ": Terraforming[i].name", 1)
    elif mutation == "sea_prereq_uses_land":
        names = names.replace("Terraforming[i].preq_tech_sea =",
                              "Terraforming[i].preq_tech =", 1)
    elif mutation == "sea_bit_uses_land":
        names = names.replace("Terraforming[i].bit_incompatible =",
                              "Terraforming[i].bit =", 1)
    elif mutation == "sea_order_uses_land":
        orders = orders.replace("else Order[i + 4].order_sea =",
                                "else Order[i + 4].order =", 1)
    return r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef char *LPSTR;
static std::vector<std::string> events;
struct RulesTerraforming {
  LPSTR name, name_sea;
  int preq_tech, preq_tech_sea;
  unsigned bit, bit_incompatible;
  int rate;
  LPSTR shortcuts;
};
struct RulesOrder { LPSTR order, order_sea, letter; };
static RulesTerraforming Terraforming[2];
static RulesOrder Order[6];
static unsigned TerraformingBits[2][2] = {{11, 12}, {21, 22}};
static char StringTemp[64];
static char stored_orders[][8] = {"stored0", "stored1", "stored2", "stored3"};
static int stored_count;
static struct { LPSTR put(LPSTR) { events.push_back("put"); return stored_orders[stored_count++]; } } StringTable;
static char rule_names[][8] = {"land0", "sea0", "land1", "sea1"};
static int rule_name_index;
static LPSTR text_item_string() {
  LPSTR result = rule_names[rule_name_index++];
  events.push_back(std::string("name:") + result);
  return result;
}
static int text_item() { events.push_back("item"); return 100 + rule_name_index; }
static int tech_name(int value) { events.push_back("tech"); return value + 1000; }
static void parse_string(LPSTR input, LPSTR output) {
  events.push_back(std::string("parse:") + input);
  std::strcpy(output, "parsed");
}
static int parse_say(int, LPSTR input, int, int) {
  events.push_back(std::string("say:") + input);
  return 0;
}
int main() {
  int i, j;
  char order_buf[16];
  for (i = 0; i < 2; ++i) {
    ''' + names + r'''
    Terraforming[i].rate = 30 + i;
    std::strcpy(order_buf, i == 0 ? "order0" : "order1");
    ''' + orders + r'''
  }
  assert(Terraforming[0].name == rule_names[0] && Terraforming[0].name_sea == rule_names[1]);
  assert(Terraforming[1].name == rule_names[2] && Terraforming[1].name_sea == rule_names[3]);
  assert(Terraforming[0].preq_tech == 1101 && Terraforming[0].preq_tech_sea == 1102);
  assert(Terraforming[1].preq_tech == 1103 && Terraforming[1].preq_tech_sea == 1104);
  assert(Terraforming[0].bit == 11 && Terraforming[0].bit_incompatible == 12);
  assert(Terraforming[1].bit == 21 && Terraforming[1].bit_incompatible == 22);
  assert(Order[4].order == stored_orders[0] && Order[4].order_sea == stored_orders[1]);
  assert(Order[5].order == stored_orders[2] && Order[5].order_sea == stored_orders[3]);
  assert(events.size() == 24);
  assert(events[0] == "name:land0" && events[1] == "item" && events[2] == "tech" &&
         events[3] == "name:sea0" && events[4] == "item" && events[5] == "tech" &&
         events[6] == "say:land0" && events[7] == "parse:order0" && events[8] == "put" &&
         events[9] == "say:sea0" && events[10] == "parse:order0" && events[11] == "put" &&
         events[12] == "name:land1" && events[13] == "item" && events[14] == "tech" &&
         events[15] == "name:sea1" && events[16] == "item" && events[17] == "tech" &&
         events[18] == "say:land1" && events[19] == "parse:order1" && events[20] == "put" &&
         events[21] == "say:sea1" && events[22] == "parse:order1" && events[23] == "put");
  puts("actual Terraforming/Order member-row controls passed");
}
'''


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler available")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_core_bodies_and_compiling_mutants(compiler: str) -> None:
    parts = source_parts()
    for mutation in (None, "exclusive_end", "nonwrapping_end", "wrong_low_fallback",
                     "label_byte_stride", "parse_before_clear"):
        source = cpp_source(parts, mutation)
        with tempfile.TemporaryDirectory(prefix="strings-core-host-") as tmp:
            binary = Path(tmp) / "controls"
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Werror"] +
                (["-fsanitize=address", "-fno-omit-frame-pointer"]
                 if mutation == "label_byte_stride" else []) +
                ["-x", "c++", "-", "-o", str(binary)],
                input=source, text=True, capture_output=True, timeout=30,
            )
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
            if mutation is None:
                assert ran.returncode == 0, ran.stderr
                assert "actual source Strings/Labels/parse_say controls passed" in ran.stdout
            else:
                assert ran.returncode != 0, f"mutant survived: {mutation}"
                if mutation == "label_byte_stride":
                    assert "AddressSanitizer" not in ran.stderr, ran.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_terraforming_pointer_member_selection_mutant(compiler: str) -> None:
    loops = terraform_loops()
    for mutation in (None, "sea_uses_land_name", "sea_prereq_uses_land",
                     "sea_bit_uses_land", "sea_order_uses_land"):
        source = terraform_cpp_source(loops, mutation)
        with tempfile.TemporaryDirectory(prefix="terraform-name-host-") as tmp:
            binary = Path(tmp) / "controls"
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(binary)],
                input=source, text=True, capture_output=True, timeout=30,
            )
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
            if mutation is None:
                assert ran.returncode == 0, ran.stderr
            else:
                assert ran.returncode != 0, "sea-name mutant survived"

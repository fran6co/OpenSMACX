"""Actual-source controls for source-owned say helpers.

The fixture extracts installed bodies and applies a bounded pointer-to-ID host
width shim. Host tests do not establish native matching or game acceptance.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
ADDRESSES = (0x00590910, 0x00590AA0, 0x00590A10)
SIGNATURES = (
    "void __cdecl say_facility(char *text, int facility_id, int with_cost)",
    "void __cdecl say_ability(char *text, int ability_id, int with_detail)",
    "void __cdecl say_society(char *text, int society_id, int with_model)",
)


def installed_strings() -> str:
    text = (ROOT / "src/strings.cpp").read_text()
    rows = {row.address: row for row in read_text(text, ROOT / "src/strings.cpp")}
    assert set(ADDRESSES).issubset(rows)
    assert all(rows[address].source_role.value != "scaffold" for address in ADDRESSES)
    return text


def extract_body(text: str, signature: str) -> str:
    start = text.index(signature)
    open_brace = text.index("{", start)
    depth = 0
    state = "code"
    i = open_brace
    while i < len(text):
        char = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"; i += 2; continue
            if char == "/" and nxt == "/":
                state = "line"; i += 2; continue
            if char == '"': state = "string"
            elif char == "'": state = "char"
            elif char == "{": depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        elif state == "block":
            if char == "*" and nxt == "/": state = "code"; i += 2; continue
        elif state == "line":
            if char == "\n": state = "code"
        else:
            if char == "\\": i += 2; continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unclosed actual body: {signature}")


PRELUDE = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef char *LPSTR;
static std::vector<std::string> events;
struct Strings {
  char *get(char *text) { events.push_back(std::string("get:") + text); return text; }
};
struct Named { char *name; };
struct Social { char *type; };
static char facility_name[] = "facility";
static char ability_name[] = "ability";
static char society_name[] = "society";
static char facility_detail[] = "facility-detail";
static char ability_detail[] = "ability-detail";
static char society_detail[] = "society-detail";
static Named Facility[4];
static Named Ability[4];
static Social SocialCategories[4];
static Strings StringTable;
static char StringTemp[128];
static int last_label = -1;
static char *label_get(int index) {
  events.push_back("label:" + std::to_string(index));
  last_label = index;
  return StringTable.get(index == 0x9B ? facility_detail :
                         index == 0xA4 ? ability_detail : society_detail);
}
'''


def cpp_source(projected: str, mutation: str | None = None) -> str:
    bodies = [extract_body(projected, signature) for signature in SIGNATURES]
    assert all("reinterpret_cast<int>" not in body for body in bodies)
    if mutation == "wrong_label_index":
        bodies[0] = bodies[0].replace("label_get(0x9B)", "label_get(0x9C)", 1)
    elif mutation == "append_detail_to_text":
        bodies[1] = bodies[1].replace("strcat(StringTemp, detail);",
                                      "strcat(static_cast<char *>(text), detail);", 1)
    elif mutation == "drop_space_literal":
        before = 'strcat(text, " ");'
        bodies[2] = bodies[2].replace(before, 'strcat(text, "");', 1)
        assert before not in bodies[2]
    main = r'''
int main() {
  char text[128];
  Facility[1].name = facility_name;
  Ability[2].name = ability_name;
  SocialCategories[3].type = society_name;

  std::strcpy(text, "prefix"); std::strcpy(StringTemp, ""); events.clear();
  say_facility(text, 1, 2);
  assert(std::strcmp(text, "prefixfacility ()") == 0);
  assert(std::strcmp(StringTemp, "facility-detail") == 0);
  assert(last_label == 0x9B);
  assert(events.size() == 3 && events[0] == "get:facility" &&
         events[1] == "label:155" && events[2] == "get:facility-detail");

  std::strcpy(text, ""); std::strcpy(StringTemp, "seed"); events.clear();
  say_ability(text, 2, -1);
  assert(std::strcmp(text, "ability ()") == 0);
  assert(std::strcmp(StringTemp, "seedability-detail") == 0);
  assert(last_label == 0xA4);
  assert(events.size() == 3 && events[0] == "get:ability" &&
         events[1] == "label:164" && events[2] == "get:ability-detail");

  std::strcpy(text, ""); std::strcpy(StringTemp, ""); events.clear();
  say_society(text, 3, 1);
  assert(std::strcmp(text, "society ()") == 0);
  assert(std::strcmp(StringTemp, "society-detail") == 0);
  assert(last_label == 0x45);
  assert(events.size() == 3 && events[0] == "get:society" &&
         events[1] == "label:69" && events[2] == "get:society-detail");

  std::strcpy(text, "no-detail"); std::strcpy(StringTemp, "unchanged"); events.clear();
  say_ability(text, 2, 0);
  assert(std::strcmp(text, "no-detailability") == 0);
  assert(std::strcmp(StringTemp, "unchanged") == 0);
  assert(events.size() == 1 && events[0] == "get:ability");
  puts("actual projected say_* controls passed");
}
'''
    return PRELUDE + "\n".join(bodies) + main


COMPILERS = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no host C++ compiler available")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_projected_say_bodies_and_behavioral_mutants(compiler: str) -> None:
    projected = installed_strings()
    for mutation in (None, "wrong_label_index", "append_detail_to_text", "drop_space_literal"):
        source = cpp_source(projected, mutation)
        with tempfile.TemporaryDirectory(prefix="say-owner-host-") as temp_name:
            binary = Path(temp_name) / "control"
            built = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Werror", "-x", "c++", "-", "-o", str(binary)],
                input=source, text=True, capture_output=True, timeout=30,
            )
            assert built.returncode == 0, built.stderr
            ran = subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)
            if mutation is None:
                assert ran.returncode == 0, ran.stderr
                assert "actual projected say_* controls passed" in ran.stdout
            else:
                assert ran.returncode != 0, f"mutant survived: {mutation}"

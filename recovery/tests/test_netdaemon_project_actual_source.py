"""Actual-source host control for the NetDaemon 0x4D00 route."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from match_recovery.source import read_file


def repository_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/netdaemon.cpp").is_file():
            return candidate
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()
SOURCE = ROOT / "src/netdaemon.cpp"
HELPER_NAME = "netdaemon_process_project_4d00"
HOST_PREAMBLE = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef int BOOL;
struct RulesFacility { LPSTR name; };
static const int FacilitySPStart = 70;
static RulesFacility Facility[134];
struct Player {
    int is_leader_female;
    LPCSTR title_leader;
    LPCSTR name_leader;
    LPCSTR adj_name_faction;
    LPCSTR noun_faction;
    int noun_gender;
    BOOL is_noun_plural;
};
static Player Players[8];
static char facility_names[134][32];
struct Strings {
    LPSTR get(LPSTR text) { return text; }
};
static Strings StringTable;
struct Base { int faction_id_current; };
static Base Bases[64];
static int SecretProject[64];
static int base_project(int project) { return SecretProject[project]; }
struct Console { int faction_id_; };
static Console ConsoleGlobal;
static int GenderDefault;
static int PluralityDefault;
static std::vector<std::string> events;
struct NetMsg {
    void pop(const char *label, int duration, int channel, const char *target) {
        events.push_back(std::string("pop:") + label + ":" +
                         std::to_string(duration) + ":" +
                         std::to_string(channel) + ":" +
                         (target == nullptr ? "null" : target));
    }
};
static NetMsg NetMsgPrimary;
static void log_say(LPCSTR label, LPCSTR value, int a, int b, int c) {
    events.push_back(std::string("log:") + label + ":" + value + ":" +
                     std::to_string(a) + ":" + std::to_string(b) + ":" +
                     std::to_string(c));
}
static void parse_says(int id, LPCSTR text, int gender, int plural) {
    events.push_back(std::string("says:") + std::to_string(id) + ":" +
                     text + ":" +
                     std::to_string(gender < 0 ? GenderDefault : gender) + ":" +
                     std::to_string(plural < 0 ? PluralityDefault : plural));
}
static int parse_say(int id, LPSTR text, int gender, int plural) {
    events.push_back(std::string("say:") + std::to_string(id) + ":" +
                     text + ":" +
                     std::to_string(gender < 0 ? GenderDefault : gender) + ":" +
                     std::to_string(plural < 0 ? PluralityDefault : plural));
    return 1;
}
"""

HOST_MAIN = r"""
static void setup(int local, int sender, int status, int project, int related,
                  int base_id = -1, int base_owner = 0) {
    events.clear();
    ConsoleGlobal.faction_id_ = local;
    GenderDefault = 91;
    PluralityDefault = 92;
    Players[sender] = {3, "title", "leader", "adj-name-faction",
                       "noun-faction", 4, 1};
    for (int i = 0; i < 64; ++i) {
        std::snprintf(facility_names[FacilitySPStart + i],
                      sizeof(facility_names[0]), "project_%d", i);
        Facility[FacilitySPStart + i].name =
            facility_names[FacilitySPStart + i];
        SecretProject[i] = -1;
        Bases[i].faction_id_current = 0;
    }
    if (base_id >= 0) {
        SecretProject[project] = base_id;
        Bases[base_id].faction_id_current = base_owner;
    }
    unsigned char packet[0x1C] = {};
    *reinterpret_cast<int *>(packet + 0x04) = sender;
    *reinterpret_cast<int *>(packet + 0x10) = project;
    *reinterpret_cast<int *>(packet + 0x14) = status;
    *reinterpret_cast<int *>(packet + 0x18) = related;
    netdaemon_process_project_4d00(packet);
}

int main() {
    setup(7, 2, 0, 3, 5);
    assert((events == std::vector<std::string>{
        "log:Received PROJECT:project_3:2:0:5",
        "says:0:title:3:0", "says:1:leader:3:0",
        "says:2:noun-faction:4:1", "say:3:project_3:4:1",
        "pop:BEGINPROJECT:5000:2:null"}));

    setup(7, 2, 1, 3, 5, 9, 2);
    assert(events.size() == 5 && events.back() == "say:3:project_3:4:1");
    setup(7, 2, 1, 3, 5, 9, 4);
    assert(events.back() == "pop:HALTPROJECT:5000:2:null");

    setup(7, 2, 2, 3, 5);
    assert((events == std::vector<std::string>{
        "log:Received PROJECT:project_3:2:2:5",
        "says:0:adj-name-faction:91:92", "say:1:project_5:91:92",
        "says:2:noun-faction:4:1", "say:3:project_3:4:1",
        "pop:CHANGEPROJECT:5000:2:null"}));
    setup(7, 2, -1, 3, 5);
    assert(events.size() == 2 && events.back() == "pop:HALTPROJECT:5000:2:null");
    setup(7, 2, 3, 3, 5);
    assert(events.size() == 1);
    setup(2, 2, -1, 3, 5);
    assert(events.size() == 1); // log precedes packet-local faction exit
    std::puts("actual-source 0x4D00 helper controls passed");
}
"""


def extract_function(source, name):
    match = re.search(
        r"(?:static\s+)?void\s+(?:NetDaemon::)?" +
        re.escape(name) + r"\s*\(", source
    )
    assert match, f"missing {name} in tracked src/netdaemon.cpp"
    opening = source.index("{", match.end())
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and next_char == "/":
                block_comment = False
                index += 1
        elif quote is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and next_char == "/":
            line_comment = True
            index += 1
        elif char == "/" and next_char == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():index + 1]
        index += 1
    raise AssertionError(f"unterminated body for {name}")


def source_parts():
    text = SOURCE.read_text()
    helper = extract_function(text, HELPER_NAME)
    process = extract_function(text, "process_message")
    return text, helper, process


def compile_and_run(helper, compiler, directory, label):
    source_path = directory / f"{label}.cpp"
    binary = directory / label
    source_path.write_text(HOST_PREAMBLE + "\n" + helper + "\n" + HOST_MAIN)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(binary)],
        check=True, capture_output=True, text=True,
    )
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_actual_source_annotation_and_4d00_route_are_pinned():
    text, helper, process = source_parts()
    rows = [row for row in read_file(SOURCE) if row.address == 0x00534400]
    assert len(rows) == 1
    assert rows[0].image_spans == (
        (0x00534400, 0x005388B9), (0x0065E0C2, 0x0065E442)
    )
    assert "// ORIGINAL: 0x00534400" in text
    assert "The cold span is the same frame's compiler-generated exception cleanup" in text
    assert (
        "// TRIED: no Ghidra hypothesis was available for this one (raw asm only). "
        "The real dispatcher spans ~67 distinct message-type case bodies across "
        "several chained range checks up to type 0x1300+; only the first band "
        "(0xF05-0xF0F, 11 cases) and the shared default \"fixup_message + "
        "send_message\" forwarding path (the target of the large majority of "
        "jump-table slots) were read from the disassembly and modelled. "
        "Everything above type 0x1101 is unimplemented."
    ) in text
    assert helper.startswith("static void netdaemon_process_project_4d00")
    assert re.search(
        r"if\s*\(type\s*==\s*0x4D00\)\s*\{\s*"
        r"netdaemon_process_project_4d00\(msg\);\s*"
        r"netdaemon_process_message_marker_tail\(\*this,\s*msg\);\s*return;",
        process,
    )
    assert "netdaemon_process_message_marker_tail(*this, msg);" in process


def test_actual_extracted_helper_compiles_and_runs_with_gcc_and_clang():
    _, helper, _ = source_parts()
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "both GCC and Clang are required for this host control"
    with tempfile.TemporaryDirectory(prefix="osmx-4d00-actual-") as temporary:
        directory = Path(temporary)
        for index, compiler in enumerate(compilers):
            result = compile_and_run(helper, compiler, directory, f"native_source_{index}")
            assert result.returncode == 0, result.stderr + result.stdout
            assert "actual-source 0x4D00 helper controls passed" in result.stdout


def test_actual_source_damage_controls_fail():
    _, helper, _ = source_parts()
    mutants = {
        "wrong_packet_identity": (
            "message + 0x04", "message + 0x08"),
        "wrong_related_project_offset": (
            "message + 0x18", "message + 0x14"),
        "wrong_status_gate": (
            "if (project_status > 2)", "if (project_status >= 2)"),
        "status_two_defaults_no_longer_inherit": (
            "parse_says(0, faction.adj_name_faction, -1, -1);",
            "GenderDefault = faction.noun_gender;\n"
            "        PluralityDefault = faction.is_noun_plural;\n"
            "        parse_says(0, faction.adj_name_faction, -1, -1);"),
        "local_exit_precedes_log": (
            "    log_say(\"Received PROJECT\", project_name, faction_id, project_status,\n"
            "            related_project_id);\n\n"
            "    if (faction_id == ConsoleGlobal.faction_id_) {\n"
            "        return;\n"
            "    }",
            "    if (faction_id == ConsoleGlobal.faction_id_) {\n"
            "        return;\n"
            "    }\n"
            "    log_say(\"Received PROJECT\", project_name, faction_id, project_status,\n"
            "            related_project_id);"),
        "wrong_parse_order": (
            "parse_says(2, faction.noun_faction, -1, -1);",
            "parse_says(1, faction.noun_faction, -1, -1);", 2),
    }
    compiler = shutil.which("g++")
    assert compiler
    with tempfile.TemporaryDirectory(prefix="osmx-4d00-mutants-") as temporary:
        directory = Path(temporary)
        for label, mutation in mutants.items():
            before, after = mutation[:2]
            count = mutation[2] if len(mutation) == 3 else 1
            assert helper.count(before) == count, label
            mutant = helper.replace(before, after, count)
            result = compile_and_run(mutant, compiler, directory, label)
            assert result.returncode != 0, f"damage control escaped: {label}"

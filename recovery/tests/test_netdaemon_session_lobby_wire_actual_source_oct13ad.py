"""Bounded wire-view and task-base controls for NetWin session/lobby packets.

These host fixtures exercise source-derived borrowed views and the mutable
fill_player path; they do not establish native ABI or full game-runtime proof.
"""
from dataclasses import fields
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.tasks import TaskStore
from test_net_hotseat_owner_actual_source_oct6 import (
    extract_definition, installed_packet_declarations,
)


ROOT = Path(__file__).resolve().parents[2]
BASE = "d8b1fa5792c3bc98771aac07f097b42c123e3b4d"
TASK = "netwin-session-lobby-wire-closure-oct13ad"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SOURCES = ("src/netwin.h", "src/netwin.cpp", "src/netdaemon.cpp")


def _cpp_tokens(source: str) -> list[str]:
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|.', re.S)
    result = []
    for match in token.finditer(source):
        value = match.group(0)
        if value.startswith("//") or value.startswith("/*") or value.isspace():
            continue
        result.append(value)
    return result


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def _git_source(path: str) -> str:
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT,
                                  text=True, timeout=30)


def _source_rows(path: str, text: str):
    return {row.address: row for row in read_text(text, ROOT / path)}


def _replace_once(source: str, before: str, after: str, label: str) -> str:
    assert source.count(before) == 1, (label, source.count(before), before[:100])
    return source.replace(before, after, 1)


def _transform_function(source: str, signature: str, replacements, label: str) -> str:
    body = extract_definition(source, signature)
    original = body
    for replacement in replacements:
        before, after = replacement[:2]
        expected_count = replacement[2] if len(replacement) == 3 else 1
        assert body.count(before) == expected_count, (label, before, body.count(before))
        body = body.replace(before, after)
    return source.replace(original, body, 1)


def _remove_decl(source: str, kind: str, name: str) -> str:
    declaration = extract_definition(source, f"{kind} {name} {{")
    if name == "NetWinLobbyPacketHeader":
        assert _cpp_tokens(declaration) == _cpp_tokens(LOBBY_HEADER_SCHEMA), (
            "NetWinLobbyPacketHeader declaration schema", _cpp_tokens(declaration))
    full = declaration + ";"
    assert source.count(full) == 1, (kind, name)
    return source.replace(full, "", 1)


def _remove_added_region(source: str, start: str, end: str, declarations) -> str:
    first = source.find(start)
    last = source.find(end, first)
    assert first >= 0 and last > first, (start, end)
    region = source[first:last]
    for kind, name in declarations:
        region = _remove_decl(region, kind, name)
    assertions = re.findall(r"static_assert\s*\(.*?\);", region, re.S)
    expected = REGION_ASSERTIONS[start]
    assert [_cpp_tokens(item) for item in assertions] == [
        _cpp_tokens(item) for item in expected
    ], (start, "static_assert schema", [_cpp_tokens(item) for item in assertions])
    for assertion in assertions:
        region = region.replace(assertion, "", 1)
    assert not _cpp_tokens(region), (start, _cpp_tokens(region)[:25])
    return source[:first] + source[last:]


def _insert_historical_region(current: str, baseline: str, start: str,
                              end: str, anchor: str, label: str) -> str:
    first = baseline.find(start)
    last = baseline.find(end, first)
    assert first >= 0 and last > first, (label, "baseline anchors")
    region = baseline[first:last]
    assert current.count(anchor) == 1, (label, current.count(anchor))
    return current.replace(anchor, region + anchor, 1)


def _restore_netdaemon(current: str, baseline: str) -> str:
    current = _transform_function(current, "static void netdaemon_rebroadcast_session_packet(", [
        ("    const NetWinPacketView packet(message);\n    unsigned short type = packet.opcode();",
         "    unsigned short type = *reinterpret_cast<unsigned short *>(message);"),
        ("packet.set_opcode(type);", "*reinterpret_cast<unsigned short *>(message) = type;")],
        "rebroadcast")
    current = _transform_function(current, "static void netdaemon_handle_session_options(", [
        ("    const NetWinPacketView packet(message);\n    NetWinSessionOptions.receive_wire_block(\n        packet.payload());",
         "    NetWinSessionOptions.receive_wire_block(\n        reinterpret_cast<const unsigned char *>(message + 0x10));"),
        ("packet.opcode()", "*reinterpret_cast<unsigned short *>(message)"),
        ("packet.set_opcode(type)", "*reinterpret_cast<unsigned short *>(message) = type")],
        "options")
    current = _transform_function(current, "static void netdaemon_handle_session_block(", [
        ("    const NetWinPacketView packet(message);\n    NetWinSessionData.receive_wire_block(\n        packet.payload());",
         "    NetWinSessionData.receive_wire_block(\n        reinterpret_cast<const unsigned char *>(message + 0x10));"),
        ("packet.opcode()", "*reinterpret_cast<unsigned short *>(message)"),
        ("packet.set_opcode(type)", "*reinterpret_cast<unsigned short *>(message) = type")],
        "session block")
    current = _transform_function(current, "static void netdaemon_handle_session_status(", [
        ("    const NetWinSessionStatusView status(message);\n", ""),
        ("status.faction()", "*reinterpret_cast<int *>(message + 4)"),
        ("status.identity()", "static_cast<int8_t>(message[0x18])"),
        ("status.field_14()", "static_cast<uint8_t>(message[0x14])"),
        ("status.field_10()", "static_cast<uint8_t>(message[0x10])")], "status")
    current = _transform_function(current, "static void netdaemon_handle_session_lobby(", [
        ("    const NetWinPacketView packet(message);\n    const NetWinLobbyPacketView lobby(message);\n", ""),
        ("const unsigned short wire_type = packet.opcode();", ""),
        ("const uint32_t sender = packet.selector();",
         "unsigned short &wire_type = *reinterpret_cast<unsigned short *>(message);\n    const uint32_t sender = *reinterpret_cast<uint32_t *>(message + 4);"),
        ("packet.set_opcode(static_cast<unsigned short>(\n            (wire_type & static_cast<unsigned short>(~0x2000u)) | 0x4000u));",
         "wire_type = static_cast<unsigned short>(\n            (wire_type & static_cast<unsigned short>(~0x2000u)) | 0x4000u);"),
        ("const NetWinFactionRecord *const records =\n                    NetWinDialog.faction_records_;",
         "const NetWinFactionRecord *const records =\n                    static_cast<const NetWinFactionRecord *>(\n                        NetWinDialog.faction_records_);"),
        ("lobby.faction_or_color()", "static_cast<int8_t>(message[0x13])", 2),
        ("lobby.owner()", "*reinterpret_cast<uint32_t *>(message + 0x188)", 2),
        ("lobby.payload()", "message + 0x10", 3),
        ("NetWinLobbyRowView(message + 0x10)",
         "reinterpret_cast<NetWinLobbyRow *>(message + 0x10)")], "lobby")
    got, wanted = _cpp_tokens(current), _cpp_tokens(baseline)
    mismatch = next((i for i, pair in enumerate(zip(got, wanted)) if pair[0] != pair[1]),
                    min(len(got), len(wanted)))
    assert got == wanted, ("netdaemon.cpp source recipe", mismatch,
                           "got=" + " ".join(got[mismatch - 12:mismatch + 20]),
                           "wanted=" + " ".join(wanted[mismatch - 12:mismatch + 20]))
    return current


def _restore_netwin_cpp(current: str, baseline: str) -> str:
    current = _insert_historical_region(current, baseline,
        "// The single-row picker packet shares the lobby button handler's wire header.",
        "// The shared tail of the eight scalar pickers runs even on cancellation:",
        "// The shared tail of the eight scalar pickers runs even on cancellation:",
        "source packet declarations")
    current = _replace_once(current, "NetWinLobbyFullPacket assigned_packet;",
        "struct PickMapLobbyFullPacket {\n        NetWinLobbyPacketHeader header_;\n        NetWinLobbyRow rows_[7];\n    };\n    struct PickMapLobbyLeavePacket {\n        NetWinLobbyPacketHeader header_;\n        uint8_t trailing_[0x10];\n    };\n    PickMapLobbyFullPacket assigned_packet;",
        "pick_map full/leave local records")
    current = _replace_once(current, "NetWinSessionLobbyPacket session_packet;",
        "struct NetWinSessionPacket {\n        NetWinLobbyPacketHeader header_;\n        NetWinSessionBlock session_;\n    } session_packet;", "pick_map session packet")
    current = _replace_once(current, "NetWinLobbyLeavePacket leave_packet;",
        "PickMapLobbyLeavePacket leave_packet;", "pick_map leave packet")
    current = _insert_historical_region(current, baseline,
        "struct NetWinLobbyFullPacket {", "/*\nPurpose: Answer the multiplayer setup dialog's button click.",
        "/*\nPurpose: Answer the multiplayer setup dialog's button click.",
        "full and leave packet declarations")
    current = _insert_historical_region(current, baseline,
        "// The original sender copies 90 dwords after a 16-byte header.",
        "/*\nPurpose: Broadcast the session state blob:",
        "/*\nPurpose: Broadcast the session state blob:",
        "session packet declaration")

    current = _replace_once(current,
        "void NetWin::fill_player(NetWinLobbyRowView lobby_row, int slot) {",
        "void NetWin::fill_player(NetWinLobbyRow *lobby_row, int slot) {",
        "fill_player signature")
    for field in ("noun_faction", "leader_name", "formal_faction_name",
                  "adjective_faction", "leader_title", "adjective_leader"):
        current = current.replace(f"lobby_row.{field}()", f"lobby_row->{field}_")
    current = _replace_once(current, "lobby_row.set_tail_0(", "lobby_row->tail_[0] = ",
                            "fill_player tail store")
    current = _replace_once(current,
        "lobby_row->tail_[0] = allocation_bytes[slot * 0x190 + 0x192]);",
        "lobby_row->tail_[0] = allocation_bytes[slot * 0x190 + 0x192];",
        "restore tail expression")
    assert _cpp_tokens(current) == _cpp_tokens(baseline), "netwin.cpp source recipe"
    return current


WIRE_DECLARATIONS = (
    ("class", "NetWinLobbyRowView"), ("struct", "NetWinLobbyPacketHeader"),
    ("struct", "NetWinLobbySinglePacket"), ("struct", "NetWinLobbyFullPacket"),
    ("struct", "NetWinLobbyLeavePacket"), ("class", "NetWinPacketView"),
    ("class", "NetWinLobbyPacketView"), ("class", "NetWinSessionStatusView"),
    ("struct", "NetWinSessionBlockPacket"), ("struct", "NetWinSessionLobbyPacket"),
    ("struct", "NetWinSessionOptionsPacket"),
)

FIRST_REGION_ASSERTIONS = (
    'static_assert(sizeof(NetWinLobbyRowView) == sizeof(void *), "lobby row view is a borrowed pointer");',
    'static_assert(sizeof(NetWinLobbyPacketHeader) == 0x10, "NetWin packet header size");',
    'static_assert(offsetof(NetWinLobbyPacketHeader, slot_) == 0x04, "NetWin packet selector offset");',
    'static_assert(offsetof(NetWinLobbyPacketHeader, field_8_) == 0x08, "NetWin packet field-eight offset");',
    'static_assert(offsetof(NetWinLobbyPacketHeader, field_C_) == 0x0C, "NetWin packet field-c offset");',
    'static_assert(sizeof(NetWinLobbySinglePacket) == 0x18C, "single lobby packet size");',
    'static_assert(offsetof(NetWinLobbySinglePacket, row_) == 0x10, "single lobby payload offset");',
    'static_assert(sizeof(NetWinLobbyFullPacket) == 0xA74, "full lobby packet size");',
    'static_assert(offsetof(NetWinLobbyFullPacket, rows_) == 0x10, "full lobby payload offset");',
    'static_assert(sizeof(NetWinLobbyLeavePacket) == 0x20, "leave lobby packet size");',
    'static_assert(sizeof(NetWinPacketView) == sizeof(void *), "packet view is a borrowed pointer");',
    'static_assert(sizeof(NetWinLobbyPacketView) == sizeof(void *), "lobby packet view is a borrowed pointer");',
    'static_assert(sizeof(NetWinSessionStatusView) == sizeof(void *), "session status view is a borrowed pointer");',
)
SESSION_REGION_ASSERTIONS = (
    'static_assert(sizeof(NetWinSessionBlockPacket) == 0x178, "session block packet size");',
    'static_assert(offsetof(NetWinSessionBlockPacket, session_) == 0x10, "session block payload offset");',
    'static_assert(sizeof(NetWinSessionLobbyPacket) == 0x178, "lobby session packet size");',
    'static_assert(offsetof(NetWinSessionLobbyPacket, session_) == 0x10, "lobby session payload offset");',
)
OPTIONS_REGION_ASSERTIONS = (
    'static_assert(sizeof(NetWinSessionOptionsPacket) == 0x28, "session options packet size");',
    'static_assert(offsetof(NetWinSessionOptionsPacket, options_) == 0x10, "session options payload offset");',
)
REGION_ASSERTIONS = {
    "class NetWinLobbyRowView {": FIRST_REGION_ASSERTIONS,
    "// send_session2_packet copies 90 dwords after a 16-byte prefix.": SESSION_REGION_ASSERTIONS,
    "// The pickers send the full 0x18-byte option state after a 16-byte prefix.": OPTIONS_REGION_ASSERTIONS,
}
LOBBY_HEADER_SCHEMA = (
    "struct NetWinLobbyPacketHeader { uint8_t opcode_; uint8_t flags_; uint16_t field_2_; "
    "int32_t slot_; int32_t field_8_; uint32_t field_C_; }"
)


def _restore_netwin_header(current: str, baseline: str) -> str:
    current = current.replace("#include <cstring>\n", "", 1)
    current = _remove_added_region(current, "class NetWinLobbyRowView {",
        "// Only NetWin::exec constructs these stack-local rows.", WIRE_DECLARATIONS[:8])
    current = _remove_added_region(current, "// send_session2_packet copies 90 dwords after a 16-byte prefix.",
        "struct NetWinSessionOptionsState {", WIRE_DECLARATIONS[8:10])
    current = _remove_added_region(current, "// The pickers send the full 0x18-byte option state after a 16-byte prefix.",
        "// Setup synchronization shared with NetDaemon's added-player callback.", WIRE_DECLARATIONS[10:])
    current = _replace_once(current,
        "void fill_player(NetWinLobbyRowView lobby_row, int slot);",
        "void fill_player(NetWinLobbyRow *lobby_row, int slot);", "fill_player declaration")
    assert _cpp_tokens(current) == _cpp_tokens(baseline), "netwin.h source recipe"
    return current


VIEW_HARNESS = r'''#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string.h>
@@WIRE_TYPES@@
static void require(bool condition, const char *name) {
    if (!condition) { std::fprintf(stderr, "wire-view: %s\n", name); std::exit(98); }
}
static unsigned label_calls;
static char *active_noun;
static const char *label_get(int) {
    ++label_calls;
    require(active_noun != 0 && active_noun[0] == 0,
            "slot-minus-one-clears-before-label");
    return "fallback";
}
class NetWin {
public:
    NetWinFactionRecord *faction_records_;
    void fill_player(NetWinLobbyRowView lobby_row, int slot);
};
@@FILL_PLAYER@@
int main() {
    unsigned char storage[0x1A0] = {};
    unsigned char *packet = storage + 1;
    NetWinLobbyRowView row(packet + sizeof(NetWinLobbyPacketHeader));
    require(sizeof(NetWinLobbyRowView) == sizeof(void *), "row-view-pointer-sized");
    require(row.leader_name() == reinterpret_cast<char *>(packet + 0x15), "leader-offset");
    require(row.formal_faction_name() == reinterpret_cast<char *>(packet + 0x2D), "formal-offset");
    require(row.noun_faction() == reinterpret_cast<char *>(packet + 0x55), "noun-offset");
    require(row.adjective_faction() == reinterpret_cast<char *>(packet + 0x6D), "adjective-offset");
    require(row.leader_title() == reinterpret_cast<char *>(packet + 0xED), "title-offset");
    require(row.adjective_leader() == reinterpret_cast<char *>(packet + 0x105), "leader-adjective-offset");
    std::strcpy(row.noun_faction(), "callback-mutates-wire");
    require(std::strcmp(reinterpret_cast<char *>(packet + 0x55), "callback-mutates-wire") == 0,
            "view-is-live-wire-alias");
    row.set_tail_0(0xA7);
    require(packet[0x185] == 0xA7, "view-tail-byte-store");

    NetWinPacketView common(packet);
    common.set_opcode(0x4F04);
    require(common.opcode() == 0x4F04, "packet-opcode-roundtrip");
    const uint32_t selector = 0xFFFFFFFFu;
    std::memcpy(packet + 4, &selector, sizeof selector);
    require(common.selector() == selector, "selector-sentinel-bits");
    require(common.payload() == packet + 0x10, "payload-start");
    NetWinLobbyPacketView lobby(packet);
    packet[0x13] = 0xFF;
    const uint32_t owner = 0x89ABCDEFu;
    std::memcpy(packet + 0x188, &owner, sizeof owner);
    require(lobby.faction_or_color() == -1, "signed-lobby-faction");
    require(lobby.owner() == owner, "row-owner-at-payload-plus-178");
    require(lobby.payload() == packet + 0x10, "lobby-payload-start");

    NetWinSessionStatusView status(reinterpret_cast<char *>(packet));
    const int faction = 0x1020304;
    std::memcpy(packet + 4, &faction, sizeof faction);
    packet[0x18] = 0xFE; packet[0x14] = 0xB7; packet[0x10] = 0xC8;
    require(status.faction() == faction, "status-faction-dword");
    require(status.identity() == -2, "status-signed-identity-byte");
    require(status.field_14() == 0xB7 && status.field_10() == 0xC8,
            "status-ordered-byte-fields");

    NetWinFactionRecord records[3] = {};
    std::strcpy(records[1].leader_name_, "leader");
    std::strcpy(records[1].formal_faction_name_, "formal");
    std::strcpy(records[1].noun_faction_, "noun");
    std::strcpy(records[1].adjective_faction_, "adjective");
    std::strcpy(records[1].leader_title_, "title");
    unsigned char *record_bytes = reinterpret_cast<unsigned char *>(records);
    std::strcpy(reinterpret_cast<char *>(record_bytes + 0x190 + 0x112), "leader-adjective");
    record_bytes[0x190 + 0x192] = 0x6D;
    std::memset(packet + 0x10, 0, sizeof(NetWinLobbyRow));
    NetWin writer; writer.faction_records_ = records;
    writer.fill_player(NetWinLobbyRowView(packet + 0x10), 1);
    require(std::strcmp(row.leader_name(), "leader") == 0, "fill-leader");
    require(std::strcmp(row.formal_faction_name(), "formal") == 0, "fill-formal");
    require(std::strcmp(row.noun_faction(), "noun") == 0, "fill-noun");
    require(std::strcmp(row.adjective_faction(), "adjective") == 0, "fill-adjective");
    require(std::strcmp(row.leader_title(), "title") == 0, "fill-title");
    require(std::strcmp(row.adjective_leader(), "leader-adjective") == 0,
            "fill-adjective-leader-source-plus-112");
    require(packet[0x10 + offsetof(NetWinLobbyRow, tail_)] == 0x6D,
            "fill-tail-source-plus-192");

    NetWinLobbyRow actual_row = {};
    std::strcpy(actual_row.noun_faction_, "old noun");
    active_noun = actual_row.noun_faction_; label_calls = 0;
    writer.fill_player(NetWinLobbyRowView(actual_row), -1);
    require(label_calls == 1 && std::strcmp(actual_row.noun_faction_, "fallback") == 0,
            "actual-row-view-and-slot-minus-one");
    std::puts("actual session/lobby borrowed-view controls passed");
}
'''


def _wire_types() -> str:
    return installed_packet_declarations((
        "NetWinLobbyRow", "NetWinLobbyRowView", "NetWinLobbyPacketHeader",
        "NetWinPacketView", "NetWinLobbyPacketView", "NetWinSessionStatusView",
        "NetWinFactionRecord"))


def _fill_player() -> str:
    source = (ROOT / "src/netwin.cpp").read_text(encoding="utf-8")
    signature = "void NetWin::fill_player(NetWinLobbyRowView lobby_row, int slot)"
    return extract_definition(source, signature)


def _build_run(tmp_path: Path, types: str, fill: str, *, expected: str | None,
               compiler: str) -> None:
    cpp = tmp_path / "borrowed_wire.cpp"
    exe = tmp_path / "borrowed_wire"
    cpp.write_text(VIEW_HARNESS.replace("@@WIRE_TYPES@@", types)
                   .replace("@@FILL_PLAYER@@", fill))
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=undefined,bounds", "-fno-sanitize-recover=all",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if expected is None:
        assert result.returncode == 0, result.stdout + result.stderr
        assert "actual session/lobby borrowed-view controls passed" in result.stdout
    else:
        assert result.returncode == 98, result.stdout + result.stderr
        assert f"wire-view: {expected}" in result.stderr, result.stderr


def test_current_task_identities_history_and_source_recipe_are_bounded():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    for path in SOURCES:
        before_text = _git_source(path)
        after_text = (ROOT / path).read_text(encoding="utf-8")
        before = _source_rows(path, before_text)
        after = _source_rows(path, after_text)
        assert set(after) == set(before), path
        for address, old in before.items():
            new = after[address]
            for item in fields(old):
                if item.name not in {"line", "region", "origin", "symbol"}:
                    assert getattr(new, item.name) == getattr(old, item.name), (path, address, item.name)
            assert _history(new) == _history(old), (path, address, "history")
            if address == 0x00481DA0:
                assert old.symbol == "?fill_player@NetWin@@QAEXPAUNetWinLobbyRow@@H@Z"
                assert new.symbol == "?fill_player@NetWin@@QAEXVNetWinLobbyRowView@@H@Z"
            else:
                assert new.symbol == old.symbol, (path, address, "symbol")

    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    source_cpp = (ROOT / "src/netwin.cpp").read_text(encoding="utf-8")
    daemon = (ROOT / "src/netdaemon.cpp").read_text(encoding="utf-8")
    lobby_handler = extract_definition(daemon, "static void netdaemon_handle_session_lobby(")
    selector_read = "const uint32_t sender = packet.selector();"
    opcode_read = "const unsigned short wire_type = packet.opcode();"
    assert lobby_handler.index(selector_read) < lobby_handler.index(opcode_read)
    assert lobby_handler.count("lobby.faction_or_color()") == 2
    assert lobby_handler.count("lobby.owner()") == 2
    assert lobby_handler.count("lobby.payload()") == 3
    old_header = _git_source("src/netwin.h")
    old_cpp = _git_source("src/netwin.cpp")
    old_daemon = _git_source("src/netdaemon.cpp")
    assert _cpp_tokens(_restore_netwin_header(header, old_header)) == _cpp_tokens(old_header)
    assert _cpp_tokens(_restore_netwin_cpp(source_cpp, old_cpp)) == _cpp_tokens(old_cpp)
    assert _cpp_tokens(_restore_netdaemon(daemon, old_daemon)) == _cpp_tokens(old_daemon)


def test_expanded_game_collateral_is_byte_preserved_with_three_subject_histories():
    store = TaskStore(ROOT)
    task = store.read(TASK)
    decisions = store.decisions(TASK)
    expanded = [subject for decision in decisions if decision["kind"] == "scope_expanded"
                for subject in decision["subjects"]]
    expected = {0x00455610, 0x00454270, 0x005A9EB0}
    assert {subject["address"] for subject in expanded} == expected
    assert all(subject["image_sha256"] == PIN and subject["address_space"] == "pe:va"
               for subject in expanded)

    baseline_bytes = subprocess.check_output(
        ["git", "show", f"{task.base_revision}:src/game.cpp"], cwd=ROOT, timeout=30)
    current_bytes = (ROOT / "src/game.cpp").read_bytes()
    assert current_bytes == baseline_bytes, "collateral src/game.cpp must remain byte-exact"

    baseline_rows = _source_rows("src/game.cpp", baseline_bytes.decode("utf-8"))
    current_rows = _source_rows("src/game.cpp", current_bytes.decode("utf-8"))
    for address in expected:
        before, after = baseline_rows[address], current_rows[address]
        assert after == before, (address, "expanded game subject mapping/history")
        assert _history(after) == _history(before), (address, "expanded game subject history")


def test_canonical_wire_view_schemas_and_payload_shapes_are_exact():
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    schemas = {
        "NetWinLobbyRowView": "class NetWinLobbyRowView { public: explicit NetWinLobbyRowView(unsigned char *bytes) : bytes_(reinterpret_cast<char *>(bytes)) {} explicit NetWinLobbyRowView(char *bytes) : bytes_(bytes) {} explicit NetWinLobbyRowView(NetWinLobbyRow &row) : bytes_(reinterpret_cast<char *>(&row)) {} char *leader_name() const { return bytes_ + offsetof(NetWinLobbyRow, leader_name_); } char *formal_faction_name() const { return bytes_ + offsetof(NetWinLobbyRow, formal_faction_name_); } char *noun_faction() const { return bytes_ + offsetof(NetWinLobbyRow, noun_faction_); } char *adjective_faction() const { return bytes_ + offsetof(NetWinLobbyRow, adjective_faction_); } char *leader_title() const { return bytes_ + offsetof(NetWinLobbyRow, leader_title_); } char *adjective_leader() const { return bytes_ + offsetof(NetWinLobbyRow, adjective_leader_); } void set_tail_0(uint8_t value) const { std::memcpy(bytes_ + offsetof(NetWinLobbyRow, tail_), &value, sizeof(value)); } private: char *bytes_; }",
        "NetWinPacketView": "class NetWinPacketView { public: explicit NetWinPacketView(unsigned char *bytes) : bytes_(bytes) {} explicit NetWinPacketView(char *bytes) : bytes_(reinterpret_cast<unsigned char *>(bytes)) {} uint16_t opcode() const { uint16_t value; std::memcpy(&value, bytes_ + offsetof(NetWinLobbyPacketHeader, opcode_), sizeof(value)); return value; } void set_opcode(uint16_t value) const { std::memcpy(bytes_ + offsetof(NetWinLobbyPacketHeader, opcode_), &value, sizeof(value)); } uint32_t selector() const { uint32_t value; std::memcpy(&value, bytes_ + offsetof(NetWinLobbyPacketHeader, slot_), sizeof(value)); return value; } unsigned char *payload() const { return bytes_ + sizeof(NetWinLobbyPacketHeader); } private: unsigned char *bytes_; }",
        "NetWinLobbyPacketView": "class NetWinLobbyPacketView { public: explicit NetWinLobbyPacketView(unsigned char *bytes) : bytes_(bytes) {} int8_t faction_or_color() const { int8_t value; std::memcpy(&value, bytes_ + sizeof(NetWinLobbyPacketHeader) + offsetof(NetWinLobbyRow, faction_or_color_), sizeof(value)); return value; } uint32_t owner() const { uint32_t value; std::memcpy(&value, bytes_ + sizeof(NetWinLobbyPacketHeader) + offsetof(NetWinLobbyRow, owner_), sizeof(value)); return value; } unsigned char *payload() const { return bytes_ + sizeof(NetWinLobbyPacketHeader); } private: unsigned char *bytes_; }",
        "NetWinSessionStatusView": "class NetWinSessionStatusView { public: explicit NetWinSessionStatusView(char *bytes) : bytes_(reinterpret_cast<unsigned char *>(bytes)) {} int faction() const { int value; std::memcpy(&value, bytes_ + 4, sizeof(value)); return value; } int8_t identity() const { return static_cast<int8_t>(bytes_[0x18]); } uint8_t field_14() const { return bytes_[0x14]; } uint8_t field_10() const { return bytes_[0x10]; } private: unsigned char *bytes_; }",
    }
    for name, expected in schemas.items():
        actual = extract_definition(header, f"class {name} {{")
        assert _cpp_tokens(actual) == _cpp_tokens(expected), name
    assert _cpp_tokens(extract_definition(
        header, "struct NetWinLobbyPacketHeader {")) == _cpp_tokens(LOBBY_HEADER_SCHEMA)
    packet_schemas = {
        "NetWinLobbySinglePacket": "struct NetWinLobbySinglePacket { NetWinLobbyPacketHeader header_; NetWinLobbyRow row_; }",
        "NetWinLobbyFullPacket": "struct NetWinLobbyFullPacket { NetWinLobbyPacketHeader header_; NetWinLobbyRow rows_[7]; }",
        "NetWinLobbyLeavePacket": "struct NetWinLobbyLeavePacket { NetWinLobbyPacketHeader header_; uint8_t trailing_[0x10]; }",
        "NetWinSessionBlockPacket": "struct NetWinSessionBlockPacket { uint16_t opcode_; uint8_t reserved_[0x0E]; NetWinSessionBlock session_; }",
        "NetWinSessionLobbyPacket": "struct NetWinSessionLobbyPacket { NetWinLobbyPacketHeader header_; NetWinSessionBlock session_; }",
        "NetWinSessionOptionsPacket": "struct NetWinSessionOptionsPacket { uint16_t opcode_; uint8_t reserved_[0x0E]; NetWinSessionOptionsState options_; }",
    }
    for name, expected in packet_schemas.items():
        assert _cpp_tokens(extract_definition(header, f"struct {name} {{")) == _cpp_tokens(expected), name
    assert "static_assert(sizeof(NetWinLobbyRowView) == sizeof(void *)" in header
    assert "static_assert(sizeof(NetWinPacketView) == sizeof(void *)" in header
    assert "static_assert(sizeof(NetWinLobbyPacketView) == sizeof(void *)" in header
    assert "static_assert(sizeof(NetWinSessionStatusView) == sizeof(void *)" in header


@pytest.mark.parametrize("damage", [
    "extra-header-field", "extra-header-method", "removed-header-offset-assert",
    "changed-header-offset-assert", "unapproved-header-assert", "unapproved-global-declaration",
])
def test_header_recipe_rejects_unapproved_declaration_or_assertion_mutants(damage):
    header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
    baseline = _git_source("src/netwin.h")
    if damage == "extra-header-field":
        before, after = "  uint8_t flags_;", "  uint8_t flags_;\n  uint32_t unapproved_;"
    elif damage == "extra-header-method":
        before, after = "  uint32_t field_C_;\n};", "  uint32_t field_C_;\n  int unapproved() const { return 0; }\n};"
    elif damage == "removed-header-offset-assert":
        before = 'static_assert(offsetof(NetWinLobbyPacketHeader, slot_) == 0x04,\n              "NetWin packet selector offset");\n'
        after = ""
    elif damage == "changed-header-offset-assert":
        before, after = "slot_) == 0x04", "slot_) == 0x08"
    elif damage == "unapproved-header-assert":
        before = 'static_assert(offsetof(NetWinLobbyPacketHeader, field_C_) == 0x0C,\n              "NetWin packet field-c offset");'
        after = before + '\nstatic_assert(sizeof(NetWinLobbyPacketHeader) == 0x10, "unapproved duplicate");'
    else:
        before = "// Only NetWin::exec constructs these stack-local rows."
        after = "int unapproved_wire_global;\n\n" + before
    assert header.count(before) == 1, damage
    damaged = header.replace(before, after, 1)
    with pytest.raises(AssertionError):
        _restore_netwin_header(damaged, baseline)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_mutable_views_and_fill_player_on_unaligned_and_object_rows(tmp_path, compiler):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    _build_run(tmp_path, _wire_types(), _fill_player(), expected=None, compiler=compiler)


@pytest.mark.parametrize("damage,expected", [
    ("wrong-noun-view-offset", "noun-offset"),
    ("wrong-tail-source-byte", "fill-tail-source-plus-192"),
    ("slot-minus-one-reorder", "slot-minus-one-clears-before-label"),
])
def test_named_compile_success_view_damage_controls_exit_98(tmp_path, damage, expected):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    types = _wire_types()
    fill = _fill_player()
    if damage == "wrong-noun-view-offset":
        before = "char *noun_faction() const {\n    return bytes_ + offsetof(NetWinLobbyRow, noun_faction_);"
        after = "char *noun_faction() const {\n    return bytes_ + offsetof(NetWinLobbyRow, leader_name_);"
        assert before in types
        types = types.replace(before, after, 1)
    elif damage == "wrong-tail-source-byte":
        before = "allocation_bytes[slot * 0x190 + 0x192]"
        assert fill.count(before) == 1
        fill = fill.replace(before, "allocation_bytes[slot * 0x190 + 0x193]", 1)
    else:
        before = "lobby_row.noun_faction()[0] = 0;\n        strcat(lobby_row.noun_faction(),\n               label_get(0xFE4 / 4));"
        after = "const char *fallback = label_get(0xFE4 / 4);\n        lobby_row.noun_faction()[0] = 0;\n        strcat(lobby_row.noun_faction(), fallback);"
        assert before in fill
        fill = fill.replace(before, after, 1)
    _build_run(tmp_path, types, fill, expected=expected, compiler=compiler)


def test_pinned_original_identity_uses_only_the_observed_fill_player_symbol_change():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    import hashlib
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    # The two known receiver extents are pinned elsewhere in this suite; this
    # local witness retains the producer's six-option dword and full-session
    # packet extents without claiming a new packet length or status producer.
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    spans = ((0x0047B260, 0x0047B2DE), (0x0047B2E0, 0x0047B379))
    decoded = []
    for start, end in spans:
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert instructions and instructions[0].address == start
        assert instructions[-1].address + instructions[-1].size == end
        decoded.append(instructions)
    assert any(i.mnemonic == "push" and i.op_str.lower() == "0x28" for i in decoded[0])
    assert any(i.mnemonic == "push" and i.op_str.lower() == "0x178" for i in decoded[1])

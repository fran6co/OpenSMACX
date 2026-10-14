"""Bounded actual-source controls for the E/F/G command wire views.

These host projections test borrowed field access and callback-time rereads;
they do not claim native ABI, packet-length, or game-runtime equivalence.
"""
from dataclasses import fields
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.tasks import TaskStore
from test_netdaemon_transfer_modal import extract


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
TASK = "netdaemon-command-wire-closure-oct13ac"
BASE = "eef7695b48b16241ae0a7b5d4e786df666bd6254"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def _cpp_tokens(source):
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|'
        r'[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|.', re.S)
    values = []
    for match in token.finditer(source):
        value = match.group(0)
        if value.startswith("//") or value.startswith("/*"):
            continue
        if value.isspace():
            continue
        values.append(value)
    return values


def _replace_once(source, before, after, label):
    assert source.count(before) == 1, (label, source.count(before), before[:100])
    return source.replace(before, after, 1)


def _replace_definition(source, signature, before, after):
    body = extract(source, signature)
    assert body.count(before) == 1, (signature, before, body.count(before))
    return source.replace(body, body.replace(before, after, 1), 1)


def _restore_function_body(candidate, baseline, signature, transform):
    actual = extract(candidate, signature)
    expected = extract(baseline, signature)
    restored = transform(actual)
    assert _cpp_tokens(restored) == _cpp_tokens(expected), signature
    return candidate.replace(actual, restored, 1)


def _restore_calls(body, replacements):
    for current, old in replacements:
        assert current in body, (current, body[:100])
        body = body.replace(current, old)
    return body


def _restore_authorized_cpp_candidate(current, baseline):
    """Reverse the scoped views to the immutable eef wire-read source.

    Each function is compared as a complete C++ token sequence after only its
    approved accessor/constructor reversal. This is intentionally separate
    from comments/annotation-history continuity.
    """
    _assert_constrained_new_declarations(current)
    _assert_table_g_variant_bindings(current)
    scopes = (
        ("static void netdaemon_complete_local_turn_wait(",
         lambda b: _restore_calls(
             b.replace("    const MessageCommandView packet(message);\n", ""),
             [("packet.payload_10()", "*reinterpret_cast<int *>(message + 0x10)")])),
        ("static void netdaemon_handle_server_upkeep_info(",
         lambda b: _restore_calls(
             b.replace("    const MessageCommandView packet(message);\n", ""),
             [("static_cast<unsigned int>(packet.payload_10())",
               "*reinterpret_cast<uint32_t *>(message + 0x10)"),
              ("packet.payload_10()", "*reinterpret_cast<int *>(message + 0x10)"),
              ("packet.payload_14()", "*reinterpret_cast<int *>(message + 0x14)"),
              ("packet.payload_18()", "*reinterpret_cast<int *>(message + 0x18)")])),
        ("static bool netdaemon_process_table_e(",
         lambda b: _restore_calls(
             b.replace("        const MessageAcknowledgementPrefixView packet(message);\n", "")
              .replace("        const MessageLockView packet(message);\n", ""),
             [("packet.player_index()", "*reinterpret_cast<int *>(message + 0x10)"),
              ("packet.sequence()", "*reinterpret_cast<int *>(message + 0x0C)"),
              ("packet.payload_10()", "*reinterpret_cast<int *>(message + 0x10)"),
              ("packet.payload_14()", "*reinterpret_cast<int *>(message + 0x14)"),
              ("packet.payload_18()", "*reinterpret_cast<int *>(message + 0x18)")])),
        ("static bool netdaemon_handle_table_f(",
         lambda b: _restore_calls(
             b.replace("    const MessageEventPrefixView event(message);\n", "")
              .replace("    const MessageBigDataView big_data(message);\n", "")
              .replace("    const MessageCommandView packet(message);\n", ""),
             [("event.payload_10()", "*reinterpret_cast<int *>(message + 0x10)"),
              ("event.faction()", "*reinterpret_cast<int *>(message + 4)"),
              ("big_data.faction()", "*reinterpret_cast<int *>(message + 4)"),
              ("big_data.data_word(index)", "*reinterpret_cast<int *>(message + 0x10 + index * sizeof(int))"),
              ("packet.payload_10()", "*reinterpret_cast<int *>(message + 0x10)")])),
        ("static bool netdaemon_process_table_g(",
         lambda b: _restore_calls(
             b.replace("    const MessageCommandView data(msg);\n", "")
              .replace("    const MessageCommonCommandPrefixView common(msg);\n", "")
              .replace("    const MessageVehicleView vehicle(msg);\n", "")
              .replace("    const MessageBaseTextView base_text(msg);\n", ""),
             [("data.faction()", "netdaemon_table_g_word(msg, 4)"),
              ("data.timestamp_bits()", "netdaemon_table_g_word(msg, 8)"),
              ("data.payload_10()", "netdaemon_table_g_word(msg, 0x10)"),
              ("data.payload_14()", "netdaemon_table_g_word(msg, 0x14)"),
              ("data.payload_18()", "netdaemon_table_g_word(msg, 0x18)"),
              ("data.payload_1C()", "netdaemon_table_g_word(msg, 0x1c)"),
              ("data.payload_18_byte()", "*reinterpret_cast<int8_t *>(msg + 0x18)"),
              ("common.faction()", "netdaemon_table_g_word(msg, 4)"),
              ("common.timestamp_bits()", "netdaemon_table_g_word(msg, 8)"),
              ("common.payload_10()", "netdaemon_table_g_word(msg, 0x10)"),
              ("common.payload_14()", "netdaemon_table_g_word(msg, 0x14)"),
              ("vehicle.faction()", "netdaemon_table_g_word(msg, 4)"),
              ("vehicle.timestamp_bits()", "netdaemon_table_g_word(msg, 8)"),
              ("vehicle.payload_10()", "netdaemon_table_g_word(msg, 0x10)"),
              ("vehicle.payload_14()", "netdaemon_table_g_word(msg, 0x14)"),
              ("base_text.faction()", "netdaemon_table_g_word(msg, 4)"),
              ("base_text.timestamp_bits()", "netdaemon_table_g_word(msg, 8)"),
              ("base_text.payload_10()", "netdaemon_table_g_word(msg, 0x10)"),
              ("base_text.text()", "reinterpret_cast<char *>(msg + 0x14)"),
              ("netdaemon_read_unserialized_table_g_y(msg)", "netdaemon_table_g_word(msg, 0x18)")])),
        ("static bool netdaemon_handle_diplo_cd_message(",
         lambda b: _restore_calls(
             b.replace("    const MessageCommandView packet(message);\n", ""),
             [("packet.faction()", "*reinterpret_cast<int *>(message + 4)"),
              ("packet.payload_10()", "*reinterpret_cast<int *>(message + 0x10)"),
              ("packet.payload_14()", "*reinterpret_cast<int *>(message + 0x14)")])),
        ("static bool netdaemon_handle_direct_diplomacy_complete(",
         lambda b: _restore_calls(
             b.replace("    const MessageCommandView packet(message);\n", ""),
             [("packet.faction()", "*reinterpret_cast<int *>(message + 4)"),
              ("packet.payload_10()", "*reinterpret_cast<int *>(message + 0x10)")])),
    )
    for signature, transform in scopes:
        current = _restore_function_body(current, baseline, signature, transform)

    # The promoted producer records are reconstructed from the immutable local
    # declarations, and their native `turn_` field spelling is restored.
    for signature, record_name in (("void __cdecl message_veh(", "MessageVehicleRecord"),
                                   ("void __cdecl message_base(", "MessageBaseTextRecord"),
                                   ("void __cdecl message_lock(", "MessageLockRecord")):
        old_body = extract(baseline, signature)
        match = re.search(r"    struct Message \{.*?\n    \} message;", old_body, re.S)
        assert match, signature
        current = _replace_definition(current, signature,
                                      f"    {record_name} message;", match.group(0))
        body = extract(current, signature).replace("message.sequence_", "message.turn_")
        current = current.replace(extract(current, signature), body, 1)

    # Constrain every added declaration before reversing the authorized source
    # migration. The round-trip below is not allowed to erase arbitrary code.
    # New borrowed declarations are checked by their independent field schema
    # and runtime projections, then removed for the full-token round trip.
    current = current.replace(
        "    int8_t payload_18_byte() const {\n"
        "        int8_t value;\n"
        "        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_18_), sizeof value);\n"
        "        return value;\n"
        "    }\n", "", 1)
    current = _remove_constrained_section(current, "struct MessageVehicleRecord {", "// ORIGINAL: 0x00592E10")
    current = _remove_constrained_section(current, "struct MessageBaseTextRecord {", "// ORIGINAL: 0x00592E70")
    current = _remove_constrained_section(current, "// The 0x4406 vehicle and data producers share only",
                                          "// ORIGINAL: 0x00592EE0")
    current = _remove_constrained_decl(current, "class MessageBigDataView {")
    current = _remove_constrained_section(current, "struct MessageLockRecord {", "// ORIGINAL: 0x00593090")
    current = _remove_constrained_section(current, "// The console 0x2417/0x2418 senders use message_veh",
                                          "// These Table G arms belong")
    old_g_helper = extract(baseline, "static int netdaemon_table_g_word(")
    current = current.replace("// These Table G arms belong", old_g_helper + "\n// These Table G arms belong", 1)
    return current


def _remove_section(source, start, end):
    first = source.find(start)
    last = source.find(end, first)
    assert first >= 0 and last > first, (start, end)
    return source[:first] + source[last:]


def _remove_balanced_decl(source, marker):
    first = source.find(marker)
    assert first >= 0, marker
    opening = source.find("{", first)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                end = pos + 1
                if source[end:end + 1] == ";":
                    end += 1
                return source[:first] + source[end:]
    raise AssertionError("unterminated declaration " + marker)


def _record_decl(name, members):
    body = "\n".join(f"    {kind} {field};" for kind, field in members)
    return f"struct {name} {{\n{body}\n}}"


_RECORD_MEMBERS = {
    "MessageVehicleRecord": (("int16_t", "opcode_"), ("unsigned char", "unresolved_2_[2]"),
        ("int32_t", "faction_"), ("uint32_t", "time_"), ("int32_t", "sequence_"),
        ("int32_t", "field_10_"), ("int32_t", "field_14_")),
    "MessageBaseTextRecord": (("int16_t", "opcode_"), ("unsigned char", "unresolved_2_[2]"),
        ("int32_t", "faction_"), ("uint32_t", "time_"), ("int32_t", "sequence_"),
        ("int32_t", "payload_10_"), ("char", "text_[24]")),
    "MessageLockRecord": (("int16_t", "opcode_"), ("unsigned char", "unresolved_2_[2]"),
        ("int32_t", "faction_"), ("uint32_t", "time_"), ("int32_t", "sequence_"),
        ("int32_t", "field_10_"), ("int32_t", "field_14_"), ("int32_t", "field_18_"),
        ("int32_t", "field_1C_"), ("int32_t", "field_20_"), ("int32_t", "field_24_")),
    "MessageAcknowledgementPrefix": (("unsigned char", "unresolved_0_[0x0C]"),
        ("int32_t", "sequence_"), ("int32_t", "player_index_")),
    "MessageEventPrefix": (("unsigned char", "unresolved_0_[0x04]"), ("int32_t", "faction_"),
        ("unsigned char", "unresolved_8_[0x08]"), ("int32_t", "payload_10_")),
    "MessageBigDataRecord": (("int16_t", "opcode_"), ("unsigned char", "unresolved_2_[2]"),
        ("int", "faction_"), ("uint32_t", "timestamp_"), ("int", "sequence_"),
        ("int", "data_[32]")),
}


def _method_tokens(return_type, method, record, member, value_type="int"):
    return _cpp_tokens(
        f"{return_type} {method}() const {{ {value_type} value; "
        f"std::memcpy(&value, bytes_ + offsetof({record}, {member}), sizeof value); "
        "return value; }")


def _view_tokens(class_name, pointer_type, methods, *, constructor_type="const unsigned char *",
                 extra_methods=()):
    chunks = [f"class {class_name} {{ public: explicit {class_name}({constructor_type} bytes) : bytes_(bytes) {{}}"]
    for ret, name, record, field, valtype in methods:
        chunks.append(" ".join((
            f"{ret} {name}() const {{", f"{valtype} value;",
            f"std::memcpy(&value, bytes_ + offsetof({record}, {field}), sizeof value);",
            "return value; }")))
    chunks.extend(extra_methods)
    chunks.append(f"private: {pointer_type} bytes_; }}")
    return _cpp_tokens(" ".join(chunks))


def _assert_constrained_new_declarations(source):
    _assert_region_declarations(source, "struct MessageVehicleRecord {", "// ORIGINAL: 0x00592E10",
                                ["struct MessageVehicleRecord"])
    _assert_region_declarations(source, "struct MessageBaseTextRecord {", "// ORIGINAL: 0x00592E70",
                                ["struct MessageBaseTextRecord"])
    _assert_region_declarations(source, "// The 0x4406 vehicle and data producers share only",
                                "// ORIGINAL: 0x00592EE0", [
        "class MessageCommonCommandPrefixView", "class MessageVehicleView",
        "class MessageBaseTextView", "struct MessageAcknowledgementPrefix",
        "class MessageAcknowledgementPrefixView", "struct MessageEventPrefix",
        "class MessageEventPrefixView"])
    _assert_region_declarations(source, "class MessageBigDataView {",
                                "static_assert(sizeof(MessageLandmarkRecord)", ["class MessageBigDataView"])
    _assert_region_declarations(source, "struct MessageLockRecord {", "// ORIGINAL: 0x00593090",
                                ["struct MessageLockRecord", "class MessageLockView"])
    _assert_region_declarations(source, "// The console 0x2417/0x2418 senders use message_veh",
                                "// These Table G arms belong", [])

    # Exact ordered field declarations prevent hidden fields or padding changes.
    for name, members in _RECORD_MEMBERS.items():
        actual = extract(source, f"struct {name} {{")
        assert _cpp_tokens(actual) == _cpp_tokens(_record_decl(name, members)), name

    methods = {
        "MessageCommonCommandPrefixView": ("const unsigned char *", [
            ("int", "faction", "MessageDataRecord", "faction_", "int"),
            ("int", "timestamp_bits", "MessageDataRecord", "timestamp_", "int"),
            ("int", "payload_10", "MessageDataRecord", "payload_10_", "int"),
            ("int", "payload_14", "MessageDataRecord", "payload_14_", "int")]),
        "MessageVehicleView": ("const unsigned char *", [
            ("int", "faction", "MessageVehicleRecord", "faction_", "int"),
            ("int", "timestamp_bits", "MessageVehicleRecord", "time_", "int"),
            ("int", "payload_10", "MessageVehicleRecord", "field_10_", "int"),
            ("int", "payload_14", "MessageVehicleRecord", "field_14_", "int")]),
        "MessageBaseTextView": ("unsigned char *", [
            ("int", "faction", "MessageBaseTextRecord", "faction_", "int"),
            ("int", "timestamp_bits", "MessageBaseTextRecord", "time_", "int"),
            ("int", "payload_10", "MessageBaseTextRecord", "payload_10_", "int")]),
        "MessageAcknowledgementPrefixView": ("const unsigned char *", [
            ("int32_t", "sequence", "MessageAcknowledgementPrefix", "sequence_", "int32_t"),
            ("int32_t", "player_index", "MessageAcknowledgementPrefix", "player_index_", "int32_t")]),
        "MessageEventPrefixView": ("const unsigned char *", [
            ("int", "faction", "MessageEventPrefix", "faction_", "int"),
            ("int", "payload_10", "MessageEventPrefix", "payload_10_", "int")]),
        "MessageLockView": ("const unsigned char *", [
            ("int", "payload_10", "MessageLockRecord", "field_10_", "int"),
            ("int", "payload_14", "MessageLockRecord", "field_14_", "int"),
            ("int", "payload_18", "MessageLockRecord", "field_18_", "int"),
            ("int", "payload_1C", "MessageLockRecord", "field_1C_", "int"),
            ("int", "payload_20", "MessageLockRecord", "field_20_", "int"),
            ("int", "payload_24", "MessageLockRecord", "field_24_", "int"),
            ("unsigned char", "payload_18_byte", "MessageLockRecord", "field_18_", "unsigned char")]),
    }
    for name, (pointer_type, schema) in methods.items():
        if name == "MessageBaseTextView":
            continue
        actual = extract(source, f"class {name} {{")
        expected = _view_tokens(name, pointer_type, schema)
        assert _cpp_tokens(actual) == expected, name

    base_text = _view_tokens("MessageBaseTextView", "unsigned char *", methods["MessageBaseTextView"][1],
        constructor_type="unsigned char *",
        extra_methods=["char *text() const { return reinterpret_cast<char *>(bytes_ + offsetof(MessageBaseTextRecord, text_)); }"])
    assert _cpp_tokens(extract(source, "class MessageBaseTextView {")) == base_text

    big = _view_tokens("MessageBigDataView", "const unsigned char *",
        [("int", "faction", "MessageBigDataRecord", "faction_", "int")],
        extra_methods=["int data_word(int index) const { int value; std::memcpy(&value, bytes_ + offsetof(MessageBigDataRecord, data_) + index * sizeof(int), sizeof value); return value; }"])
    assert _cpp_tokens(extract(source, "class MessageBigDataView {")) == big

    command = extract(source, "class MessageCommandView {")
    assert _cpp_tokens(extract(command, "int8_t payload_18_byte() const")) == _cpp_tokens(
        "int8_t payload_18_byte() const { int8_t value; std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_18_), sizeof value); return value; }")

    helper = extract(source, "static int netdaemon_read_unserialized_table_g_y(")
    assert _cpp_tokens(helper) == _cpp_tokens(
        "static int netdaemon_read_unserialized_table_g_y(const unsigned char *message) { int value; "
        "std::memcpy(&value, message + 0x18, sizeof value); return value; }")


def _assert_region_declarations(source, start, end, expected):
    first = source.find(start)
    last = source.find(end, first)
    assert first >= 0 and last > first, (start, end)
    found = re.findall(r"\b(struct|class)\s+([A-Za-z_]\w*)\s*\{", source[first:last])
    actual = [f"{kind} {name}" for kind, name in found]
    assert actual == expected, (start, actual, expected)


def _remove_constrained_section(source, start, end):
    first = source.find(start)
    last = source.find(end, first)
    assert first >= 0 and last > first, (start, end)
    section = source[first:last]
    # Remove only complete declarations that have already passed the fixed
    # schemas. Any leftover C++ token (including an added global) refuses.
    markers = re.findall(r"(?:struct|class)\s+[A-Za-z_]\w*\s*\{", section)
    for marker in markers:
        declaration = extract(section, marker)
        full = declaration + ";"
        assert section.count(full) == 1, marker
        section = section.replace(full, "", 1)
    helper_marker = "static int netdaemon_read_unserialized_table_g_y("
    if helper_marker in section:
        helper = extract(section, helper_marker)
        assert section.count(helper) == 1
        section = section.replace(helper, "", 1)
    assertions = re.findall(r"static_assert\s*\(.*?\);", section, re.S)
    for assertion in assertions:
        section = section.replace(assertion, "", 1)
    assert not _cpp_tokens(section), (start, _cpp_tokens(section)[:30])
    return source[:first] + source[last:]


def _remove_constrained_decl(source, marker):
    declaration = extract(source, marker)
    # Its complete class token schema was checked above; no broad section erase.
    full = declaration + ";"
    assert source.count(full) == 1, marker
    return source.replace(full, "", 1)


def _assert_table_g_variant_bindings(source):
    body = extract(source, "static bool netdaemon_process_table_g(")
    cases = re.split(r"(?=\bcase\s+0x[0-9A-Fa-f]+\s*:)", body)
    expected = {
        "0x4405": "data.", "0x4406": "common.", "0x4407": "vehicle.",
        "0x4408": "base_text.", "0x4409": "vehicle.", "0x4410": "vehicle.",
        "0x4411": "data.", "0x4412": "data.", "0x4413": "data.",
        "0x4414": "data.", "0x4415": "data.", "0x4416": "vehicle.",
        "0x4417": "vehicle.", "0x4418": "vehicle.",
    }
    for opcode, view in expected.items():
        arm = next((part for part in cases if re.search(rf"\bcase\s+{opcode}\s*:", part)), None)
        assert arm is not None and view in arm, (opcode, view)
        for wrong in {"data.", "common.", "vehicle.", "base_text."} - {view}:
            assert wrong not in arm, (opcode, "wrong view", wrong)


def test_current_netdaemon_identities_claims_and_history_match_immutable_task_base():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    old_text = subprocess.check_output(
        ["git", "show", f"{BASE}:src/netdaemon.cpp"], cwd=ROOT,
        text=True, timeout=30)
    old = {row.address: row for row in read_text(old_text, SOURCE)}
    current = {row.address: row for row in read_text(SOURCE.read_text(), SOURCE)}
    assert set(current) == set(old)
    for address, before in old.items():
        after = current[address]
        for item in fields(before):
            if item.name not in {"line", "region", "origin"}:
                assert getattr(after, item.name) == getattr(before, item.name)
        assert _history(after) == _history(before)


def test_scoped_wire_migration_reverses_to_immutable_full_cpp_tokens():
    baseline = subprocess.check_output(
        ["git", "show", f"{BASE}:src/netdaemon.cpp"], cwd=ROOT,
        text=True, timeout=30)
    reconstructed = _restore_authorized_cpp_candidate(SOURCE.read_text(), baseline)
    assert _cpp_tokens(reconstructed) == _cpp_tokens(baseline)


@pytest.mark.parametrize("damage", [
    "extra-record-field", "unused-view-method", "unrelated-global",
    "wrong-table-g-view",
])
def test_source_recipe_rejects_unconstrained_declarations_and_variant_bindings(damage):
    baseline = subprocess.check_output(
        ["git", "show", f"{BASE}:src/netdaemon.cpp"], cwd=ROOT,
        text=True, timeout=30)
    candidate = SOURCE.read_text()
    if damage == "extra-record-field":
        candidate = _replace_once(candidate,
            "    int32_t field_14_;\n};\n\nstatic_assert(sizeof(MessageVehicleRecord)",
            "    int32_t field_14_;\n    int32_t unauthorized_;\n};\n\nstatic_assert(sizeof(MessageVehicleRecord)",
            damage)
    elif damage == "unused-view-method":
        view = extract(candidate, "class MessageVehicleView {")
        changed = _replace_once(view, "private:\n    const unsigned char *bytes_;",
            "int unused() const { return 0; }\nprivate:\n    const unsigned char *bytes_;", damage)
        candidate = candidate.replace(view, changed, 1)
    elif damage == "unrelated-global":
        candidate = _replace_once(candidate, "// The message builders.",
            "// The message builders.\nstatic int unauthorized_global;", damage)
    elif damage == "wrong-table-g-view":
        body = extract(candidate, "static bool netdaemon_process_table_g(")
        arm = re.search(r"case 0x4408:.*?(?=\n\s*case 0x[0-9A-Fa-f]+:)", body, re.S)
        assert arm and "base_text.payload_10()" in arm.group(0)
        changed_arm = arm.group(0).replace("base_text.payload_10()", "vehicle.payload_10()", 1)
        candidate = candidate.replace(arm.group(0), changed_arm, 1)
    with pytest.raises(AssertionError):
        reconstructed = _restore_authorized_cpp_candidate(candidate, baseline)
        assert _cpp_tokens(reconstructed) == _cpp_tokens(baseline), damage


def test_new_view_methods_are_single_field_memcpy_accessors():
    source = SOURCE.read_text()
    record_schema = {
        "MessageVehicleRecord": ("0x18", ("faction_", "time_", "sequence_", "field_10_", "field_14_")),
        "MessageBaseTextRecord": ("0x2C", ("faction_", "time_", "sequence_", "payload_10_", "text_[24]")),
        "MessageLockRecord": ("0x28", ("faction_", "time_", "sequence_", "field_10_", "field_14_", "field_18_")),
        "MessageAcknowledgementPrefix": (None, ("sequence_", "player_index_")),
        "MessageEventPrefix": (None, ("faction_", "payload_10_")),
        "MessageBigDataRecord": ("0x90", ("faction_", "timestamp_", "sequence_", "data_[32]")),
    }
    for name, (width, members) in record_schema.items():
        record = extract(source, f"struct {name} {{")
        for member in members:
            assert member in record, (name, member)
        if width:
            assert f"sizeof({name}) == {width}" in source, name
    schema = {
        "MessageVehicleView": {
            "faction": ("MessageVehicleRecord", "faction_"),
            "timestamp_bits": ("MessageVehicleRecord", "time_"),
            "payload_10": ("MessageVehicleRecord", "field_10_"),
            "payload_14": ("MessageVehicleRecord", "field_14_"),
        },
        "MessageBaseTextView": {
            "faction": ("MessageBaseTextRecord", "faction_"),
            "timestamp_bits": ("MessageBaseTextRecord", "time_"),
            "payload_10": ("MessageBaseTextRecord", "payload_10_"),
        },
        "MessageLockView": {
            "payload_10": ("MessageLockRecord", "field_10_"),
            "payload_14": ("MessageLockRecord", "field_14_"),
            "payload_18": ("MessageLockRecord", "field_18_"),
            "payload_1C": ("MessageLockRecord", "field_1C_"),
            "payload_20": ("MessageLockRecord", "field_20_"),
            "payload_24": ("MessageLockRecord", "field_24_"),
        },
        "MessageAcknowledgementPrefixView": {
            "sequence": ("MessageAcknowledgementPrefix", "sequence_"),
            "player_index": ("MessageAcknowledgementPrefix", "player_index_"),
        },
        "MessageEventPrefixView": {
            "faction": ("MessageEventPrefix", "faction_"),
            "payload_10": ("MessageEventPrefix", "payload_10_"),
        },
        "MessageBigDataView": {
            "faction": ("MessageBigDataRecord", "faction_"),
        },
        "MessageCommonCommandPrefixView": {
            "faction": ("MessageDataRecord", "faction_"),
            "timestamp_bits": ("MessageDataRecord", "timestamp_"),
            "payload_10": ("MessageDataRecord", "payload_10_"),
            "payload_14": ("MessageDataRecord", "payload_14_"),
        },
    }
    for class_name, methods in schema.items():
        view = extract(source, f"class {class_name} {{")
        pointer_type = "unsigned char *bytes_;" if class_name == "MessageBaseTextView" else "const unsigned char *bytes_;"
        assert pointer_type in view, class_name
        for method, (record, member) in methods.items():
            return_type = "int32_t" if class_name == "MessageAcknowledgementPrefixView" else "int"
            body = extract(view, f"{return_type} {method}() const")
            assert body.count("std::memcpy(") == 1, (class_name, method)
            assert f"offsetof({record}, {member})" in body, (class_name, method)
    big = extract(source, "class MessageBigDataView {")
    word = extract(big, "int data_word(int index) const")
    assert word.count("std::memcpy(") == 1
    assert "offsetof(MessageBigDataRecord, data_)" in word and "index * sizeof(int)" in word
    base = extract(source, "class MessageBaseTextView {")
    assert "return reinterpret_cast<char *>(bytes_ + offsetof(MessageBaseTextRecord, text_));" in base
    command = extract(source, "class MessageCommandView {")
    byte_method = extract(command, "int8_t payload_18_byte() const")
    assert "offsetof(MessageDataRecord, payload_18_)" in byte_method
    assert "std::memcpy(" in byte_method
    lock = extract(source, "class MessageLockView {")
    lock_byte = extract(lock, "unsigned char payload_18_byte() const")
    assert _cpp_tokens(lock_byte) == _cpp_tokens(
        "unsigned char payload_18_byte() const { unsigned char value; "
        "std::memcpy(&value, bytes_ + offsetof(MessageLockRecord, field_18_), "
        "sizeof value); return value; }")
    assert "explicit MessageCommandView(const unsigned char *bytes)" in command
    assert "explicit MessageCommandView(const char *bytes)" in command
    for class_name in ("MessageVehicleView", "MessageLockView",
                       "MessageAcknowledgementPrefixView", "MessageEventPrefixView",
                       "MessageBigDataView", "MessageCommonCommandPrefixView"):
        assert f"explicit {class_name}(const unsigned char *bytes)" in extract(source, f"class {class_name} {{")


VIEW_HARNESS = r'''#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
static const unsigned char *read_base;
static int read_count;
static size_t read_offsets[64], read_widths[64];
static void observed_read(void *to, const void *from, size_t width) {
    if (read_count < 64) {
        read_offsets[read_count] = (const unsigned char *)from - read_base;
        read_widths[read_count] = width;
    }
    ++read_count;
    std::memcpy(to, from, width);
}
@@ACTUAL_TYPES@@
static bool require(bool ok, const char *label) {
    if (!ok) std::fprintf(stderr, "command-wire: %s\n", label);
    return ok;
}
static void put32(unsigned char *p, size_t at, uint32_t value) {
    std::memcpy(p + at, &value, sizeof value);
}
int main() {
    unsigned char storage[0xB2] = {};
    unsigned char *p = storage + 1;
    read_base = p;
    const uint32_t values[] = {0xA1B2C3D4u, 0x81234567u, 0xFFFFFFFEu,
                               0x76543210u, 0x89ABCDEFu, 0xFEDCBA98u,
                               0x13579BDFu, 0x2468ACE0u};
    put32(p, 4, values[0]); put32(p, 8, values[1]);
    put32(p, 12, values[2]); put32(p, 16, values[3]);
    put32(p, 20, values[4]); put32(p, 24, values[5]);
    put32(p, 28, values[6]); put32(p, 32, values[7]);
    put32(p, 36, 0x65E6F708u);
    MessageCommandView command(p);
    read_count = 0;
    if (!require((uint32_t)command.faction() == values[0], "command-faction")) return 98;
    if (!require((uint32_t)command.timestamp_bits() == values[1], "command-clock")) return 98;
    if (!require(command.sequence() == values[2], "command-sequence")) return 98;
    if (!require((uint32_t)command.payload_10() == values[3], "command-payload10")) return 98;
    if (!require((uint32_t)command.payload_14() == values[4], "command-payload14")) return 98;
    if (!require((uint32_t)command.payload_18() == values[5], "command-payload18")) return 98;
    if (!require((uint32_t)command.payload_1C() == values[6], "command-payload1c")) return 98;
    if (!require(read_count == 7, "command-one-field-reads")) return 98;
    const size_t command_offsets[] = {4,8,12,16,20,24,28};
    for (int i=0;i<7;++i) if (!require(read_offsets[i] == command_offsets[i] &&
                                        read_widths[i] == 4, "command-unaligned-extents")) return 98;

    // The complete 0x2c allocation supports base text; text is a borrowed
    // pointer to +0x14, not a pointer-valued serialized field.
    put32(p, 0x10, values[7]);
    std::memcpy(p + 0x14, "native-text", 12);
    MessageBaseTextView base(p);
    if (!require((uint32_t)base.payload_10() == values[7], "base-payload")) return 98;
    if (!require(base.text() == (char *)(p + 0x14) &&
                 std::strcmp(base.text(), "native-text") == 0, "base-text-borrow")) return 98;

    put32(p, 0x10, values[7]); put32(p, 0x14, values[0]);
    put32(p, 0x18, values[5]);
    put32(p, 0x1C, values[6]); put32(p, 0x20, values[7]);
    put32(p, 0x24, 0x65E6F708u);
    MessageVehicleView vehicle(p);
    if (!require((uint32_t)vehicle.payload_10() == values[7] &&
                 (uint32_t)vehicle.payload_14() == values[0], "vehicle-fields")) return 98;
    MessageCommonCommandPrefixView common(p);
    if (!require((uint32_t)common.payload_10() == values[7] &&
                 (uint32_t)common.payload_14() == values[0], "shared-prefix")) return 98;
    MessageAcknowledgementPrefixView ack(p);
    if (!require((uint32_t)ack.sequence() == values[2] &&
                 (uint32_t)ack.player_index() == values[7], "ack-prefix")) return 98;
    MessageEventPrefixView event(p);
    if (!require((uint32_t)event.faction() == values[0] &&
                 (uint32_t)event.payload_10() == values[7], "event-prefix")) return 98;
    MessageBigDataView big(p);
    if (!require((uint32_t)big.data_word(0) == values[7] &&
                 (uint32_t)big.data_word(1) == values[0], "big-data-words")) return 98;

    MessageLockView lock(p);
    read_count = 0;
    if (!require((uint32_t)lock.payload_10() == values[7] &&
                 (uint32_t)lock.payload_14() == values[0] &&
                 (uint32_t)lock.payload_18() == values[5] &&
                 (uint32_t)lock.payload_1C() == values[6] &&
                 (uint32_t)lock.payload_20() == values[7] &&
                 lock.payload_24() == 0x65E6F708 &&
                 lock.payload_18_byte() == (unsigned char)values[5], "lock-payloads")) return 98;
    const size_t lock_offsets[] = {0x10,0x14,0x18,0x1C,0x20,0x24,0x18};
    const size_t lock_widths[] = {4,4,4,4,4,4,1};
    if (!require(read_count == 7, "lock-one-field-extents")) return 98;
    for (int i=0;i<7;++i) if (!require(read_offsets[i] == lock_offsets[i] &&
                                        read_widths[i] == lock_widths[i],
                                        "lock-one-field-extents")) return 98;

    // The original reads +0x18 from shared receive scratch although vehicle
    // sends only 0x18 bytes. This larger host allocation is a safe access-shape
    // witness, not a serialized member or server-side length extension.
    put32(p, 0x18, 0xCAFEBABEu);
    read_count = 0;
    if (!require((uint32_t)netdaemon_read_unserialized_table_g_y(p) == 0xCAFEBABEu,
                 "unserialized-tail-value")) return 98;
    if (!require(read_count == 1 && read_offsets[0] == 0x18 &&
                 read_widths[0] == 4, "unserialized-tail-extent")) return 98;

    // Fresh accessor call after a callback-like mutation proves the view does
    // not cache payloads. +0x18 here is allocated scratch, not a record member.
    put32(p, 0x10, 0x10203040u);
    if (!require((uint32_t)command.payload_10() == 0x10203040u, "fresh-field-read")) return 98;
    return 0;
}
'''


def actual_types(source):
    markers = (
        "struct MessageVehicleRecord {", "struct MessageBaseTextRecord {",
        "struct MessageDataRecord {", "class MessageCommandView {",
        "class MessageCommonCommandPrefixView {", "class MessageVehicleView {",
        "class MessageBaseTextView {", "struct MessageAcknowledgementPrefix {",
        "class MessageAcknowledgementPrefixView {", "struct MessageEventPrefix {",
        "class MessageEventPrefixView {", "struct MessageBigDataRecord {",
        "class MessageBigDataView {", "struct MessageLockRecord {",
        "class MessageLockView {",
    )
    types = "\n".join(extract(source, marker) + ";" for marker in markers)
    helper = extract(source, "static int netdaemon_read_unserialized_table_g_y(")
    return (types + "\n" + helper).replace("std::memcpy(", "observed_read(")


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_borrowed_wire_views_unaligned_and_field_specific(tmp_path, compiler):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    source = SOURCE.read_text()
    for token in ("class MessageLockView", "class MessageEventPrefixView",
                  "netdaemon_read_unserialized_table_g_y"):
        assert token in source, token
    generated = VIEW_HARNESS.replace("@@ACTUAL_TYPES@@", actual_types(source))
    cpp = tmp_path / "command_wire.cpp"
    exe = tmp_path / "command_wire"
    cpp.write_text(generated)
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("damage,expected", [
    ("wrong-command-offset", "command-wire: command-payload10"),
    ("wrong-lock-offset", "command-wire: lock-payloads"),
    ("wrong-lock-byte-width", "command-wire: lock-one-field-extents"),
    ("normalized-scratch-y", "command-wire: unserialized-tail-value"),
])
def test_named_compile_success_wire_damage_controls_98(tmp_path, damage, expected):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    code = actual_types(SOURCE.read_text())
    edits = {
        "wrong-command-offset": (
            "offsetof(MessageDataRecord, payload_10_)",
            "offsetof(MessageDataRecord, payload_14_)"),
        "wrong-lock-offset": (
            "offsetof(MessageLockRecord, field_18_)",
            "offsetof(MessageLockRecord, field_14_)"),
        "wrong-lock-byte-width": (
            "unsigned char payload_18_byte() const {\n"
            "        unsigned char value;\n",
            "unsigned char payload_18_byte() const {\n"
            "        int value;\n"),
        "normalized-scratch-y": ("message + 0x18", "message + 0x14"),
    }
    before, after = edits[damage]
    assert code.count(before) >= 1, (damage, code.count(before))
    code = code.replace(before, after, 1)
    cpp = tmp_path / (damage + ".cpp")
    exe = tmp_path / damage
    cpp.write_text(VIEW_HARNESS.replace("@@ACTUAL_TYPES@@", code))
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 98, result.stdout + result.stderr
    assert expected in result.stderr, result.stderr


def test_task_source_covers_proven_command_variants_and_unserialized_tail():
    source = SOURCE.read_text()
    for name in ("MessageVehicleRecord", "MessageBaseTextRecord", "MessageLockRecord",
                 "MessageAcknowledgementPrefix", "MessageEventPrefix",
                 "MessageBigDataRecord", "MessageCommandView",
                 "MessageCommonCommandPrefixView", "MessageVehicleView",
                 "MessageBaseTextView", "MessageLockView", "MessageEventPrefixView",
                 "MessageBigDataView"):
        assert f"struct {name}" in source or f"class {name}" in source, name
    assert "static int netdaemon_table_g_word" not in source
    assert "netdaemon_read_unserialized_table_g_y" in source
    g = extract(source, "static bool netdaemon_process_table_g(")
    for op in ("0x4405", "0x4406", "0x4407", "0x4408", "0x4409",
               "0x4410", "0x4411", "0x4412", "0x4413", "0x4414", "0x4415",
               "0x4416", "0x4417", "0x4418"):
        assert op in g
    assert "packet.payload_18()" not in g
    assert "netdaemon_read_unserialized_table_g_y(msg)" in g
    for handler, views in (
        ("static void netdaemon_complete_local_turn_wait(", ("MessageCommandView", "packet.payload_10()")),
        ("static void netdaemon_handle_server_upkeep_info(", ("MessageCommandView", "packet.payload_10()")),
        ("static bool netdaemon_process_table_e(", ("MessageAcknowledgementPrefixView", "MessageLockView")),
        ("static bool netdaemon_handle_table_f(", ("MessageEventPrefixView", "MessageBigDataView")),
        ("static bool netdaemon_handle_diplo_cd_message(", ("MessageCommandView",)),
        ("static bool netdaemon_handle_direct_diplomacy_complete(", ("MessageCommandView",)),
    ):
        body = extract(source, handler)
        for view in views:
            assert view in body, (handler, view)
        assert "reinterpret_cast<int *>(message + 0x" not in body, handler


def test_pinned_wire_producers_and_fixed_receive_scratch_anchor():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    spans = ((0x592E10, 0x592E70), (0x592E70, 0x592EDA),
             (0x593090, 0x593108), (0x537F7A, 0x537FC6))
    decoded = {}
    for start, end in spans:
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert instructions and instructions[0].address == start
        assert instructions[-1].address + instructions[-1].size == end
        decoded.update({i.address: (i.mnemonic, i.op_str.lower()) for i in instructions})
    assert decoded[0x592E13] == ("sub", "esp, 0x18")
    assert decoded[0x592E30] == ("mov", "edx, dword ptr [0x93e8bc]")
    assert decoded[0x592E59] == ("push", "0x18")
    assert decoded[0x592E61] == ("call", "0x532940")
    assert decoded[0x592E73] == ("sub", "esp, 0x2c")
    assert decoded[0x592EA7] == ("call", "0x645460")
    assert decoded[0x593093] == ("sub", "esp, 0x28")
    assert decoded[0x5930F9] == ("call", "0x532940")
    assert decoded[0x537F7A] == ("test", "ch, 0xa0")
    assert decoded[0x537FC3] == ("mov", "dword ptr [ebx + 0xc], eax")

"""Strict source-rooted host controls for the imported NetDaemon synch."""
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

import pytest


HERE = Path(__file__).resolve()
_explicit_root = os.environ.get("OPENSMACX_ROOT")
if _explicit_root:
    ROOT = Path(_explicit_root).resolve()
else:
    ROOT = HERE.parents[2]
    if not (ROOT / "src/netdaemon.cpp").is_file():
        raise RuntimeError("set OPENSMACX_ROOT to an explicit source checkout")

NET = ROOT / "src/netdaemon.cpp"
ALPHA = ROOT / "src/alpha.cpp"
VEH = ROOT / "src/veh.cpp"


def _mask_cpp(text):
    masked = list(text)
    mode = "code"
    i = 0
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if mode == "code":
            if c == "/" and n == "/":
                masked[i] = masked[i + 1] = " "
                i += 2
                mode = "line"
                continue
            if c == "/" and n == "*":
                masked[i] = masked[i + 1] = " "
                i += 2
                mode = "block"
                continue
            if c == '"':
                masked[i] = " "
                mode = "string"
            elif c == "'":
                masked[i] = " "
                mode = "char"
        elif mode == "line":
            if c == "\n":
                mode = "code"
            else:
                masked[i] = " "
        elif mode == "block":
            masked[i] = " "
            if c == "*" and n == "/":
                masked[i + 1] = " "
                i += 2
                mode = "code"
                continue
        else:
            masked[i] = " "
            if c == "\\" and i + 1 < len(text):
                masked[i + 1] = " "
                i += 2
                continue
            if (mode == "string" and c == '"') or (mode == "char" and c == "'"):
                mode = "code"
        i += 1
    return "".join(masked)


def _definition(text, signature, label):
    code = _mask_cpp(text)
    matches = list(re.finditer(signature + r"\s*\(", code))
    definitions = []
    for match in matches:
        depth = 1
        cursor = match.end()
        while cursor < len(code) and depth:
            depth += (code[cursor] == "(") - (code[cursor] == ")")
            cursor += 1
        if depth:
            continue
        while cursor < len(code) and code[cursor].isspace():
            cursor += 1
        if cursor < len(code) and code[cursor] == "{":
            definitions.append((match, cursor))
    assert len(definitions) == 1, f"expected one source-owned {label} definition, got {len(definitions)}"
    match, opening = definitions[0]
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            body = text[match.start():end + 1]
            assert "BODY GOES HERE" not in body, f"unfinished source sentinel in {label}"
            return body
    raise AssertionError(f"unterminated source definition: {label}")


def _plain_definition(text, signature, label):
    return _definition(text, signature, label)


def _struct_definition(text, struct_name):
    code = _mask_cpp(text)
    pattern = r"struct\s+" + re.escape(struct_name) + r"\s*\{"
    matches = list(re.finditer(pattern, code))
    assert len(matches) == 1, f"expected actual {struct_name} declaration"
    match = matches[0]
    opening = code.find("{", match.start())
    depth = 0
    for end in range(opening, len(code)):
        depth += (code[end] == "{") - (code[end] == "}")
        if depth == 0:
            return text[match.start():end + 1] + ";"
    raise AssertionError(f"unterminated {struct_name}")


def _templated_function(text, signature, label):
    function = _definition(text, signature, label)
    start = text.rfind("template", 0, text.index(function))
    if start < 0 or "\n" not in text[start:text.index(function)]:
        raise AssertionError(f"missing template declaration for {label}")
    return text[start:text.index(function)] + function


def _map_storage_extract():
    header = (ROOT / "src/map.h").read_text()
    declarations = [_struct_definition(header, name) for name in
                    ("Map", "Landmark", "MapMetadataFields", "MapStorageWindow")]
    aliases = re.findall(r"^#define Map\w+ \(MapStorage\.[^\n]+", header, re.M)
    assert len(aliases) == 23, "extract every actual MapStorage member alias"
    count, = re.findall(r"^static const int MaxLandmarkNum = \d+;", header, re.M)
    return (count + "\n" +
            "\n".join(declarations) + "\nMapStorageWindow MapStorage = {};\n" +
            "\n".join(aliases) + "\n")


def _must_source_is_imported():
    net = NET.read_text()
    alpha = ALPHA.read_text()
    # Strict product-source precondition. Never substitute the /tmp proposal.
    core = _definition(net, r"void\s+NetDaemon::synch", "NetDaemon::synch")
    nonrule = _definition(net, r"bool\s+copy_nonrule_chunk", "copy_nonrule_chunk")
    rule_copy = _definition(alpha, r"bool\s+copy_rule_wire_chunk", "copy_rule_wire_chunk")
    rule_commit = _definition(alpha, r"bool\s+commit_staged_rules", "commit_staged_rules")
    assert "NETDAEMON_SYNCH_HOST_STUBS" not in core
    assert "NETDAEMON_SYNCH_HOST_STUBS" not in net
    assert "NETDAEMON_SYNCH_HOST_STUBS" not in alpha
    assert "#include \"/tmp/" not in net
    assert "#include \"/tmp/" not in alpha
    assert "message_data(0x2308, 0, 0, 0, 0, 0)" in net
    assert 'X_pop("VEHSYNCH", 0)' in net
    assert "if (!ConsoleGroupAction) GetAsyncKeyState(VK_CONTROL);" in net
    return net, alpha, core, nonrule, rule_copy, rule_commit


def _compiler_names():
    requested = os.environ.get("NETDAEMON_HOST_CXX")
    names = [requested] if requested else ["g++", "clang++"]
    return [name for name in names if name and shutil.which(name)]


def _compile_run(tmp_path, compiler, harness, inject):
    source = harness.read_text()
    source = source.replace("/* SOURCE EXTRACTS */", inject)
    cpp = tmp_path / "source-rooted.cpp"
    exe = tmp_path / "source-rooted"
    cpp.write_text(source)
    command = [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
               "-pedantic", "-fsanitize=address,undefined",
               "-fno-omit-frame-pointer", "-I", str(ROOT), str(cpp), "-o", str(exe)]
    built = subprocess.run(command, capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    # Keep overlap/undefined-behavior detection active, but avoid external
    # symbolizer/slow unwinding during deliberately failing mutation controls.
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0:symbolize=0:fast_unwind_on_fatal=1:abort_on_error=1",
               UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(exe)], capture_output=True, text=True,
                          timeout=20, env=env)


def _compile_injected(tmp_path, compiler, harness, inject, expected=True):
    run = _compile_run(tmp_path, compiler, harness, inject)
    if expected:
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0, "deliberate source-derived mutation survived"
    return run


def _core_extract(net):
    struct = ("namespace NetDaemonSyncWire {\n" +
        _struct_definition(net, "MapRouteCapture") + "\n" +
        "bool compare_and_notify(int16_t, int, int, int, const void *, const void *, const void *, int, int);\n"
        "static bool native_default_copy(uint32_t, unsigned char *, uint32_t, bool);\n} ")
    notification_views = ("namespace NetDaemonSyncWire {\n" +
        _plain_definition(net, r"static\s+const\s+unsigned\s+char\s*\*\s*native_notification_view",
                          "native notification view") + "\n" +
        _plain_definition(net, r"static\s+const\s+unsigned\s+char\s*\*\s*native_notification_owner_view",
                          "nominal notification owner view") + "\n}")
    pieces = [
        struct,
        "namespace NetDaemonSyncWire {\n#define memmove observe_default_memmove\n#define memcpy observe_default_memmove\n" +
        _definition(net, r"static\s+bool\s+native_default_copy", "native default byte copy") +
        "\n#undef memcpy\n#undef memmove\n}\n",
        _plain_definition(net, r"static\s+int\s+native_map_sync_rows", "native map rows"),
        _plain_definition(net, r"static\s+NetDaemonSyncWire::MapRouteCapture\s+capture_map_route", "map route capture"),
        _plain_definition(net, r"static\s+int\s+native_sync_signed_word", "signed native word"),
        _plain_definition(net, r"static\s+void\s+synch_notify_or_queue", "notify helper"),
        _plain_definition(net, r"static\s+void\s+synch_debug_popup", "debug popup helper"),
        notification_views,
        _definition(net, r"bool\s+NetDaemonSyncWire::compare_and_notify", "compare-and-notify"),
        _plain_definition(net, r"static\s+int\s+netdaemon_sync_payload_size", "payload size router"),
        _plain_definition(net, r"static\s+void\s+netdaemon_sync_send_chunk", "outgoing chunk sender"),
        _definition(net, r"void\s+NetDaemon::synch", "NetDaemon::synch"),
    ]
    return struct, "\n\n".join(pieces[1:])


def _owner_copy_extract(net):
    veh = VEH.read_text()
    funcs = [
        _templated_function(net, r"static\s+uint32_t\s+native_word", "native word"),
        _plain_definition(net, r"static\s+uint32_t\s+sign_extended_char", "signed byte"),
        _definition(net, r"void\s+gather_saved_diplomacy", "saved diplomacy gather"),
        _templated_function(net, r"static\s+void\s+scatter_word", "native word scatter"),
        _definition(net, r"void\s+scatter_saved_diplomacy", "saved diplomacy scatter"),
        _definition(net, r"static\s+bool\s+native_owner_copy", "native owner byte copy"),
        "#define memmove observe_default_memmove\n#define memcpy observe_default_memmove\n" +
        _definition(net, r"static\s+bool\s+native_default_copy", "native default byte copy") +
        "\n#undef memcpy\n#undef memmove\n",
        _definition(net, r"bool\s+copy_nonrule_chunk", "nonrule owner router"),
    ]
    veh_copy = _definition(veh, r"bool\s+veh_bung_copy_chunk", "Veh BungList owner copy")
    global_api = "bool veh_bung_copy_chunk(size_t, unsigned char *, size_t, bool);\n"
    return (global_api + "namespace NetDaemonSyncWire {\n" +
            "\n\n".join(funcs) + "\n}\n" + veh_copy + "\n")


def _assert_contiguous_owner_source_contract(nonrule):
    dispatch = nonrule.find("switch (opcode)")
    split_bounds = nonrule.find("#undef NATIVE_ARRAY")
    assert 0 <= dispatch < split_bounds, "native owner dispatch must precede default pointer copy"
    assert "if (offset > payload_size || count > payload_size - offset)" not in nonrule
    assert "return native_default_copy(chunk, packet, static_cast<uint32_t>(count)," in nonrule
    owner_dispatch = nonrule[dispatch:split_bounds]
    for opcode in ("0x00", "0x04", "0x05", "0x06", "0x07", "0x08", "0x09",
                   "0x0A", "0x0B", "0x0C", "0x0D", "0x0E", "0x0F",
                   "0x10", "0x11", "0x12", "0x13", "0x14", "0x15",
                   "0x16", "0x17", "0x18", "0x19", "0x20", "0x21", "0x22",
                   "0x23", "0x24", "0x25", "0x26", "0x27", "0x29", "0x30"):
        assert f"case {opcode}" in owner_dispatch, f"missing contiguous opcode {opcode}"
    assert "native_owner_copy" in nonrule
    assert "offsetof(PlayerData, energy_reserves)" in owner_dispatch
    assert "offsetof(PlayerData, ai_fight)" in owner_dispatch
    assert "offsetof(PlayerData, unk_37)" in owner_dispatch
    assert "offsetof(PlayerData, saved_queue_name) == 0x470" in nonrule
    assert "offsetof(VehPrototype, obsolete_factions)" in owner_dispatch
    assert "offsetof(Base, resource_sqr_radius)" in owner_dispatch
    assert "NATIVE_COPY_MOV(&ReplayEventBytes)" in owner_dispatch
    assert "NATIVE_COPY_MOV(&NetDaemonFileTransferPath)" in owner_dispatch
    assert "case 0x02: case 0x1002" in owner_dispatch
    assert "NATIVE_COPY(&MapStorage, offsetof(MapStorageWindow, metadata_))" in owner_dispatch
    assert "case 0x02" not in nonrule[split_bounds:]
    assert "case 0x30" not in nonrule[split_bounds:]
    assert "NATIVE_COPY(captured_map_tiles, captured_map_tile_offset)" in owner_dispatch
    assert "captured_map_tile_capacity" not in nonrule
    assert "if (a2 < 0" not in owner_dispatch
    assert "bounded_array_copy" not in owner_dispatch


def _rule_extract(alpha):
    signatures = [
        (r"static\s+bool\s+copy_rule_object_chunk", "typed rule object copy", True),
        (r"bool\s+copy_rule_wire_chunk", "rule wire copy", False),
        (r"void\s+compact_time_controls", "time control compaction", False),
        (r"void\s+compact_facility_table", "facility compaction", False),
        (r"void\s+compact_social_categories", "social compaction", False),
        (r"void\s+compact_technology_fields", "technology compaction", False),
        (r"void\s+compact_terraforming_fields", "terraforming compaction", False),
        (r"void\s+compact_reactor_prerequisites", "reactor compaction", False),
        (r"void\s+compact_chassis_fields", "chassis compaction", False),
        (r"void\s+compact_weapon_fields", "weapon compaction", False),
        (r"void\s+compact_armor_fields", "armor compaction", False),
        (r"void\s+compact_ability_fields", "ability compaction", False),
        (r"void\s+compact_citizen_fields", "citizen compaction", False),
        (r"bool\s+commit_staged_rules", "rule commit", False),
    ]
    pieces = []
    for signature, label, templated in signatures:
        if templated:
            pieces.append(_templated_function(alpha, signature, label))
        else:
            pieces.append(_definition(alpha, signature, label))
    return "namespace NetDaemonSyncRules {\n" + "\n\n".join(pieces) + "\n}\n"


def _rule_declarations_extract():
    # Actual source declarations, with only LPSTR represented as a 32-bit
    # host fixture word. This is not native compiler/ABI evidence.
    groups = {
        "alpha.h": ("RulesBasic", "RulesStorageWindow", "RulesWorldbuilder",
                    "RulesResourceinfo", "RulesTimeControl"),
        "technology.h": ("RulesTechnology",),
        "terraforming.h": ("RulesTerraforming",),
        "veh.h": ("RulesChassis", "RulesReactor", "RulesWeapon", "RulesArmor", "RulesAbility"),
        "base.h": ("RulesFacility", "RulesCitizen"),
        "faction.h": ("SocialEffect", "RulesSocialCategory"),
    }
    pieces = []
    for header, names in groups.items():
        source = (ROOT / "src" / header).read_text()
        pieces.extend(_struct_definition(source, name) for name in names)
    alpha_h = (ROOT / "src/alpha.h").read_text()
    for name in ("TechValidCount", "TechCommerceCount", "Rules"):
        match = re.search(r"^#define " + name + r" .*", alpha_h, re.MULTILINE)
        assert match, name
        pieces.append(match.group(0))
    return "\n\n".join(pieces)


def test_actual_product_definitions_are_selected_not_tmp_proposals():
    net, alpha, core, nonrule, rule_copy, rule_commit = _must_source_is_imported()
    assert "NetDaemonSyncWire::copy_out" in net
    assert "NetDaemonSyncWire::copy_in" in net
    assert "opcode0_copy_chunk" not in net
    assert "NATIVE_COPY(&GameDataStateStorage, 0)" in nonrule
    game_window_h = (ROOT / "src/game_data_state_window.h").read_text()
    assert "game_wire_opaque_9a67c9_" in game_window_h
    assert "veh_bung_copy_chunk" in nonrule
    assert "copy_rule_wire_chunk" in rule_copy
    assert "commit_staged_rules" in rule_commit
    assert (ROOT / "src/game.cpp").is_file()
    game = (ROOT / "src/game.cpp").read_text()
    game_h = (ROOT / "src/game.h").read_text()
    assert re.search(r"uint8_t wire_opaque_94b4d8\[0x80\];", game_h)
    assert re.search(r"^char NetDaemonFileTransferPath\[0x104\];$", net, re.MULTILINE)
    assert re.search(r"^static unsigned char NetDaemonRulesSyncStaging\[0x1920\];$",
                     net, re.MULTILINE)
    assert "memset(NetDaemonRulesSyncStaging" not in net


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_synch_method_source_with_callback_damage_controls(tmp_path, compiler):
    net, _, _, _, _, _ = _must_source_is_imported()
    template = HERE.with_name("netdaemon_synch_core_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_core_harness.cpp beside this test before importing it"
    interface, injection = _core_extract(net)
    harness = (template.read_text().replace("/* SOURCE INTERFACE */", interface)
               .replace("/* MAP STORAGE DECLARATIONS */", _map_storage_extract()))
    source_template = tmp_path / "core-template.cpp"
    source_template.write_text(harness)
    _compile_injected(tmp_path, compiler, source_template, injection)

    # Damage controls operate on the extracted product definition itself.
    for old, new in [
        ("total_chunks == 1 ? 0 :", "total_chunks != 1 ? 0 :"),
        ("bytes + 0x24,", "bytes + 0x20,"),
        ("(opcode >= 0x85 && opcode <= 0x8D))",
         "(opcode >= 0x85 && opcode <= 0x8D && false))"),
        ("if (rules_notify && !scripted_turn)",
         "if (rules_notify && scripted_turn)"),
        ("message_data(0x2308, 0, 0, 0, 0, 0)",
         "message_data(0, 0, 0, 0, 0, 0x2308)"),
        ("if (!ConsoleGroupAction) GetAsyncKeyState(VK_CONTROL);",
         "if (ConsoleGroupAction) GetAsyncKeyState(VK_CONTROL);"),
        ("map_strip_quotient = q;", "map_strip_quotient = a2 == 0xF ? latitude - q * 15 : q;"),
        ("opcode == 0x8D && rules_phase == 4", "opcode == 0x8D"),
        ("do_all_draws(); rules_phase = 1;", "rules_phase = 1; do_all_draws();"),
        ("const int quotient = size / 0x800;", "if (size < 0) return;\n    const int quotient = size / 0x800;"),
        ("remainder != 0 ? 1 : 0", "remainder > 0 ? 1 : 0"),
        ("if (size == -1 && (opcode == 0x12 || opcode == 0x14)) return;",
         "if (size == -1 && false && (opcode == 0x12 || opcode == 0x14)) return;"),
        ("native_sync_signed_word(static_cast<uint32_t>(a3) * 52u)", "a3 * 52"),
        ("static_cast<uint32_t>(payload_length), map_capture,", "static_cast<size_t>(payload_length), map_capture,"),
        ("const uint32_t packet_length = static_cast<uint32_t>(payload_length) + 0x24u;",
         "const size_t packet_length = static_cast<size_t>(payload_length + 0x24);"),
        ("const int current_length = opcode == 0x16 ? 0x48 :",
         "const int current_length = opcode == 0x16 && false ? 0x48 :"),
        ("const size_t current_offset = opcode == 0x16 ? 0 :",
         "const size_t current_offset = opcode == 0x16 && false ? 0 :"),
        ("static_cast<uintptr_t>(captured_address)",
         "static_cast<uintptr_t>(captured_address & 0u)"),
        ("static_cast<uintptr_t>(captured_address)",
         "static_cast<uintptr_t>(static_cast<int32_t>(captured_address))"),
        ("if (packet_to_owner) memmove(address, packet, count);",
         "if (count & 0x80000000u) return false;\n"
         "    if (packet_to_owner) memmove(address, packet, count);"),
        ("if (packet_to_owner) memmove(address, packet, count);",
         "if (packet_to_owner) memmove(packet, address, count);"),
        ("static_cast<uint32_t>(map_strip_quotient) *",
         "(static_cast<uint32_t>(MapLatitudeBounds / 16) + "
         "0u * static_cast<uint32_t>(map_strip_quotient)) *"),
    ]:
        assert old in injection
        _compile_injected(tmp_path, compiler, source_template,
                          injection.replace(old, new, 1), expected=False)
    stale_notify_gate = injection.replace(
        "if (total_chunks == 1 &&", "if (current_length == size &&", 1)
    assert stale_notify_gate != injection
    _compile_injected(tmp_path, compiler, source_template,
                      stale_notify_gate, expected=False)
    nominal_record_notify_view = injection.replace(
        "uint32_t native_offset = static_cast<uint32_t>(offset);",
        "uint32_t native_offset = 0; (void)offset;",
        1)
    assert nominal_record_notify_view != injection
    _compile_injected(tmp_path, compiler, source_template,
                      nominal_record_notify_view, expected=False)
    selected_faction_substitute = injection.replace(
        "owner_bytes + offsetof(Veh, faction_id)",
        "current_bytes + offsetof(Veh, faction_id)", 1)
    assert selected_faction_substitute != injection
    _compile_injected(tmp_path, compiler, source_template,
                      selected_faction_substitute, expected=False)
    selected_proto_substitute = injection.replace(
        "owner_bytes + offsetof(Veh, proto_id)",
        "current_bytes + offsetof(Veh, proto_id)", 1)
    assert selected_proto_substitute != injection
    _compile_injected(tmp_path, compiler, source_template,
                      selected_proto_substitute, expected=False)


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_nonrule_owner_source_routes(tmp_path, compiler):
    net, _, _, nonrule, _, _ = _must_source_is_imported()
    _assert_contiguous_owner_source_contract(nonrule)
    template = HERE.with_name("netdaemon_synch_owner_copy_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_owner_harness.cpp beside this test before importing it"
    harness = template.read_text().replace("/* MAP STORAGE DECLARATIONS */", _map_storage_extract())
    source_template = tmp_path / "owner-template.cpp"
    source_template.write_text(harness)
    injection = _owner_copy_extract(net)
    _compile_injected(tmp_path, compiler, source_template, injection)
    old = "NATIVE_COPY(&GameDataStateStorage, 0);"
    assert old in injection
    _compile_injected(tmp_path, compiler, source_template,
                      injection.replace(old, "NATIVE_COPY(&GameDataStateStorage, 4);", 1),
                      expected=False)
    offset_guard = injection.replace(
        "const uint32_t chunk = static_cast<uint32_t>(offset);",
        "const uint32_t chunk = static_cast<uint32_t>(offset);\n    if (offset > payload_size || count > payload_size - offset) return false;",
        1)
    assert offset_guard != injection
    _compile_injected(tmp_path, compiler, source_template, offset_guard,
                      expected=False)
    record_intersection = injection.replace(
        "const uint32_t chunk = static_cast<uint32_t>(offset);",
        "const uint32_t chunk = static_cast<uint32_t>(offset);\n    if (offset >= 0x59C) return false;",
        1)
    assert record_intersection != injection
    _compile_injected(tmp_path, compiler, source_template, record_intersection,
                      expected=False)
    memcpy_substitute = injection.replace("memmove(", "memcpy(")
    assert memcpy_substitute != injection
    overlap_failure = _compile_injected(
        tmp_path, compiler, source_template, memcpy_substitute, expected=False)
    assert "memcpy-param-overlap" in overlap_failure.stderr
    map_route = "NATIVE_COPY(&MapStorage, offsetof(MapStorageWindow, metadata_));"
    assert map_route in injection
    for replacement in (
        "if (offset > payload_size || count > payload_size - offset) return false;\n        " + map_route,
        "if (offset + count > sizeof(MapStorage.metadata_)) return false;\n        " + map_route,
        "native_owner_copy(&MapStorage.metadata_, chunk + 4, packet, count, packet_to_owner);",
    ):
        _compile_injected(tmp_path, compiler, source_template,
                          injection.replace(map_route, replacement, 1), expected=False)
    for old, new in (
        ("return native_default_copy(chunk, packet, static_cast<uint32_t>(count),",
         "return native_default_copy(static_cast<uint32_t>(a2), packet, static_cast<uint32_t>(count),"),
        ("return native_default_copy(chunk, packet, static_cast<uint32_t>(count),",
         "if (count != 0) return false;\n"
         "    return native_default_copy(chunk, packet, static_cast<uint32_t>(count),"),
        ("native_owner_copy(diplomacy_wire, 0, packet, 0x48,",
         "native_owner_copy(diplomacy_wire, chunk, packet, 0x48,"),
        ("native_owner_copy(diplomacy_wire, 0, packet, 0x48,",
         "native_owner_copy(diplomacy_wire, 0, packet, count,"),
        ("NATIVE_COPY(captured_map_tiles, 0);",
         "NATIVE_COPY(MapTiles, 0);"),
        ("NATIVE_COPY(captured_map_tiles, captured_map_tile_offset);",
         "NATIVE_COPY(MapTiles, captured_map_tile_offset);"),
        ("NATIVE_COPY(captured_map_tiles, captured_map_tile_offset);",
         "if (offset + count > payload_size) return false; "
         "NATIVE_COPY(captured_map_tiles, captured_map_tile_offset);"),
    ):
        assert old in injection
        _compile_injected(tmp_path, compiler, source_template,
                          injection.replace(old, new, 1), expected=False)


def test_pinned_contiguous_size_and_address_dispatch_paths():
    from hashlib import sha256
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_override = os.environ.get("OPENSMACX_IMAGE")
    image_path = (Path(image_override).resolve() if image_override else
                  ROOT / ".opensmacx/game/terranx_original.exe")
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable")
    image_bytes = image_path.read_bytes()
    digest = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(image_bytes).hexdigest() == digest
    image = PeImage(image_bytes, digest)
    try:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        pytest.skip("Capstone unavailable for bounded i386 instruction decoding")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    contiguous = {
        0x00: (0x532E55, 0x533454), 0x01: (0x532F93, 0x53345F),
        0x02: (0x533342, 0x533A71),
        0x04: (0x532EBC, 0x533480),
        0x05: (0x532EFF, 0x53348B), 0x06: (0x532F38, 0x53372A),
        0x07: (0x532F71, 0x533496), 0x08: (0x5330C0, 0x5334AD),
        0x09: (0x53334C, 0x5334C9), 0x0A: (0x532EB2, 0x5334E5),
        0x0B: (0x532EB2, 0x533501), 0x0C: (0x53334C, 0x53351D),
        0x0D: (0x5330C0, 0x533539), 0x0E: (0x532F7B, 0x533555),
        0x0F: (0x532F85, 0x533567), 0x10: (0x532F93, 0x533579),
        0x11: (0x532F9D, 0x53358B), 0x12: (0x532FFD, 0x5335B5),
        0x13: (0x533048, 0x5335C0), 0x14: (0x5330A1, 0x5335ED),
        0x15: (0x532E99, 0x53346A), 0x17: (0x532EB2, 0x53372A),
        0x18: (0x5330D4, 0x533746), 0x19: (0x5330DE, 0x533762),
        0x20: (0x533117, 0x53376D), 0x21: (0x533125, 0x533778),
        0x22: (0x533166, 0x533782), 0x23: (0x5330C0, 0x5335F8),
        0x24: (0x532E8F, 0x533475), 0x25: (0x532EB2, 0x53378D),
        0x26: (0x532EB2, 0x533797), 0x27: (0x533178, 0x5337AB),
        0x28: (0x533171, 0x5337A1), 0x29: (0x533182, 0x5337B5),
    }
    for opcode, (size_target_expected, address_target_expected) in contiguous.items():
        size_target, = struct.unpack(
            "<I", image.read(Span(0x53412C + opcode * 4,
                                  0x534130 + opcode * 4)))
        address_target, = struct.unpack(
            "<I", image.read(Span(0x53420C + opcode * 4,
                                  0x534210 + opcode * 4)))
        assert (size_target, address_target) == (
            size_target_expected, address_target_expected), hex(opcode)

    # 0x1002 bypasses both low-opcode tables and selects opcode 2's exact
    # payload/address routes. Their ADD retains the incoming chunk displacement.
    metadata_pins = {
        0x5331C8: ("cmp", "eax, 0x1002"),
        0x5331D3: ("je", "0x533342"),
        0x533342: ("mov", "esi, 0xa9c"),
        0x533347: ("jmp", "0x53334c"),
        0x5337E6: ("cmp", "eax, 0x1002"),
        0x5337F1: ("je", "0x533a71"),
        0x533A71: ("add", "esi, 0x949870"),
    }
    metadata_instructions = {
        item.address: (item.mnemonic, item.op_str.lower())
        for start, end in ((0x5331C8, 0x5331D9), (0x533342, 0x53334C),
                           (0x5337E6, 0x5337F7), (0x533A71, 0x533A77))
        for item in decoder.disasm(image.read(Span(start, end)), start)
    }
    for address, expected in metadata_pins.items():
        assert metadata_instructions[address] == expected

    map_pins = {
        0x532E3F: ("je", "0x53318c"),
        0x532EEB: ("mov", "esi, dword ptr [0x949884]"),
        0x532EF7: ("shl", "esi, 2"),
        0x53318C: ("mov", "ecx, dword ptr [0x949874]"),
        0x53319A: ("sar", "eax, 4"),
        0x5331A0: ("mov", "dword ptr [ebp - 0x34], eax"),
        0x5331AB: ("sub", "ecx, eax"),
        0x5331B5: ("sar", "esi, 1"),
        0x5331B7: ("imul", "esi, eax"),
        0x5331C0: ("shl", "esi, 2"),
        0x533347: ("jmp", "0x53334c"),
        0x533357: ("sar", "eax, 0xb"),
        0x533362: ("sub", "esi, edx"),
        0x533369: ("inc", "eax"),
        0x533376: ("jle", "0x533f95"),
        0x53343E: ("je", "0x5337c0"),
        0x533480: ("add", "esi, dword ptr [0x94a30c]"),
        0x5337C0: ("mov", "ecx, dword ptr [ebp - 0x34]"),
        0x5337C3: ("mov", "eax, dword ptr [0x68faf0]"),
        0x5337C8: ("imul", "ecx, ebx"),
        0x5337CB: ("imul", "eax, ecx"),
        0x5337D7: ("mov", "eax, dword ptr [0x94a30c]"),
        0x5337DC: ("lea", "ecx, [eax + edx*4]"),
        0x5337DF: ("add", "esi, ecx"),
    }
    map_instructions = {
        item.address: (item.mnemonic, item.op_str.lower())
        for start, end in ((0x532E2F, 0x532E4E), (0x532EEB, 0x532EFF),
                           (0x53318C, 0x5331C8), (0x533342, 0x53337C),
                           (0x533431, 0x533454), (0x533480, 0x53348B),
                           (0x5337C0, 0x5337E6))
        for item in decoder.disasm(image.read(Span(start, end)), start)
    }
    for address, expected in map_pins.items():
        assert map_instructions[address] == expected
    for start, end in ((0x533480, 0x53348B), (0x5337C0, 0x5337E6)):
        route = tuple(decoder.disasm(image.read(Span(start, end)), start))
        assert route[-1].mnemonic == "jmp" and route[-1].op_str == "0x533a77"
        assert not any(i.mnemonic.startswith("j") and i.mnemonic != "jmp" for i in route)

    # The native address table has both additive views and scalar MOV routes.
    # Do not turn those scalar routes into per-index array offsets.
    for address, expected in ((0x533778, ("mov", "esi, 0x9469f8")),
                              (0x53378D, ("mov", "esi, 0x9a64c8")),
                              (0x533797, ("mov", "esi, 0x9a64cc")),
                              (0x5337A1, ("mov", "esi, 0x9b86a0")),
                              (0x5337AB, ("mov", "esi, 0x93ac88")),
                              (0x533454, ("add", "esi, 0x9a6490")),
                              (0x53345F, ("add", "esi, 0x9a64e8"))):
        instruction, = tuple(decoder.disasm(
            image.read(Span(address, address + 8)), address))[:1]
        assert (instruction.mnemonic, instruction.op_str.lower()) == expected

    # Opcode 0x18 starts at PlayerData+0x46C (unk_37), immediately before the
    # named saved-queue array at +0x470. The old saved_queue_name+20 route was
    # not the address table's pointer target.
    queue_path = tuple(decoder.disasm(
        image.read(Span(0x533746, 0x53375D)), 0x533746))
    queue_target = {item.address: (item.mnemonic, item.op_str.lower())
                    for item in queue_path}
    assert queue_target[0x533756] == (
        "lea", "esi, [esi + eax*4 + 0x96ce4c]")
    assert 0x96CE4C - 0x96C9E0 == 0x46C
    vehicle_identity = {item.address: (item.mnemonic, item.op_str.lower())
                        for item in decoder.disasm(
                            image.read(Span(0x53396E, 0x5339A5)), 0x53396E)}
    assert vehicle_identity[0x533979] == (
        "mov", "cl, byte ptr [esi + 0x952836]")
    assert vehicle_identity[0x533987] == (
        "movsx", "edx, word ptr [esi + 0x952832]")
    assert vehicle_identity[0x53399E] == (
        "movsx", "eax, word ptr [esi + 0x952832]")
    assert 0x952836 - 0x952828 == 0x0E
    assert 0x952832 - 0x952828 == 0x0A

    # First dispatch: opcode 6 selects fixed payload size 0x20CC; there is no
    # a2 validation before the common size/chunk derivation.
    size_table_target, = struct.unpack(
        "<I", image.read(Span(0x53412C + 6 * 4, 0x53412C + 7 * 4)))
    assert size_table_target == 0x532F38
    size_path = tuple(decoder.disasm(image.read(Span(size_table_target,
                                                    size_table_target + 0x39)),
                                     size_table_target))
    size_instructions = {item.address: (item.mnemonic, item.op_str.lower())
                         for item in size_path}
    assert size_instructions[0x532F38] == ("test", "ecx, ecx")
    assert size_instructions[0x532F3A] == ("jne", "0x532f67")
    assert size_instructions[0x532F67] == ("mov", "esi, 0x20cc")
    assert size_instructions[0x532F6C] == ("jmp", "0x53334c")
    assert not any("ebx" in item.op_str.lower() or "[ebp + 0xc]" in item.op_str.lower()
                   for item in size_path)

    # Second dispatch: derives the real PlayerData array base plus 0x20CC*a2
    # and reaches common copy without a conditional index/range branch.
    address_table_target, = struct.unpack(
        "<I", image.read(Span(0x53420C + 6 * 4, 0x53420C + 7 * 4)))
    assert address_table_target == 0x53372A
    address_path = tuple(decoder.disasm(image.read(Span(address_table_target,
                                                       0x533746)),
                                        address_table_target))
    assert not any(item.mnemonic.startswith("j") and item.mnemonic != "jmp"
                   for item in address_path)
    assert (address_path[-2].mnemonic, address_path[-2].op_str.lower()) == (
        "lea", "esi, [esi + edx*4 + 0x96c9e0]")
    assert (address_path[-1].mnemonic, address_path[-1].op_str.lower()) == (
        "jmp", "0x533a77")

    # Incoming index is sign-extended/equality-tested only. A nonfinal index
    # shifts by 11 for its byte displacement; both directions enter the common
    # copy path. No out-of-array host address is executed by these controls.
    chunk_path = {item.address: (item.mnemonic, item.op_str.lower()) for item in
        decoder.disasm(image.read(Span(0x53339A, 0x5333DD)), 0x53339A)}
    assert chunk_path[0x5333A0] == ("movsx", "ecx, word ptr [ecx + 0x12]")
    assert chunk_path[0x5333A4] == ("cmp", "ecx, edi")
    assert chunk_path[0x5333A6] == ("jne", "0x5333d1")
    assert chunk_path[0x5333D1] == ("mov", "esi, ecx")
    assert chunk_path[0x5333DA] == ("shl", "esi, 0xb")
    common = tuple(decoder.disasm(image.read(Span(0x533A77, 0x533C45)), 0x533A77))
    assert any(item.address == 0x533A7C and item.mnemonic == "je" and
               item.op_str.lower() == "0x5338e6" for item in common)
    assert any(item.address == 0x533AE3 and item.mnemonic == "call" and
               item.op_str.lower() == "0x645930" for item in common)
    assert any(item.address == 0x533C3F and item.mnemonic == "call" and
               item.op_str.lower() == "0x645930" for item in common)


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_alpha_rule_source_compactions(tmp_path, compiler):
    _, alpha, _, _, _, _ = _must_source_is_imported()
    template = HERE.with_name("netdaemon_synch_rules_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_rules_harness.cpp beside this test before importing it"
    harness = template.read_text().replace("/* RULE DECLARATIONS */", _rule_declarations_extract())
    source_template = tmp_path / "rules-template.cpp"
    source_template.write_text(harness)
    injection = _rule_extract(alpha)
    _compile_injected(tmp_path, compiler, source_template, injection)
    old = "src + 0x04, sizeof dst.turn"
    assert old in injection
    _compile_injected(tmp_path, compiler, source_template,
                      injection.replace(old, "src + 0x08, sizeof dst.turn", 1),
                      expected=False)
    for old, new in (
        ("copy_rule_object_chunk(RulesStorage, 0,", "copy_rule_object_chunk(RulesStorage, static_cast<uint32_t>(offset),"),
        ("copy_rule_object_chunk(WorldBuilder, 0,", "copy_rule_object_chunk(WorldBuilder, static_cast<uint32_t>(offset),"),
        ("copy_rule_object_chunk(ResourceInfo, 0,", "copy_rule_object_chunk(ResourceInfo, static_cast<uint32_t>(offset),"),
        ("const uint32_t chunk_offset = static_cast<uint32_t>(offset);",
         "const uint32_t chunk_offset = static_cast<uint32_t>(offset); if (opcode == 0x86 && offset + count > 0x510) return false;"),
        ("const uint32_t chunk_offset = static_cast<uint32_t>(offset);",
         "const size_t chunk_offset = offset;"),
        ("memmove(", "memcpy("),
        ("case 0x81: return copy_rule_object_chunk(RulesStorage, 0, chunk, count, packet_to_owner);",
         "case 0x81: if (packet_to_owner) { memmove(shared_staging, chunk, count); return true; } return copy_rule_object_chunk(RulesStorage, 0, chunk, count, packet_to_owner);"),
    ):
        assert old in injection
        _compile_injected(tmp_path, compiler, source_template,
                          injection.replace(old, new), expected=False)

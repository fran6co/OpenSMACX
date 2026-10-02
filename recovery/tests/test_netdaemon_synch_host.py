"""Strict source-rooted host controls for the imported NetDaemon synch."""
import os
from pathlib import Path
import re
import shutil
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


def _must_source_is_imported():
    net = NET.read_text()
    alpha = ALPHA.read_text()
    # Strict product-source precondition. Never substitute the /tmp proposal.
    core = _definition(net, r"void\s+NetDaemon::synch", "NetDaemon::synch")
    wire0 = _definition(net, r"bool\s+opcode0_copy_chunk", "opcode0_copy_chunk")
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
    return net, alpha, core, wire0, nonrule, rule_copy, rule_commit


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
               "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)]
    built = subprocess.run(command, capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
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
        "bool compare_and_notify(int16_t, int, int, int, const void *, size_t, int, int);\n} ")
    pieces = [
        struct,
        _plain_definition(net, r"static\s+int\s+native_map_sync_rows", "native map rows"),
        _plain_definition(net, r"static\s+NetDaemonSyncWire::MapRouteCapture\s+capture_map_route", "map route capture"),
        _plain_definition(net, r"static\s+void\s+synch_notify_or_queue", "notify helper"),
        _plain_definition(net, r"static\s+void\s+synch_debug_popup", "debug popup helper"),
        _definition(net, r"bool\s+NetDaemonSyncWire::compare_and_notify", "compare-and-notify"),
        _plain_definition(net, r"static\s+int\s+netdaemon_sync_payload_size", "payload size router"),
        _plain_definition(net, r"static\s+void\s+netdaemon_sync_send_chunk", "outgoing chunk sender"),
        _definition(net, r"void\s+NetDaemon::synch", "NetDaemon::synch"),
    ]
    return struct, "\n\n".join(pieces[1:])


def _wire0_extract(net):
    funcs = [
        _templated_function(net, r"static\s+size_t\s+copy_member_bytes", "member byte copy"),
        _plain_definition(net, r"static\s+bool\s+wire0_range_is_owned", "wire0 preflight"),
        _definition(net, r"bool\s+opcode0_copy_chunk", "opcode0 serializer"),
    ]
    api = "bool opcode0_copy_chunk(size_t, unsigned char *, size_t, bool);\nbool veh_bung_copy_chunk(size_t, unsigned char *, size_t, bool);\n"
    return "namespace NetDaemonSyncWire {\n" + "\n\n".join(funcs) + "\n}\n"


def _owner_copy_extract(net):
    funcs = [
        _plain_definition(net, r"static\s+size_t\s+overlap_copy", "overlap copy"),
        _templated_function(net, r"static\s+size_t\s+member_copy", "member copy"),
        _templated_function(net, r"static\s+size_t\s+bounded_array_copy", "bounded array copy"),
        _templated_function(net, r"static\s+uint32_t\s+native_word", "native word"),
        _plain_definition(net, r"static\s+uint32_t\s+sign_extended_char", "signed byte"),
        _definition(net, r"void\s+gather_saved_diplomacy", "saved diplomacy gather"),
        _templated_function(net, r"static\s+void\s+scatter_word", "native word scatter"),
        _definition(net, r"void\s+scatter_saved_diplomacy", "saved diplomacy scatter"),
        _definition(net, r"bool\s+copy_nonrule_chunk", "nonrule owner router"),
    ]
    api = "bool opcode0_copy_chunk(size_t, unsigned char *, size_t, bool);\n"
    global_api = "bool veh_bung_copy_chunk(size_t, unsigned char *, size_t, bool);\n"
    return (_wire0_extract(net) + "\n" + global_api +
            "namespace NetDaemonSyncWire {\n" + api +
            "\n\n".join(funcs) + "\n}\n")


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


def test_actual_product_definitions_are_selected_not_tmp_proposals():
    net, alpha, core, wire0, nonrule, rule_copy, rule_commit = _must_source_is_imported()
    assert "NetDaemonSyncWire::copy_out" in net
    assert "NetDaemonSyncWire::copy_in" in net
    assert "copy_member_bytes" in wire0
    assert "GameWireOpaque_9A67C9" in wire0
    assert "veh_bung_copy_chunk" in nonrule
    assert "copy_rule_wire_chunk" in rule_copy
    assert "commit_staged_rules" in rule_commit
    assert (ROOT / "src/game.cpp").is_file()
    game = (ROOT / "src/game.cpp").read_text()
    assert re.search(r"^uint8_t GameWireOpaque_94B4D8\[0x80\];$", game, re.MULTILINE)
    assert re.search(r"^char NetDaemonFileTransferPath\[0x104\];$", net, re.MULTILINE)
    assert re.search(r"^static unsigned char NetDaemonRulesSyncStaging\[0x1920\];$",
                     net, re.MULTILINE)
    assert "memset(NetDaemonRulesSyncStaging" not in net


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_synch_method_source_with_callback_damage_controls(tmp_path, compiler):
    net, _, _, _, _, _, _ = _must_source_is_imported()
    template = HERE.with_name("netdaemon_synch_core_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_core_harness.cpp beside this test before importing it"
    interface, injection = _core_extract(net)
    harness = template.read_text().replace("/* SOURCE INTERFACE */", interface)
    source_template = tmp_path / "core-template.cpp"
    source_template.write_text(harness)
    _compile_injected(tmp_path, compiler, source_template, injection)

    # Damage controls operate on the extracted product definition itself.
    for old, new in [
        ("total_chunks == 1 ? 0 :", "total_chunks != 1 ? 0 :"),
        ("bytes + 0x24,", "bytes + 0x20,"),
        ("opcode >= 0x80 && opcode <= 0x8D)",
         "opcode >= 0x80 && opcode <= 0x8D && false)"),
        ("if (rules_notify && !scripted_turn)",
         "if (rules_notify && scripted_turn)"),
        ("message_data(0x2308, 0, 0, 0, 0, 0)",
         "message_data(0, 0, 0, 0, 0, 0x2308)"),
        ("if (!ConsoleGroupAction) GetAsyncKeyState(VK_CONTROL);",
         "if (ConsoleGroupAction) GetAsyncKeyState(VK_CONTROL);"),
    ]:
        assert old in injection
        _compile_injected(tmp_path, compiler, source_template,
                          injection.replace(old, new, 1), expected=False)


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_opcode0_owner_source_roundtrip(tmp_path, compiler):
    net, _, _, _, _, _, _ = _must_source_is_imported()
    template = HERE.with_name("netdaemon_synch_opcode0_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_wire_harness.cpp beside this test before importing it"
    harness = template.read_text()
    source_template = tmp_path / "wire-template.cpp"
    source_template.write_text(harness)
    injection = _wire0_extract(net)
    _compile_injected(tmp_path, compiler, source_template, injection)
    old = "COPY0(0x239, LegacyGameSaveName);"
    assert old in injection
    _compile_injected(tmp_path, compiler, source_template,
                      injection.replace(old, "", 1), expected=False)


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_nonrule_owner_source_routes(tmp_path, compiler):
    net, _, _, _, _, _, _ = _must_source_is_imported()
    template = HERE.with_name("netdaemon_synch_owner_copy_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_owner_harness.cpp beside this test before importing it"
    harness = template.read_text()
    source_template = tmp_path / "owner-template.cpp"
    source_template.write_text(harness)
    injection = _owner_copy_extract(net)
    _compile_injected(tmp_path, compiler, source_template, injection)
    old = "COPY_BYTES(0x18, GameWireOpaque_94B4D8,\n                   sizeof GameWireOpaque_94B4D8);"
    assert old in injection
    _compile_injected(tmp_path, compiler, source_template,
                      injection.replace(old, "", 1), expected=False)


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_alpha_rule_source_compactions(tmp_path, compiler):
    _, alpha, _, _, _, _, _ = _must_source_is_imported()
    template = HERE.with_name("netdaemon_synch_rules_harness.cpp")
    assert template.is_file(), "copy netdaemon_synch_rules_harness.cpp beside this test before importing it"
    harness = template.read_text()
    source_template = tmp_path / "rules-template.cpp"
    source_template.write_text(harness)
    injection = _rule_extract(alpha)
    _compile_injected(tmp_path, compiler, source_template, injection)
    old = "src + 0x04, sizeof dst.turn"
    assert old in injection
    _compile_injected(tmp_path, compiler, source_template,
                      injection.replace(old, "src + 0x08, sizeof dst.turn", 1),
                      expected=False)

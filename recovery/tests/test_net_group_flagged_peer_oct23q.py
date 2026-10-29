"""Defined prefix of actual send_group_flagged; stop at its first wait."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_netdaemon_group_host import HARNESS, extract_braced


ROOT = Path(__file__).resolve().parents[2]


def send_group_flagged_prefix(source):
    body = extract_braced(source, "int Net::send_group_flagged(")
    wait_start = body.index("WaitForMultipleObjects(")
    depth = 0
    close = -1
    for pos in range(body.index("(", wait_start), len(body)):
        if body[pos] == "(":
            depth += 1
        elif body[pos] == ")":
            depth -= 1
            if depth == 0:
                close = pos
                break
    assert close >= 0
    end = body.index(";", close) + 1
    return body[:end] + "\n        }\n    }\n    return 0;\n}"


def extract_type(source, type_name):
    start = source.index("struct " + type_name)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                end = source.index(";", pos) + 1
                return source[start:end]
    raise AssertionError("unterminated source type: " + type_name)


def prefix_harness(source):
    callback = extract_braced(
        source, "static int __stdcall net_group_player_callback(")
    envelope = extract_type(source, "NetPacketEnvelope")
    prefix = send_group_flagged_prefix(source)
    template = HARNESS.read_text()
    template = template[:template.index("int main() {")]
    template = template.replace("@@NET_PACKET_ENVELOPE@@", envelope, 1)
    declaration = "int send_group(void *, int, long, int);"
    assert template.count(declaration) == 1
    template = template.replace(
        declaration,
        declaration + "\n    int send_group_flagged(void *, int, long, unsigned short, unsigned int);",
        1,
    )
    selection_globals = (
        "static int g_mutate_selection_in_getplayerdata;\n"
        "static unsigned long g_enum_ids[16];\n"
        "static unsigned long g_enum_id_count;"
    )
    marker = "static int g_mutate_selection_in_getplayerdata;"
    assert template.count(marker) == 1
    template = template.replace(marker, selection_globals, 1)
    old_enumeration = """    if (NetPlayerTable != 0) {
        for (int i = 0; i < 16; ++i) {
            if (NetPlayerTable[i].field_0_ != 0)
                g_enum_callback(NetPlayerTable[i].field_0_, 0, 0, 0, 0);
        }
    }"""
    controlled_enumeration = """    if (NetPlayerTable != 0) {
        for (unsigned long i = 0; i < g_enum_id_count; ++i)
            g_enum_callback(g_enum_ids[i], 0, 0, 0, 0);
    }"""
    assert template.count(old_enumeration) == 1
    template = template.replace(old_enumeration, controlled_enumeration, 1)
    template = template.replace("@@PRODUCT_CALLBACK_CREATE_SEND@@", callback + "\n" + prefix, 1)
    template = template.replace("@@PRODUCT_ENUM_PLAYERS@@", "", 1)
    return template + r"""
static int run_case(int first_index, unsigned long first_timeout,
                    int later_index, unsigned long later_timeout) {
    Net net;
    std::memset(&net, 0, sizeof(net));
    net.field_6E4_ = 64;
    net.field_E4_ = 9;
    net.local_player_id_ = 0x77;
    net.thread_info_.events_[0] = reinterpret_cast<HANDLE>(1);
    net.thread_info_.events_[1] = reinterpret_cast<HANDLE>(2);
    NetDirectPlay = &DirectPlayObject;
    NetPlayerTable = 0;
    NetPlayerWriteIndex = 0;
    g_send_calls = g_enum_calls = g_wait_calls = 0;
    g_wait_timeout = 0;
    g_enum_id_count = 0;
    std::memset(g_enum_ids, 0, sizeof(g_enum_ids));
    if (first_index >= 0) {
        net.players_[first_index].field_0_ = 0x4400 + first_index;
        net.players_[first_index].field_C_ = first_timeout;
    }
    if (later_index >= 0) {
        net.players_[later_index].field_0_ = 0x5500 + later_index;
        net.players_[later_index].field_C_ = later_timeout;
        net.players_[later_index].flags_14_ = 2;
        g_enum_ids[g_enum_id_count++] = net.players_[later_index].field_0_;
    }
    char payload[4] = {1, 2, 3, 4};
    bool stopped = false;
    int result = -1;
    try {
        result = net.send_group_flagged(payload, sizeof(payload), 0x99, 0x10, 1);
    } catch (const StopAtFirstWait &) {
        stopped = true;
    }
    if (first_index < 0) {
        if (stopped || result != 10 || g_wait_calls != 0 || g_send_calls != 0)
            return 31;
        return 0;
    }
    if (!stopped || g_wait_calls != 1 || g_wait_timeout != first_timeout)
        return 32;
    if (g_enum_calls != 1 || !g_enum_saw_published_table || !g_enum_saw_null_guid)
        return 33;
    if (later_index >= 0 && net.players_[later_index].field_8_ != 1)
        return 34;
    if (net.players_[first_index].field_8_ != 0)
        return 35;
    if (g_send_calls != 1 || g_last_packet_type != 0x14 || g_last_reliable != 0)
        return 36;
    return 0;
}

int main() {
    const unsigned long infinite = 0xFFFFFFFFU;
    // The first occupied row has no callback mark. A later occupied row is
    // marked by enumeration, but its different field_C timeout must not win.
    if (run_case(1, 111, 8, 222) != 0) return 40;
    // Empty leading rows do not change which occupied peer supplies the delay.
    if (run_case(5, 111, 12, 222) != 0) return 41;
    // The original infinite timeout sentinel remains the selected peer's value.
    if (run_case(2, infinite, 9, 222) != 0) return 42;
    // An entirely empty table exits before send/wait or indeterminate stack reads.
    if (run_case(-1, 0, -1, 0) != 0) return 43;
    return 0;
}
"""


def run_prefix(tmp_path, compiler, source):
    cpp = tmp_path / "net_group_flagged_prefix.cpp"
    executable = tmp_path / "net_group_flagged_prefix"
    cpp.write_text(prefix_harness(source))
    built = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=10,
        env={"ASAN_OPTIONS": "detect_leaks=0"},
    )


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_first_occupied_peer_sets_actual_wait_prefix(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    source = (ROOT / "src/net_class.cpp").read_text()
    send = extract_braced(source, "int Net::send_group_flagged(")
    assert "if (players_[first_index].field_0_ != 0)" in send
    assert "retry_delay[selected] = players_[selected].field_C_;" in send
    assert "const DWORD waited = WaitForMultipleObjects(" in send
    run = run_prefix(tmp_path, compiler, source)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_callback_flag_selection_mutant_fails_defined_prefix(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} required")
    source = (ROOT / "src/net_class.cpp").read_text()
    original = "if (players_[first_index].field_0_ != 0)"
    mutant = "if (players_[first_index].field_8_ != 0)"
    assert source.count(original) == 1
    damaged = source.replace(original, mutant, 1)
    run = run_prefix(tmp_path, compiler, damaged)
    assert run.returncode != 0, "old callback-flag selector survived the wait-prefix witness"

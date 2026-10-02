"""Actual-source Net group controls; host doubles are not an ABI proof.

Native VC6 compilation separately establishes SDK signatures and source ABI.
These controls exercise packet routing, peer acknowledgements and callback
mutation through the product bodies; no retained seed or private candidate is
substituted for missing source.
"""
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
TEMPLATE = Path(__file__).with_name("net-group-host-harness.cpp")


def extract_braced(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        raise AssertionError(f"required actual definition missing: {signature}")
    brace = text.find("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def source_text() -> str:
    return (ROOT / "src/net_class.cpp").read_text()


def compile_and_run(source: str, enum_source: str,
                    approved_all_active_peers: bool = False) -> subprocess.CompletedProcess:
    callback = extract_braced(source, "static int __stdcall net_group_player_callback(")
    create = extract_braced(source, "int Net::create_group(")
    send = extract_braced(source, "int Net::send_group(")
    enum_players = extract_braced(enum_source, "int Net::enum_players(")
    # Source spelling is kept verbatim; only the platform interface type is
    # represented by the typed test double declared in the harness.
    bodies = "\n".join((callback, create, send)).replace("IDirectPlay2", "FakeDirectPlay")
    template = TEMPLATE.read_text()
    with tempfile.TemporaryDirectory(prefix="net-group-ctest-") as scratch:
        cpp = Path(scratch) / "harness.cpp"
        exe = Path(scratch) / "harness"
        cpp.write_text(template.replace("@@SOURCE_BODIES@@", bodies)
                       .replace("@@ENUM_PLAYERS_BODY@@", enum_players))
        compilers = []
        for compiler in ("g++", "clang++"):
            found = subprocess.run(["sh", "-c", f"command -v {compiler}"],
                                   capture_output=True, text=True)
            if found.returncode == 0:
                compilers.append(compiler)
        assert compilers, "no host C++ compiler available"
        for compiler in compilers:
            build = subprocess.run(
                [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                 *( ["-DAPPROVED_ALL_ACTIVE_PEERS"] if approved_all_active_peers else [] ),
                 "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                 str(cpp), "-o", str(exe)], capture_output=True, text=True)
            assert build.returncode == 0, build.stdout + build.stderr
            run_env = dict(os.environ)
            run_env["ASAN_OPTIONS"] = "detect_leaks=0"
            run = subprocess.run([str(exe)], capture_output=True, text=True,
                                 env=run_env)
            if run.returncode != 0:
                return run
            assert "net group host controls passed" in run.stdout
        return run


def test_actual_source_net_group_lifetime_and_callbacks():
    source = source_text()
    enum_source = (ROOT / "src/net_class.cpp").read_text()
    result = compile_and_run(source, enum_source)
    assert result.returncode == 0, result.stdout + result.stderr


def test_shared_context_and_callback_mutations_are_detected():
    source = source_text()
    enum_source = (ROOT / "src/net_class.cpp").read_text()
    mutations = (
        ("players[index].flags_14_ & 2", "players[index].flags_14_ & 4"),
        ("NetPlayerTable = players_;", "NetPlayerTable = 0;"),
        ("NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(group_id), 0,",
         "NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(group_id), reinterpret_cast<const GUID *>(1),"),
        ("return 1;\n}\n\n// ORIGINAL: 0x0062ED40",
         "return 0;\n}\n\n// ORIGINAL: 0x0062ED40"),
    )
    for old, new in mutations:
        assert old in source, f"mutation anchor missing: {old}"
        damaged = source.replace(old, new, 1)
        result = compile_and_run(damaged, enum_source)
        assert result.returncode != 0, f"mutation survived: {old}"


def test_enum_players_native_callback_contract_damage_controls():
    source = (ROOT / "src/net_class.cpp").read_text()
    group_source = source_text()
    mutations = (
        ("NetDirectPlay->GetPlayerData(original_id, &player_id, &data_size, 0)",
         "NetDirectPlay->GetPlayerData(original_id, &player_id, &data_size, data_size)"),
        ("const unsigned long original_id = player_id;",
         "const unsigned long original_id = 0;"),
        ("++NetPlayerWriteIndex;", "NetPlayerWriteIndex = 0;"),
    )
    for old, new in mutations:
        assert old in source, f"missing enum_players mutation anchor: {old}"
        result = compile_and_run(group_source, source.replace(old, new, 1))
        assert result.returncode != 0, f"enum_players mutation survived: {old}"

    cached = source.replace(
        "const unsigned long original_id = player_id;",
        "const unsigned long original_id = player_id;\n"
        "    NetPlayer *const selected_table = NetPlayerTable;", 1)
    cached = cached.replace("NetPlayerTable[NetPlayerWriteIndex]",
                            "selected_table[NetPlayerWriteIndex]")
    result = compile_and_run(group_source, cached)
    assert result.returncode != 0, "cached shared table selection survived callback mutation"


def test_approved_all_active_peers_acknowledgements():
    source = source_text()
    enum_source = (ROOT / "src/net_class.cpp").read_text()
    result = compile_and_run(source, enum_source,
                             approved_all_active_peers=True)
    assert result.returncode == 0, result.stdout + result.stderr


def test_approved_peer_initialization_and_retry_damage_controls():
    source = source_text()
    for original, damaged in (
        ("pending[peer] = 1;", "pending[peer] = peer == 0;"),
        ("const unsigned long resend_id = player.field_0_;",
         "const unsigned long resend_id = players_[0].field_0_;"),
    ):
        assert original in source, f"missing peer-state mutation anchor: {original}"
        result = compile_and_run(source.replace(original, damaged, 1), source,
                                 approved_all_active_peers=True)
        assert result.returncode != 0, f"peer-state mutation survived: {original}"

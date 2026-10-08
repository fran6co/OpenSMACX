"""Actual-source Net controls; reliable sends stop before indeterminate reads.

Host wait doubles throw at the first wait, before retry-array reads. This
checks the defined prefix, not original stack contents or runtime equivalence.
"""
import hashlib
from pathlib import Path
import os
import subprocess
import tempfile
import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
HARNESS = Path(__file__).with_name("net-group-host-harness.cpp")
PIN = ROOT / ".opensmacx/game/terranx_original.exe"
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def extract_braced(text: str, signature: str) -> str:
    search_from = 0
    while True:
        start = text.find(signature, search_from)
        if start < 0:
            raise AssertionError(f"required actual definition missing: {signature}")
        paren = text.find("(", start)
        depth = 0
        close = -1
        for pos in range(paren, len(text)):
            if text[pos] == "(":
                depth += 1
            elif text[pos] == ")":
                depth -= 1
                if depth == 0:
                    close = pos
                    break
        if close < 0:
            raise AssertionError(f"unterminated function signature: {signature}")
        after = close + 1
        while after < len(text) and text[after].isspace():
            after += 1
        if after < len(text) and text[after] == "{":
            brace = after
            break
        if after < len(text) and text[after] == ";":
            search_from = after + 1
            continue
        raise AssertionError(f"expected function body or declaration: {signature}")
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def product_bodies(source: str):
    callback = extract_braced(source,
                              "static int __stdcall net_group_player_callback(")
    create = extract_braced(source, "int Net::create_group(")
    send = extract_braced(source, "int Net::send_group(")
    enum_players = extract_braced(source, "int Net::enum_players(")
    owner = "\n".join((callback, create, send)).replace("IDirectPlay2", "FakeDirectPlay")
    return owner, enum_players, send


def compile_and_run(source: str, enum_source: str, compiler: str):
    owner, _, _ = product_bodies(source)
    enum_players = extract_braced(enum_source, "int Net::enum_players(")
    template = HARNESS.read_text()
    harness = (template.replace("@@PRODUCT_CALLBACK_CREATE_SEND@@", owner)
               .replace("@@PRODUCT_ENUM_PLAYERS@@", enum_players))
    with tempfile.TemporaryDirectory(prefix="net-group-faithful-") as scratch:
        cpp = Path(scratch) / "harness.cpp"
        exe = Path(scratch) / "harness"
        cpp.write_text(harness)
        build = subprocess.run(
            [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
             str(cpp), "-o", str(exe)], capture_output=True, text=True)
        assert build.returncode == 0, build.stdout + build.stderr
        run_env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
        return subprocess.run([str(exe)], capture_output=True, text=True,
                              env=run_env)


def compilers():
    found = [name for name in ("g++", "clang++")
             if subprocess.run(["sh", "-c", f"command -v {name}"],
                               capture_output=True).returncode == 0]
    assert found, "no host C++ compiler found"
    return found


def test_extractor_skips_forward_declaration():
    source = "static int callback(int);\nint other() { return 8; }\nstatic int callback(int value) { return value; }"
    assert extract_braced(source, "static int callback(") == "static int callback(int value) { return value; }"


def test_send_group_defined_prefix_actual_source():
    source = (ROOT / "src/net_class.cpp").read_text()
    _, _, send = product_bodies(source)
    assert "if (players_[index].field_0_ != 0) break;" in send
    assert send.count("pending[index] = 1;") == 1
    assert send.count("retry_delay[index] = players_[index].field_C_;") == 1
    init_region = send[send.index("int pending[16];"):send.index("for (;;) {")]
    assert "pending[index] = 0;" not in init_region
    assert "retry_delay[index] = 0;" not in init_region
    assert "for (int peer = 0; peer < 16; ++peer)" not in init_region
    assert "for (index = 0; index < 16; ++index) {\n            base_time[index] = initial_tick;" in send
    assert send.index("base_time[index] = initial_tick;") < send.index("const HRESULT resent =")
    for compiler in compilers():
        run = compile_and_run(source, source, compiler)
        assert run.returncode == 0, run.stdout + run.stderr
        assert "bounded actual-source Net group controls passed" in run.stdout


def test_pinned_initialization_and_base_time_witness():
    if not PIN.is_file():
        pytest.skip("pinned original image unavailable")
    image_bytes = PIN.read_bytes()
    image_sha = hashlib.sha256(image_bytes).hexdigest()
    assert image_sha == PIN_SHA256
    image = PeImage(image_bytes, image_sha)
    # Exact pinned instruction bytes establish first nonzero-DPID scan,
    # one-index pending/retry stores and full 16-entry base-time loop.
    assert image.read(Span(0x62F038, 0x62F046)) == bytes.fromhex(
        "83 39 00 75 2D 40 83 C1 58 83 F8 10 7C F2")
    assert image.read(Span(0x62F06E, 0x62F076)) == bytes.fromhex(
        "C7 44 84 70 01 00 00 00")
    assert image.read(Span(0x62F080, 0x62F087)) == bytes.fromhex(
        "89 8C 84 B0 00 00 00")
    assert image.read(Span(0x62F0A2, 0x62F0BB)) == bytes.fromhex(
        "89 44 24 30 8D 44 24 34 B9 0F 00 00 00 8B 54 24 30 "
        "89 10 83 C0 04 49 75 F4")


def test_callback_and_send_damage_controls_use_bounded_main():
    source = (ROOT / "src/net_class.cpp").read_text()
    enum_source = source
    callback = extract_braced(source,
                              "static int __stdcall net_group_player_callback(")
    send = extract_braced(source, "int Net::send_group(")
    mutations = (
        (callback, "players[index].flags_14_ & 2", "players[index].flags_14_ & 4",
         "callback bit predicate"),
        (send, "NetPlayerTable = players_;", "NetPlayerTable = 0;",
         "published callback table"),
        (send, "NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(group_id), 0,",
         "NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(group_id), reinterpret_cast<const GUID *>(1),",
         "null group GUID"),
    )
    for body, old, new, label in mutations:
        assert body.count(old) == 1, f"mutation anchor missing: {label}"
        damaged = source.replace(body, body.replace(old, new, 1), 1)
        for compiler in compilers():
            run = compile_and_run(damaged, enum_source, compiler)
            assert run.returncode != 0, f"mutation survived: {label} ({compiler})"

    callback_success = "players[index].field_8_ = 1;\n            }\n            return 1;"
    assert callback.count(callback_success) == 1
    damaged = source.replace(callback,
                             callback.replace(callback_success,
                                              callback_success.replace("return 1;", "return 0;"), 1),
                             1)
    for compiler in compilers():
        run = compile_and_run(damaged, enum_source, compiler)
        assert run.returncode != 0, f"callback return mutation survived ({compiler})"


def test_enum_players_damage_controls_use_bounded_main():
    source = (ROOT / "src/net_class.cpp").read_text()
    mutations = (
        ("NetDirectPlay->GetPlayerData(original_id, &player_id, &data_size, 0)",
         "NetDirectPlay->GetPlayerData(original_id, &player_id, &data_size, data_size)"),
        ("const unsigned long original_id = player_id;",
         "const unsigned long original_id = 0;"),
        ("++NetPlayerWriteIndex;", "NetPlayerWriteIndex = 0;"),
    )
    for old, new in mutations:
        assert source.count(old) == 1, f"missing enum_players mutation anchor: {old}"
        damaged = source.replace(old, new, 1)
        for compiler in compilers():
            run = compile_and_run(source, damaged, compiler)
            assert run.returncode != 0, f"enum_players mutation survived: {old} ({compiler})"

    cached = source.replace(
        "const unsigned long original_id = player_id;",
        "const unsigned long original_id = player_id;\n"
        "    NetPlayer *const selected_table = NetPlayerTable;", 1)
    cached = cached.replace("NetPlayerTable[NetPlayerWriteIndex]",
                            "selected_table[NetPlayerWriteIndex]")
    for compiler in compilers():
        run = compile_and_run(source, cached, compiler)
        assert run.returncode != 0, f"cached shared selector survived ({compiler})"

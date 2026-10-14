"""Strict source extraction for the native F05/F06/F07/F0B/F08/F0F modal sides."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).parents[2]
TEMPLATE = Path(__file__).with_name("netdaemon-transfer-modal-harness.cpp")


def extract(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        raise AssertionError(f"required body missing: {signature}")
    brace = text.find("{", start)
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated body: " + signature)


def source_texts():
    actual = (ROOT / "src/netdaemon.cpp").read_text()
    return actual, actual


def compile_and_run(source: str, owner_source: str):
    record_types = "\n".join(
        extract(owner_source, signature) + ";"
        for signature in (
            "struct NetDaemonFileIndexRecord {",
            "struct NetDaemonFilePathPacket {",
            "struct NetDaemonFileControlPacket {",
            "struct NetDaemonFileChunkPacket {",
        )
    )
    guard = extract(source, "class NetDaemonTransferModalCloseGuard {") + ";"
    helper = extract(source, "static bool netdaemon_transfer_modal_cleanup(")
    byte_helpers = "\n".join(
        extract(source, signature)
        for signature in (
            "static uint16_t netdaemon_transfer_read_u16(",
            "static uint32_t netdaemon_transfer_read_u32(",
            "static int32_t netdaemon_transfer_read_i32(",
            "static void netdaemon_transfer_write_u16(",
        ))
    record_allocator = extract(owner_source,
                               "static NetDaemonFileIndexRecord *netdaemon_allocate_file_index_records(")
    packet_views = "\n".join(
        extract(source, signature) + ";"
        for signature in (
            "class NetDaemonFileIndexPacketView {",
            "class NetDaemonFilePathPacketView {",
            "class NetDaemonFileControlPacketView {",
            "class NetDaemonFileChunkPacketView {",
        )
    )
    body = extract(source, "bool NetDaemon::process_file_transfer_message(")
    generated = TEMPLATE.read_text().replace(
        "struct NetDaemonFileIndexRecord {\n    uint32_t ready_flags;\n    uint32_t chunk_count;\n    uint32_t file_size;\n    char filename[260];\n};",
        record_types,
    ).replace(
        "    uint32_t local_player_id_;\n    int pid_2_idx(uint32_t) { return 3; }",
        "    uint32_t local_player_id_;\n    unsigned long local_player_id() { return local_player_id_; }\n"
        "    int pid_2_idx(uint32_t) { return 3; }",
    ).replace(
        "@@TRANSFER_CALLBACK_BODIES@@",
        guard + "\n" + record_allocator + "\n" + byte_helpers + "\n" +
        packet_views + "\n" +
        helper + "\n" + extract(source, "static void netdaemon_forward_default_message(") + "\n" + body)
    generated = "#include <cstddef>\n" + generated
    with tempfile.TemporaryDirectory(prefix="net-transfer-modal-") as scratch:
        cpp = Path(scratch) / "harness.cpp"
        exe = Path(scratch) / "harness"
        cpp.write_text(generated)
        compilers = []
        for compiler in ("g++", "clang++"):
            found = subprocess.run(["sh", "-c", "command -v " + compiler],
                                   capture_output=True)
            if found.returncode == 0:
                compilers.append(compiler)
        assert compilers
        for compiler in compilers:
            build = subprocess.run(
                [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                 "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                 str(cpp), "-o", str(exe)], capture_output=True, text=True)
            assert build.returncode == 0, build.stdout + build.stderr
            env = dict(os.environ)
            env["ASAN_OPTIONS"] = "detect_leaks=0"
            run = subprocess.run([str(exe)], capture_output=True, text=True, env=env)
            if run.returncode != 0:
                return run
            assert "transfer modal cleanup controls passed" in run.stdout
        return run


def test_actual_host_modal_cleanup_routes():
    result = compile_and_run(*source_texts())
    assert result.returncode == 0, result.stdout + result.stderr


def test_modal_routing_and_owner_cleanup_mutants_are_detected():
    source, owner_source = source_texts()
    mutations = (
        ("if (local_is_host) return netdaemon_transfer_modal_cleanup(guard);",
         "if (!local_is_host) return netdaemon_transfer_modal_cleanup(guard);"),
        ("void arm() { armed_ = true; }",
         "void arm() { armed_ = false; }"),
    )
    for old, new in mutations:
        assert old in source
        result = compile_and_run(source.replace(old, new, 1), owner_source)
        assert result.returncode != 0, f"modal mutation survived: {old}"

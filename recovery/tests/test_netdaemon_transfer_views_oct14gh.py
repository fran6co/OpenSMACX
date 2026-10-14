"""Actual receiver controls for borrowed transfer packet field views."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_netdaemon_transfer_modal import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
HARNESS = Path(__file__).with_name("netdaemon-transfer-receive-harness.cpp")


def receiver_translation_unit(source: str, extra_body: str = "",
                              extra_main: str = "",
                              mutate_checksum: bool = False) -> str:
    record_types = "\n".join(
        extract(source, signature) + ";"
        for signature in (
            "struct NetDaemonFileIndexRecord {",
            "struct NetDaemonFilePathPacket {",
            "struct NetDaemonFileControlPacket {",
            "struct NetDaemonFileChunkPacket {",
        )
    )
    definitions = (
        "class NetDaemonTransferModalCloseGuard {",
        "static NetDaemonFileIndexRecord *netdaemon_allocate_file_index_records(",
        "static uint16_t netdaemon_transfer_read_u16(",
        "static uint32_t netdaemon_transfer_read_u32(",
        "static int32_t netdaemon_transfer_read_i32(",
        "static void netdaemon_transfer_write_u16(",
        "static void netdaemon_transfer_write_i32(",
        "class NetDaemonFileIndexPacketView {",
        "class NetDaemonFilePathPacketView {",
        "class NetDaemonFileControlPacketView {",
        "class NetDaemonFileChunkPacketView {",
        "static bool netdaemon_transfer_modal_cleanup(",
        "static void netdaemon_forward_default_message(",
        "bool NetDaemon::process_file_transfer_message(",
    )
    bodies = "\n".join(
        extract(source, signature) + (";" if signature.startswith("class ") else "")
        for signature in definitions
    ) + "\n" + extra_body
    reset_definition = next(
        line for line in source.splitlines()
        if line.startswith("int NetDaemonTransferResetWord =")
    )
    harness_template = HARNESS.read_text()
    if mutate_checksum:
        old_checksum = "static unsigned char checksum_file(const char *) { return 0; }"
        new_checksum = ("static int mutate_checksum_during_call;\n"
                        "static unsigned char checksum_file(const char *) { "
                        "if (mutate_checksum_during_call) packet[0x11]=0; return 0; }")
        assert harness_template.count(old_checksum) == 1
        harness_template = harness_template.replace(old_checksum, new_checksum, 1)
    harness = (harness_template
            .replace("@@RECORD@@", record_types)
            .replace("@@BODIES@@", bodies)
            .replace("@@RESET_WORD@@", reset_definition)
            .replace("    int pid_2_idx(uint32_t) {",
                     "    unsigned long local_player_id() { return local_player_id_; }\n"
                     "    int pid_2_idx(uint32_t) {"))
    if extra_main:
        tail = ('    reset(11,0xF05); std::strcpy(reinterpret_cast<char *>(packet+0x18),'
                '"transfer/path"); invoke();\n'
                '    assert(NetDaemonTransferResetWord==-1 && NetDaemonFileTransferMode==1 && '
                'NetDaemonFileTransferCurrentFile==-1);\n'
                '    assert(!std::strcmp(NetDaemonFileTransferPath,"transfer/path")); clean();\n}')
        assert harness.count(tail) == 1
        harness = harness.replace(tail, tail[:-1] + extra_main + "\n}", 1)
    return "#include <cstddef>\n" + harness


def compile_and_run(tmp_path: Path, source: str, extra_body: str = "",
                    extra_main: str = "", mutate_checksum: bool = False):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / "transfer-views.cpp"
    exe = tmp_path / "transfer-views"
    cpp.write_text(receiver_translation_unit(
        source, extra_body, extra_main, mutate_checksum))
    build = subprocess.run(
        [compiler, "-std=c++03", "-Wall", "-Wextra", "-Wno-unused-function",
         "-fsanitize=address,undefined",
         "-fno-omit-frame-pointer", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=60,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    return subprocess.run(
        [str(exe)], capture_output=True, text=True,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"), timeout=10,
    )


def test_packet_views_are_borrowed_offset_anchored_and_keep_distinct_indices(tmp_path):
    source = SOURCE.read_text()
    assert "offsetof(NetDaemonFileIndexRecord, file_index)" in extract(
        source, "class NetDaemonFileIndexPacketView {"
    )
    assert "offsetof(NetDaemonFileChunkPacket, file_index)" in extract(
        source, "class NetDaemonFileChunkPacketView {"
    )
    assert "bytes_[offsetof(NetDaemonFileIndexRecord, file_index)]" in source
    # The receive harness mutates F06's byte index from3 to4 during send and
    # F0B's dword file/chunk fields during UI callbacks. Its table assertions
    # therefore exercise the actual body after reentrant mutation, not cached
    # packet snapshots or a view-only simulation.
    result = compile_and_run(tmp_path, source)
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_f06_receiver_reloads_byte_index_after_send_callback(tmp_path):
    source = SOURCE.read_text()
    body = extract(source, "bool NetDaemon::process_file_transfer_message(")
    before = "NetDaemonFileIndexRecord &record = records[index_packet.file_index()];"
    after = "NetDaemonFileIndexRecord &record = records[file_index];"
    assert body.count(before) == 1
    mutant = source.replace(before, after, 1)
    result = compile_and_run(tmp_path, mutant)
    assert result.returncode != 0, "cached pre-callback F06 byte index survived"


def test_actual_f0b_uses_dword_file_index_not_chunk_or_f06_byte_index(tmp_path):
    source = SOURCE.read_text()
    view = extract(source, "class NetDaemonFileChunkPacketView {")
    before = "bytes_ + offsetof(NetDaemonFileChunkPacket, file_index)"
    after = "bytes_ + offsetof(NetDaemonFileChunkPacket, chunk_index)"
    assert view.count(before) == 1
    mutant = source.replace(before, after, 1)
    result = compile_and_run(tmp_path, mutant)
    assert result.returncode != 0, "wrong F0B dword field binding survived"


ROUTE_HELPERS = r'''
static uint32_t invoke_host_ready(unsigned file_index, uint32_t slot) {
    NetDaemon daemon; std::memset(&daemon,0,sizeof daemon);
    daemon.primary_state_.leading_words_[0]=1;
    std::memset(packet,0,sizeof packet); uint16_t opcode=0xF08;
    std::memcpy(packet,&opcode,2); packet[0x10]=static_cast<unsigned char>(file_index);
    put(4,slot); put(8,255);
    Popup popup; NetDaemonTransferModalCloseGuard guard(popup); Filemap filemap;
    assert(daemon.process_file_transfer_message(reinterpret_cast<char *>(packet),
        incoming_size,0x44,guard,filemap));
    return rows[file_index].ready_flags;
}
static uint8_t invoke_slot(unsigned short opcode, uint32_t slot) {
    NetDaemon daemon; std::memset(&daemon,0,sizeof daemon);
    std::memset(packet,0,sizeof packet); std::memcpy(packet,&opcode,2); put(4,slot);
    Popup popup; NetDaemonTransferModalCloseGuard guard(popup); Filemap filemap;
    assert(daemon.process_file_transfer_message(reinterpret_cast<char *>(packet),
        incoming_size,0x44,guard,filemap));
    return daemon.process_slots_[slot].status_bytes_[0];
}
static uint32_t invoke_host_ack(uint32_t slot) {
    NetDaemon daemon; std::memset(&daemon,0,sizeof daemon);
    daemon.primary_state_.leading_words_[0]=1;
    std::memset(packet,0,sizeof packet); uint16_t opcode=0xF0F;
    std::memcpy(packet,&opcode,2); put(4,slot);
    Popup popup; NetDaemonTransferModalCloseGuard guard(popup); Filemap filemap;
    assert(daemon.process_file_transfer_message(reinterpret_cast<char *>(packet),
        incoming_size,0x44,guard,filemap));
    return NetDaemonFileTransferAckMask;
}
'''


ROUTE_MAIN = r'''
    reset(12,0xF08); assert(invoke_host_ready(7,34)==(1U<<2));
    assert(rows[255].ready_flags==0); clean();
    assert(invoke_slot(0xF0A,5)==1);
    assert(invoke_slot(0xF0C,6)==2);
    NetDaemonFileTransferAckMask=0;
    assert(invoke_host_ack(35)==(1U<<3));
'''


def test_actual_f08_f0a_f0c_f0f_payload_routes_and_field_mutants(tmp_path):
    source = SOURCE.read_text()
    result = compile_and_run(tmp_path, source, ROUTE_HELPERS, ROUTE_MAIN)
    assert result.returncode == 0, result.stdout + result.stderr
    for before, after in (
        ("bytes_ + offsetof(NetDaemonFileControlPacket, source_player)",
         "bytes_ + offsetof(NetDaemonFileControlPacket, state)"),
        ("type == 0xF0A ? 1 : 2", "type == 0xF0A ? 2 : 1"),
    ):
        assert before in source
        mutant = source.replace(before, after, 1)
        result = compile_and_run(tmp_path, mutant, ROUTE_HELPERS, ROUTE_MAIN)
        assert result.returncode != 0, f"route payload mutant survived: {before}"


def test_f06_checksum_byte_is_read_after_checksum_call_mutation(tmp_path):
    source = SOURCE.read_text()
    extra_main = r'''
    reset(13,0xF06); packet[0x10]=3; packet[0x11]=1;
    std::strcpy(reinterpret_cast<char *>(packet+0x18),"received.bin");
    mutate_checksum_during_call=1; invoke();
    assert(!std::strcmp(rows[3].filename,"received.bin") && send_calls==0);
    mutate_checksum_during_call=0; clean();
'''
    result = compile_and_run(tmp_path, source, extra_main=extra_main,
                            mutate_checksum=True)
    assert result.returncode == 0, result.stdout + result.stderr
    early_read = source.replace(
        "        const unsigned char calculated_checksum =\n"
        "            checksum_file(index_packet.filename());\n"
        "        if (calculated_checksum == index_packet.checksum()) {",
        "        const unsigned char packet_checksum = index_packet.checksum();\n"
        "        const unsigned char calculated_checksum =\n"
        "            checksum_file(index_packet.filename());\n"
        "        if (calculated_checksum == packet_checksum) {")
    assert early_read != source
    result = compile_and_run(tmp_path, early_read, extra_main=extra_main,
                            mutate_checksum=True)
    assert result.returncode != 0, "pre-call checksum read survived mutation control"

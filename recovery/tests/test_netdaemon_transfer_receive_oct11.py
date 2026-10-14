"""Pinned transfer arithmetic/order with actual source and fixed reviewed edits."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_netdaemon_transfer_modal import extract

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
BASE = "1b8b5853e"
FIXTURE = Path(__file__).with_name("netdaemon-transfer-receive-oct11.patch")


def apply_transfer_receive_migration(source):
    # A checked-in fixed edit recipe, not a diff recomputed from current source.
    # Each literal context must occur exactly once; all other text is protected.
    hunks = FIXTURE.read_text().split("\n@@ ")[1:]
    assert hunks
    for hunk in hunks:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(line[1:] for line in lines if line.startswith((" ", "-")))
        after = "".join(line[1:] for line in lines if line.startswith((" ", "+")))
        assert before and source.count(before) == 1, before
        source = source.replace(before, after, 1)
    from test_netdaemon_typed_storage_oct11 import apply_typed_storage_migration
    return apply_typed_storage_migration(source)


def test_reviewed_receiver_recipe_preserves_all_other_source_and_identities():
    before = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT, text=True)
    assert apply_transfer_receive_migration(before) == SOURCE.read_text()
    from match_recovery.source import read_text
    from dataclasses import fields
    old = {r.address:r for r in read_text(before,SOURCE)}
    current = {r.address:r for r in read_text(SOURCE.read_text(),SOURCE)}
    assert old.keys() == current.keys()
    for address, record in current.items():
        for field in fields(record):
            if field.name not in {"line","region","origin"}:
                assert getattr(record,field.name) == getattr(old[address],field.name)


@pytest.mark.parametrize("damage", ["none", "create_sequence", "zero_create", "predecrement", "unsigned_count", "global_count", "compressed_extent", "fixed_send_length", "early_index", "early_close", "lost_progress_reload", "missing_reset"])
def test_actual_receiver_packet_ui_and_completion_semantics(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    actual = SOURCE.read_text()
    definitions = ["class NetDaemonTransferModalCloseGuard {", "static NetDaemonFileIndexRecord *netdaemon_allocate_file_index_records(",
                   "static uint16_t netdaemon_transfer_read_u16(", "static uint32_t netdaemon_transfer_read_u32(",
                   "static int32_t netdaemon_transfer_read_i32(", "static void netdaemon_transfer_write_u16(",
                   "static void netdaemon_transfer_write_i32(",
                   "class NetDaemonFileIndexPacketView {", "class NetDaemonFilePathPacketView {",
                   "class NetDaemonFileControlPacketView {", "class NetDaemonFileChunkPacketView {",
                   "static bool netdaemon_transfer_modal_cleanup(",
                   "static void netdaemon_forward_default_message(", "bool NetDaemon::process_file_transfer_message("]
    body = "\n".join(extract(actual,s) + (";" if s.startswith("class ") else "") for s in definitions)
    changes = {
        "create_sequence": ("decompressed_size, 0)", "decompressed_size, 1)"),
        "zero_create": ("filemap.create(record.filename, 0, 0);", "filemap.create(record.filename, record.file_size, 0);"),
        "predecrement": ("parse_num(0, NetDaemonFileTransferProgress);", "parse_num(0, --NetDaemonFileTransferProgress);"),
        "unsigned_count": ("static_cast<uint32_t>(rounded_size / 0x800)", "rounded_bits >> 11"),
        "global_count": ("int32_t rounded_size;", "NetDaemonFileTransferChunkCount=0; int32_t rounded_size;"),
        "compressed_extent": ("static_cast<unsigned int>(compressed_size)", "netdaemon_transfer_read_u32(bytes + 4)"),
        "fixed_send_length": ("send_message(message, static_cast<int>(message_size), sender_id);", "send_message(message, 0x20, sender_id);"),
        "early_index": ("NetDaemonFileIndexRecord &record = records[index_packet.file_index()];", "NetDaemonFileIndexRecord &record = records[file_index];"),
        "early_close": ("PopupModalState = 6;", "PopupModalState = 6; pop_close();"),
        "lost_progress_reload": ("--NetDaemonFileTransferProgress;", "NetDaemonFileTransferProgress=7;"),
        "missing_reset": ("        NetDaemonTransferResetWord = -1;", "        NetDaemonTransferResetWord = 0;"),
    }
    if damage != "none":
        before, after = changes[damage]
        assert body.count(before) == (2 if damage=="missing_reset" else 1), before
        body = body.replace(before,after,1)
    template = Path(__file__).with_name("netdaemon-transfer-receive-harness.cpp").read_text()
    definition=next(line for line in actual.splitlines() if line.startswith("int NetDaemonTransferResetWord ="))
    records = "\n".join(extract(actual, signature) + ";" for signature in (
        "struct NetDaemonFileIndexRecord {", "struct NetDaemonFilePathPacket {",
        "struct NetDaemonFileControlPacket {", "struct NetDaemonFileChunkPacket {"))
    generated = template.replace("@@RECORD@@",records).replace("@@BODIES@@",body).replace("@@RESET_WORD@@",definition)
    generated = generated.replace("    int pid_2_idx(uint32_t) {",
                                  "    unsigned long local_player_id() { return local_player_id_; }\n"
                                  "    int pid_2_idx(uint32_t) {")
    generated = "#include <cstddef>\n" + generated
    cpp=tmp_path/"receive.cpp"; exe=tmp_path/"receive"
    cpp.write_text(generated)
    subprocess.run([compiler,"-std=c++03","-fsanitize=address,undefined","-fno-omit-frame-pointer",str(cpp),"-o",str(exe)], check=True,capture_output=True,timeout=60)
    environment=dict(os.environ,ASAN_OPTIONS="detect_leaks=0")
    result=subprocess.run([str(exe)],capture_output=True,env=environment,timeout=10)
    assert (result.returncode==0)==(damage=="none"),result.stderr


def test_native_receiver_calls_signed_arithmetic_and_reload_order():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=ROOT/".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),"01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    assert image.read(Span(0x68C11C,0x68C120)) == b"\xff"*4
    bands=[(0x534890,0x5348E5),(0x5349EC,0x534AE7),(0x534B1D,0x534D21)]
    instructions={i.address:(i.mnemonic,i.op_str) for start,end in bands
                  for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(start,end)),start)}
    expected={
        0x53489A:("call","0x4e25e0"), 0x5348D5:("mov","eax, dword ptr [ebp + 0xc]"),
        0x534890:("mov","dword ptr [0x68c11c], 0xffffffff"),
        0x534A51:("mov","al, byte ptr [ebx + 0x10]"),
        0x534A8F:("push","0"),0x534A9B:("push","0"),
        0x534B7E:("add","eax, 0x800"),0x534B86:("cdq",""),
        0x534B87:("and","edx, 0x7ff"),0x534B94:("sar","ecx, 0xb"),
        0x534BA4:("mov","dword ptr [eax + edx*4 + 0xc], ecx"),
        0x534BC2:("call","0x645930"),0x534BF2:("mov","ecx, dword ptr [0x93f7ac]"),
        0x534C1D:("call","0x5bf420"),0x534C22:("mov","eax, dword ptr [0x93f7ac]"),
        0x534C2A:("dec","eax"),0x534C5A:("push","0"),
        0x534C6C:("mov","edx, dword ptr [ebx + 0xc]"),0x534C78:("add","edx, edi"),
        0x534CD3:("jne","0x534d17"),0x534D17:("call","0x6272f0"),
    }
    for address,instruction in expected.items():
        assert instructions[address]==instruction

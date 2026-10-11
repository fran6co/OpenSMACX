"""Strict source extractor plus host behavioral controls for send_files."""
from pathlib import Path
import os
import subprocess
import tempfile
import pytest

ROOT = Path(__file__).parents[2]
HARNESS = Path(__file__).with_name("netdaemon-send-files-harness.cpp")


def extract_method(text: str) -> str:
    signature = "int NetDaemon::send_files()"
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
    raise AssertionError("unterminated send_files definition")


def owner_source() -> str:
    text = (ROOT / "src/netdaemon.cpp").read_text()
    # Keep source declarations/accessors verbatim with the function. The one
    # 32-bit-owner assertion is removed only for the 64-bit host fixture; the
    # fixture stores an 8-byte pointer representation across adjacent words.
    begin = text.find("struct NetDaemonFileIndexRecord {")
    end = text.find("int NetDaemon::send_files()")
    if begin < 0 or end < 0:
        raise AssertionError("complete file-transfer owner declarations/body required")
    globals_start = text.find("// RECOVERY_OBJECT: v1 0x0093F79C", begin, end)
    if globals_start >= 0:
        end = globals_start
    prefix = text[begin:end]
    prefix = prefix.replace(
        'static_assert(sizeof(void *) == sizeof(uint32_t),\n'
        '              "the native transfer pointer word is 32-bit");',
        "/* host-only: source target is 32-bit */")
    return prefix + extract_method(text)


def compile_and_run(owner: str) -> subprocess.CompletedProcess:
    template = HARNESS.read_text()
    with tempfile.TemporaryDirectory(prefix="net-send-files-ctest-") as scratch:
        cpp = Path(scratch) / "harness.cpp"
        exe = Path(scratch) / "harness"
        cpp.write_text(template.replace("@@OWNER_SOURCE@@", owner))
        compilers = []
        for compiler in ("g++", "clang++"):
            found = subprocess.run(["sh", "-c", f"command -v {compiler}"],
                                   capture_output=True, text=True)
            if found.returncode == 0:
                compilers.append(compiler)
        assert compilers
        for compiler in compilers:
            build = subprocess.run(
                [compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                 "-Wno-unused-function", "-Wno-unused-variable",
                 "-Wno-write-strings",
                 "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                 str(cpp), "-o", str(exe)], capture_output=True, text=True)
            assert build.returncode == 0, build.stdout + build.stderr
            env = dict(os.environ)
            env["ASAN_OPTIONS"] = "detect_leaks=0"
            run = subprocess.run([str(exe)], capture_output=True, text=True, env=env)
            if run.returncode:
                return run
            assert "send_files host controls passed" in run.stdout
        return run


def test_actual_send_files_owner():
    result = compile_and_run(owner_source())
    assert result.returncode == 0, result.stdout + result.stderr


def test_native_transfer_damage_controls():
    owner = owner_source()
    mutants = (
        ("strcat(path_message.path, NetDaemonFileTransferPath);",
         "strcat(reinterpret_cast<char *>(&path_message) + 2, NetDaemonFileTransferPath);"),
        ("announce.source_player = local_index;",
         "memcpy(reinterpret_cast<unsigned char *>(&announce) + 2, &local_index, sizeof local_index);"),
        ("list_done.state = 2;",
         "const int bad_list_state = 2; memcpy(reinterpret_cast<unsigned char *>(&list_done) + 2, &bad_list_state, sizeof bad_list_state);"),
        ("transfer_done.state = 3;",
         "const int bad_done_state = 3; memcpy(reinterpret_cast<unsigned char *>(&transfer_done) + 2, &bad_done_state, sizeof bad_done_state);"),
        ("send_buffer->file_index = file_index;", "send_buffer->file_index = 0;"),
        ("while (NetDaemonFileTransferAckMask != send_mask)",
         "while (NetDaemonFileTransferAckMask == send_mask)"),
        ("while (remaining > 0x7FF)", "while (remaining > 0x800)"),
        ("process_slots_[slot].status_bytes_[0] > 1",
         "process_slots_[slot].status_bytes_[0] > 0"),
        ("send_group(send_buffer, packet_length,",
         "send_group(send_buffer, 0x820,"),
        ("parse_num(0, ready_count--);", "parse_num(0, --ready_count);"),
        ("local_index == slot ? 2 : 0", "local_index == slot ? 0 : 2"),
        ("NetDaemonNet.channels_[1].trailing_words_[9] = 0;\n"
         "    const int local_index = NetDaemonNet.pid_2_idx(local_player_id);",
         "const int local_index = NetDaemonNet.pid_2_idx(local_player_id);\n"
         "    NetDaemonNet.channels_[1].trailing_words_[9] = 0;"),
        ("(ready_flags & (1U << (slot & 31)))",
         "(record.ready_flags & (1U << (slot & 31)))"),
        ("NetDaemonFileIndexRecord &checked_record =\n"
         "                        netdaemon_file_index_records()[file_count];",
         "NetDaemonFileIndexRecord &checked_record = record;"),
        ("netdaemon_file_index_records()[file_index].filename, TRUE)",
         "record.filename, TRUE)"),
        ("netdaemon_file_index_records()[file_index];", "records[file_index];"),
        ("x_pop_caption_flags(ScriptPopupFile,", "x_pop_caption_flags(g_script_file,"),
        ("Net::send(&announce,", "NetDaemonNet.Net::send(&announce,"),
        ("process_slots_[slot].status_bytes_[0] =",
         "NetDaemonNet.process_slots_[slot].status_bytes_[0] ="),
        ("process_slots_[slot].status_bytes_[0] == 0",
         "NetDaemonNet.process_slots_[slot].status_bytes_[0] == 0"),
        ("process_slots_[slot].status_bytes_[0] > 1",
         "NetDaemonNet.process_slots_[slot].status_bytes_[0] > 1"),
        ("group_handle = create_group(", "group_handle = NetDaemonNet.create_group("),
        ("add_to_group(group_handle, process_slots_[slot].pid_)",
         "NetDaemonNet.add_to_group(group_handle, NetDaemonNet.process_slots_[slot].pid_)"),
        ("send_group(send_buffer, 0x820,", "NetDaemonNet.send_group(send_buffer, 0x820,"),
        ("NetDaemonNet.channels_[1].trailing_words_[9] != 0",
         "channels_[1].trailing_words_[9] != 0"),
        ("NetDaemonNet.channels_[1].trailing_words_[9] = 1;",
         "channels_[1].trailing_words_[9] = 1;"),
        ("if (end_word > source_word)", "if (remaining > 0)"),
        ("const uint32_t end_word = static_cast<uint32_t>(",
         "const uint64_t end_word = static_cast<uint64_t>("),
        ("tail_length + 0x20u", "tail_length"),
        ("memmove(send_buffer->payload, source, tail_length);",
         "memcpy(send_buffer->payload, source, tail_length);"),
    )
    for old, new in mutants:
        assert old in owner, f"missing mutation anchor: {old}"
        result = compile_and_run(owner.replace(old, new, 1))
        assert result.returncode != 0, f"mutation survived: {old}"


def test_pinned_send_files_callback_owners_and_unsigned_tail():
    from hashlib import sha256
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == pin
    image = PeImage(raw, pin)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {
        i.address: (i.mnemonic, i.op_str.lower())
        for start, end in ((0x52ED40, 0x52F517), (0x533178, 0x533182),
                           (0x5337AB, 0x5337B5), (0x53476E, 0x5347C1))
        for i in decoder.disasm(image.read(Span(start, end)), start)
    }
    pins = {
        0x52ED61: ("mov", "edi, ecx"),
        0x52ED81: ("mov", "dword ptr [0x93e96c], esi"),
        0x52ED73: ("mov", "eax, dword ptr [0x93d4f0]"),
        0x52ED87: ("call", "0x4e25e0"),
        0x52ED9B: ("mov", "dword ptr [0x93e970], eax"),
        0x52EDE2: ("lea", "eax, [edi + 0x930]"),
        0x52EDE8: ("cmp", "ebx, ecx"),
        0x52EDF7: ("setne", "dl"), 0x52EDFA: ("dec", "edx"),
        0x52EE00: ("and", "edx, 2"),
        0x52EE04: ("mov", "byte ptr [eax - 0x19c], dl"),
        0x52EE66: ("push", "0x93ac88"),
        0x52EE6B: ("mov", "byte ptr [0x93ac88], 0"),
        0x52EF40: ("mov", "ecx, edi"),
        0x52F06B: ("call", "0x538fe0"),
        0x52F070: ("mov", "edx, dword ptr [0x93e970]"),
        0x52F076: ("mov", "byte ptr [esi + edx + 0x11], al"),
        0x52F09B: ("mov", "eax, dword ptr [0x93e970]"),
        0x52F138: ("lea", "ecx, [edi + 0x930]"),
        0x52F1B0: ("mov", "eax, dword ptr [0x93e970]"),
        0x52F1B8: ("mov", "ecx, dword ptr [ecx + eax + 8]"),
        0x52F1BE: ("mov", "dword ptr [ebp - 0x10], ecx"),
        0x52F1DA: ("mov", "eax, esi"), 0x52F1DC: ("dec", "esi"),
        0x52F1DD: ("push", "eax"), 0x52F1E3: ("call", "0x625e30"),
        0x52F1F7: ("mov", "ecx, dword ptr [0x691b0c]"),
        0x52F229: ("mov", "edx, eax"),
        0x52F23B: ("call", "0x62ed40"),
        0x52F248: ("add", "ebx, 0x928"),
        0x52F257: ("test", "dword ptr [ebp - 0x10], eax"),
        0x52F262: ("mov", "ecx, dword ptr [ebp - 0x18]"),
        0x52F277: ("mov", "eax, dword ptr [0x93e970]"),
        0x52F285: ("lea", "ecx, [ebp - 0x44]"),
        0x52F2F2: ("lea", "eax, [edx + esi]"),
        0x52F2FC: ("cmp", "eax, 0x800"), 0x52F301: ("jl", "0x52f350"),
        0x52F315: ("call", "0x645930"),
        0x52F32C: ("mov", "ecx, dword ptr [ebp - 0x18]"),
        0x52F356: ("cmp", "eax, esi"), 0x52F358: ("jbe", "0x52f37f"),
        0x52F35F: ("sub", "ebx, esi"), 0x52F364: ("call", "0x645930"),
        0x52F36F: ("add", "ebx, 0x20"),
        0x52F375: ("mov", "ecx, dword ptr [ebp - 0x18]"),
        0x52F40D: ("lea", "ecx, [esi + 0x930]"),
        0x52F441: ("mov", "dword ptr [0x93e96c], 1"),
        0x52F461: ("mov", "eax, dword ptr [0x93e96c]"),
        0x52F4A9: ("mov", "dword ptr [0x93e96c], 1"),
        0x52F4CF: ("mov", "eax, dword ptr [0x93e970]"),
        0x52F4EF: ("mov", "esi, dword ptr [0x93e96c]"),
        0x533178: ("mov", "esi, 0x104"),
        0x5337AB: ("mov", "esi, 0x93ac88"),
        0x5347AC: ("mov", "byte ptr [0x93ac88], 0"),
    }
    for address, expected in pins.items():
        assert instructions[address] == expected, hex(address)
    # Shared path identity and observed widths only. Its adjacency to stage
    # 93AD8C proves no enclosing allocation or malformed overwrite behavior.

"""Recover the shared native unmarked session-options redraw tail."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_netdaemon_transfer_modal import extract

ROOT = Path(__file__).resolve().parents[2]
BEFORE = '''    NetWinSessionOptions.receive_wire_block(
        reinterpret_cast<const unsigned char *>(message + 0x10));
    netdaemon_rebroadcast_session_packet(daemon, message, message_size);
'''
AFTER = '''    NetWinSessionOptions.receive_wire_block(
        reinterpret_cast<const unsigned char *>(message + 0x10));
    // Native 0x537C57 shares the unmarked redraw tail at 0x537CB4
    // with the session-data packet; the option block is already copied.
    unsigned short type = *reinterpret_cast<unsigned short *>(message);
    if ((type & 0x2000u) == 0) {
        NetWinDialog.on_redraw();
    } else {
        type = static_cast<unsigned short>((type & 0xDFFFu) | 0x4000u);
        *reinterpret_cast<unsigned short *>(message) = type;
        daemon->send_message(message, message_size, 0);
    }
'''


def apply_session_options_migration(source, reverse=False):
    from test_netdaemon_project_notification_oct11 import apply_project_notification_migration
    if reverse:
        source = apply_project_notification_migration(source, reverse=True)
    before, after = (AFTER, BEFORE) if reverse else (BEFORE, AFTER)
    assert source.count(before) == 1
    source = source.replace(before, after, 1)
    return source if reverse else apply_project_notification_migration(source)


def test_only_reviewed_options_tail_changed():
    before = subprocess.check_output(
        ["git", "show", "7cbf413a2:src/netdaemon.cpp"], cwd=ROOT, text=True)
    assert apply_session_options_migration(before) == (ROOT / "src/netdaemon.cpp").read_text()


def test_options_rereads_opcode_once_after_copy():
    body = extract((ROOT / "src/netdaemon.cpp").read_text(),
                   "static void netdaemon_handle_session_options(")
    read = "unsigned short type = *reinterpret_cast<unsigned short *>(message);"
    assert body.count(read) == 1
    assert body.index("receive_wire_block(") < body.index(read)
    assert "netdaemon_rebroadcast_session_packet(" not in body


@pytest.mark.parametrize("damage", ["none", "omit_redraw", "invert_marker", "early_redraw", "short_copy"])
def test_actual_options_copy_then_redraw_or_rebroadcast(tmp_path, damage):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/netdaemon.cpp").read_text()
    body = "\n".join(extract(source, signature) for signature in (
        "static void netdaemon_rebroadcast_session_packet(",
        "static void netdaemon_handle_session_options("))
    declaration = extract((ROOT / "src/netwin.h").read_text(),
                          "struct NetWinSessionOptionsState {") + ";"
    copy = extract((ROOT / "src/netwin.cpp").read_text(),
                   "void NetWinSessionOptionsState::receive_wire_block(")
    changes = {
        "omit_redraw": ("NetWinDialog.on_redraw();", "(void)0;"),
        "invert_marker": ("& 0x2000u) == 0", "& 0x2000u) != 0"),
        "early_redraw": ("    NetWinSessionOptions.receive_wire_block(",
                          "    NetWinDialog.on_redraw();\n    NetWinSessionOptions.receive_wire_block("),
    }
    if damage == "short_copy":
        assert "sizeof(*this)" in copy
        copy = copy.replace("sizeof(*this)", "0x14")
    elif damage != "none":
        before, after = changes[damage]
        assert body.count(before) == 1
        body = body.replace(before, after, 1)
    harness = r'''
#include <cassert>
#include <stdint.h>
#include <cstring>
@@DECLARATION@@
@@COPY@@
static NetWinSessionOptionsState NetWinSessionOptions;
static unsigned char expected[0x18];
static unsigned redraws, sends;
static char *active_message;
static unsigned short original_type;
static void check_copy() {
    assert(sizeof(NetWinSessionOptionsState) == 0x18);
    assert(std::memcmp(&NetWinSessionOptions, expected, sizeof expected) == 0);
}
static struct Dialog {
    void on_redraw() {
        check_copy();
        assert(*reinterpret_cast<unsigned short *>(active_message) == original_type);
        ++redraws;
    }
} NetWinDialog;
struct NetDaemon {
    void send_message(char *message, unsigned long size, int target) {
        check_copy();
        assert(message == active_message && size == 0x28 && target == 0);
        assert(*reinterpret_cast<unsigned short *>(message) == 0x4F02);
        ++sends;
    }
};
@@BODY@@
int main() {
    const unsigned short types[] = {0x1F02, 0x2F02, 0x4F02};
    for (int route = 0; route < 3; ++route) {
        uint32_t packet[10] = {0};
        active_message = reinterpret_cast<char *>(packet);
        original_type = types[route];
        std::memcpy(active_message, &original_type, 2);
        for (unsigned byte = 0; byte < sizeof expected; ++byte)
            expected[byte] = active_message[0x10 + byte] = static_cast<unsigned char>(byte * 7 + 1);
        std::memset(&NetWinSessionOptions, 0xA5, sizeof NetWinSessionOptions);
        sends = redraws = 0;
        NetDaemon daemon;
        netdaemon_handle_session_options(&daemon, active_message, sizeof packet);
        check_copy();
        assert(sends == (route == 1 ? 1u : 0u));
        assert(redraws == (route == 1 ? 0u : 1u));
        assert(*reinterpret_cast<unsigned short *>(active_message) == (route == 1 ? 0x4F02 : types[route]));
        assert(std::memcmp(active_message + 0x10, expected, sizeof expected) == 0);
    }
}
'''
    cpp = tmp_path / "options.cpp"
    cpp.write_text(harness.replace("@@DECLARATION@@", declaration)
                   .replace("@@COPY@@", copy).replace("@@BODY@@", body))
    exe = tmp_path / "options"
    subprocess.run([compiler, "-std=c++03", "-fsanitize=undefined,bounds",
                    "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, timeout=60)
    result = subprocess.run([str(exe)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage == "none"), result.stderr


def test_pinned_native_options_copy_and_shared_redraw():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    instructions = {i.address: (i.mnemonic, i.op_str) for i in
                    Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x537C42, 0x537CC6)), 0x537C42)}
    expected = {
        0x537C45: ("mov", "ecx, 6"),
        0x537C4A: ("mov", "edi, 0x90e8e0"),
        0x537C4F: ("rep movsd", "dword ptr es:[edi], dword ptr [esi]"),
        0x537C51: ("mov", "ax, word ptr [ebx]"),
        0x537C54: ("test", "ah, 0x20"),
        0x537C57: ("je", "0x537cb4"),
        0x537C59: ("and", "eax, 0xdfff"),
        0x537C61: ("or", "ah, 0x40"),
        0x537C64: ("push", "0"),
        0x537C66: ("mov", "word ptr [ebx], ax"),
        0x537C69: ("mov", "eax, dword ptr [ebp + 0xc]"),
        0x537C6C: ("push", "eax"),
        0x537C6D: ("push", "ebx"),
        0x537C6E: ("call", "0x532940"),
        0x537CB4: ("mov", "ecx, 0x80a6f8"),
        0x537CB9: ("call", "0x481ad0"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction

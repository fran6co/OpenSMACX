"""Actual-source host controls, not native matching or game acceptance."""
import shutil
import subprocess
import tempfile
from pathlib import Path

from recovery.tests.test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = r"""
#include <cstdlib>
#define __cdecl
int NetBusyCount;
int PopupWriteCheckState;
static void require(bool value) { if (!value) std::abort(); }
static char *expected_message;
static unsigned long expected_length;
static int expected_target;
struct NetDaemon {
    void send_message(char *message, unsigned long length, int target) {
        require(NetBusyCount == 5);
        require(message == expected_message);
        require(length == expected_length);
        require(target == expected_target);
    }
};
NetDaemon NetDaemonNet;
// @@BODIES@@
int main() {
    char payload[7] = {'p', 'a', 'y', 'l', 'o', 'a', 'd'};
    expected_message = payload;
    expected_length = sizeof payload;
    expected_target = -3;
    NetBusyCount = 4;
    netdaemon_send_message_counted(payload, sizeof payload, -3);
    require(NetBusyCount == 4);
    for (int bit = 0; bit != 32; ++bit) {
        const unsigned int mask = 1U << bit;
        PopupWriteCheckState = static_cast<int>(0xA55A39C3U);
        const unsigned int before = static_cast<unsigned int>(PopupWriteCheckState);
        popup_set_write_check_bit(bit, 7);
        require(static_cast<unsigned int>(PopupWriteCheckState) == (before | mask));
        popup_set_write_check_bit(bit, 0);
        require(static_cast<unsigned int>(PopupWriteCheckState) == (before & ~mask));
    }
    PopupWriteCheckState = 0;
    popup_set_write_check_bit(-1, 1);
    require(static_cast<unsigned int>(PopupWriteCheckState) == 0x80000000U);
    popup_set_write_check_bit(32, 0);
    require(static_cast<unsigned int>(PopupWriteCheckState) == 0x80000000U);
    popup_set_write_check_bit(64, -7);
    require(static_cast<unsigned int>(PopupWriteCheckState) == 0x80000001U);
    return 0;
}
"""


def actual_bodies():
    return "\n".join((
        definition((ROOT / "src/netdaemon.cpp").read_text(),
                   r"void\s+__cdecl\s+netdaemon_send_message_counted\s*\("),
        definition((ROOT / "src/popup.cpp").read_text(),
                   r"void\s+__cdecl\s+popup_set_write_check_bit\s*\("),
    ))


def test_actual_short_owner_behavior_and_damage_controls():
    bodies = actual_bodies()
    mutations = (
        ("++NetBusyCount;", "/* missing increment */"),
        ("--NetBusyCount;", "/* missing decrement */"),
        ("enabled != 0", "enabled == 0"),
        ("& 31U", "& 15U"),
    )
    compilers = [path for name in ("g++", "clang++")
                 if (path := shutil.which(name))]
    assert compilers, "GCC or Clang required for host controls"
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="short-owner-control-") as temp:
            cpp = Path(temp) / "control.cpp"
            exe = Path(temp) / "control"
            variants = [bodies]
            for old, new in mutations:
                assert old in bodies
                variants.append(bodies.replace(old, new, 1))
            for index, variant in enumerate(variants):
                cpp.write_text(FIXTURE.replace("// @@BODIES@@", variant))
                result = subprocess.run(
                    [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                     "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                     str(cpp), "-o", str(exe)],
                    capture_output=True, text=True, timeout=45,
                )
                assert result.returncode == 0, result.stdout + result.stderr
                result = subprocess.run([str(exe)], capture_output=True,
                                        text=True, timeout=15)
                if index == 0:
                    assert result.returncode == 0, result.stdout + result.stderr
                else:
                    assert result.returncode != 0, "mutated actual source escaped detection"

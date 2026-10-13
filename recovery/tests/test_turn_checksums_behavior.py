"""Actual-source checksum controls. Defined producer paths and diagnostic formatting only; the original unwritten tail and native runtime remain unproven."""
from pathlib import Path
import shutil
import subprocess

import pytest
from test_net_checksum_report_oct13at import report_declaration


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
FIXTURE = r'''#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
REPORT_DECLARATION

#define __cdecl

class NetDaemon {
public:
    void game_checksum(NetChecksumReport *results);
};

extern NetDaemon NetDaemonNet;
extern int NetTurnChecksums[21];
extern int BaseAtDebugPopupFlag;
extern int TurnCurrentNum;
enum { MaxPlayerNum = 8 };
struct HostPlayer { char adj_name_faction[64]; };
extern HostPlayer Players[MaxPlayerNum];
extern char StringTemp[1032];
struct HostGameFields { int turn_current_num_; };
struct HostGameState { HostGameFields fields_; };
extern HostGameState GameDataStateStorage;

char *_itoa(int value, char *text, int radix);
void log_say(const char *text, int value, int, int);
void log_say_hex(const char *text, int value, int, int);

#if defined(OPENSMACX_DAMAGE_COPY_SOURCE)
static void *host_damaged_memcpy(void *destination, const void *source,
                                 size_t count) {
    return std::memcpy(destination,
                       static_cast<const char *>(source) + sizeof(int), count);
}
#define memcpy(destination, source, count) \
    host_damaged_memcpy(destination, source, count)
#endif

/* ACTUAL_DO_CHECKSUMS_BODY */

#if defined(OPENSMACX_DAMAGE_COPY_SOURCE)
#undef memcpy
#endif

NetDaemon NetDaemonNet;
int NetTurnChecksums[21];
int BaseAtDebugPopupFlag;
int TurnCurrentNum = 37;
HostPlayer Players[MaxPlayerNum];
char StringTemp[1032];
HostGameState GameDataStateStorage;

static int checksum_calls;
static std::vector<int> hex_values;
static std::vector<std::string> hex_messages;
static std::vector<std::string> scratch_at_hex_call;
static std::vector<std::string> say_messages;
static std::vector<int> say_values;

void NetDaemon::game_checksum(NetChecksumReport *results) {
    ++checksum_calls;
    for (int index = 0; index < 5; ++index) results->common[index] = 0x100 + index;
    for (int index = 0; index < 16; ++index) results->map_sections[index] = 0x105 + index;
    if (BaseAtDebugPopupFlag != 0) {
        for (int index = 0; index < 3; ++index) results->diagnostic_aggregates[index] = 0x115 + index;
        for (int index = 0; index < 7; ++index) results->faction_values[index] = 0x118 + index;
        for (int index = 0; index < 7; ++index) results->map_values[index] = 0x11f + index;
        for (int index = 0; index < 8; ++index) results->vehicles_by_faction[index] = 0x126 + index;
        results->base_value = 0x12e;
    }
}

char *_itoa(int value, char *text, int radix) {
    assert(radix == 10);
    std::sprintf(text, "%d", value);
    return text;
}

void log_say(const char *text, int value, int, int) {
    say_messages.push_back(text);
    say_values.push_back(value);
}

void log_say_hex(const char *message, int value, int, int) {
    hex_values.push_back(value);
    hex_messages.push_back(message);
    scratch_at_hex_call.push_back(StringTemp);
}

static void clear_logs() {
    hex_values.clear();
    hex_messages.clear();
    scratch_at_hex_call.clear();
    say_messages.clear();
    say_values.clear();
}

int main() {
    GameDataStateStorage.fields_.turn_current_num_ = TurnCurrentNum;
    for (int index = 0; index < MaxPlayerNum; ++index) {
        std::sprintf(Players[index].adj_name_faction, "Faction%d", index);
    }

    static const char *const common_labels[5] = {
        "GAME checksum = ", "LEADERS checksum = ", "MAP checksum = ",
        "VEHICLES checksum = ", "BASES checksum = "
    };
    static const char *const aggregate_labels[3] = {
        "Rules checksum = ", "Projects checksum = ", "Tech checksum = "
    };
    static const char *const map_labels[7] = {
        "Map climate checksum = ", "Map region checksum = ",
        "Map bit checksum = ", "Map contour checksum = ",
        "Map terr checksum = ", "Map bitwho[0] checksum = ",
        "Params checksum = "
    };
    char expected[256];

    BaseAtDebugPopupFlag = 0;
    do_checksums(0);
    assert(checksum_calls == 0);
    assert(hex_messages.empty());

    NetChecksumReport ordinary_results;
    std::memset(&ordinary_results, 0x5a, sizeof(ordinary_results));
    NetDaemonNet.game_checksum(&ordinary_results);
    assert(checksum_calls == 1);
    for (int index = 0; index < 5; ++index) assert(ordinary_results.common[index] == 0x100 + index);
    for (int index = 0; index < 16; ++index) assert(ordinary_results.map_sections[index] == 0x105 + index);
    // This is caller poison only; no assertion gives native uninitialized bytes a value.
    const unsigned char *ordinary_bytes=reinterpret_cast<const unsigned char *>(&ordinary_results);
    for (int index=0x54;index<0xBC;++index) assert(ordinary_bytes[index]==0x5a);

    clear_logs();
    checksum_calls = 0;
    BaseAtDebugPopupFlag = 1;
    std::strcpy(StringTemp, "sentinel");
    do_checksums(4);
    assert(checksum_calls == 1);
    assert(hex_values.size() == 47);
    assert(say_messages.size() == 3);
    assert(say_messages[0] == " ");
    assert(say_messages[1] == "Checksums at beginning of turn");
    assert(say_values[1] == TurnCurrentNum);
    assert(say_messages[2] == " ");

    for (int index = 0; index < 47; ++index) {
        assert(hex_values[index] == 0x100 + index);
    }
    for (int index = 0; index < 5; ++index) {
        std::sprintf(expected, "%d %s", 0x100 + index, common_labels[index]);
        assert(hex_messages[index] == expected);
        assert(scratch_at_hex_call[index] == "sentinel");
    }
    for (int index = 0; index < 16; ++index) {
        std::sprintf(expected, "%d Map SECTION %d = ",
                     0x100 + index + 5, index);
        assert(hex_messages[index + 5] == expected);
        std::sprintf(expected, "Map SECTION %d = ", index);
        assert(scratch_at_hex_call[index + 5] == expected);
    }
    for (int index = 0; index < 3; ++index) {
        std::sprintf(expected, "%d %s", 0x100 + index + 21,
                     aggregate_labels[index]);
        assert(hex_messages[index + 21] == expected);
        assert(scratch_at_hex_call[index + 21] == "Map SECTION 15 = ");
    }
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        std::sprintf(expected, "%d Faction%d checksum = ",
                     0x100 + faction + 23, faction);
        assert(hex_messages[faction + 23] == expected);
        std::sprintf(expected, "Faction%d checksum = ", faction);
        assert(scratch_at_hex_call[faction + 23] == expected);
    }
    for (int index = 0; index < 7; ++index) {
        std::sprintf(expected, "%d %s", 0x100 + index + 31, map_labels[index]);
        assert(hex_messages[index + 31] == expected);
        assert(scratch_at_hex_call[index + 31] == "Faction7 checksum = ");
    }
    for (int faction = 0; faction < MaxPlayerNum; ++faction) {
        std::sprintf(expected, "%d Faction%d Vehicles checksum = ",
                     0x100 + faction + 38, faction);
        assert(hex_messages[faction + 38] == expected);
        std::sprintf(expected, "Faction%d Vehicles checksum = ", faction);
        assert(scratch_at_hex_call[faction + 38] == expected);
    }
    std::sprintf(expected, "%d Bases checksum = ", 0x100 + 46);
    assert(hex_messages[46] == expected);
    assert(scratch_at_hex_call[46] == "Faction7 Vehicles checksum = ");
    assert(std::strcmp(StringTemp, "Faction7 Vehicles checksum = ") == 0);

#if defined(OPENSMACX_EXPECT_DAMAGE)
    assert(NetTurnChecksums[0] != 0x100);
#else
    for (int index = 0; index < 21; ++index) {
        assert(NetTurnChecksums[index] == 0x100 + index);
    }
#endif
}
'''
SIGNATURE = "void __cdecl do_checksums(int type)"
MARKER = "/* ACTUAL_DO_CHECKSUMS_BODY */"


def extract_actual_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated actual body: {signature}")


def compile_and_run(compiler: str, tmp_path: Path, *, damaged: bool) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")

    source = SOURCE.read_text()
    assert source.count(SIGNATURE) == 1, "expected one source-owned body, not a forwarder"
    body = extract_actual_body(source, SIGNATURE)
    fixture = FIXTURE
    assert fixture.count(MARKER) == 1
    cpp = tmp_path / f"actual_{compiler}_{'damage' if damaged else 'clean'}.cpp"
    binary = tmp_path / f"control_{compiler}_{'damage' if damaged else 'clean'}"
    cpp.write_text(fixture.replace("REPORT_DECLARATION", report_declaration())
                   .replace(MARKER, body))

    flags = ["-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic"]
    if damaged:
        flags += ["-DOPENSMACX_DAMAGE_COPY_SOURCE", "-DOPENSMACX_EXPECT_DAMAGE"]
    built = subprocess.run(
        [executable, *flags, str(cpp), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr

    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damaged", [False, True], ids=["actual-body", "damaged-copy-control"])
def test_actual_checksum_body_host_control(compiler, damaged, tmp_path):
    compile_and_run(compiler, tmp_path, damaged=damaged)


def test_host_fixture_marks_actual_body_once():
    fixture = FIXTURE
    assert fixture.count(MARKER) == 1

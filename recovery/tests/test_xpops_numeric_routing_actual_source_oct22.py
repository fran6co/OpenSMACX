"""Actual-source host controls for two mapped X_pop wrapper bodies.

These controls observe the recovered source's routing and arguments through
small test doubles. They do not establish target ABI, runtime file behavior,
or native linker selection; the optional pinned listing is an independent
instruction/operand witness.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.pe_coff import PeImage, Span
from match_recovery.source.asm import Listing
from match_recovery.source import read_file
from test_buffer_typed_font_consumers_oct10 import ROOT, extract_definition


SOURCE = ROOT / "src/xpops.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ASK_NUMBER = 0x005BFD70
XPOPS_FLAGS = 0x005BF730
ASK_NUMBER_NAME = "?X_pop_ask_number@@YAHPADPBDHP6AHXZ@Z"
XPOPS_FLAGS_NAME = "?X_pops@@YAHPADPBDPADHPAUSprite@@HHP6AHXZ@Z"
XPOPS_FLAGS_SYMBOL = "?X_pops@@YAHPADPBD0HPAVSprite@@HHP6AHXZ@Z"


NUMBER_SIGNATURE = "int __cdecl X_pop_ask_number(char *caption, const char *label, int value,"
FLAGS_SIGNATURE = "int __cdecl X_pops(char *caption, const char *label, char *text, int value,"


NUMBER_FIXTURE = r'''#include <cassert>
#include <cstdio>
#include <cstring>
#define __cdecl

struct PlayerRow { unsigned int rule_flags; };
static PlayerRow Players[9];
static const unsigned int RFLAG_ALIEN = 0x80u;
struct ConsoleState { int faction_id_; };
static ConsoleState ConsoleGlobal;
static int PopupDialogFactionID;
static int ExpansionEnabled;

static char ScriptPopupFile[] = "alienIscript.txt";
static char XPopModCheckString2Text[] = "SCRIPT.txt";
static char XPopModCheckFile1Text[] = "alienIscript.txt";
static char XPopModCheckMode1Text[] = "rb";
static char XPopModCheckFile2Text[] = "alienuscript.txt";
static char XPopModCheckMode2Text[] = "rb";
static char PopupStartAltFile1[] = "ALT-ISCRIPT";
static char PopupStartAltFile2[] = "ALT-USCRIPT";

static int file1_opens;
static int file2_opens;
static int close_calls;
static int popup_calls;
static int callback_calls;
static int file1_succeeds;
static int file2_succeeds;
static int popup_results[4];
static char events[16];
static int event_count;
static const char *opened_name;
static const char *opened_mode;
static char *popup_captions[4];
static const char *popup_labels[4];
static int popup_values[4];
static int (*popup_callbacks[4])();

static FILE *test_fopen(const char *name, const char *mode) {
    events[event_count++] = 'O';
    opened_name = name;
    opened_mode = mode;
    if (std::strcmp(name, XPopModCheckFile1Text) == 0) {
        ++file1_opens;
        return file1_succeeds ? reinterpret_cast<FILE *>(1) : 0;
    }
    if (std::strcmp(name, XPopModCheckFile2Text) == 0) {
        ++file2_opens;
        return file2_succeeds ? reinterpret_cast<FILE *>(1) : 0;
    }
    return 0;
}

static int test_fclose(FILE *file) {
    events[event_count++] = 'C';
    if (file != reinterpret_cast<FILE *>(1)) return -9;
    ++close_calls;
    return 0;
}

static const char *XPopModCheckString1() { return ScriptPopupFile; }

static int pop_ask_number(char *caption, const char *label, int value,
                          int (__cdecl *callback)()) {
    events[event_count++] = 'P';
    const int index = popup_calls++;
    if (index >= 4) return -99;
    popup_captions[index] = caption;
    popup_labels[index] = label;
    popup_values[index] = value;
    popup_callbacks[index] = callback;
    return popup_results[index];
}

static int callback() { ++callback_calls; return 71; }
'''


NUMBER_MAIN = r'''
static void reset_number_case(int current_alien, int expansion, int dialog_id,
                              int dialog_alien, int open1, int open2,
                              int alt_result, int fallback_result) {
    for (int i = 0; i < 9; ++i) Players[i].rule_flags = 0;
    ConsoleGlobal.faction_id_ = 2; // always a valid console faction
    Players[2].rule_flags = current_alien ? RFLAG_ALIEN : 0;
    if (dialog_id >= 0 && dialog_id < 9)
        Players[dialog_id].rule_flags = dialog_alien ? RFLAG_ALIEN : 0;
    PopupDialogFactionID = dialog_id;
    ExpansionEnabled = expansion;
    file1_succeeds = open1;
    file2_succeeds = open2;
    file1_opens = file2_opens = close_calls = popup_calls = 0;
    callback_calls = event_count = 0;
    opened_name = opened_mode = 0;
    popup_results[0] = alt_result;
    popup_results[1] = fallback_result;
    for (int i = 0; i < 4; ++i) {
        popup_captions[i] = 0;
        popup_labels[i] = 0;
        popup_values[i] = 0;
        popup_callbacks[i] = 0;
    }
}

static void verify_numeric_case(char *caption, int current_alien, int expansion,
                                int dialog_id, int dialog_alien,
                                int open1, int open2, int alt_result,
                                int fallback_result, int expected_opens,
                                int expected_closes, int expected_popups,
                                char *expected_alt_caption,
                                const char *expected_file,
                                const char *expected_events) {
    const char label[] = "INTEGRITY_LABEL";
    reset_number_case(current_alien, expansion, dialog_id, dialog_alien,
                      open1, open2, alt_result, fallback_result);
    popup_results[0] = expected_alt_caption ? alt_result : fallback_result;
    const int result = X_pop_ask_number(caption, label, 0x1234, callback);
    const int expected_result = expected_alt_caption && alt_result != -1
        ? alt_result : fallback_result;
    assert(result == expected_result);
    assert(file1_opens + file2_opens == expected_opens);
    assert(close_calls == expected_closes);
    assert(popup_calls == expected_popups);
    assert(callback_calls == 0);
    assert(event_count == (int)std::strlen(expected_events));
    assert(std::memcmp(events, expected_events, event_count) == 0);
    if (expected_file) {
        assert(opened_name && std::strcmp(opened_name, expected_file) == 0);
        assert(opened_mode && std::strcmp(opened_mode, "rb") == 0);
    } else {
        assert(opened_name == 0 && opened_mode == 0);
    }
    if (expected_popups > 0) {
        assert(popup_captions[0] == (expected_alt_caption ? expected_alt_caption : caption));
        assert(popup_labels[0] == label && popup_values[0] == 0x1234);
        assert(popup_callbacks[0] == callback);
    }
    if (expected_popups == 2) {
        assert(popup_captions[1] == caption);
        assert(popup_labels[1] == label && popup_values[1] == 0x1234);
        assert(popup_callbacks[1] == callback);
    }
}

int main() {
    static char other_caption[] = "OTHER";

    // Caption bypass and expansion-disabled gates do not probe a file.
    verify_numeric_case(other_caption, 1, 1, 1, 1, 1, 1, 0, 55,
                        0, 0, 1, 0, 0, "P");
    verify_numeric_case(ScriptPopupFile, 1, 0, 0, 0, 1, 1, 0, 56,
                        0, 0, 1, 0, 0, "P");

    // Both special captions use the current alien's first file. Open failure
    // falls back; a successful open is closed before the alternate popup.
    verify_numeric_case(ScriptPopupFile, 1, 1, 0, 0, 0, 0, 12, 57,
                        1, 0, 1, 0, XPopModCheckFile1Text, "OP");
    verify_numeric_case(XPopModCheckString2Text, 1, 1, 0, 0, 1, 1, 0, 58,
                        1, 1, 1, PopupStartAltFile1,
                        XPopModCheckFile1Text, "OCP");
    verify_numeric_case(ScriptPopupFile, 1, 1, 0, 0, 1, 1, -2, 59,
                        1, 1, 1, PopupStartAltFile1,
                        XPopModCheckFile1Text, "OCP");

    // The non-alien path has strict dialog ID bounds, expansion gate and
    // alien-bit test. IDs 0 and 8 must not route, while both edge-valid IDs
    // 1 and 7 are accepted when their row is alien.
    verify_numeric_case(XPopModCheckString2Text, 0, 1, 0, 1, 1, 1, 4, 60,
                        0, 0, 1, 0, 0, "P");
    verify_numeric_case(ScriptPopupFile, 0, 0, 1, 1, 1, 1, 4, 61,
                        0, 0, 1, 0, 0, "P");
    verify_numeric_case(XPopModCheckString2Text, 0, 1, 1, 1, 1, 1, -1, 62,
                        1, 1, 2, PopupStartAltFile2,
                        XPopModCheckFile2Text, "OCPP");
    verify_numeric_case(ScriptPopupFile, 0, 1, 7, 1, 1, 1, -7, 63,
                        1, 1, 1, PopupStartAltFile2,
                        XPopModCheckFile2Text, "OCP");
    verify_numeric_case(XPopModCheckString2Text, 0, 1, 8, 1, 1, 1, 2, 64,
                        0, 0, 1, 0, 0, "P");
    verify_numeric_case(ScriptPopupFile, 0, 1, 1, 0, 1, 1, 2, 65,
                        0, 0, 1, 0, 0, "P");
    return 0;
}
'''


FLAGS_FIXTURE = r'''#include <cassert>
#include <cstring>
#define __cdecl

struct Sprite { int marker; };
static int callback_calls;
static int core_calls;
static char *seen_caption;
static const char *seen_label;
static int seen_length;
static char *seen_text;
static int seen_value;
static Sprite *seen_sprite;
static int seen_flag1;
static int seen_flag2;
static int (*seen_callback)();
static int core_result;

static int callback() { ++callback_calls; return 99; }

static int __cdecl X_pops(char *caption, const char *label, int length,
                          char *text, int value, Sprite *sprite,
                          int flag1, int flag2, int (__cdecl *cb)()) {
    ++core_calls;
    seen_caption = caption;
    seen_label = label;
    seen_length = length;
    seen_text = text;
    seen_value = value;
    seen_sprite = sprite;
    seen_flag1 = flag1;
    seen_flag2 = flag2;
    seen_callback = cb;
    return core_result;
}
'''


FLAGS_MAIN = r'''
int main() {
    static char caption[] = "caller caption";
    static const char label[] = "caller label";
    Sprite sprite = {7};
    char empty[] = "";
    char text[] = "four";

    core_calls = callback_calls = 0;
    assert(X_pops(caption, label, static_cast<char *>(0), 19,
                  &sprite, -4, 0, callback) == -1);
    assert(core_calls == 0 && callback_calls == 0);

    core_result = -37;
    assert(X_pops(caption, label, empty, 0, &sprite, -1, 0, callback) == -37);
    assert(core_calls == 1 && seen_caption == caption && seen_label == label);
    assert(seen_length == 0 && seen_text == empty && seen_value == 0);
    assert(seen_sprite == &sprite && seen_flag1 == -1 && seen_flag2 == 0);
    assert(seen_callback == callback && callback_calls == 0);

    core_result = 42;
    assert(X_pops(caption, label, text, 0x1234, &sprite, 7, -5, callback) == 42);
    assert(core_calls == 2 && seen_caption == caption && seen_label == label);
    assert(seen_length == 4 && seen_text == text && seen_value == 0x1234);
    assert(seen_sprite == &sprite && seen_flag1 == 7 && seen_flag2 == -5);
    assert(seen_callback == callback && callback_calls == 0);
    return 0;
}
'''


NUMBER_MUTANTS = {
    "caption_route": (
        "std::strcmp(caption, XPopModCheckString1()) != 0 &&\n        std::strcmp(caption, XPopModCheckString2Text) != 0",
        "std::strcmp(caption, XPopModCheckString1()) != 0 ||\n        std::strcmp(caption, XPopModCheckString2Text) != 0",
    ),
    "expansion_gate": ("if (ExpansionEnabled != 0) {", "if (ExpansionEnabled == 0) {"),
    "alternate_return_minus_one": ("if (result != -1) return result;",
                                    "if (result >= 0) return result;"),
    "fallback_caption": ("return pop_ask_number(caption, section, value, callback);",
                         "return pop_ask_number(PopupStartAltFile1, section, value, callback);"),
}

FLAGS_MUTANTS = {
    "length": ("static_cast<int>(std::strlen(text))",
               "static_cast<int>(std::strlen(text)) + 1"),
    "null_result": ("return -1;", "return 0;"),
    "flags": ("value, sprite, flag1, flag2, callback",
              "value, sprite, flag1 ^ 1, flag2, callback"),
}


def mapped_body(address: int, original_name: str, emitted_symbol: str,
                signature: str) -> str:
    records = {record.address: record for record in read_file(SOURCE)}
    record = records[address]
    assert record.name == original_name
    assert record.symbol == emitted_symbol
    assert record.region
    return extract_definition(record.region, f"0x{address:08X}", signature)


def numeric_body() -> str:
    return mapped_body(ASK_NUMBER, ASK_NUMBER_NAME, "", NUMBER_SIGNATURE)


def flags_body() -> str:
    return mapped_body(XPOPS_FLAGS, XPOPS_FLAGS_NAME, XPOPS_FLAGS_SYMBOL,
                       FLAGS_SIGNATURE)


def compile_and_run(compiler: str, tmp_path: Path, source: str,
                    name: str) -> subprocess.CompletedProcess:
    executable = tmp_path / name
    built = subprocess.run(
        [compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-",
         "-o", str(executable)], input=source, text=True, capture_output=True,
        cwd=tmp_path, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          cwd=tmp_path, timeout=10)


def _compiler_names() -> list[str]:
    return [found for name in ("g++", "clang++") if (found := shutil.which(name))]


def numeric_program(body: str) -> str:
    return (NUMBER_FIXTURE + "\n#define fopen test_fopen\n#define fclose test_fclose\n" +
            body + "\n#undef fopen\n#undef fclose\n" + NUMBER_MAIN)


def flags_program(body: str) -> str:
    return FLAGS_FIXTURE + "\n" + body + "\n" + FLAGS_MAIN


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_numeric_router_preserves_faction_file_and_popup_order(compiler, tmp_path):
    result = compile_and_run(compiler, tmp_path, numeric_program(numeric_body()), "numeric-router")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", _compiler_names())
@pytest.mark.parametrize("mutant", sorted(NUMBER_MUTANTS))
def test_numeric_router_compilable_mutants_fail_runtime_control(compiler, mutant, tmp_path):
    body = numeric_body()
    before, after = NUMBER_MUTANTS[mutant]
    assert body.count(before) >= 1, f"mutant anchor missing: {mutant}"
    damaged = body.replace(before, after)
    result = compile_and_run(compiler, tmp_path, numeric_program(damaged), "numeric-mutant")
    assert result.returncode != 0, f"compiled mutant escaped runtime control: {mutant}"


@pytest.mark.parametrize("compiler", _compiler_names())
def test_actual_xpops_flags_wrapper_preserves_null_length_pointers_flags_and_result(compiler, tmp_path):
    result = compile_and_run(compiler, tmp_path, flags_program(flags_body()), "xpops-flags")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", _compiler_names())
@pytest.mark.parametrize("mutant", sorted(FLAGS_MUTANTS))
def test_xpops_flags_wrapper_compilable_mutants_fail_runtime_control(compiler, mutant, tmp_path):
    body = flags_body()
    before, after = FLAGS_MUTANTS[mutant]
    assert body.count(before) == 1, f"mutant anchor must be unique: {mutant}"
    damaged = body.replace(before, after, 1)
    result = compile_and_run(compiler, tmp_path, flags_program(damaged), "xpops-flags-mutant")
    assert result.returncode != 0, f"compiled mutant escaped runtime control: {mutant}"


def test_optional_pinned_numeric_and_flags_instruction_witnesses():
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        image_path = Path(configured).expanduser().resolve()
        assert image_path.is_file(), f"configured pinned image is missing: {image_path}"
    else:
        image_path = ROOT / ".opensmacx/game/terranx_original.exe"
        if not image_path.is_file():
            image_path = next((parent / ".opensmacx/game/terranx_original.exe"
                               for parent in ROOT.parents
                               if (parent / ".opensmacx/game/terranx_original.exe").is_file()),
                              image_path)
        if not image_path.is_file():
            pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    number = Listing(image.read(Span(0x005BFD70, 0x005BFE8E)), 0x005BFD70)
    flags = Listing(image.read(Span(0x005BF730, 0x005BF770)), 0x005BF730)
    assert len(number.instructions) == 111
    assert len(flags.instructions) == 31
    numeric_calls = [(ins.address, ins.op_str) for ins in number.instructions
                     if ins.mnemonic == "call"]
    assert numeric_calls == [
        (0x005BFD80, "0x645660"), (0x005BFD92, "0x645660"),
        (0x005BFDAB, "0x627c30"), (0x005BFDF3, "0x645646"),
        (0x005BFE00, "0x645598"), (0x005BFE0F, "0x627c30"),
        (0x005BFE51, "0x645646"), (0x005BFE5E, "0x645598"),
        (0x005BFE6D, "0x627c30"), (0x005BFE81, "0x627c30"),
    ]
    numeric_by_address = {ins.address: (ins.mnemonic, ins.op_str)
                          for ins in number.instructions}
    assert numeric_by_address[0x005BFDD3] == (
        "test", "byte ptr [ecx*4 + 0x946f58], dl")
    assert numeric_by_address[0x005BFE2C] == ("jle", "0x5bfe7a")
    assert numeric_by_address[0x005BFE2E] == ("cmp", "eax, 8")
    assert numeric_by_address[0x005BFE31] == ("jge", "0x5bfe7a")
    assert numeric_by_address[0x005BFE17] == ("cmp", "eax, -1")
    assert numeric_by_address[0x005BFE1A] == ("je", "0x5bfe7a")
    assert numeric_by_address[0x005BFE75] == ("cmp", "eax, -1")
    assert numeric_by_address[0x005BFE78] == ("jne", "0x5bfe89")

    flags_calls = [(ins.address, ins.op_str) for ins in flags.instructions
                   if ins.mnemonic == "call"]
    assert flags_calls == [(0x005BF755, "0x6453e0"), (0x005BF766, "0x5bf930")]
    flags_by_address = {ins.address: (ins.mnemonic, ins.op_str)
                        for ins in flags.instructions}
    assert flags_by_address[0x005BF736] == ("test", "eax, eax")
    assert flags_by_address[0x005BF738] == ("jne", "0x5bf73f")
    assert flags_by_address[0x005BF73A] == ("or", "eax, 0xffffffff")

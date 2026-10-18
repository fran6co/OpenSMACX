"""Actual-source controls for BasePop's display/modal pump and cleanup.

The C++ harness uses host stand-ins for the Win hierarchy and callbacks. It
checks source control flow and virtual dispatch; it is not Win32, VC6 ABI, or
game-runtime evidence.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/basepop.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXEC_SPAN = Span(0x00602600, 0x006037F5)
BLOCK = "/* ACTUAL_BASEPOP_PUMP */"


HARNESS = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };
struct Win;
struct BasePop;
static BasePop *active = 0;
static char events[128];
static int event_count = 0;
static Win *WinModalFocus = 0;
static Win *mutation_owner = 0;
static int BasePopModalLeft = 0;
static int BasePopModalTop = 0;
static int BasePopFalloutGate = 0;
static int BasePopFalloutFlag = 1;
static void add(char c) { events[event_count++] = c; events[event_count] = 0; }
static void fail(int id, const char *name) {
    fprintf(stderr, "WITNESS_%s\n", name);
    exit(id);
}

typedef void (*ConfigCallback)(void *, int);
typedef int (*ModalCallback)();

struct Win {
    virtual void show(int mode);
    virtual int set_modal(int show_bit, ModalCallback callback, Win *owner);
    virtual void hide();
    virtual ~Win() {}
};

struct DialogState {
    int kind_;
    DialogState() : kind_(-1) {}
    const char *get_text(int) { return ""; }
};

struct BasePop : Win {
    uint32_t field_30A8_;
    ConfigCallback field_3094_;
    int text_block_a_;
    int text_block_b_;
    Win *owner_window_;
    Win *external_window_;
    RECT client_rect_;
    DialogState dialogs_;
    ModalCallback callback;
    int pump_result;
    int selected_id_;
    int show_count;
    int hide_count;
    int modal_count;
    int show_hook;
    int modal_hook;
    int hide_hook;
    int parsed;

    BasePop() : field_30A8_(0), field_3094_(0), text_block_a_(11),
        text_block_b_(22), owner_window_(this), external_window_(0),
        callback(0), pump_result(0), selected_id_(7), show_count(0),
        hide_count(0), modal_count(0), show_hook(0), modal_hook(0),
        hide_hook(0), parsed(0) {
        client_rect_.left = 10; client_rect_.top = 20;
        client_rect_.right = 300; client_rect_.bottom = 200;
    }

    void pump() {
/* ACTUAL_BASEPOP_PUMP */
    }

    void parse_says(int, const char *, int, int) { ++parsed; add('P'); }
};

struct PopupProbe : BasePop {
    virtual void show(int mode) {
        ++show_count; add(mode == 3 ? 'S' : 's');
        if (show_hook == 1) field_30A8_ = 0x4100;
    }
    virtual int set_modal(int show_bit, ModalCallback, Win *owner) {
        ++modal_count; add('M');
        if (owner != 0) fail(91, "MODAL_OWNER_ARGUMENT");
        if (show_bit != static_cast<int>((field_30A8_ >> 21) & 1U)) fail(92, "SHOW_BIT");
        WinModalFocus = this;
        if (modal_hook == 1) field_30A8_ |= 0x200;
        if (modal_hook == 2) owner_window_ = mutation_owner;
        client_rect_.left = -31; client_rect_.top = 47;
        return 0;
    }
    virtual void hide() {
        ++hide_count; add('H');
        if (hide_hook == 1) field_3094_ = close_replacement;
    }
    static void close_replacement(void *, int);
};

struct ExternalProbe : Win {
    int modal_count;
    int last_show_bit;
    ExternalProbe() : modal_count(0), last_show_bit(-1) {}
    virtual int set_modal(int show_bit, ModalCallback, Win *owner) {
        ++modal_count; last_show_bit = show_bit; add('E');
        if (owner != 0) fail(93, "EXTERNAL_MODAL_OWNER_ARGUMENT");
        WinModalFocus = active;
        active->client_rect_.left = 61; active->client_rect_.top = -73;
        return 0;
    }
};

void Win::show(int) { fail(94, "BASE_SHOW_DISPATCH"); }
int Win::set_modal(int, ModalCallback, Win *) { fail(95, "BASE_MODAL_DISPATCH"); return 0; }
void Win::hide() { fail(96, "BASE_HIDE_DISPATCH"); }

static int open_calls = 0;
static int close_calls = 0;
static int replacement_close_calls = 0;
static int callback_mode = 0;
static void open_config(void *, int open) {
    if (open) {
        ++open_calls; add('O');
        if (callback_mode == 1) active->field_30A8_ = 0x4100;
    } else {
        ++close_calls; add('C');
    }
}
void PopupProbe::close_replacement(void *, int open) {
    if (!open) { ++replacement_close_calls; add('R'); }
}

static void reset(PopupProbe &p) {
    active = &p;
    events[0] = 0; event_count = 0;
    open_calls = close_calls = replacement_close_calls = 0;
    callback_mode = 0;
    WinModalFocus = reinterpret_cast<Win *>(static_cast<uintptr_t>(0x1234));
    BasePopModalLeft = 0; BasePopModalTop = 0;
    BasePopFalloutGate = 0; BasePopFalloutFlag = 1;
    mutation_owner = 0;
    p.field_30A8_ = 0; p.field_3094_ = open_config;
    p.owner_window_ = &p; p.external_window_ = 0;
    p.callback = 0; p.pump_result = 0; p.dialogs_.kind_ = -1;
    p.show_count = p.hide_count = p.modal_count = p.parsed = 0;
    p.show_hook = p.modal_hook = p.hide_hook = 0;
    p.client_rect_.left = 10; p.client_rect_.top = 20;
}

static void check(int cond, int id, const char *name) { if (!cond) fail(id, name); }

static void run_case(const char *name) {
    PopupProbe p;
    reset(p);
    if (strcmp(name, "nonmodal") == 0) {
        p.field_30A8_ = 0x4000;
        p.pump();
        check(strcmp(events, "OS") == 0, 1, "NONMODAL_SHOW_ORDER");
        check(p.show_count == 1 && p.modal_count == 0 && p.hide_count == 0, 2, "NONMODAL_NOHIDE");
        check(BasePopFalloutGate == 0 && close_calls == 0, 3, "NONMODAL_STATE");
    } else if (strcmp(name, "normal") == 0) {
        p.hide_hook = 1;
        p.pump();
        check(strcmp(events, "OSMHR") == 0, 5, "NORMAL_CALL_ORDER");
        check(p.show_count == 1 && p.modal_count == 1 && p.hide_count == 1, 6, "NORMAL_COUNTS");
        check(WinModalFocus == reinterpret_cast<Win *>(static_cast<uintptr_t>(0x1234)), 7, "FOCUS_RESTORE");
        check(BasePopModalLeft == -31 && BasePopModalTop == 47, 8, "RECT_CAPTURE");
        check(BasePopFalloutGate == 0 && BasePopFalloutFlag == 0, 9, "FALLOUT_CLEANUP");
        check(close_calls == 0 && replacement_close_calls == 1, 10, "CALLBACK_RELOAD_AFTER_HIDE");
    } else if (strcmp(name, "bit200") == 0) {
        p.modal_hook = 1;
        p.pump();
        check(strcmp(events, "OSM") == 0, 11, "BIT200_RETAINS_PUMP");
        check(BasePopFalloutGate == 1 && BasePopFalloutFlag == 1, 12, "BIT200_RETAINS_GATE");
        check(p.hide_count == 0 && close_calls == 0, 13, "BIT200_NO_CLEANUP");
        check(WinModalFocus == reinterpret_cast<Win *>(static_cast<uintptr_t>(0x1234)), 14, "BIT200_FOCUS_RESTORE");
    } else if (strcmp(name, "external") == 0) {
        ExternalProbe ext;
        p.external_window_ = &ext;
        p.field_30A8_ = 0x200000;
        p.pump();
        check(strcmp(events, "OSEHC") == 0, 15, "EXTERNAL_ORDER");
        check(ext.modal_count == 1 && ext.last_show_bit == 1, 16, "EXTERNAL_RECEIVER_SHOWBIT");
        check(p.modal_count == 0 && p.hide_count == 1, 17, "EXTERNAL_NO_SELF_MODAL");
        check(BasePopModalLeft == 61 && BasePopModalTop == -73, 18, "EXTERNAL_RECT_CAPTURE");
    } else if (strcmp(name, "show_mutates") == 0) {
        p.show_hook = 1;
        p.pump();
        check(strcmp(events, "OS") == 0, 19, "FRESH_INITIAL_GATE_AFTER_SHOW");
        check(p.modal_count == 0 && p.hide_count == 0, 20, "SHOW_MUTATED_GATES");
    } else if (strcmp(name, "open_mutates") == 0) {
        callback_mode = 1;
        p.pump();
        check(strcmp(events, "OS") == 0, 21, "OPEN_BEFORE_SHOW_AND_GATE");
        check(p.modal_count == 0 && p.hide_count == 0, 22, "FRESH_GATE_AFTER_CONFIG");
    } else if (strcmp(name, "modal_owner_mutates") == 0) {
        ExternalProbe ext;
        p.modal_hook = 2;
        mutation_owner = &ext;
        p.pump();
        check(strcmp(events, "OSM") == 0, 23, "FRESH_OWNER_CLEANUP_GATE");
        check(p.hide_count == 0 && BasePopFalloutGate == 1, 24, "OWNER_MUTATION_SKIPS_CLEANUP");
    } else if (strcmp(name, "kind4") == 0) {
        p.field_30A8_ = 0x4000;
        p.dialogs_.kind_ = 4;
        p.pump();
        check(p.parsed == 8 && strcmp(events, "OSPPPPPPPP") == 0, 25, "KIND4_AFTER_PUMP");
    } else {
        fail(97, "UNKNOWN_CASE");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) return 98;
    run_case(argv[1]);
    return 0;
}
'''


def _pump_block() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    start = source.index("pump:", source.index("int BasePop::exec(int flags,"))
    end = source.index("    // THE ANSWER:", start)
    block = source[start:end]
    block = block[block.index("\n") + 1:].rstrip()
    assert "show(3);" in block
    assert "(field_30A8_ & 0x4100)" in block
    assert "(field_30A8_ & 0x4300)" in block
    return block


def _compilers() -> list[str]:
    compilers = [c for c in (shutil.which("g++"), shutil.which("clang++")) if c]
    if not compilers:
        pytest.skip("host C++ compiler unavailable")
    return compilers


def _compile(tmp_path: Path, block: str, compiler: str) -> Path:
    cpp = tmp_path / "popup_display_modal.cpp"
    exe = tmp_path / "popup_display_modal"
    cpp.write_text(HARNESS.replace(BLOCK, block), encoding="utf-8")
    result = subprocess.run(
        [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45,
    )
    assert result.returncode == 0, result.stderr
    return exe


def _run(exe: Path, case: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run([str(exe), case], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("case", (
    "nonmodal", "normal", "bit200", "external", "show_mutates",
    "open_mutates", "modal_owner_mutates", "kind4",
))
def test_actual_source_pump_and_fresh_control_flow(
    tmp_path: Path, compiler: str, case: str,
) -> None:
    result = _run(_compile(tmp_path, _pump_block(), compiler), case)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("damage,case,witness", (
    ("prior-hide", "nonmodal", "WITNESS_NONMODAL_SHOW_ORDER"),
    ("wrong-cleanup-mask", "bit200", "WITNESS_BIT200_RETAINS_PUMP"),
))
def test_compiling_display_damage_controls_fail_named_behavior(
    tmp_path: Path, compiler: str, damage: str, case: str, witness: str,
) -> None:
    block = _pump_block()
    if damage == "prior-hide":
        assert block.count("show(3);") == 1
        block = block.replace("show(3);", "hide();", 1)
    else:
        assert block.count("0x4300") == 1
        block = block.replace("0x4300", "0x4100", 1)
    run = _run(_compile(tmp_path, block, compiler), case)
    assert run.returncode != 0, "damaged source unexpectedly passed behavior witness"
    assert witness in run.stderr


def _pinned_instructions() -> dict[int, object]:
    candidates = []
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        candidates.append(Path(configured))
    candidates.extend((ROOT / ".opensmacx/game/terranx_original.exe",
                       ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"))
    path = next((candidate for candidate in candidates if candidate.is_file()), candidates[0])
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    result: dict[int, object] = {}
    cursor = EXEC_SPAN.start
    for ins in decoder.disasm(image.read(EXEC_SPAN), EXEC_SPAN.start):
        assert ins.address == cursor, "decode gap in original BasePop::exec span"
        result[ins.address] = ins
        cursor += ins.size
    assert cursor == EXEC_SPAN.end
    return result


def test_pinned_exec_show_modal_and_postgate_cleanup_anchors() -> None:
    ins = _pinned_instructions()
    assert (ins[0x006036A1].mnemonic, ins[0x006036A1].op_str) == ("mov", "edx, dword ptr [esi]")
    assert (ins[0x006036A3].mnemonic, ins[0x006036A3].op_str) == ("push", "3")
    assert (ins[0x006036A5].mnemonic, ins[0x006036A5].op_str) == ("mov", "ecx, esi")
    assert (ins[0x006036A7].mnemonic, ins[0x006036A7].op_str) == ("call", "dword ptr [edx + 4]")
    assert any(i.mnemonic == "test" and "0x41" in i.op_str
               for a, i in ins.items() if 0x006036A8 <= a < 0x006036D0)
    assert any(i.mnemonic in ("cmp", "test") and "[esi + 0xa3c]" in i.op_str
               for a, i in ins.items() if 0x006036A8 <= a < 0x006036F0)
    assert any(i.mnemonic == "test" and "0x43" in i.op_str
               for a, i in ins.items() if 0x00603730 <= a < 0x00603770)
    assert any(i.mnemonic == "call" and "[edx + 8]" in i.op_str
               for a, i in ins.items() if 0x00603730 <= a < 0x00603790)
    assert any(i.mnemonic == "cmp" and "[esi + 0xa3c]" in i.op_str
               for a, i in ins.items() if 0x00603730 <= a < 0x00603790)

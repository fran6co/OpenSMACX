"""Compile and exercise the installed MultiWin::draw_time source body.

This intentionally reads only src/multiwin.cpp. It never falls back to an
ignored proposal or a seed when the product method is missing.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/multiwin.cpp"
MULTIWIN_HEADER = ROOT / "src/multiwin.h"
GRAPHICWIN_HEADER = ROOT / "src/graphicwin.h"
COUNCWIN_HEADER = ROOT / "src/councwin.h"
NETDAEMON_HEADER = ROOT / "src/netdaemon.h"


def _method(source: str) -> str:
    marker = "void MultiWin::draw_time("
    start = source.find(marker)
    assert start >= 0, "installed src/multiwin.cpp has no MultiWin::draw_time definition"
    opening = source.find("{", start)
    assert opening >= 0
    depth = 0
    quote = None
    line_comment = False
    block_comment = False
    escaped = False
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and following == "/":
                block_comment = False
                index += 1
        elif quote is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and following == "/":
            line_comment = True
            index += 1
        elif char == "/" and following == "*":
            block_comment = True
            index += 1
        elif char in "\"'":
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError("unterminated MultiWin::draw_time definition")


HOST = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
struct RECT { int left, top, right, bottom; };
struct Font {};
struct Spot { int args[6]; void add(int a,int b,int c,int d,int e,int f) { int v[]={a,b,c,d,e,f}; memcpy(args,v,sizeof(v)); } };
struct Buffer {
  int color;
  char last[1032];
  Font *font_seen;
  int font_args[3];
  Buffer *receiver_seen;
  unsigned int written_length;
  Buffer() : color(-1), last(), font_seen(0), font_args(), receiver_seen(0), written_length(0) {}
  void set_font(Font *f, int a, int b, int c) { font_seen=f; font_args[0]=a; font_args[1]=b; font_args[2]=c; receiver_seen=this; }
  void set_text_color(int c, int, int, int) { color = c; }
  void write_cent_l(char *s, RECT *, unsigned int n) { strcpy(last, s); written_length=n; receiver_seen=this; }
};
struct Win {};
struct GraphicWin : Win, Buffer { Spot spot_; };
struct MultiWin : GraphicWin {
  Font font2_;
  void draw_time(RECT *, int);
};
int NetDaemonWaitTicks, TurnRedrawPending, NetBusyCount, IsMultiplayerNet;
unsigned int NetDaemonProgressInterval;
int MultiWinState7FFF78;
uint32_t MultiWinState7FFF7C;
int ConsoleControlTurnActive, NetUpkeepPhase, TurnControlWord93A954;
int CouncilDialogActive;
struct Console { int faction_id_; } ConsoleGlobal;
struct GameDataFields {
  uint32_t game_state_, net_turn_flags_;
  int net_turn_faction_, time_controls_selection_;
  union { uint32_t enable_mask_; struct { unsigned char human_mask_, alive_mask_, pad_[2]; } bytes_; } lock_masks_;
};
struct { GameDataFields fields_; } GameDataStateStorage;
struct Council {
  uint32_t field_A24_, field_A3C_, leader_label_kind_[8];
} CouncilWindow;
struct LockState { uint32_t lead_[4], payload_, trailing_words_[10]; };
struct Network {
  struct PrimaryState { uint32_t leading_words_[3], flags_, trailing_words_[10]; } primary_state_;
  LockState channels_[2];
  uint32_t &factions_waiting_mask() { return primary_state_.trailing_words_[7]; }
  int who_2_pid(int identity) { return identity + 40; }
  char *get_player_name(unsigned long id) { static char name[32]; sprintf(name, "Player%lu", id); return name; }
} NetDaemonNet;
uint32_t &ConsoleFactionsWaitingMask = NetDaemonNet.factions_waiting_mask();
char StringTemp[1032];
char *label_get(int id) { static char labels[1200][32]; sprintf(labels[id], "L%d", id); return labels[id]; }
char *CharUpperA(char *s) { for (char *p=s; *p; ++p) *p=(char)toupper((unsigned char)*p); return s; }
char *_itoa(int value, char *out, int base) { (void)base; sprintf(out, "%d", value); return out; }
int MessageCount, WaveCount, EffectCount, MessagePopCount;
int MessageArgs[6], WaveArg, PopDelay, PopArgs[2], FXArg;
int message_data(int a, int b, int c, int d, int e, int f) { int v[]={a,b,c,d,e,f}; memcpy(MessageArgs,v,sizeof(v)); ++MessageCount; return 0; }
void wave_it(int a) { WaveArg=a; ++WaveCount; }
struct NetMsg { char text[32]; void pop(const char *s, int a, int b, int c) { strcpy(text,s); PopDelay=a; PopArgs[0]=b; PopArgs[1]=c; ++MessagePopCount; } } NetMsgPrimary;
struct FX { void play(int a) { FXArg=a; ++EffectCount; } } g_FX;
static RECT bounds = {0,0,40,12};
static void clear_state() {
  NetDaemonWaitTicks=0; TurnRedrawPending=0; NetBusyCount=0; IsMultiplayerNet=0;
  NetDaemonProgressInterval=0; MultiWinState7FFF78=0; MultiWinState7FFF7C=0;
  ConsoleControlTurnActive=0; NetUpkeepPhase=0; TurnControlWord93A954=0;
  GameDataStateStorage.fields_.time_controls_selection_=1; CouncilDialogActive=0; ConsoleFactionsWaitingMask=0;
  ConsoleGlobal.faction_id_=0;
  GameDataStateStorage.fields_.game_state_=0; GameDataStateStorage.fields_.net_turn_flags_=0;
  GameDataStateStorage.fields_.net_turn_faction_=0; GameDataStateStorage.fields_.lock_masks_.enable_mask_=0xff;
  CouncilWindow.field_A24_=0; CouncilWindow.field_A3C_=0;
  memset(CouncilWindow.leader_label_kind_,0,sizeof(CouncilWindow.leader_label_kind_));
  memset(&NetDaemonNet,0,sizeof(NetDaemonNet));
  MessageCount=WaveCount=EffectCount=MessagePopCount=0;
  WaveArg=PopDelay=FXArg=0; memset(MessageArgs,0,sizeof(MessageArgs)); memset(PopArgs,0,sizeof(PopArgs)); NetMsgPrimary.text[0]=0;
  StringTemp[0]=0;
}
int main() {
  MultiWin view;
  clear_state();
  NetDaemonWaitTicks=1500;
  view.draw_time(&bounds,1);
  if (NetDaemonWaitTicks != 1000 || MultiWinState7FFF7C != 1 || MultiWinState7FFF78 != 1) return 1;
  if (view.spot_.args[0]!=0 || view.spot_.args[1]!=2 || view.spot_.args[2]!=0 || view.spot_.args[3]!=0 || view.spot_.args[4]!=40 || view.spot_.args[5]!=12) return 8;
  if (view.font_seen!=&view.font2_ || view.font_args[0]!=0 || view.font_args[1]!=0 || view.font_args[2]!=0 || view.receiver_seen!=static_cast<Buffer*>(&view) || view.written_length!=strlen(view.last)) return 9;

  clear_state();
  NetDaemonWaitTicks=1000; NetDaemonProgressInterval=1;
  view.draw_time(&bounds,1);
  if (NetDaemonWaitTicks != 1000 || NetDaemonProgressInterval != 0 || view.color != 0) return 2;

  clear_state();
  NetDaemonWaitTicks=1000; NetBusyCount=1; NetDaemonProgressInterval=2;
  view.draw_time(&bounds,1);
  if (NetDaemonWaitTicks != 1000 || NetDaemonProgressInterval != 1 || MultiWinState7FFF7C != 1 || view.color != 0) return 3;

  clear_state();
  NetDaemonWaitTicks=1000; NetDaemonProgressInterval=0x80000000u;
  view.draw_time(&bounds,1);
  if (NetDaemonProgressInterval!=0x80000000u || NetDaemonWaitTicks!=1000) return 14;

  clear_state();
  NetDaemonWaitTicks=1500; NetDaemonNet.primary_state_.trailing_words_[4]=1;
  view.draw_time(&bounds,1);
  if (NetDaemonWaitTicks!=1500 || NetDaemonProgressInterval!=0 || MultiWinState7FFF7C!=1 || view.color!=0) return 10;

  clear_state();
  NetDaemonWaitTicks=200; MultiWinState7FFF78=5;
  NetDaemonNet.primary_state_.trailing_words_[1]=1;
  view.draw_time(&bounds,1);
  if (NetDaemonWaitTicks != 0 || (GameDataStateStorage.fields_.game_state_ & 0x800)==0 || WaveCount != 1) return 4;
  if (WaveArg!=0x21) return 11;

  clear_state();
  NetDaemonWaitTicks=15500; MultiWinState7FFF78=14;
  NetDaemonNet.primary_state_.trailing_words_[1]=1;
  view.draw_time(&bounds,1);
  if (MessagePopCount != 1 || EffectCount != 1 || MultiWinState7FFF78 != 15) return 5;
  if (strcmp(NetMsgPrimary.text,"TIMEWARNING")!=0 || PopDelay!=-5000 || PopArgs[0]!=0 || PopArgs[1]!=0 || FXArg!=0x23) return 12;

  clear_state();
  TurnRedrawPending=1; CouncilDialogActive=1; CouncilWindow.field_A3C_=500;
  view.draw_time(&bounds,1);
  if (CouncilWindow.field_A3C_ != 0 || MessageCount != 1 || MultiWinState7FFF78 != 0) return 6;
  if (MessageArgs[0]!=0x2601 || MessageArgs[1]!=0 || MessageArgs[2]!=0 || MessageArgs[3]!=-3 || MessageArgs[4]!=0 || MessageArgs[5]!=0) return 13;

  clear_state();
  GameDataStateStorage.fields_.net_turn_flags_=0x10;
  GameDataStateStorage.fields_.net_turn_faction_=1;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  view.draw_time(&bounds,0);
  if (strstr(view.last, "Player41") == 0) return 7;
  return 0;
}
'''


def test_draw_time_compiles_and_preserves_clock_paths_from_installed_source(tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    method = _method(source)
    header = MULTIWIN_HEADER.read_text(encoding="utf-8")
    graphicwin = GRAPHICWIN_HEADER.read_text(encoding="utf-8")
    council = COUNCWIN_HEADER.read_text(encoding="utf-8")
    netdaemon = NETDAEMON_HEADER.read_text(encoding="utf-8")
    assert "class MultiWin : public GraphicWin" in header
    assert "class GraphicWin : public Win, public Buffer" in graphicwin
    assert "extern unsigned int NetDaemonProgressInterval;" in netdaemon
    assert "Buffer *canvas = static_cast<Buffer *>(this);" in method
    assert "uint32_t field_A24_;  // 0xA24" in council
    assert "uint32_t field_A3C_;  // 0xA3C" in council
    assert "RECOVERY_MEMBER: v1 CouncWin leader_label_kind_ 0xA54" in council
    assert "uint32_t leader_label_kind_[8];  // 0xA54" in council
    required = (
        "MultiWinState7FFF7C", "NetDaemonWaitTicks", "CouncilWindow.field_A3C_",
        "NetDaemonNet.primary_state_.trailing_words_[4]",
        "spot_.add(0, 2, bounds->left, bounds->top,",
        "bounds->right - bounds->left, bounds->bottom - bounds->top);",
        "canvas->set_font(&font2_, 0, 0, 0);",
        "canvas->write_cent_l(StringTemp, bounds, text_length);",
        "message_data(0x2601, 0, ConsoleGlobal.faction_id_, -3, 0, 0)",
        "wave_it(0x21)", "NetMsgPrimary.pop(\"TIMEWARNING\", -5000, 0, 0)",
        "g_FX.play(0x23)", "canvas->set_text_color(0xe4, -1, 1, 1);",
        "canvas->set_text_color(0, -1, 1, 1);",
    )
    for token in required:
        assert token in method, f"installed draw_time lost pinned effect: {token}"
    # Mutation controls: exact helper arguments are enforced, not just call
    # presence.
    with pytest.raises(AssertionError):
        assert "wave_it(0x21)" in method.replace("wave_it(0x21)", "wave_it(0x22)")
    with pytest.raises(AssertionError):
        assert "message_data(0x2601, 0, ConsoleGlobal.faction_id_, -3, 0, 0)" in method.replace(
            "message_data(0x2601, 0, ConsoleGlobal.faction_id_, -3, 0, 0)",
            "message_data(0x2601, 0, ConsoleGlobal.faction_id_, -2, 0, 0)")

    # Original stack locals are intentionally left uninitialized when draw_type
    # is zero; this test checks the source form rather than normalizing it.
    assert "int local_14;" in method and "unsigned int local_10;" in method
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    program = HOST + "\n" + method + "\n"
    executable = tmp_path / "draw_time_host_control"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wno-uninitialized", "-Wno-unused-variable",
         "-x", "c++", "-", "-o", str(executable)],
        input=program, text=True, capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run([str(executable)], text=True, capture_output=True)
    assert run.returncode == 0, f"host scenario {run.returncode}: {run.stderr}"

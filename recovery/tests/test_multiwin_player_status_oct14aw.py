"""Actual-source controls for MultiWin::draw_player; not native/runtime proof."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
BASE = "14c1a91785303a2e1062e15c655e31914072c475"
SOURCE = ROOT / "src/multiwin.cpp"
SIGNATURE = "void MultiWin::draw_player(int faction_id, int row, RECT *bounds) {"


def extract_method(source: str) -> str:
    start = source.index(SIGNATURE)
    brace = source.index("{", start)
    depth = 0
    quote = ""
    line_comment = False
    block_comment = False
    escaped = False
    index = brace
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
        elif quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = ""
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
    raise AssertionError("unterminated MultiWin::draw_player definition")


HOST = r'''#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct RECT { int left, top, right, bottom; };
RECT *callback_bounds;
RECT post_font_bounds, post_spot_bounds, post_sprite_bounds;
int set_font_mutates_bounds, spot_mutates_bounds, sprite_mutates_bounds;
void assign_bounds(const RECT &r) { if (callback_bounds) *callback_bounds=r; }
struct Font {};
struct Spot {
  int calls, args[2][6];
  Spot() : calls(0), args() {}
  void add(int a,int b,int c,int d,int e,int f) {
    int *out=args[calls++]; int values[]={a,b,c,d,e,f};
    memcpy(out,values,sizeof(values));
    if (spot_mutates_bounds) assign_bounds(post_spot_bounds);
  }
};
struct Buffer {
  Font *font; int font_args[3], colors[4], color_calls;
  int write_calls, right_calls; char written[1032], right[1032];
  RECT written_rect, right_rect; unsigned int written_length, right_length;
  Buffer() : font(0), font_args(), colors(), color_calls(0), write_calls(0),
    right_calls(0), written(), right(), written_rect(), right_rect(),
    written_length(0), right_length(0) {}
  void set_font(Font *f,int a,int b,int c) {
    font=f; font_args[0]=a; font_args[1]=b; font_args[2]=c;
    if (set_font_mutates_bounds) assign_bounds(post_font_bounds);
  }
  void set_text_color(int a,int b,int c,int d) { colors[0]=a; colors[1]=b; colors[2]=c; colors[3]=d; ++color_calls; }
  int write_l(char *s,RECT *r,unsigned int n) { ++write_calls; strcpy(written,s); written_rect=*r; written_length=n; return 0; }
  int write_right_l(char *s,RECT *r,unsigned int n) { ++right_calls; strcpy(right,s); right_rect=*r; right_length=n; return 0; }
};
struct GraphicWin : Buffer {};
int sprite_draw_calls, sprite_selected, sprite_transparent, sprite_x, sprite_y;
int sprite_w, sprite_h; Buffer *sprite_receiver;
int sprite_mutates_turn_active;
void mutate_turn_active();
struct Sprite {
  int h, key, index;
  int height() const { return h; }
  int transparent_index() const { return key; }
  int draw(Buffer *dst,int transparent,int x,int y,int a,int b) {
    (void)a; (void)b; ++sprite_draw_calls; sprite_receiver=dst;
    sprite_selected=index; sprite_transparent=transparent; sprite_x=x; sprite_y=y;
    sprite_w=0; sprite_h=h;
    if (sprite_mutates_bounds) assign_bounds(post_sprite_bounds);
    if (sprite_mutates_turn_active) mutate_turn_active();
    return 0;
  }
};
struct MultiWin : GraphicWin {
  Spot spot_; Font font1_; Sprite sprites2_[3];
  MultiWin() { for (int i=0;i<3;++i) { sprites2_[i].index=i; sprites2_[i].h=8+i*5; sprites2_[i].key=20+i; } }
  void draw_player(int faction_id,int row,RECT *bounds);
};
struct Player { int is_leader_female; char name_leader[64]; int rule_flags; };
struct PlayerData { uint32_t diplo_treaties[8]; };
Player Players[8]; PlayerData PlayersData[8];
enum { RFLAG_AQUATIC=0x100 };
int ChatItemIdTable[8]={11,12,13,14,15,16,17,18};
int ChatShadowColors[8]={21,22,23,24,25,26,27,28};
int GenderDefault, PluralityDefault, IsMultiplayerNet;
int MultiWindowContactStatusMask;
int ConsoleControlTurnActive, NetUpkeepPhase, label_calls, upper_calls, itoa_calls;
int label_flip_multiplayer, label_mode_flip_seen;
int label_ids[64], active_calls, active_value, active_mutates_leading;
int active_leading_value;
struct Console { int faction_id_; } ConsoleGlobal;
struct ReadWord {
  // This scripted double exercises two distinct source reads in the actual
  // method; it is not a native layout or runtime model of the game global.
  uint32_t value, sequence[4]; int sequence_count; mutable int reads;
  ReadWord() : value(0), sequence(), sequence_count(0), reads(0) {}
  ReadWord &operator=(uint32_t n) { value=n; sequence_count=0; reads=0; return *this; }
  operator uint32_t() const {
    if (reads < sequence_count) return sequence[reads++];
    ++reads; return value;
  }
  void script(uint32_t a,uint32_t b) { sequence[0]=a; sequence[1]=b; sequence_count=2; reads=0; }
};
// @@LOCK_MASK_TYPES@@
struct GameFields {
  ReadWord game_state_;
  uint32_t net_turn_flags_;
  int net_turn_faction_;
  GameDataLockMaskView lock_masks_;
} ;
struct GameStorage { GameFields fields_; } GameDataStateStorage;
struct LockStateEntry { uint32_t leading_words_[4], payload_, trailing_words_[10]; };
struct PrimaryState { uint32_t leading_words_[3], flags_, trailing_words_[10]; };
struct PlayerLock { int active(); };
struct Lock { PlayerLock records_[8]; };
struct NetDaemon { Lock locks_; PrimaryState primary_state_; LockStateEntry channels_[2]; } NetDaemonNet;
void mutate_turn_active() { ConsoleControlTurnActive=1; }
int PlayerLock::active() {
  ++active_calls;
  if (active_mutates_leading) NetDaemonNet.primary_state_.leading_words_[0]=active_leading_value;
  return active_value;
}
char StringTemp[1032], BufferJoinSpace[]=" ";
char *label_get(int id) {
  label_ids[label_calls++]=id;
  static char labels[1200][32];
  sprintf(labels[id],"label%d",id);
  if (label_flip_multiplayer) { IsMultiplayerNet=1; label_flip_multiplayer=0; label_mode_flip_seen=1; }
  return labels[id];
}
char *CharUpperA(char *s) {
  ++upper_calls;
  for (char *p=s;*p;++p) *p=(char)toupper((unsigned char)*p);
  return s;
}
char *_itoa(int n,char *s,int radix) { (void)radix; ++itoa_calls; sprintf(s,"%d",n); return s; }
'''


def lock_mask_declarations() -> str:
    header = (ROOT / "src/game_data_state_window.h").read_text(encoding="utf-8")
    match = re.search(
        r"struct GameDataLockMaskBytes\s*\{.*?\};\s*union GameDataLockMaskView\s*\{.*?\};",
        header, re.DOTALL,
    )
    assert match, "production lock-mask byte/dword declarations missing"
    return match.group(0)


def compile_run(compiler: str, method: str, main: str, tmp_path: Path) -> subprocess.CompletedProcess[str]:
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(compiler + " unavailable")
    source_path = tmp_path / (compiler.replace("+", "p") + ".cpp")
    binary_path = tmp_path / (compiler.replace("+", "p") + ".bin")
    source_path.write_text(HOST.replace("// @@LOCK_MASK_TYPES@@", lock_mask_declarations()) +
                           "\n" + method + "\n" + main, encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary_path)], capture_output=True, text=True, timeout=10)


def test_draw_player_identity_history_and_complete_owner_body():
    source = SOURCE.read_text(encoding="utf-8")
    method = extract_method(source)
    base = subprocess.check_output(
        ["git", "show", BASE + ":src/multiwin.cpp"], cwd=ROOT, text=True
    )
    assert "// ORIGINAL: 0x00478630 ?draw_player@MultiWin@@QAEXHHPAURECT@@@Z 0x00478630-0x00478BDB" in source
    assert "// symbol    ?draw_player@MultiWin@@QAEXHHPAUtagRECT@@@Z" in source
    history_prefixes = ("// TRIED: MISMATCH #5", "// ORIGINAL: 0x00478630")
    for prefix in history_prefixes:
        current_line = next(line for line in source.splitlines() if line.startswith(prefix))
        original_line = next(line for line in base.splitlines() if line.startswith(prefix))
        assert current_line == original_line
    for token in (
        "sprites2_[icon_variant]", "sprites2_[0].height()",
        "if (IsMultiplayerNet)", "RFLAG_AQUATIC", "0xA1C / 4",
        "0x358 / 4", "NetUpkeepPhase", "records_[faction_id].active()",
        "primary_state_.leading_words_[0] != 0 &&",
    ):
        assert token in method, f"draw_player is missing required owner behavior: {token}"


MAIN = r'''
static void require(bool ok,const char *what) { if (!ok) { fprintf(stderr,"FAIL:%s\n",what); exit(97); } }
static void reset(MultiWin &w) {
  w.spot_.calls=0; memset(w.spot_.args,0,sizeof(w.spot_.args));
  w.font=0; memset(w.font_args,0,sizeof(w.font_args)); memset(w.colors,0,sizeof(w.colors));
  w.color_calls=w.write_calls=w.right_calls=0; w.written[0]=w.right[0]=0;
  sprite_draw_calls=0; sprite_receiver=0; sprite_mutates_turn_active=0;
  callback_bounds=0; set_font_mutates_bounds=spot_mutates_bounds=sprite_mutates_bounds=0;
  GenderDefault=PluralityDefault=0; IsMultiplayerNet=0; MultiWindowContactStatusMask=0;
  ConsoleControlTurnActive=NetUpkeepPhase=label_calls=upper_calls=itoa_calls=0;
  label_flip_multiplayer=label_mode_flip_seen=0;
  active_calls=active_value=active_mutates_leading=active_leading_value=0;
  ConsoleGlobal.faction_id_=0; GameDataStateStorage.fields_.game_state_=0;
  GameDataStateStorage.fields_.net_turn_flags_=0; GameDataStateStorage.fields_.net_turn_faction_=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0xff;
  memset(&NetDaemonNet,0,sizeof(NetDaemonNet)); memset(Players,0,sizeof(Players));
  memset(PlayersData,0,sizeof(PlayersData)); memset(label_ids,0,sizeof(label_ids));
  strcpy(Players[1].name_leader,"Leader"); Players[1].is_leader_female=1;
  for (int i=0;i<3;++i) { w.sprites2_[i].h=8+i*5; w.sprites2_[i].key=20+i; w.sprites2_[i].index=i; }
  StringTemp[0]=0;
}
static RECT bounds={20,31,220,55};
int main() {
  MultiWin w;
  reset(w);
  IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  MultiWindowContactStatusMask=2; NetDaemonNet.channels_[1].trailing_words_[6]=2;
  w.draw_player(1,3,&bounds);
  require(sprite_draw_calls==1 && sprite_selected==1,"contact status wins icon precedence");
  require(sprite_receiver==static_cast<Buffer*>(&w) && sprite_transparent==21,"selected icon receiver and key");
  require(sprite_x==27 && sprite_y==39,"icon uses sprite zero height for centering");
  require(w.spot_.calls==2,"icon and label hotspots");
  require(w.spot_.args[0][0]==1 && w.spot_.args[0][1]==0 && w.spot_.args[0][2]==27 &&
          w.spot_.args[0][3]==31 && w.spot_.args[0][4]==24 && w.spot_.args[0][5]==24,"icon hotspot geometry");
  require(w.spot_.args[1][0]==1 && w.spot_.args[1][1]==1 && w.spot_.args[1][2]==54 &&
          w.spot_.args[1][3]==31 && w.spot_.args[1][4]==156 && w.spot_.args[1][5]==24,"text hotspot geometry");
  require(w.font==&w.font1_ && w.font_args[0]==0 && w.font_args[1]==0 && w.font_args[2]==0,"font call");
  require(w.color_calls==1 && w.colors[0]==12 && w.colors[1]==22 && w.colors[2]==1 && w.colors[3]==1,"faction text colors");
  require(w.write_calls==1 && strcmp(w.written,"3 Leader")==0 && w.written_length==8,"row and leader text");
  require(w.written_rect.left==54 && w.written_rect.right==210 && w.written_rect.top==31 && w.written_rect.bottom==55,"text bounds");
  require(GenderDefault==1 && PluralityDefault==0,"leader morphology globals");

  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[1].trailing_words_[6]=2; w.draw_player(1,1,&bounds);
  require(sprite_draw_calls==1 && sprite_selected==2,"selected channel row uses icon two");
  require(sprite_y==39,"icon-zero centering remains fixed");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  w.draw_player(1,1,&bounds);
  require(sprite_draw_calls==1 && sprite_selected==0 && sprite_y==39,"fallback icon zero and fixed centering");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  w.draw_player(1,1,&bounds);
  require(sprite_draw_calls==0 && w.spot_.calls==1,"disabled mask suppresses icon but keeps text spot");
  require(w.spot_.args[0][2]==54 && w.spot_.args[0][4]==156,"multiplayer text geometry remains reserved when icon gate is absent");

  reset(w); PlayersData[0].diplo_treaties[1]=0x10; Players[1].rule_flags=RFLAG_AQUATIC;
  w.draw_player(1,4,&bounds);
  require(w.right_calls==1 && strcmp(w.right,"LABEL647")==0,"offline aquatic treaty label and uppercase");
  require(label_calls==1 && label_ids[0]==0xA1C/4 && upper_calls==1,"aquatic label lookup and uppercase call");
  reset(w); PlayersData[0].diplo_treaties[1]=0x10; Players[1].rule_flags=0;
  w.draw_player(1,4,&bounds);
  require(strcmp(w.right,"label214")==0 && label_ids[0]==0x358/4 && upper_calls==0,"offline non-aquatic treaty path");
  reset(w); PlayersData[0].diplo_treaties[1]=0x11; Players[1].rule_flags=RFLAG_AQUATIC;
  w.draw_player(1,4,&bounds);
  require(strcmp(w.right,"label211")==0 && label_ids[0]==0x34c/4 && upper_calls==0,"pact priority precedes aquatic treaty");
  for (int i=1;i<3;++i) {
    reset(w); PlayersData[0].diplo_treaties[1]=0x10 | (1u<<i);
    Players[1].rule_flags=RFLAG_AQUATIC; w.draw_player(1,4,&bounds);
    char expected[32]; sprintf(expected,"label%d",0x34c/4+i);
    require(strcmp(w.right,expected)==0 && label_ids[0]==0x34c/4+i && upper_calls==0,
            "higher-priority treaty bits precede aquatic treaty");
  }
  reset(w); PlayersData[0].diplo_treaties[1]=0x10; label_flip_multiplayer=1;
  w.draw_player(1,4,&bounds);
  require(IsMultiplayerNet==1 && strcmp(w.right,"label214")==0 &&
          label_calls==1 && label_ids[0]==0x358/4,"offline branch stays selected when label callback flips mode");
  reset(w); IsMultiplayerNet=1; ConsoleControlTurnActive=1;
  PlayersData[0].diplo_treaties[1]=0x10; Players[1].rule_flags=RFLAG_AQUATIC;
  w.draw_player(1,4,&bounds);
  require(w.right_calls==1 && strcmp(w.right,"*label267 0")==0 &&
          label_calls==1 && label_ids[0]==0x42C/4,"multiplayer active-control status is separate from offline treaty gate");

  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.primary_state_.trailing_words_[4]=2;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"label273")==0 && label_ids[0]==0x444/4,"primary status word selects label 444");

  reset(w); IsMultiplayerNet=1; ConsoleControlTurnActive=1; NetUpkeepPhase=7;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[1]=0;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"*label267 7")==0 && itoa_calls==2,"active turn nonselected suffix and phase");
  reset(w); IsMultiplayerNet=1; ConsoleControlTurnActive=1; NetUpkeepPhase=12;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[1]=2;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"label267 12")==0 && itoa_calls==2,"active turn selected suffix and phase");

  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.net_turn_flags_=0x10;
  GameDataStateStorage.fields_.net_turn_faction_=1;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"(*)")==0,"net turn faction marker");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.net_turn_flags_=0x10;
  GameDataStateStorage.fields_.net_turn_faction_=0;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"(+)")==0,"net turn order plus marker");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.net_turn_flags_=0x10;
  GameDataStateStorage.fields_.net_turn_faction_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"(-)")==0,"net turn order minus marker");

  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=0;
  GameDataStateStorage.fields_.game_state_=0x80;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"(*)")==0,"game-state high bit permits target-not-selected marker");

  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  NetDaemonNet.primary_state_.leading_words_[0]=1;
  GameDataStateStorage.fields_.game_state_.script(0,0);
  w.draw_player(1,1,&bounds);
  require(w.right_calls==0 && active_calls==0 &&
          GameDataStateStorage.fields_.game_state_.reads==2,"stable clear game-state/local-bit path keeps status empty");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  NetDaemonNet.primary_state_.leading_words_[0]=1; active_value=1;
  GameDataStateStorage.fields_.game_state_.script(0,0x80);
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"(?)")==0 && active_calls==1,"active lock adds question marker after callback path");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  NetDaemonNet.primary_state_.leading_words_[0]=1;
  NetDaemonNet.channels_[0].trailing_words_[0]=1; active_value=0;
  GameDataStateStorage.fields_.game_state_.script(0,0x80);
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"(@)")==0 && active_calls==1,"inactive lock plus primary/channel state adds at marker");
  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  NetDaemonNet.primary_state_.leading_words_[0]=1;
  NetDaemonNet.channels_[0].trailing_words_[0]=1;
  active_mutates_leading=1; active_leading_value=0; active_value=0;
  GameDataStateStorage.fields_.game_state_.script(0,0x80);
  w.draw_player(1,1,&bounds);
  require(active_calls==1 && w.right_calls==0 &&
          GameDataStateStorage.fields_.game_state_.reads==2,"primary word is reread after PlayerLock callback");

  reset(w); IsMultiplayerNet=1; GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  NetDaemonNet.primary_state_.leading_words_[0]=1;
  NetDaemonNet.channels_[0].leading_words_[0]=2;
  NetDaemonNet.channels_[0].leading_words_[1]=0;
  NetUpkeepPhase=5; sprite_mutates_turn_active=1;
  w.draw_player(1,1,&bounds);
  require(strcmp(w.right,"label267 5")==0,"selected channel is captured before icon callback changes control-active state");

  reset(w); IsMultiplayerNet=1; ConsoleControlTurnActive=1; NetUpkeepPhase=6;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=2;
  NetDaemonNet.channels_[0].leading_words_[1]=0;
  RECT callback_area={1,2,31,32};
  callback_bounds=&callback_area;
  post_font_bounds.left=10; post_font_bounds.top=20; post_font_bounds.right=110; post_font_bounds.bottom=50;
  post_spot_bounds.left=40; post_spot_bounds.top=50; post_spot_bounds.right=140; post_spot_bounds.bottom=90;
  post_sprite_bounds.left=70; post_sprite_bounds.top=80; post_sprite_bounds.right=170; post_sprite_bounds.bottom=130;
  set_font_mutates_bounds=1; spot_mutates_bounds=1; sprite_mutates_bounds=1;
  w.draw_player(1,2,&callback_area);
  require(callback_area.left==70 && callback_area.top==80 && callback_area.right==170 && callback_area.bottom==130,
          "fixture callbacks did mutate the caller bounds");
  require(w.spot_.args[0][2]==17 && w.spot_.args[0][3]==20 &&
          w.spot_.args[0][4]==30 && w.spot_.args[0][5]==30,
          "icon hotspot snapshots post-font geometry before Spot callbacks");
  require(w.spot_.args[1][2]==50 && w.spot_.args[1][3]==20 &&
          w.spot_.args[1][4]==50 && w.spot_.args[1][5]==30,
          "text hotspot ignores later Spot callback bounds changes");
  require(sprite_x==17 && sprite_y==31,"icon draw uses captured post-font geometry after callbacks mutate bounds");
  require(w.written_rect.left==50 && w.written_rect.top==20 &&
          w.written_rect.right==100 && w.written_rect.bottom==50,
          "row label rectangle uses captured post-font geometry");
  require(strcmp(w.right,"*label267 6")==0 &&
          w.right_rect.left==50 && w.right_rect.top==20 &&
          w.right_rect.right==100 && w.right_rect.bottom==50,
          "multiplayer suffix rectangle also ignores later callback bounds changes");
  return 0;
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_draw_player_status_contract_runs_on_host(compiler, tmp_path):
    method = extract_method(SOURCE.read_text(encoding="utf-8"))
    ran = compile_run(compiler, method, MAIN, tmp_path)
    assert ran.returncode == 0, ran.stdout + ran.stderr


MUTANTS = (
    ("center with selected sprite", "sprites2_[0].height()", "icon.height()", "icon uses sprite zero height for centering"),
    ("retest mode after callback", "  if (StringTemp[0]) {\n    canvas->write_right_l", "  if (label_mode_flip_seen && IsMultiplayerNet) strcat(StringTemp, \"MP\");\n  if (StringTemp[0]) {\n    canvas->write_right_l", "offline branch stays selected when label callback flips mode"),
    ("snapshot before font callback", "  canvas->set_font(&font1_, 0, 0, 0);\n\n  const RECT row_bounds = *bounds;", "  const RECT row_bounds = *bounds;\n  canvas->set_font(&font1_, 0, 0, 0);", "icon hotspot snapshots post-font geometry before Spot callbacks"),
    ("refresh text rectangle after callbacks", "RECT text_bounds = row_bounds;", "RECT text_bounds = *bounds;", "row label rectangle uses captured post-font geometry"),
    ("refresh second hotspot after Spot callback", "spot_.add(faction_id, 1, text_left, row_bounds.top,", "spot_.add(faction_id, 1, text_left, bounds->top,", "text hotspot ignores later Spot callback bounds changes"),
    ("wrong aquatic label", "label_get(0xA1C / 4)", "label_get(0x358 / 4)", "offline aquatic treaty label and uppercase"),
    ("remove aquatic uppercase", "CharUpperA(StringTemp);", "StringTemp[0] = StringTemp[0];", "offline aquatic treaty label and uppercase"),
    ("re-read selected turn word after callback", "(selected_turn_word & faction_bit) != 0", "((ConsoleControlTurnActive != 0 ? NetDaemonNet.channels_[0].leading_words_[1] : NetDaemonNet.channels_[0].leading_words_[0]) & faction_bit) != 0", "selected channel is captured before icon callback changes control-active state"),
    ("skip live primary recheck", "NetDaemonNet.primary_state_.leading_words_[0] != 0 &&", "true &&", "primary word is reread after PlayerLock callback"),
)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("name,old,new,oracle", MUTANTS)
def test_behavioral_mutants_compile_but_fail_the_named_oracle(compiler, name, old, new, oracle, tmp_path):
    method = extract_method(SOURCE.read_text(encoding="utf-8"))
    assert old in method, f"mutant anchor missing for {name}"
    if name in ("wrong aquatic label", "remove aquatic uppercase", "skip live primary recheck"):
        mutant = method.replace(old, new)
        assert mutant != method
    else:
        mutant = method.replace(old, new, 1)
    ran = compile_run(compiler, mutant, MAIN, tmp_path)
    assert ran.returncode == 97, f"{name} did not fail through an expected assertion: {ran.stdout}{ran.stderr}"
    assert ran.stderr == f"FAIL:{oracle}\n", f"{name} failed the wrong oracle: {ran.stderr}"

"""Actual editor body controls; not native ABI or whole-game acceptance."""

from dataclasses import fields
from pathlib import Path
import subprocess

import pytest

from match_recovery.source.reader import read_file, read_text
from test_console_typed_owner_members_oct10 import run_control
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]
BASE = "87414f049"


def test_original_identities_spans_claims_and_history_preserved():
    path = ROOT / "src/console.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/console.cpp"], cwd=ROOT).decode()
    before = {r.address: r for r in read_text(old, path)}
    after = {r.address: r for r in read_file(path)}
    assert set(before) == set(after)
    for address, row in before.items():
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(after[address], field.name)
        def lessons(record):
            return [(lesson.kind, lesson.key, lesson.prose,
                     tuple(text for _, text in lesson.lines)) for lesson in record.origin]
        assert lessons(row) == lessons(after[address])
    for signature in ("void Console::editor_view()", "void Console::editor_veh()"):
        assert "reinterpret_cast" not in extract(path.read_text(), signature)


def control():
    body = extract((ROOT / "src/console.cpp").read_text(), "void Console::editor_view()")
    header = (ROOT / "src/game_data_state_window.h").read_text()
    layout = extract(header, "struct GameDataLockMaskBytes") + ";\n"
    layout += extract(header, "union GameDataLockMaskView") + ";\n"
    return r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <vector>
LAYOUT
static_assert(sizeof(GameDataLockMaskView)==4,"original dword extent");
static_assert(offsetof(GameDataLockMaskBytes,alive_mask_)==1,"original alive byte");
struct {struct {GameDataLockMaskView lock_masks_;uint32_t game_state_;} fields_;} GameDataStateStorage;
static int selected,answer,modal_calls,draws,desktops,focuses,views,shown,hidden;
static bool mutate_items,mutate_prompt;
static int PopupModalState,Language,GenderDefault,PluralityDefault;
static const int MaxPlayerNum=8;
static char PopupStartCaption[8];
struct Player {char formal_name_faction[8],noun_faction[8];int noun_gender,is_noun_plural;};
static Player Players[8];
static std::vector<int> items,defaults;
struct Dialogs {void item(char *text,int id) {
    assert(PopupModalState==6 && text);
    items.push_back(id);
    if(id==1 && mutate_items) GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=0x16;
}};
struct Popup {
    Dialogs list;
    void start(char*,const char *key,int n,void*,int flags) {
        assert(!strcmp(key,"EDITORWHO") && n==-1 && flags==0x40);
    }
    Dialogs &dialogs(){return list;}
    void set_value(int n){defaults.push_back(n);}
    int exec(int,void*){assert(PopupModalState==6);return selected;}
};
struct Button {void show(int n){assert(n==0);++shown;}void hide(){++hidden;}};
static struct {Button flat_button_[3];} MainInterfaceGlobal;
struct Console {
    int faction_id_,cursor_history_index_,cursor_history_x_[32],cursor_history_y_[32];
    void editor_view();
    void focus(int x,int y,int faction){assert(x==17 && y==23 && faction==selected);++focuses;}
    void set_view(int mode){assert(mode==1);++views;}
};
static Console ConsoleGlobal;
static int X_pop(const char *key,void*) {
    assert(PopupModalState==6);++modal_calls;
    assert(!strcmp(key,selected==0x63?"EDITORCONTROLALL":"EDITORCONTROL"));
    if(mutate_prompt)GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=0xB5;
    return answer;
}
static void draw_map(int n){assert(n==1);++draws;}
static void desktop_update(){++desktops;GameDataStateStorage.fields_.game_state_|=0xC0;}
static void parse_says(int n,char *text,int a,int b){assert(n==0 && a==-1 && b==-1 && text==Players[selected].noun_faction);}
static int base_find(int x,int y,int faction){assert(x==17 && y==23 && faction==selected);return 0;}
BODY
static void reset() {
    GameDataStateStorage.fields_.lock_masks_.enable_mask_=0xABCD0E00;
    GameDataStateStorage.fields_.game_state_=0x80000080;
    ConsoleGlobal.faction_id_=2;ConsoleGlobal.cursor_history_index_=1;
    ConsoleGlobal.cursor_history_x_[1]=17;ConsoleGlobal.cursor_history_y_[1]=23;
    PopupModalState=0;Language=1;answer=1;mutate_items=mutate_prompt=false;
    modal_calls=draws=desktops=focuses=views=shown=hidden=0;items.clear();defaults.clear();
    for(int n=0;n!=8;++n){strcpy(Players[n].formal_name_faction,"Formal");strcpy(Players[n].noun_faction,"Noun");Players[n].noun_gender=n;Players[n].is_noun_plural=n&1;}
}
int main() {
    Console receiver={};receiver.faction_id_=7;
    reset();selected=-1;mutate_items=true;receiver.editor_view();
    assert((items==std::vector<int>{1,2,4,99}) && defaults==std::vector<int>{2});
    assert(PopupModalState==0 && !draws && !desktops && !modal_calls);
    assert(GameDataStateStorage.fields_.lock_masks_.enable_mask_==0xABCD1600);
    for(int choice=0;choice!=2;++choice) {
        reset();selected=99;answer=choice;mutate_prompt=true;receiver.editor_view();
        assert(GameDataStateStorage.fields_.lock_masks_.enable_mask_==(choice?0xABCDB5B4u:0xABCDB500u));
        assert(draws==1 && desktops==1 && modal_calls==1 && !views);
        assert(ConsoleGlobal.faction_id_==2 && receiver.faction_id_==7 && !PopupModalState);
    }
    for(int choice=0;choice!=2;++choice) {
        reset();selected=2;answer=choice;receiver.editor_view();
        assert(GameDataStateStorage.fields_.lock_masks_.enable_mask_==(choice?0xABCD0E04u:0xABCD0E00u));
        assert(draws==1 && desktops==1 && focuses==1 && views==1 && shown==1 && !hidden);
        assert(ConsoleGlobal.faction_id_==2 && receiver.faction_id_==7);
        assert(GenderDefault==2 && PluralityDefault==0 && !PopupModalState);
    }
    reset();selected=3;GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=8;receiver.editor_view();
    assert(!modal_calls && ConsoleGlobal.faction_id_==3 && views==1);
    assert(GameDataStateStorage.fields_.lock_masks_.enable_mask_==0xABCD0E08);
}
'''.replace("LAYOUT", layout).replace("BODY", body)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [
    None,
    ("& 0xFEu", "& 0xFFu"),
    ("static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & 0xFEu)", "static_cast<uint8_t>(0x0Eu & 0xFEu)"),
    ("(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << faction))", "(0x0Eu & (1u << faction))"),
])
def test_actual_editor_view_live_byte_ownership(tmp_path, compiler, damage):
    run_control(tmp_path, compiler, control(), damage)

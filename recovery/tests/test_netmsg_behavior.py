#!/usr/bin/env python3
"""Compile literal candidate NetMsg bodies against named host doubles."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
import pytest


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for i in range(brace, len(source)):
        if source[i] == "{":
            depth += 1
        elif source[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start : i + 1]
    raise ValueError(signature)



def _source():
    candidate = (ROOT / "src/netmsg.cpp").read_text()
    netmsg = (ROOT / "src/netmsg.cpp").read_text()
    popup = (ROOT / "src/popup.cpp").read_text()
    time_lifecycle = (ROOT / "src/time_lifecycle.cpp").read_text()
    
    actual = "\n".join(
        [
            body(candidate, "void __cdecl NetMsg::timer_daemon(int token)"),
            body(candidate, "bool NetMsg::is_active() const"),
            body(netmsg, "NetMsg::NetMsg(int a1, int a2, int a3)"),
            body(netmsg, "void NetMsg::close()"),
            body(candidate, "NetMsg::~NetMsg()"),
            body(candidate, "void NetMsg::pop(const char *label, int duration, int channel,"),
            body(candidate, "void NetMsg::pop(const char *label, const char *target)"),
            body(popup, "void Popup::close()"),
            body(popup, "Popup::~Popup()"),
            body(time_lifecycle, "Time::~Time()"),
        ]
    )
    globals_text = "\n".join(re.search(pattern, candidate).group(0) for pattern in (r"NetMsg NetMsgPrimary\(4096, 8, 0\);", r"NetMsg NetMsgSecondary\(-5, 40, 1\);"))
    
    prefix = r'''#include <cassert>
    #include <cstdint>
    #include <cstring>
    #include <string>
    #include <vector>
    #include <algorithm>
    using std::string;
    using std::vector;
    static vector<string> events;
    static int full_start_result = 0;
    static int local_start_result = 0;
    static int dims_result = 0;
    static int sprite_result = 0;
    static int dimensions_calls = 0;
    static int sprite_init_calls = 0;
    static int selected_picture_calls = 0;
    static int popup_close_calls = 0;
    static int time_teardown_calls = 0;
    static int exec_calls = 0;
    static int last_exec_field = -1;
    static int last_start_flags = -1;
    static int last_start_result = -1;
    static int base_init_calls = 0;
    static int last_init_a = -1, last_init_b = -1;
    static int last_loc_x = -1, last_loc_y = -1;
    static int button_calls = 0;
    static string button_text;
    static int IsMultiplayerNet = 0;
    static int PopupModalState = 0;
    static char PopupStartCaption[] = "caption";
    static int IfaceIdToSlotStorage[30] = {};
    static int ActiveIfaceIdStorage = 0;
    static int console_visible = 1;
    static const char *label_get(int) { return "OK"; }
    
    struct Time {
      using Callback = void (__cdecl *)(int);
      Callback callback = nullptr;
      int token = 0;
      uint32_t duration = 0;
      int resolution = 0;
      int close_calls = 0;
      Time() { events.push_back("time.ctor"); }
      void close() { ++close_calls; events.push_back("time.close"); }
      void start(Callback cb, int t, uint32_t d, int r) {
        callback = cb; token = t; duration = d; resolution = r;
        events.push_back("time.start");
      }
      void close_impl() { ++time_teardown_calls; events.push_back("time.destroy"); }
      ~Time();
    };
    
    struct Picture {
      int init(char *, int, int) { ++sprite_init_calls; return sprite_result; }
    };
    struct BasePop {
      uint32_t field_30AC_ = 0;
      int init(int a, int b) { ++base_init_calls; last_init_a=a; last_init_b=b; return 0; }
      void set_loc(int x, int y) { last_loc_x=x; last_loc_y=y; }
      void set_auto_hide_value(uint32_t v) { field_30AC_=v; }
      void button(const char *s) { ++button_calls; button_text=s; }
      void exec(int, int) { ++exec_calls; last_exec_field=(int)field_30AC_; events.push_back("exec"); }
      void close() { ++popup_close_calls; events.push_back("base.close"); }
      Picture &owned_picture() { static Picture picture; return picture; }
      void select_owned_picture() { ++selected_picture_calls; }
    };
    struct Scroll { void close() { events.push_back("scroll.close"); } };
    struct Popup : BasePop {
      Scroll scroll_;
      Popup() { events.push_back("popup.ctor"); }
      ~Popup();
      void close();
      void hide() { events.push_back("popup.hide"); }
      int start(char *, const char *, int, char *, int flags, void *) {
        last_start_flags=flags;
        last_start_result=full_start_result;
        events.push_back("popup.start");
        return full_start_result;
      }
      int start(char *, const char *, int, char *, int flags, std::nullptr_t) {
        last_start_flags=flags;
        last_start_result=(flags == 0 ? local_start_result : full_start_result);
        events.push_back("popup.start");
        return last_start_result;
      }
    };
    struct Console { int is_visible() const { return console_visible; } };
    static Console ConsoleGlobal;
    struct Buffer { static int get_pcx_dimensions(const char *, int *w, int *h) {
      ++dimensions_calls; *w=32; *h=16; return dims_result;
    } };
    class NetMsg : public Popup {
     public:
      NetMsg(int,int,int);
      ~NetMsg();
      void close();
      bool is_active() const;
      void pop(const char *,int,int,const char *);
      void pop(const char *,const char *);
      static void __cdecl timer_daemon(int);
      int32_t field_5384_=0;
      uint32_t field_537C_=0, field_5380_=0;
      int32_t field_5388_=0;
      Time timer_;
    };
    extern NetMsg NetMsgPrimary;
    extern NetMsg NetMsgSecondary;
    '''
    
    suffix = r'''
    void reset_observations() {
      events.clear(); dimensions_calls=0; sprite_init_calls=0;
      selected_picture_calls=0; popup_close_calls=0;
      exec_calls=0; last_exec_field=-1; last_start_flags=-1; last_start_result=-1;
      base_init_calls=0; last_init_a=last_init_b=-1; last_loc_x=last_loc_y=-1;
      button_calls=0; button_text.clear(); full_start_result=0; local_start_result=0;
      dims_result=0; sprite_result=0;
    }
    static int event_index(const char *name) {
      for (int i=0;i<(int)events.size();++i) if(events[i]==name) return i;
      return -1;
    }
    int main() {
      // Both actual source-owned objects carry the original constructor fields.
      assert(NetMsgPrimary.field_537C_==4096 && NetMsgPrimary.field_5380_==8 && NetMsgPrimary.field_5388_==0);
      assert(NetMsgSecondary.field_537C_==static_cast<uint32_t>(-5));
      assert(NetMsgSecondary.field_5380_==40 && NetMsgSecondary.field_5388_==1);
      assert(events.size()>=4 && events[0]=="popup.ctor" && events[1]=="time.ctor" &&
             events[2]=="popup.ctor" && events[3]=="time.ctor");
    
      // Four-gate selector matrix: channel <= 1, selected slot, then hidden
      // Console + network fallback. Full mode operates on the receiver.
      struct Gate { int channel, slot, visible, network, full; } gates[] = {
        {0,0,1,0,0}, {1,0,1,0,1}, {2,0,1,0,0}, {2,1,1,0,1},
        {0,0,0,1,1}, {0,0,0,0,0},
      };
      for (const auto &g : gates) {
        reset_observations();
        ActiveIfaceIdStorage=0; IfaceIdToSlotStorage[0]=g.slot;
        console_visible=g.visible; IsMultiplayerNet=g.network;
        NetMsgPrimary.field_537C_=17; NetMsgPrimary.field_5380_=29;
        NetMsgPrimary.field_5384_=0;
        NetMsgPrimary.pop("label",4,g.channel,nullptr);
        if (g.full) {
          assert(base_init_calls==1 && last_init_a==0x100 && last_init_b==0);
          assert(last_loc_x==17 && last_loc_y==29);
          assert(last_start_flags==0x180 && exec_calls==1 && button_calls==1);
          assert(button_text=="OK");
          assert(PopupModalState==0);
          assert(last_exec_field==0); // BasePop+0x30AC is untouched in full path.
        } else {
          assert(base_init_calls==0 && exec_calls==1 && last_start_flags==0);
          assert(last_exec_field == (g.network ? 1 : 1));
          assert(PopupModalState==0);
        }
      }
    
      // Full-mode failed start still clears modal state, but does not activate,
      // execute, add a button or arm a timer.
      reset_observations(); console_visible=1; IsMultiplayerNet=0;
      NetMsgPrimary.field_5384_=0; NetMsgPrimary.timer_.callback=nullptr;
      full_start_result=7;
      NetMsgPrimary.pop("fail",60,1,nullptr);
      assert(base_init_calls==1 && exec_calls==0 && button_calls==0);
      assert(NetMsgPrimary.field_5384_==0 && PopupModalState==0);
      assert(NetMsgPrimary.timer_.callback==nullptr);
    
      // The non-full Popup::start result is intentionally ignored: the image
      // still stores auto-hide and executes the local popup after a load failure.
      reset_observations(); console_visible=1; IsMultiplayerNet=0;
      local_start_result=9;
      NetMsgPrimary.pop("local-fail",0,0,nullptr);
      assert(last_start_result==9 && exec_calls==1 && last_exec_field==1);
    
      // Full-mode active replacement closes timer then hides before init; success
      // arms only for positive duration and carries the original token/resolution.
      reset_observations(); console_visible=1; IsMultiplayerNet=0;
      NetMsgPrimary.field_5384_=1; NetMsgPrimary.field_5388_=0;
      NetMsgPrimary.pop("ok",20,1,nullptr);
      assert(event_index("time.close")>=0 && event_index("popup.hide")>event_index("time.close"));
      assert(NetMsgPrimary.field_5384_==1 && NetMsgPrimary.timer_.callback==NetMsg::timer_daemon);
      assert(NetMsgPrimary.timer_.token==0x46 && NetMsgPrimary.timer_.duration==20);
      assert(NetMsgPrimary.timer_.resolution==0x3c);
      NetMsgPrimary.timer_.callback(0x46);
      assert(NetMsgPrimary.field_5384_==0);
    
      // Positive/zero/negative duration and network state determine local popup's
      // saved BasePop flag. The actual exec body observes the saved field value.
      struct Active { int network, duration, expected; } active_cases[] = {
        {0,-1,0}, {0,0,1}, {0,1,1}, {1,-1,1}, {1,0,1},
      };
      for (const auto &a : active_cases) {
        reset_observations(); console_visible=1; IsMultiplayerNet=a.network;
        NetMsgPrimary.pop("local",a.duration,0,nullptr);
        assert(exec_calls==1 && last_exec_field==a.expected);
      }
    
      // Icon work occurs only on inactive local popup; both dimension and Sprite
      // init must succeed before selecting the owned picture.
      reset_observations(); console_visible=1; IsMultiplayerNet=0;
      dims_result=0; sprite_result=0;
      NetMsgPrimary.pop("icon",-1,0,"ICON.PCX");
      assert(dimensions_calls==1 && sprite_init_calls==1 && selected_picture_calls==1);
      reset_observations(); dims_result=1;
      NetMsgPrimary.pop("icon",-1,0,"ICON.PCX");
      assert(dimensions_calls==1 && sprite_init_calls==0 && selected_picture_calls==0);
      reset_observations(); dims_result=0; sprite_result=1;
      NetMsgPrimary.pop("icon",-1,0,"ICON.PCX");
      assert(dimensions_calls==1 && sprite_init_calls==1 && selected_picture_calls==0);
      reset_observations(); IsMultiplayerNet=1;
      NetMsgPrimary.pop("icon",-1,0,"ICON.PCX");
      assert(dimensions_calls==0 && sprite_init_calls==0 && selected_picture_calls==0);
    
      // Timer callback routes exactly 0x46 to primary and every other token to
      // secondary, and NetMsg::close gates duplicate cleanup on active state.
      reset_observations();
      NetMsgPrimary.timer_.close_calls=0; NetMsgSecondary.timer_.close_calls=0;
      NetMsgPrimary.field_5384_=1; NetMsgSecondary.field_5384_=1;
      NetMsg::timer_daemon(0x46);
      assert(NetMsgPrimary.field_5384_==0 && NetMsgSecondary.field_5384_==1);
      assert(NetMsgPrimary.timer_.close_calls==1);
      NetMsg::timer_daemon(0x47);
      assert(NetMsgSecondary.field_5384_==0 && NetMsgSecondary.timer_.close_calls==1);
      int pc=NetMsgPrimary.timer_.close_calls;
      NetMsgPrimary.close(); assert(NetMsgPrimary.timer_.close_calls==pc);
    
      // The two-string wrapper sends the exact sentinel duration/channel/target
      // to Primary regardless of the syntactic receiver.
      reset_observations(); NetMsgSecondary.pop("wrapped","TGT");
      assert(last_start_flags==0 && exec_calls==1 && last_exec_field==1);
    
      // Actual destructors: Time member teardown precedes the Popup base dtor;
      // Popup's own dtor closes once. This control intentionally stubs deeper
      // Scroll/BasePop destructors and does not claim their internal close counts.
      reset_observations(); time_teardown_calls=0;
      { NetMsg local(1,2,3); }
      assert(time_teardown_calls==1 && popup_close_calls==1);
      assert(event_index("time.destroy") < event_index("base.close"));
      return 0;
    }
    '''
    
    # A __cdecl spelling is meaningful in the candidate's original ABI but not a
    # host GCC keyword. The alias keeps the literal method declarations intact.
    source = prefix.replace("#include <cassert>", "#define __cdecl\n#include <cassert>") + "\n" + actual + "\n" + globals_text + "\n" + suffix
    source = source.replace(
        "assert(NetMsgPrimary.timer_.callback(0), true); // syntactic no-op assertion below checks callback effect",
        "NetMsgPrimary.timer_.callback(0x4f);",
    )
    
    
    return source
    

@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["none", "omit_saved_auto_hide", "wrong_timer_target", "skip_full_modal_reset"])
def test_netmsg_actual_body_controls(tmp_path, compiler_name, mutation):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = _source()
    replacements = {
        "omit_saved_auto_hide": ("popup.set_auto_hide_value(active ? 1u : 0u);", ""),
        "wrong_timer_target": ("NetMsgSecondary.close();", "NetMsgPrimary.close();"),
        "skip_full_modal_reset": ("PopupModalState = 0;", "/* deliberate mutant: keep modal state */"),
    }
    if mutation != "none":
        old, new = replacements[mutation]
        assert old in source, "actual implementation mutation anchor missing"
        if mutation == "skip_full_modal_reset":
            source, count = re.subn(r"(?m)^(\s*)PopupModalState = 0;$", r"\1/* deliberate mutant: keep modal state */", source)
            assert count == 2, "full-path reset anchors changed"
        else:
            source = source.replace(old, new)
    exe = tmp_path / "netmsg-control"
    result = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", str(exe)],
        input=source, text=True, capture_output=True, timeout=30,
    )
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, timeout=30)
    if mutation == "none":
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, f"{mutation} escaped the actual-body controls"


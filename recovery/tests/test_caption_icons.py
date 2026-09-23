"""Actual caption ownership bodies with bounded host service projections.

Independent originals:625330/340/380,5F2760,5F2700 and58F2F0. The real method
bodies, declarations, inherited types and pointer owners are extracted. Access
control is exposed only in projected declarations for observation. Graphics,
OS, BaseButton, startup services and their callback effects are explicit doubles,
not native layout/ABI, full Win::close/init, startup or working-game proof.
ASan/UBSan remain strict; LSan is disabled because the sandbox uses ptrace.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def one(pattern, source):
    match, = re.findall(pattern, source, re.M)
    return match


def actual_source():
    h = {n: (ROOT / "src" / (n + ".h")).read_text() for n in
         ("imagebutton", "basebutton", "graphicwin", "win", "sprite", "buffer", "game", "strings", "alpha", "map", "time", "general", "random", "log", "basepop")}
    c = {n: (ROOT / "src" / (n + ".cpp")).read_text() for n in
         ("imagebutton", "win", "sprite", "game", "strings", "alpha", "map")}
    parts = {}
    for name, unit in (("Win", "win"), ("GraphicWin", "graphicwin"), ("BaseButton", "basebutton")):
        parts[name.upper() + "_BASE"] = one(r"^class " + name + r" : [^{]+\{", h[unit])
    parts["IMAGE_DECL"] = (braced(h["imagebutton"], "class ImageButton :") + ";").replace(" private:", " public:")
    fields = []
    for name in ("field_128_", "caption_height_", "border_thickness_", "client_rect_", "minimize_button_", "zoom_button_", "close_button_"):
        fields.append(one(r"^  (?:int |uint32_t |RECT |ImageButton \*)" + name + r";", h["win"]))
    parts["WIN_FIELDS"] = "\n".join(fields)
    parts["WIN_METHODS"] = "\n".join(one(r"^  " + p, h["win"]) for p in
        (r"int move\([^;]+;", r"void redo_caption_buttons\(\);", r"void do_caption_buttons\(\);"))
    parts["WIN_PAINT"] = braced(h["win"], "virtual void draw_contents()")
    parts["BASE_INIT_DECL"] = one(r"  int init\(LPCSTR name,[^;]+;", h["basebutton"])
    parts["DIB"] = braced(h["buffer"], "struct Dib : BITMAPINFO") + ";"
    parts["DIB_MEMBER"] = one(r"^  Dib dib_;", h["buffer"])
    parts["SPRITE_FIELDS"] = "\n".join(one(r"^  int " + n + r";", h["sprite"]) for n in
        ("iWidth_", "iHeight_", "iSpriteWidth_", "iSpriteHeight_"))
    parts["SPRITE_ACCESSORS"] = braced(h["sprite"], "int width() const") + "\n" + braced(h["sprite"], "int height() const")
    parts["CTOR"] = braced(c["imagebutton"], "ImageButton::ImageButton()")
    parts["DTOR"] = braced(c["imagebutton"], "ImageButton::~ImageButton()")
    parts["CLOSE"] = braced(c["imagebutton"], "void ImageButton::close()")
    for signature in re.findall(r"int ImageButton::init\([^)]*\)\s*\{", c["imagebutton"]):
        parts["EXPLICIT" if "int width" in signature else "AUTOMATIC"] = braced(c["imagebutton"], signature)
    parts["REDO"] = braced(c["win"], "void Win::redo_caption_buttons()")
    parts["DELETE"] = braced(c["win"], "void Win::do_caption_buttons()")
    definitions, declarations = [], []
    for n in ("WinCloseIcon", "WinMaximizeIcon", "WinRestoreIcon", "WinMinimizeIcon"):
        definitions.append(one(r"^Sprite \*" + n + r";", c["win"]))
        declarations.append(one(r"^extern Sprite \*" + n + r";", h["win"]))
    menu = one(r"^Sprite g_SILVER_MENU_ICON_SPRITES\[4\];", c["sprite"])
    definitions.append(menu)
    declarations.append(one(r"^extern " + re.escape(menu), h["sprite"]))
    parts["ICONS"] = "\n".join(declarations + definitions)
    parts["GAME"] = braced(c["game"], "int __cdecl game_init(")
    parts["GAME_DECL"] = one(r"^int __cdecl game_init\([^;]+;", h["game"])
    parts["UPPER_TYPE"] = one(r"^typedef void\(__stdcall func_string_upper\)\(char \*\);", h["game"])
    parts["STRINGS_INIT"] = one(r"^  BOOL init\(size_t mem_size\);", h["strings"])
    parts["SETTERS"] = "\n".join(one(r"^  static MEASURED int __cdecl set_def_" + n + r"_text\(LPSTR text\);", h["basepop"]) for n in ("ok", "cancel"))
    declarations = [one(r"^void __cdecl load_sprites\(\);", h["game"])]
    for unit, pattern in (("general", r"^void __cdecl my_srand\([^;]+;"),
                          ("random", r"^void __cdecl random_reseed\([^;]+;"),
                          ("log", r"^void __cdecl log_set_state\([^;]+;"),
                          ("alpha", r"^BOOL __cdecl read_rules\([^;]+;"),
                          ("alpha", r"^BOOL __cdecl labels_init\(\);"),
                          ("alpha", r"^LPSTR __cdecl label_get\([^;]+;"),
                          ("time", r"^void __cdecl start_timers\(\);")):
        declarations.append(one(pattern, h[unit]))
    parts["GAME_SERVICES"] = "\n".join(declarations)
    parts["GAME_GLOBALS"] = "\n".join((
        one(r"^char StringTemp\[1032\];", c["strings"]),
        one(r"^Strings StringTable;", c["strings"]),
        one(r"^uint32_t Language;", c["alpha"]),
        one(r"^uint32_t WorldClimateSkipTerrainClear = \d+;", c["map"])))
    return parts


COMMON = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>
#define __cdecl
#define __stdcall
#define WINAPI
#define MEASURED
typedef const char *LPCSTR;typedef char *LPSTR;typedef int BOOL;typedef uint32_t DWORD;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(96);}}
#define require(x) check((x),__LINE__)
struct Sprite { SPRITE_FIELDS SPRITE_ACCESSORS };
ICONS
'''


BUTTONS = COMMON + r'''
struct RECT {int32_t left,top,right,bottom;};
struct RGBQUAD {uint8_t blue,green,red,reserved;};
struct BITMAPINFOHEADER {uint32_t biSize;int32_t biWidth,biHeight;uint16_t biPlanes,biBitCount;
    uint32_t biCompression,biSizeImage;int32_t biXPelsPerMeter,biYPelsPerMeter;uint32_t biClrUsed,biClrImportant;};
struct BITMAPINFO {BITMAPINFOHEADER bmiHeader;RGBQUAD bmiColors[1];};
DIB
struct Buffer { DIB_MEMBER };
struct AutoSound {virtual ~AutoSound(){}};
class ImageButton;
WIN_BASE public: WIN_FIELDS WIN_METHODS WIN_PAINT int identity; };
GRAPHICWIN_BASE public: };
BASEBUTTON_BASE public: BASE_INIT_DECL void close(); };
IMAGE_DECL
// Painting belongs to its own actual-body fixture. The caption service
// projection must never reach it; keep the real override declaration linked
// without substituting a passing paint implementation here.
void ImageButton::draw_contents(){require(false);}
static_assert(std::is_same<decltype(Win::close_button_),ImageButton*>::value,"typed caption receiver");
static_assert(std::is_same<decltype(ImageButton::sprite_),Sprite*>::value,"borrowed sprite pointer");
static_assert(std::is_same<decltype(WinRestoreIcon),Sprite*>::value,"typed global owner");
CTOR
DTOR
Sprite a,b,c;
ImageButton *expected_button;Sprite *expected_sprite;
Win *expected_parent;const char *expected_name;
int base_calls,base_closes,base_result;bool mutate_base;
std::vector<int> expected_arguments;
int BaseButton::init(LPCSTR name,int id,int x,int y,int width,int height,Win*parent,int style){
    require(this==static_cast<BaseButton*>(expected_button));require(expected_button->sprite_==expected_sprite);
    require(name==expected_name&&parent==expected_parent);
    require(expected_arguments==std::vector<int>({id,x,y,width,height,style}));++base_calls;
    if(mutate_base)expected_button->sprite_=&c;return base_result;}
void BaseButton::close(){require(this==static_cast<BaseButton*>(expected_button)&&expected_button->sprite_==nullptr);
    ++base_closes;if(mutate_base)expected_button->sprite_=&c;}
CLOSE
EXPLICIT
AUTOMATIC
Win *owner;ImageButton *buttons[6];int scenario,zoom_result;
void *HandleMain;void *expected_handle;
struct Move {int receiver,x,y;};std::vector<Move> moves;int queries;
int Win::move(int x,int y){moves.push_back({identity,x,y});
    if(scenario==1&&identity==10){owner->close_button_=buttons[1];owner->zoom_button_=buttons[2];
        owner->client_rect_={0,0,1000,800};owner->field_128_=9;owner->border_thickness_=100;owner->caption_height_=200;
    }else if(scenario==1&&identity==12){owner->zoom_button_=buttons[3];HandleMain=buttons[3];
    }else if(scenario==2&&identity==10){owner->zoom_button_=nullptr;
    }else if(scenario==3&&identity==12){owner->minimize_button_=nullptr;}
    return -29;}
int IsZoomed(void *window){require(window==expected_handle);++queries;
    if(scenario==1){owner->zoom_button_=buttons[4];owner->minimize_button_=buttons[5];
        WinMaximizeIcon=&b;WinRestoreIcon=&c;}return zoom_result;}
REDO
DELETE
void clear_owner(Win&w){w.client_rect_={11,0,211,100};w.field_128_=2;w.border_thickness_=7;w.caption_height_=19;
    w.close_button_=w.zoom_button_=w.minimize_button_=nullptr;}
void test_methods(){ImageButton button;Win parent;require(button.sprite_==nullptr);
    expected_button=&button;expected_parent=&parent;char name[]="caption";expected_name=name;
    a.iWidth_=31;a.iHeight_=-17;a.iSpriteWidth_=107;a.iSpriteHeight_=109;
    for(int result: {0,3,4,-19})for(int mutation=0;mutation<2;++mutation){
        base_result=result;mutate_base=mutation!=0;base_calls=base_closes=0;button.sprite_=&b;
        expected_sprite=&a;expected_arguments={27,-3,5,-41,19,-7};
        require(button.init(&a,name,27,-3,5,-41,19,&parent,-7)==result);
        require(base_calls==1&&base_closes==0&&button.sprite_==(mutate_base?&c:&a));
        expected_sprite=nullptr;expected_parent=nullptr;expected_arguments={28,4,-6,0,-23,9};
        require(button.init(nullptr,name,28,4,-6,0,-23,nullptr,9)==result);
        require(base_calls==2&&base_closes==0&&button.sprite_==(mutate_base?&c:nullptr));
        expected_parent=&parent;expected_sprite=&a;expected_arguments={31,7,-8,31,-17,-9};
        require(button.init(&a,name,31,7,-8,&parent,-9)==result);
        require(base_calls==3&&base_closes==0&&button.sprite_==(mutate_base?&c:&a));
        button.sprite_=&b;require(button.init(nullptr,name,1,2,3,&parent,4)==3);
        require(base_calls==3&&base_closes==0&&button.sprite_==&b);
        button.close();require(base_closes==1&&button.sprite_==(mutate_base?&c:nullptr));
    }
    // Publication borrows, never destroys or reconstructs the Sprite owner.
    require(a.iWidth_==31&&a.iHeight_==-17&&a.iSpriteWidth_==107&&a.iSpriteHeight_==109);
}
void test_redo(){Win w;ImageButton objects[6];owner=&w;
    for(int n=0;n<6;++n){buttons[n]=&objects[n];objects[n].identity=10+n;objects[n].sprite_=&a;}
    for(int mask=0;mask<8;++mask)for(int zoom: {0,7})for(int negative=0;negative<2;++negative){
        clear_owner(w);scenario=0;zoom_result=zoom;queries=0;moves.clear();HandleMain=expected_handle=&w;
        w.close_button_=mask&1?buttons[0]:nullptr;w.zoom_button_=mask&2?buttons[2]:nullptr;w.minimize_button_=mask&4?buttons[5]:nullptr;
        buttons[0]->dib_.bmiHeader.biWidth=negative?-15:15;buttons[2]->dib_.bmiHeader.biWidth=negative?-17:17;
        buttons[2]->sprite_=&a;WinMaximizeIcon=&b;WinRestoreIcon=zoom==7?nullptr:&c;
        w.redo_caption_buttons();int x=168;size_t k=0;
        if(mask&1){require(k<moves.size());require(moves[k].receiver==10&&moves[k].x==x&&moves[k].y==2);++k;x+=negative?12:-18;}
        if(mask&2){require(k<moves.size());require(moves[k].receiver==12&&moves[k].x==x&&moves[k].y==2);++k;x+=negative?16:-18;
            require(buttons[2]->sprite_==(zoom?WinRestoreIcon:WinMaximizeIcon));}
        else require(buttons[2]->sprite_==&a);
        if(mask&4){require(k<moves.size());require(moves[k].receiver==15&&moves[k].x==x&&moves[k].y==2);++k;}
        require(moves.size()==k&&queries==((mask&2)?1:0));
    }
    for(int zoom: {0,7}){clear_owner(w);scenario=1;zoom_result=zoom;queries=0;moves.clear();
        for(auto*p:buttons)p->sprite_=&a;
        buttons[1]->dib_.bmiHeader.biWidth=29;buttons[4]->dib_.bmiHeader.biWidth=31;
        buttons[0]->dib_.bmiHeader.biWidth=3;buttons[2]->dib_.bmiHeader.biWidth=5;buttons[3]->dib_.bmiHeader.biWidth=7;
        w.close_button_=buttons[0];HandleMain=&w;expected_handle=buttons[3];WinMaximizeIcon=WinRestoreIcon=&a;
        w.redo_caption_buttons();require(moves.size()==3&&queries==1);
        require(moves[0].receiver==10&&moves[0].x==168&&moves[0].y==2);
        require(moves[1].receiver==12&&moves[1].x==136&&moves[1].y==2);
        require(moves[2].receiver==15&&moves[2].x==104&&moves[2].y==2);
        require(buttons[4]->sprite_==(zoom?&c:&b)&&buttons[2]->sprite_==&a&&buttons[3]->sprite_==&a);
    }
    for(int mode: {2,3}){clear_owner(w);scenario=mode;zoom_result=0;queries=0;moves.clear();
        w.close_button_=buttons[0];w.zoom_button_=buttons[2];w.minimize_button_=buttons[5];
        buttons[0]->dib_.bmiHeader.biWidth=15;buttons[2]->dib_.bmiHeader.biWidth=17;HandleMain=expected_handle=&w;
        w.redo_caption_buttons();require(moves.size()==2&&moves[0].receiver==10);
        require(moves[1].receiver==(mode==2?15:12)&&moves[1].x==150&&moves[1].y==2);require(queries==(mode==3));
    }
}
std::vector<int> destroyed;int deletion_mode;ImageButton *replacement;
struct Tracked:ImageButton {int slot;Tracked(int n,int s):slot(s){identity=n;}
    ~Tracked(){ImageButton*current=slot==0?owner->minimize_button_:slot==1?owner->zoom_button_:owner->close_button_;
        require(current==this);destroyed.push_back(identity);
        if(deletion_mode&&slot==0){owner->minimize_button_=replacement;owner->zoom_button_=new Tracked(22,1);}
        if(deletion_mode&&slot==1)owner->close_button_=new Tracked(23,2);
    }};
void test_deletion(){Win w;owner=&w;
    for(int mask=0;mask<8;++mask){clear_owner(w);destroyed.clear();deletion_mode=0;
        if(mask&1)w.minimize_button_=new Tracked(1,0);if(mask&2)w.zoom_button_=new Tracked(2,1);if(mask&4)w.close_button_=new Tracked(3,2);
        w.do_caption_buttons();std::vector<int> wanted;if(mask&1)wanted.push_back(1);if(mask&2)wanted.push_back(2);if(mask&4)wanted.push_back(3);
        require(destroyed==wanted&&!w.minimize_button_&&!w.zoom_button_&&!w.close_button_);
    }
    clear_owner(w);destroyed.clear();deletion_mode=1;replacement=new ImageButton;w.minimize_button_=new Tracked(21,0);
    w.do_caption_buttons();require(destroyed==std::vector<int>({21,22,23}));
    require(!w.minimize_button_&&!w.zoom_button_&&!w.close_button_);deletion_mode=0;delete replacement;
}
int main(){test_methods();test_redo();test_deletion();}
'''


GAME = COMMON + r'''
struct Strings { STRINGS_INIT };
struct BasePop { SETTERS };
UPPER_TYPE
GAME_DECL
GAME_SERVICES
GAME_GLOBALS
std::vector<std::string> events;
int mode_case,label_result,rules_result,clock_calls,upper_calls;bool mutate_services;
Sprite sentinels[4];Sprite*old_icons[4];
void icons_unchanged(){require(WinCloseIcon==old_icons[0]&&WinMaximizeIcon==old_icons[1]&&WinRestoreIcon==old_icons[2]&&WinMinimizeIcon==old_icons[3]);}
void event(const char*s){icons_unchanged();events.push_back(s);}
void upper_a(char*text);void upper_b(char*text);
func_string_upper *upper_target;
func_string_upper *&string_upper_slot(){event("upper_slot");return upper_target;}
DWORD first_clock();DWORD second_clock(){require(false);return 0;}
DWORD (WINAPI *timeGetTime)()=first_clock;
DWORD first_clock(){event("clock");require(WorldClimateSkipTerrainClear==1);++clock_calls;
    timeGetTime=second_clock;return clock_calls==1?101:202;}
void my_srand(int seed){event("srand");require(seed==101);}
void random_reseed(uint32_t seed){event("random");require(seed==202);}
void log_set_state(BOOL value){event("log");require(value==0);}
BOOL Strings::init(size_t size){event("strings");require(this==&StringTable&&size==49952);return 9;}
BOOL labels_init(){event("labels");return label_result;}
LPSTR label_get(int label){event(label==102?"cancel_label":"ok_label");require(label==102||label==101);
    static char cancel[]="cancel",ok[]="ok";return label==102?cancel:ok;}
void upper_a(char*text){event("upper");require(text==StringTemp);++upper_calls;
    for(char*p=text;*p;++p)if(*p>='a'&&*p<='z')*p-=32;}
void upper_b(char*){require(false);}
int BasePop::set_def_cancel_text(LPSTR text){event("cancel");require(text==StringTemp);
    require(std::strcmp(text,Language==2?"cancel":"CANCEL")==0);upper_target=upper_b;return 7;}
int BasePop::set_def_ok_text(LPSTR text){event("ok");require(text==StringTemp&&std::strcmp(text,"OK")==0);return 8;}
void mutate_icons(int phase){if(!mutate_services)return;
    WinCloseIcon=old_icons[0]=&sentinels[(phase+0)%4];WinMaximizeIcon=old_icons[1]=&sentinels[(phase+1)%4];
    WinRestoreIcon=old_icons[2]=&sentinels[(phase+2)%4];WinMinimizeIcon=old_icons[3]=&sentinels[(phase+3)%4];}
void load_sprites(){event("load");mutate_icons(1);}
BOOL read_rules(BOOL reload){event("rules");require(reload==-7);mutate_icons(2);return rules_result;}
void start_timers(){event("timers");mutate_icons(3);}
GAME
void test_game(){for(int mode: {0,1,-2})for(int language: {0,2})for(int label_fail: {0,1})for(int rules_fail: {0,1})for(int mutation: {0,1}){
    mode_case=mode;Language=language;label_result=label_fail;rules_result=rules_fail;mutate_services=mutation;
    for(int repeat=0;repeat<2;++repeat){
        events.clear();clock_calls=upper_calls=0;timeGetTime=first_clock;upper_target=upper_a;
        WorldClimateSkipTerrainClear=0;std::strcpy(StringTemp,"old caption");
        WinCloseIcon=old_icons[0]=&sentinels[0];WinMaximizeIcon=old_icons[1]=&sentinels[1];
        WinRestoreIcon=old_icons[2]=&sentinels[2];WinMinimizeIcon=old_icons[3]=&sentinels[3];
        std::vector<std::string>wanted={"clock","srand","clock","random"};
        if(mode==0)wanted.push_back("log");else{wanted.push_back("strings");wanted.push_back("labels");
            if(!label_fail){wanted.push_back("cancel_label");wanted.push_back("upper_slot");if(language!=2)wanted.push_back("upper");
                wanted.push_back("cancel");wanted.push_back("ok_label");wanted.push_back("upper");wanted.push_back("ok");}}
        const bool reaches_rules=mode==0||!label_fail;
        if(reaches_rules){wanted.push_back("load");wanted.push_back("rules");if(!rules_fail)wanted.push_back("timers");}
        const bool success=reaches_rules&&!rules_fail;
        require(game_init(mode,-7)==(success?0:1));require(events==wanted&&clock_calls==2);
        if(success){require(WinCloseIcon==&g_SILVER_MENU_ICON_SPRITES[1]);require(WinMaximizeIcon==&g_SILVER_MENU_ICON_SPRITES[2]);
            require(WinMinimizeIcon==&g_SILVER_MENU_ICON_SPRITES[0]&&WinRestoreIcon==old_icons[2]);}
        else icons_unchanged();
    }
}}
int main(){test_game();}
'''


def run_host(tmp_path, parts, harness):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "caption.cpp", tmp_path / "caption"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], harness))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all", str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=20, env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


@pytest.mark.parametrize("harness", [BUTTONS, GAME], ids=["image_button_caption", "game_init_publication"])
def test_actual_caption_bodies(tmp_path, harness):
    result = run_host(tmp_path, actual_source(), harness)
    assert result.returncode == 0, result.stderr


# These modify real extracted C++, not the independent event/geometry oracle.
# Every control must compile and reach an explicit assertion, never just crash.
DAMAGE = [
    ("constructor_null", "CTOR", [("sprite_ = 0;", "sprite_ = reinterpret_cast<Sprite*>(1);")]),
    ("close_no_publication", "CLOSE", [("sprite_ = 0;", "")]),
    ("close_late_publication", "CLOSE", [("sprite_ = 0;\n    BaseButton::close();", "BaseButton::close();\n    sprite_ = 0;")]),
    ("close_discards_callback", "CLOSE", [("BaseButton::close();", "BaseButton::close(); sprite_ = 0;")]),
    ("explicit_no_publication", "EXPLICIT", [("sprite_ = sprite;", "")]),
    ("explicit_null_guard", "EXPLICIT", [("sprite_ = sprite;", "if (!sprite) return 3; sprite_ = sprite;")]),
    ("explicit_dimensions", "EXPLICIT", [("x, y, width, height,", "x, y, height, width,")]),
    ("explicit_parent", "EXPLICIT", [("height, parent, style_flag", "height, nullptr, style_flag")]),
    ("explicit_style", "EXPLICIT", [("height, parent, style_flag", "height, parent, 0")]),
    ("explicit_discard_result", "EXPLICIT", [("return BaseButton::init", "BaseButton::init"), ("parent, style_flag);", "parent, style_flag); return 0;")]),
    ("explicit_failure_cleanup", "EXPLICIT", [("return BaseButton::init", "int result = BaseButton::init"), ("parent, style_flag);", "parent, style_flag); if (result) close(); return result;")]),
    ("explicit_overwrites_callback", "EXPLICIT", [("return BaseButton::init", "int result = BaseButton::init"), ("parent, style_flag);", "parent, style_flag); sprite_ = sprite; return result;")]),
    ("automatic_null_store", "AUTOMATIC", [("return 3;", "sprite_ = nullptr; return 3;")]),
    ("automatic_no_publication", "AUTOMATIC", [("sprite_ = sprite;", "")]),
    ("automatic_dimensions", "AUTOMATIC", [("int height = sprite->height();", "int height = sprite->width();")]),
    ("automatic_trimmed_width", "SPRITE_ACCESSORS", [("return iWidth_;", "return iSpriteWidth_;")]),
    ("automatic_trimmed_height", "SPRITE_ACCESSORS", [("return iHeight_;", "return iSpriteHeight_;")]),
    ("automatic_id", "AUTOMATIC", [("name, id, x, y,", "name, 0, x, y,")]),
    ("automatic_discard_result", "AUTOMATIC", [("return BaseButton::init", "BaseButton::init"), ("style_flag);", "style_flag); return 0;")]),
    ("set_sprite_wrong_owner", "IMAGE_DECL", [("sprite_ = sprite;", "sprite_ = nullptr;")]),
    ("redo_wrong_receiver", "REDO", [("close_button_->move(x, w);", "move(x, w);")]),
    ("redo_xy_swap", "REDO", [("close_button_->move(x, w);", "close_button_->move(w, x);")]),
    ("redo_close_gap", "REDO", [("x += -3 -", "x += -1 -")]),
    ("redo_zoom_gap", "REDO", [("x += -1 -", "x += -3 -")]),
    ("redo_wrong_icon_arm", "REDO", [("IsZoomed(HandleMain) != 0", "IsZoomed(HandleMain) == 0")]),
    ("redo_stale_close_width", "REDO", [("int w = field_128_;", "ImageButton *saved_close = close_button_; int w = field_128_;"), ("close_button_->dib_", "saved_close->dib_")]),
    ("redo_stale_zoom_after_query", "REDO", [("zoom_button_->move(x, w);", "zoom_button_->move(x, w); ImageButton *saved_zoom = zoom_button_;"), ("zoom_button_->set_sprite", "saved_zoom->set_sprite")]),
    ("redo_stale_icon_globals", "REDO", [("if (IsZoomed(HandleMain)", "Sprite *saved_restore = WinRestoreIcon, *saved_maximize = WinMaximizeIcon; if (IsZoomed(HandleMain)"), ("set_sprite(WinRestoreIcon)", "set_sprite(saved_restore)"), ("set_sprite(WinMaximizeIcon)", "set_sprite(saved_maximize)")]),
    ("redo_stale_handle", "REDO", [("int w = field_128_;", "void *saved_handle = HandleMain; int w = field_128_;"), ("IsZoomed(HandleMain)", "IsZoomed(saved_handle)")]),
    ("redo_spacing_reloaded", "REDO", [("zoom_button_->move(x, w);", "zoom_button_->move(x, field_128_);")]),
    ("redo_stale_minimize", "REDO", [("int w = field_128_;", "ImageButton *saved_minimize = minimize_button_; int w = field_128_;"), ("if (minimize_button_ != 0)", "if (saved_minimize != 0)"), ("minimize_button_->move", "saved_minimize->move")]),
    ("redo_missing_minimize", "REDO", [("minimize_button_->move(x, w);", "")]),
    ("delete_clear_before_callback", "DELETE", [("delete obj;\n        minimize_button_ = 0;", "minimize_button_ = 0;\n        delete obj;")]),
    ("delete_bypass_destructor", "DELETE", [("delete obj;", "::operator delete(obj);")]),
    ("delete_missing_clear", "DELETE", [("minimize_button_ = 0;", "")]),
    ("delete_stale_zoom", "DELETE", [("Win *obj;", "Win *obj; Win *saved_zoom = zoom_button_;"), ("obj = zoom_button_;", "obj = saved_zoom;")]),
    ("delete_stale_close", "DELETE", [("Win *obj;", "Win *obj; Win *saved_close = close_button_;"), ("obj = close_button_;", "obj = saved_close;")]),
    ("game_close_owner", "GAME", [("WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[1];", "WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[3];")]),
    ("game_maximize_owner", "GAME", [("WinMaximizeIcon = &g_SILVER_MENU_ICON_SPRITES[2];", "WinMaximizeIcon = &g_SILVER_MENU_ICON_SPRITES[1];")]),
    ("game_minimize_owner", "GAME", [("WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[0];", "WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[2];")]),
    ("game_restore_overwritten", "GAME", [("WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[0];", "WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[0]; WinRestoreIcon = &g_SILVER_MENU_ICON_SPRITES[3];")]),
    ("game_publication_before_timer", "GAME", [("start_timers();", "WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[1]; start_timers();")]),
    ("game_publication_before_rules", "GAME", [("load_sprites();", "load_sprites(); WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[1];")]),
    ("game_rules_failure", "GAME", [("if (read_rules(reload) != 0)", "if (read_rules(reload) == 0)")]),
    ("game_labels_failure", "GAME", [("if (labels_init() != 0)", "if (labels_init() == 0)")]),
    ("game_failure_return", "GAME", [("return 1;", "return 0;")]),
    ("game_reload_argument", "GAME", [("read_rules(reload)", "read_rules(0)")]),
    ("game_missing_log", "GAME", [("log_set_state(0);", "")]),
    ("game_missing_load", "GAME", [("load_sprites();", "")]),
    ("game_missing_timer", "GAME", [("start_timers();", "")]),
    ("game_terrain_flag", "GAME", [("WorldClimateSkipTerrainClear = 1;", "WorldClimateSkipTerrainClear = 0;")]),
    ("game_clock_reloaded", "GAME", [("random_reseed(time_get_time());", "random_reseed(timeGetTime());")]),
    ("game_cancel_uppercase", "GAME", [("Language != 2", "Language == 2")]),
    ("game_ok_uppercase", "GAME", [("string_upper(StringTemp);\n        BasePop::set_def_ok_text", "BasePop::set_def_ok_text")]),
    ("game_upper_target_reloaded", "GAME", [("string_upper(StringTemp);\n        BasePop::set_def_ok_text", "string_upper_slot()(StringTemp);\n        BasePop::set_def_ok_text")]),
    ("game_string_failure_exit", "GAME", [("StringTable.init(0xC320);", "if (StringTable.init(0xC320)) return 1;")]),
    ("game_setter_failure_exit", "GAME", [("BasePop::set_def_cancel_text(StringTemp);", "if (BasePop::set_def_cancel_text(StringTemp)) return 1;")]),
]


@pytest.mark.parametrize("name,part,changes", DAMAGE, ids=[row[0] for row in DAMAGE])
def test_compiling_caption_damage(tmp_path, name, part, changes):
    parts = actual_source()
    for before, after in changes:
        assert before in parts[part], (name, before)
        # Multiple occurrences only occur for the two icon-selection arms;
        # both intentionally use the same cached or wrong receiver mutation.
        parts[part] = parts[part].replace(before, after)
    result = run_host(tmp_path, parts, GAME if part == "GAME" else BUTTONS)
    assert result.returncode == 96 and "assertion line" in result.stderr, (name, result.returncode, result.stderr)

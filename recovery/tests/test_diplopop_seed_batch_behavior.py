"""Run actual DiploPop definitions against embedded host service doubles.

The test extracts current owner bodies at runtime; it does not import a worker
fragment or use a fallback body. The host hierarchy is a test double, not proof
of target ABI/layout, native references, graphics output, or runtime behavior.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


HERE = Path(__file__).resolve().parent
ROOT = Path(os.environ.get("OPENSMACX_SOURCE_ROOT", HERE.parents[1])).resolve()
SOURCE = ROOT / "src" / "diplopop.cpp"
FIXTURE = "#ifndef DIPLOPOP_HOST_FIXTURE_HPP\n#define DIPLOPOP_HOST_FIXTURE_HPP\n#include <stdint.h>\n#include <stddef.h>\n#include <string.h>\n#include <ctype.h>\n#include <stdio.h>\n#include <stdlib.h>\n\nstruct RECT { int left, top, right, bottom; };\nstruct Palette {};\nstruct Font {\n  int height, line_height, adjustment, style;\n  Font() : height(0), line_height(0), adjustment(-1), style(0) {}\n  int init(const char *, int h, int s) { height=h; line_height=h+2; adjustment=-1; style=s; return 1; }\n  int line_advance() const { return adjustment < 0 ? line_height : height + adjustment; }\n};\nstruct CallLog {\n  int font_init_count, flic_init_count, flic_close_count, win_init_count;\n  int sync_count, fill_count, box_count, button_init_count, button_color_count;\n  int button_style_count, redraw_count, hide_count, hline_count, pixel_count;\n  int write_count, center_count, sprite_draw_count, say_count, upper_count;\n  int button_x[3], button_y[3], button_w[3], button_h[3], button_id[3];\n  int box_style[16]; RECT boxes[16];\n  int write_x[8], write_y[8], write_centered[8]; char writes[8][256];\n  int sprite_x, sprite_y, sprite_transparent; void *sprite_target;\n  int rule_x1, rule_x2, rule_y, rule_color;\n  int get_pixel_x, get_pixel_y;\n  int window_width, window_height, move_x, move_y, palette_synced, fill_color;\n  const char *say_text[8]; int say_faction[8];\n  int button_border_indices[3][3]; int button_color[3][4]; char button_names[3][32];\n  CallLog() { memset(this,0,sizeof(*this)); }\n};\nextern CallLog log_calls;\n\nstruct Buffer {\n  virtual ~Buffer() {}\n  RECT clip;\n  Buffer() { clip.left=0; clip.top=0; clip.right=700; clip.bottom=500; }\n  int sync_to_palette(Palette *) { ++log_calls.sync_count; return 0; }\n  int box_sprite(RECT *r, struct BoxSpriteParams *p);\n  void set_font(Font *, Font *, Font *, Font *) { }\n  void set_text_color(int,int,int,int) { }\n  int write_l(char *s,int x,int y,int len) { record_write(s,x,y,0,len); return len; }\n  int write_cent_l(char *s,RECT *r,int len) { record_write(s,r->left,r->top,1,len); return len; }\n  int get_pixel(int x,int y) { ++log_calls.pixel_count; log_calls.get_pixel_x=x; log_calls.get_pixel_y=y; return 0x2A; }\n  void hline(int x1,int x2,int y,int c) { ++log_calls.hline_count; log_calls.rule_x1=x1; log_calls.rule_x2=x2; log_calls.rule_y=y; log_calls.rule_color=c; }\n  const RECT &clip_rect() const { return clip; }\n  int pixel_width() const { return log_calls.window_width; }\n  int pixel_height() const { return log_calls.window_height; }\n private:\n  void record_write(char *s,int x,int y,int centered,int len) {\n    int n=log_calls.write_count++;\n    if (n<8) { strncpy(log_calls.writes[n],s,255); log_calls.writes[n][255]=0; log_calls.write_x[n]=x; log_calls.write_y[n]=y; log_calls.write_centered[n]=centered; }\n    (void)len;\n  }\n};\nstruct Win {\n  virtual ~Win() {}\n  virtual void show(int) {}\n  virtual void hide() { ++log_calls.hide_count; }\n  virtual void redraw() { ++log_calls.redraw_count; }\n  int field_16C_;\n  int move(int x,int y) { log_calls.move_x=x; log_calls.move_y=y; return 0; }\n};\nstruct GraphicWin : Win, Buffer {\n  int init(int,int,int w,int h,char *,int,Win *,void *,void *) {\n    ++log_calls.win_init_count; log_calls.window_width=w; log_calls.window_height=h;\n    clip.left=0; clip.top=0; clip.right=w; clip.bottom=h; return 0;\n  }\n  void close() {}\n  void fill(int c) { ++log_calls.fill_count; log_calls.fill_color=c; }\n};\nstruct SubInterface { protected: uint32_t field_4_; };\nstruct BoxSpriteParams { int id; };\nstruct Sprite {\n  int w,h,transparent;\n  Sprite():w(0),h(0),transparent(0){}\n  int width() const { return w; }\n  int height() const { return h; }\n  unsigned char transparent_index() const { return (unsigned char)transparent; }\n  int draw(Buffer *b,int t,int x,int y,int,int) { ++log_calls.sprite_draw_count; log_calls.sprite_target=b; log_calls.sprite_transparent=t; log_calls.sprite_x=x; log_calls.sprite_y=y; return 0; }\n};\nstruct Flic {\n  int width;\n  Flic():width(0){}\n  int init(Buffer *,char *,int,int,int) { ++log_calls.flic_init_count; width=55; return 0; }\n  int decoded_width() const { return width; }\n  void close() { ++log_calls.flic_close_count; }\n  ~Flic() { close(); }\n};\nstruct Time { void stop() {} };\nstruct Button : GraphicWin {\n  void set_border_styles(BoxSpriteParams *a,BoxSpriteParams *b,BoxSpriteParams *c) {\n    int n=log_calls.button_style_count++; if(n<3){ log_calls.button_border_indices[n][0]=a->id; log_calls.button_border_indices[n][1]=b->id; log_calls.button_border_indices[n][2]=c->id; }\n  }\n  void set_text_color(int a,int b,int c,int d) { int n=log_calls.button_color_count++; if(n<3){log_calls.button_color[n][0]=a;log_calls.button_color[n][1]=b;log_calls.button_color[n][2]=c;log_calls.button_color[n][3]=d;} }\n  int init(const char *name,int id,int x,int y,int w,int h,Win *,int) {\n    int n=log_calls.button_init_count++; if(n<3){ log_calls.button_id[n]=id; log_calls.button_x[n]=x; log_calls.button_y[n]=y; log_calls.button_w[n]=w; log_calls.button_h[n]=h; strncpy(log_calls.button_names[n],name,31); log_calls.button_names[n][31]=0; }\n    return 0;\n  }\n};\ntypedef Button FlatButton;\nstruct MainInterface : GraphicWin {\n  RECT right_frame_, lower_frame_, main_frame_, main_layout_, rect2_, rect3_;\n  int panel;\n  const RECT &right_frame_rect() const { return right_frame_; }\n  const RECT &lower_frame_rect() const { return lower_frame_; }\n  const RECT &main_frame_rect() const { return main_frame_; }\n  const RECT &main_layout_rect() const { return main_layout_; }\n  const RECT &rect2() const { return rect2_; }\n  const RECT &rect3() const { return rect3_; }\n  int panel_height() const { return panel; }\n};\nstruct Console : Win { int faction_id_; };\nstruct Player { char name_leader[24]; };\nstruct PlayerData { int integrity_blemishes; int diplo_friction[8]; };\n\nstruct DiploPop : GraphicWin, SubInterface {\n  void init(); void draw_portrait(); void draw_bioscan();\n  uint32_t interface_state() const { return field_4_; }\n  Font font1_,font2_,font3_; int counterpart_faction_; Time time1_,time2_;\n  Flic flic1_,flic2_; uint32_t field_20B0_; Sprite *landscape_sprite_,*frame_sprite_,*portrait_sprite_,*hq_sprite_;\n  RECT bioscan_title_rect_,bioscan_picture_rect_,portrait_text_rect_; RECT rect1_;\n  RECT content_rect_,frame_inset_rect_,button_left_rect_,button_right_rect_,right_panel_button_rect_;\n  uint32_t field_2154_; FlatButton flatButtons_[3];\n  DiploPop() { memset(&bioscan_title_rect_,0,9*sizeof(uint32_t)); field_20B0_=0; field_2154_=0; landscape_sprite_=frame_sprite_=portrait_sprite_=hq_sprite_=0; counterpart_faction_=0; }\n};\n\nextern MainInterface MainInterfaceGlobal;\nextern Console ConsoleGlobal;\nextern Player Players[8];\nextern PlayerData PlayersData[8];\nextern char StringTemp[1032];\nextern char *Mood[9]; extern char *Repute[8];\nextern BoxSpriteParams InterfaceBoxStyles[51];\nextern Palette *PaletteCurrent;\nextern int WinScreenWidth,WinScreenHeight;\nextern const char *DefaultFontFace;\nextern const char *LabelTexts[4096];\nstruct Strings { char *get(int); };\nextern Strings StringTable;\nchar *label_get(int);\nchar *CharUpperA(char *);\nvoid say_special(char *,char *,int);\nuint32_t get_mood(int);\nstruct BoxSpriteParams;\n#endif\n"
CONTROL = "#include \"fixture.hpp\"\n\nCallLog log_calls;\nMainInterface MainInterfaceGlobal;\nConsole ConsoleGlobal;\nPlayer Players[8];\nPlayerData PlayersData[8];\nchar StringTemp[1032];\nchar *Mood[9];\nchar *Repute[8];\nBoxSpriteParams InterfaceBoxStyles[51];\nPalette *PaletteCurrent = 0;\nint WinScreenWidth = 0x320, WinScreenHeight = 0x258;\nconst char *DefaultFontFace = \"Arial\";\nconst char *LabelTexts[4096];\nStrings StringTable;\nstatic char *PointerEntries[32];\nstatic int PointerKeys[32];\nstatic int PointerCount;\nstatic int failures;\nstatic char mood0[]=\"M0\", mood3[]=\"M3\", repute0[]=\"R0\", repute7[]=\"R7\";\n\nstatic void check(bool ok, const char *name) {\n  if (!ok) { fprintf(stderr,\"FAIL: %s\\n\",name); ++failures; }\n}\nint Buffer::box_sprite(RECT *r, BoxSpriteParams *p) {\n  int n=log_calls.box_count++;\n  if(n<16) { log_calls.box_style[n]=p->id; log_calls.boxes[n]=*r; }\n  return 0;\n}\nchar *Strings::get(int address) {\n  for(int i=0;i<PointerCount;++i) if(PointerKeys[i]==address) return PointerEntries[i];\n  for(int i=0;i<4096;++i) if((int)(i*4)==address) return const_cast<char *>(LabelTexts[i]);\n  return const_cast<char *>(\"?\");\n}\nchar *label_get(int id) { return const_cast<char *>(LabelTexts[id]); }\nchar *CharUpperA(char *text) {\n  ++log_calls.upper_count;\n  for(char *p=text;*p;++p) *p=(char)toupper((unsigned char)*p);\n  return text;\n}\nvoid say_special(char *output,char *text,int faction) {\n  int n=log_calls.say_count++;\n  if(n<8) { log_calls.say_text[n]=text; log_calls.say_faction[n]=faction; }\n  strcat(output,\"|\"); strcat(output,text);\n}\nuint32_t get_mood(int friction) { return (uint32_t)(friction<0?0:(friction>8?8:friction)); }\n\nstatic void add_pointer(char *p) {\n  PointerKeys[PointerCount]=(int)(intptr_t)p; PointerEntries[PointerCount]=p; ++PointerCount;\n}\nstatic void reset_log() { log_calls=CallLog(); }\nstatic void setup_geometry() {\n  MainInterfaceGlobal.right_frame_=(RECT){100,0,300,200};\n  MainInterfaceGlobal.rect3_=(RECT){0,20,40,140};\n  MainInterfaceGlobal.rect2_=(RECT){0,10,400,150};\n  MainInterfaceGlobal.lower_frame_=(RECT){10,20,500,350};\n  MainInterfaceGlobal.main_layout_=(RECT){30,300,630,340};\n  MainInterfaceGlobal.main_frame_=(RECT){50,10,650,430};\n  MainInterfaceGlobal.panel=50;\n}\n\nint main() {\n  for(int i=0;i<51;++i) InterfaceBoxStyles[i].id=i;\n  LabelTexts[0xE8C/4]=\"bioscan\"; LabelTexts[0xE90/4]=\"Repute\";\n  LabelTexts[0xE94/4]=\"Mood\"; LabelTexts[0xEA8/4]=\"Diplomacy\";\n  LabelTexts[0xEAC/4]=\"Intel\"; LabelTexts[0x774/4]=\"Exchange\";\n  LabelTexts[0x100/4]=\"unused\";\n  Mood[0]=mood0; Mood[3]=mood3; Repute[0]=repute0; Repute[7]=repute7;\n  add_pointer(Mood[0]); add_pointer(Mood[3]); add_pointer(Repute[0]); add_pointer(Repute[7]);\n  setup_geometry();\n\n  // The complete candidate init is exercised on both native screen branches.\n  DiploPop popup;\n  WinScreenWidth=0x320; WinScreenHeight=0x258; reset_log();\n  popup.init();\n  check(log_calls.flic_init_count==1 && log_calls.flic_close_count==2,\"init temporary Flic lifecycle\");\n  check(log_calls.win_init_count==1 && log_calls.window_width==0x2BC && log_calls.window_height==0x258-50,\"320px window dimensions\");\n  check(log_calls.move_x==(0x320-0x2BC+0x20)/2 && log_calls.move_y==0,\"320px centered placement\");\n  check(popup.font1_.height==10 && popup.font2_.height==10 && popup.font3_.height==12,\"320px font branch\");\n  check(popup.interface_state()==4 && popup.field_16C_==0x1A && popup.field_2154_==1,\"init native state fields\");\n  check(popup.portrait_text_rect_.left==589 && popup.portrait_text_rect_.top==14 && popup.portrait_text_rect_.right==646 && popup.portrait_text_rect_.bottom==426,\"portrait rect geometry\");\n  check(popup.bioscan_title_rect_.left==54 && popup.bioscan_title_rect_.top==14 && popup.bioscan_title_rect_.right==586 && popup.bioscan_title_rect_.bottom==34,\"bioscan title geometry\");\n  check(popup.bioscan_picture_rect_.left==54 && popup.bioscan_picture_rect_.top==36 && popup.bioscan_picture_rect_.right==586 && popup.bioscan_picture_rect_.bottom==426,\"bioscan picture geometry\");\n  check(log_calls.button_init_count==3 && log_calls.button_id[0]==0 && log_calls.button_id[1]==2 && log_calls.button_id[2]==1,\"button id/order\");\n  check(log_calls.button_style_count==3 && log_calls.button_color_count==3 && log_calls.redraw_count==3 && log_calls.hide_count==4,\"button style/color/redraw/hide order coverage\");\n  check(strcmp(log_calls.button_names[0],\"Diplomacy\")==0 && strcmp(log_calls.button_names[1],\"Intel\")==0 && strcmp(log_calls.button_names[2],\"EXCHANGE\")==0,\"button label selection and uppercase third label\");\n  check(log_calls.button_x[0]==37 && log_calls.button_y[0]==313 && log_calls.button_w[0]==291 && log_calls.button_h[0]==20,\"left button rectangle\");\n  check(popup.right_panel_button_rect_.top==10 && popup.right_panel_button_rect_.right==-3,\"right-panel button uses rect2 top/left, not rect3\");\n  check(popup.rect1_.bottom==310,\"native scratch store reuses rect1 bottom at +0x2100\");\n  check(popup.frame_inset_rect_.left==104 && popup.frame_inset_rect_.top==4 && popup.frame_inset_rect_.right==296 && popup.frame_inset_rect_.bottom==4,\"frame inset bottom uses right-panel top minus six\");\n  check(log_calls.button_x[2]==331 && log_calls.button_y[2]==313 && log_calls.button_w[2]==291 && log_calls.button_h[2]==20,\"right button rectangle\");\n  check(log_calls.button_border_indices[0][0]==27 && log_calls.button_border_indices[0][1]==28 && log_calls.button_border_indices[0][2]==29 && log_calls.button_color[0][0]==0x41,\"native button border/color tuple\");\n\n  WinScreenWidth=640; WinScreenHeight=480; reset_log();\n  popup.init();\n  check(log_calls.window_height==0x1C2 && popup.font2_.height==12 && popup.font3_.height==14,\"non-320px dimensions and fonts\");\n  check(log_calls.move_x==(640-0x2BC+0x20)/2 && log_calls.move_y==480-0x1C2-50,\"non-320px placement\");\n\n  // Renderer fixture distinguishes style 6/0, their inset rectangles, the\n  // local frame Sprite, all text rows, pointer conversion, and callback order.\n  Sprite frame, landscape;\n  frame.w=20; frame.h=12; frame.transparent=7;\n  landscape.w=40; landscape.h=15; landscape.transparent=2;\n  popup.frame_sprite_=&frame; popup.landscape_sprite_=&landscape;\n  reset_log(); popup.draw_bioscan();\n  check(log_calls.box_count==2 && log_calls.box_style[0]==6 && log_calls.box_style[1]==0,\"bioscan panel style order\");\n  check(strcmp(log_calls.writes[0],\"BIOSCAN\")==0 && log_calls.write_centered[0]==1,\"bioscan uppercase centered title\");\n  check(log_calls.sprite_draw_count==1 && log_calls.sprite_target==static_cast<Buffer *>(&MainInterfaceGlobal) && log_calls.sprite_x==546 && log_calls.sprite_y==218 && log_calls.sprite_transparent==7,\"bioscan frame Sprite placement\");\n\n  strcpy(Players[2].name_leader,\"Morgan\");\n  popup.counterpart_faction_=2; ConsoleGlobal.faction_id_=1;\n  PlayersData[2].integrity_blemishes=12; PlayersData[2].diplo_friction[1]=3;\n  reset_log(); popup.draw_portrait();\n  check(log_calls.box_count==1 && log_calls.box_style[0]==0,\"portrait box style\");\n  check(log_calls.pixel_count==1 && log_calls.get_pixel_x==589 && log_calls.get_pixel_y==30,\"portrait separator pixel coordinates\");\n  check(log_calls.hline_count==1 && log_calls.rule_x1==590 && log_calls.rule_x2==645 && log_calls.rule_y==30 && log_calls.rule_color==0x2A,\"portrait separator line\");\n  check(log_calls.write_count==3 && strcmp(log_calls.writes[0],\"MORGAN\")==0,\"portrait uppercase leader name\");\n  check(log_calls.say_count==2 && log_calls.say_faction[0]==2 && log_calls.say_faction[1]==2,\"portrait repute/mood faction\");\n  check(log_calls.say_text[0]==Repute[7] && log_calls.say_text[1]==Mood[3],\"portrait clamped repute and local-friction mood\");\n  check(log_calls.write_x[0]==594 && log_calls.write_y[0]==34 && log_calls.write_y[1]==log_calls.write_y[0]+popup.font3_.line_advance() && log_calls.write_y[2]==log_calls.write_y[1]+popup.font2_.line_advance(),\"portrait font row advances\");\n\n  if(failures) return 1;\n  puts(\"DiploPop private actual-body controls passed\");\n  return 0;\n}\n".replace('#include "fixture.hpp"\n', "")
SIGNATURES = (
    r"void\s+DiploPop::init\s*\(\)\s*\{",
    r"void\s+DiploPop::draw_bioscan\s*\(\)\s*\{",
    r"void\s+DiploPop::draw_portrait\s*\(\)\s*\{",
)


def _matching_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        ch = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if ch == "\\":
                index += 1
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif ch == "/" and nxt == "/":
            state = "line"
            index += 1
        elif ch == "/" and nxt == "*":
            state = "block"
            index += 1
        elif ch == '"':
            state = "string"
        elif ch == "'":
            state = "char"
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return index
        index += 1
    raise AssertionError("unterminated DiploPop source definition")


def _actual_bodies(text: str) -> list[str]:
    bodies = []
    for signature in SIGNATURES:
        matches = list(re.finditer(signature, text))
        assert len(matches) == 1, "expected exactly one actual body: " + signature
        start = matches[0].start()
        opening = text.find("{", matches[0].start(), matches[0].end())
        end = _matching_brace(text, opening)
        body = text[start:end + 1]
        assert "BODY GOES HERE" not in body and "PENDING_BODY" not in body
        bodies.append(body.replace("nullptr", "0"))
    return bodies


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_diplopop_actual_source_bodies(compiler, tmp_path):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip("host compiler unavailable")
    assert SOURCE.is_file(), "missing actual owner source: " + str(SOURCE)
    bodies = _actual_bodies(SOURCE.read_text())
    generated = tmp_path / "diplopop_actual.cpp"
    generated.write_text(FIXTURE + "\n\n" +
                         "\n\n".join(bodies) + "\n\n" + CONTROL)
    binary = tmp_path / "diplopop_actual"
    built = subprocess.run(
        [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=undefined", "-fno-sanitize-recover=all", str(generated),
         "-o", str(binary)], capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert run.returncode == 0, run.stdout + run.stderr

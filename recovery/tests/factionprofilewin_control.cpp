#include "factionprofilewin_control.hpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>

std::vector<std::string> HostEvents;
int IfaceScreenWidth = 1000;
Win HostOpeningWindow;
Win *OpeningWindow = &HostOpeningWindow;
Player Players[8];
char HostFilenames[8][24];
Sprite HostPortraits[8];
Sprite *FactionDiplomacyPortraits[8];
BoxSpriteParams InterfaceBoxStyles[51];
Font HostFonts[3];
Font *PopupMenuItalicFont = &HostFonts[0];
Font *PopupMenuBoldFont = &HostFonts[1];
Font *PopupMenuFont = &HostFonts[2];
char StringTemp[1024];
bool HostPictureEnabled = false;
FactionProfileWin *HostActiveProfile = 0;
int HostExpectedDrawFaction = 3;
int HostExpectedSpriteFaction = 5;
int HostFactionAfterSprite = 6;

void host_event(const std::string &event) { HostEvents.push_back(event); }

Buffer::Buffer() { host_event("buffer.ctor"); }
Buffer::~Buffer() { host_event("buffer.dtor"); }
RECT Buffer::clip_rect() const { RECT r = {1, 2, 301, 202}; return r; }
void Buffer::box_sprite(RECT *r, BoxSpriteParams *style) {
  assert(r->left == 1 && r->top == 2 && style == &InterfaceBoxStyles[9]);
  host_event("box");
}

void GraphicWin::init(RECT *r, char *caption, int flags, Win *parent,
                      void *, void *) {
  assert(r->left == 5 && r->top == 0x2D);
  assert(r->right == (IfaceScreenWidth >= 1000 ? 0x2C5 : 0x22B));
  assert(r->bottom == (IfaceScreenWidth >= 1000 ? 0x1AD : 0x159));
  assert(strcmp(caption, "...") == 0 && flags == 0x1400000);
  assert(parent == OpeningWindow);
  host_event("init");
}
void GraphicWin::fill(int colour) { assert(colour == 9); host_event("fill"); }
void GraphicWin::show(int arg) { assert(arg == 0); host_event("show"); }
void GraphicWin::set_caption(char *text) {
  host_event(std::string("caption:") + text);
}
void GraphicWin::on_update() { host_event("update"); }

int Sprite::transparent_index() const { return 13; }
void Sprite::draw(Buffer *, int index, int x, int y) {
  assert(id == HostExpectedDrawFaction);
  assert(index == 13 && x == 0 && y == 2);
  host_event(std::string("portrait.draw:") + char('0' + id));
}
void Dialogs::sprite(Sprite *portrait, int x, int y) {
  assert(portrait == FactionDiplomacyPortraits[HostExpectedSpriteFaction]);
  assert(x == 0 && y == 0);
  host_event("dialog.sprite");
  HostActiveProfile->host_set_faction(HostFactionAfterSprite);
}
void Popup::start(char *file, char *section, int a, int b, int flags,
                  GraphicWin *parent) {
  const int expected_file_faction = strcmp(section, "DATALINKS2") == 0 ? 4 : 3;
  char expected_file[24];
  sprintf(expected_file, "faction%d.txt", expected_file_faction);
  assert(strcmp(file, expected_file) == 0 && a == -1 && b == 0);
  assert(parent != 0);
  host_event(std::string("popup.start:") + section + ":" + file + ":" +
             (flags == 0x400 ? "1024" : "128"));
  if (strcmp(section, "DATALINKS1") == 0 || strcmp(section, "BLURB") == 0) {
    HostActiveProfile->host_set_faction(4);
  } else if (strcmp(section, "DATALINKS2") == 0) {
    HostActiveProfile->host_set_faction(5);
  }
}
void Popup::set_caller_picture(Sprite *picture) {
  assert(picture == (HostPictureEnabled ? 0 :
                     FactionDiplomacyPortraits[HostExpectedSpriteFaction]));
  host_event("caller.picture");
}
Dialogs &Popup::dialogs() { static Dialogs value; return value; }
bool Popup::picture_enabled() const { return HostPictureEnabled; }
void Popup::set_string_font(Font *italic, Font *bold, Font *normal, int arg) {
  assert(italic == PopupMenuItalicFont && bold == PopupMenuBoldFont);
  assert(normal == PopupMenuFont && arg == 0);
  host_event("fonts");
}
void Popup::set_loc(int x, int y) { assert(x == 0xA2 && y == 2); host_event("loc"); }
void Popup::set_width(int width) {
  assert(width == (IfaceScreenWidth >= 1000 ? 0x217 : 0x17D));
  host_event("width");
}
void Popup::exec(int a, int b) { assert(a == 0 && b == 0); host_event("exec"); }

static void clear_events() { HostEvents.clear(); }
static void expect_event(int index, const char *event) {
  assert(index < (int)HostEvents.size());
  assert(HostEvents[index] == event);
}

int main() {
  for (int i = 0; i != 8; ++i) {
    HostPortraits[i].id = i;
    FactionDiplomacyPortraits[i] = &HostPortraits[i];
    sprintf(HostFilenames[i], "faction%d.txt", i);
    Players[i].filename = HostFilenames[i];
    sprintf(Players[i].formal_name_faction, "Faction%d", i);
  }
  FactionProfileWin profile;
  HostActiveProfile = &profile;
  clear_events();
  profile.initialize(3, 1);
  assert(HostEvents.size() == 5);
  expect_event(0, "buffer.ctor");
  expect_event(1, "init");
  expect_event(2, "fill");
  expect_event(3, "show");
  expect_event(4, "buffer.dtor");

  clear_events();
  profile.show_profile();
  assert(HostEvents.size() == 13);
  expect_event(0, "box");
  expect_event(1, "caption:Faction3");
  expect_event(2, "portrait.draw:3");
  expect_event(3, "popup.start:DATALINKS1:faction3.txt:1024");
  expect_event(4, "popup.start:DATALINKS2:faction4.txt:128");
  expect_event(5, "caller.picture");
  expect_event(6, "dialog.sprite");
  expect_event(7, "caption:Faction6");
  expect_event(8, "fonts");
  expect_event(9, "loc");
  expect_event(10, "width");
  expect_event(11, "exec");
  expect_event(12, "update");

  HostPictureEnabled = true;
  profile.initialize(3, 1);
  HostExpectedSpriteFaction = 5;
  HostFactionAfterSprite = 6;
  clear_events();
  profile.show_profile();
  expect_event(5, "caller.picture");

  IfaceScreenWidth = 999;
  HostExpectedDrawFaction = 3;
  HostExpectedSpriteFaction = 4;
  HostFactionAfterSprite = 5;
  clear_events();
  profile.initialize(3, 0);
  assert(HostEvents.size() == 5);
  clear_events();
  profile.show_profile();
  assert(HostEvents.size() == 12);
  expect_event(3, "popup.start:BLURB:faction3.txt:1024");
  expect_event(4, "caller.picture");
  expect_event(5, "dialog.sprite");
  expect_event(6, "caption:Faction5");
  expect_event(10, "exec");
  expect_event(11, "update");
  return 0;
}

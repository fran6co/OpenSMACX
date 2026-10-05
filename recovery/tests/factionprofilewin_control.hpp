#ifndef PAIR589560_HOST_FIXTURE_HPP
#define PAIR589560_HOST_FIXTURE_HPP

#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
typedef unsigned int uint32_t;

struct BoxSpriteParams { int id; };
struct Sprite {
  int id;
  int transparent_index() const;
  void draw(class Buffer *, int, int, int);
};
struct Player { const char *filename; char formal_name_faction[40]; };
struct Win {};
struct Font {};

extern std::vector<std::string> HostEvents;
void host_event(const std::string &);

class Buffer {
 public:
  Buffer();
  ~Buffer();
  RECT clip_rect() const;
  void box_sprite(RECT *, BoxSpriteParams *);
};

class GraphicWin : public Buffer {
 public:
  void init(RECT *, char *, int, Win *, void *, void *);
  void fill(int);
  void show(int);
  void set_caption(char *);
  void on_update();
};

class Dialogs {
 public:
  void sprite(Sprite *, int, int);
};

class Popup {
 public:
  void start(char *, char *, int, int, int, GraphicWin *);
  void set_caller_picture(Sprite *);
  Dialogs &dialogs();
  bool picture_enabled() const;
  void set_string_font(Font *, Font *, Font *, int);
  void set_loc(int, int);
  void set_width(int);
  void exec(int, int);
};

class FactionProfileWin : public GraphicWin {
 public:
  void initialize(int faction_id, int profile_mode);
  int show_profile();
  void host_set_faction(int faction_id) { faction_id_ = faction_id; }

 private:
  uint32_t field_A14_;
  uint32_t field_A18_;
  int width_;
  int height_;
  RECT bounds_;
  int faction_id_;
  int profile_mode_;
  Popup popup_;
};

extern int IfaceScreenWidth;
extern Win *OpeningWindow;
extern Player Players[8];
extern Sprite *FactionDiplomacyPortraits[8];
extern BoxSpriteParams InterfaceBoxStyles[51];
extern Font *PopupMenuItalicFont;
extern Font *PopupMenuBoldFont;
extern Font *PopupMenuFont;
extern char StringTemp[1024];
extern bool HostPictureEnabled;
extern FactionProfileWin *HostActiveProfile;
extern int HostExpectedDrawFaction;
extern int HostExpectedSpriteFaction;
extern int HostFactionAfterSprite;

#endif

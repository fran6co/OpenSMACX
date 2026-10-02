#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef void *HWND;
typedef void *HMENU;
typedef void *HINSTANCE;
typedef uint32_t DWORD;
#ifndef __cdecl
#define __cdecl
#endif
struct RECT { int32_t left, top, right, bottom; };
#define CW_USEDEFAULT ((int)0x80000000u)

struct FileWinConstruct {
  int32_t field_00_;
  RECT list_box_bounds_;
  RECT edit_box_bounds_;
  RECT button2_bounds_;
  RECT button1_bounds_;
  int32_t field_44_;
};
typedef char FileWinConstructSizeCheck[(sizeof(FileWinConstruct) == 0x48) ? 1 : -1];
struct FileWinConstructPrefix {
  int32_t field_00_;
  RECT list_box_bounds_;
  RECT edit_box_bounds_;
  RECT button2_bounds_;
  RECT button1_bounds_;
};
typedef char FileWinConstructPrefixSizeCheck[
    (sizeof(FileWinConstructPrefix) == 0x44) ? 1 : -1];

static int event_log[64];
static int event_count;
static int prepare_result, edit_result, button2_result, button1_result;
static int list_result, create_result;
static int edit_args[6], button_args[2][8], dialog_rect[4];
static int window_args[12];
static const char *window_name_seen;
static HWND window_result = reinterpret_cast<HWND>(1);

static void event(int value) { event_log[event_count++] = value; }
struct Win {
  void show(int visible) { event(visible == 0 ? 11 : 12); }
};
struct GraphicWin : Win {};
struct Sprite {
  int width_value;
  int width() const { return width_value; }
};
struct Dialog {
  uint32_t field_34_;
  int init(RECT *rect, int flags) {
    dialog_rect[0] = rect->left; dialog_rect[1] = rect->top;
    dialog_rect[2] = rect->right; dialog_rect[3] = rect->bottom;
    event(flags == 0 ? 40 : 41);
    return list_result;
  }
};
struct EditBox : Win {
  uint32_t field_B14_;
  int init(int x, int y, int width, int height, Win *, int focus_wrap) {
    edit_args[0] = x; edit_args[1] = y; edit_args[2] = width;
    edit_args[3] = height; edit_args[4] = focus_wrap;
    event(10);
    return edit_result;
  }
};
struct FlatButton {
  int result;
  int init(char *, int id, int x, int y, int width, int height, Win *, int) {
    const int slot = id == -1 ? 0 : 1;
    button_args[slot][0] = id; button_args[slot][1] = x;
    button_args[slot][2] = y; button_args[slot][3] = width;
    button_args[slot][4] = height;
    event(id == -1 ? 20 : 30);
    return result;
  }
};
struct ListBox : Dialog {
  int field_8_;
  void close() { event(39); }
  void set_callbacks(void (*draw)(void *, int, int, int, int, int),
                     void *, void *) {
    assert(draw != 0); event(50);
  }
};
struct FileWin {
  uint32_t field_638_;
  Win *modal_window_;
  RECT list_box_bounds_, edit_box_bounds_;
  FlatButton flat_button2_, flat_button1_;
  EditBox edit_box_;
  ListBox list_box_;
  HWND directory_list_handle_;
  int init(char *, GraphicWin *, FileWinConstruct *);
  int prepare_sprites() { event(1); return prepare_result; }
};

static char ok_text[] = "OK";
static char cancel_text[] = "Cancel";
char *BasePopDefaultOkText = ok_text;
char *BasePopDefaultCancelText = cancel_text;
FileWinConstructPrefix FileWinInitSharedParameters;
Sprite FileWinSprite0;
char FileWinListWindowTitle[32] = "";
int SpriteScaleNumerator, SpriteScaleDenominator;
HWND HandleMain = reinterpret_cast<HWND>(0x1234);
HINSTANCE WinInstance = reinterpret_cast<HINSTANCE>(0x5678);
void __cdecl filebox_draw(void *, int, int, int, int, int) {}

static HWND create_window(DWORD ex_style, const char *class_name,
                          const char *window_name, DWORD style, int x, int y,
                          int width, int height, HWND parent, HMENU menu,
                          HINSTANCE instance, void *parameter) {
  window_args[0] = static_cast<int>(ex_style);
  window_args[1] = style; window_args[2] = x; window_args[3] = y;
  window_args[4] = width; window_args[5] = height;
  window_args[6] = parent == HandleMain; window_args[7] = menu == (HMENU)1;
  window_args[8] = instance == WinInstance; window_args[9] = parameter == 0;
  window_name_seen = window_name;
  assert(strcmp(class_name, "LISTBOX") == 0);
  event(60);
  return create_result ? 0 : window_result;
}
#define CreateWindowExA create_window

/* FILEWIN_INIT_BODY */

static void reset_controls() {
  memset(event_log, 0, sizeof(event_log)); event_count = 0;
  memset(&FileWinInitSharedParameters, 0,
         sizeof(FileWinInitSharedParameters));
  prepare_result = edit_result = button2_result = button1_result = 0;
  list_result = create_result = 0; window_name_seen = 0;
  SpriteScaleNumerator = SpriteScaleDenominator = 0;
  FileWinSprite0.width_value = 9;
}

static FileWinConstruct parameters() {
  FileWinConstruct value;
  value.field_00_ = 0x12345678;
  value.list_box_bounds_.left = 11; value.list_box_bounds_.top = 12;
  value.list_box_bounds_.right = 111; value.list_box_bounds_.bottom = 112;
  value.edit_box_bounds_.left = 21; value.edit_box_bounds_.top = 22;
  value.edit_box_bounds_.right = 121; value.edit_box_bounds_.bottom = 122;
  value.button2_bounds_.left = 31; value.button2_bounds_.top = 32;
  value.button2_bounds_.right = 131; value.button2_bounds_.bottom = 132;
  value.button1_bounds_.left = 41; value.button1_bounds_.top = 42;
  value.button1_bounds_.right = 141; value.button1_bounds_.bottom = 142;
  value.field_44_ = 0x76543210;
  return value;
}

int main() {
  reset_controls();
  FileWin win;
  GraphicWin parent;
  FileWinConstruct args = parameters();
  assert(win.init(0, &parent, 0) == 0x10 && event_count == 0);
  assert(win.init(0, 0, &args) == 0x10 && event_count == 0);

  prepare_result = 7;
  assert(win.init(0, &parent, &args) == 7);
  assert(event_count == 1 && event_log[0] == 1);
  assert(win.modal_window_ == &parent);
  assert(SpriteScaleNumerator == 1 && SpriteScaleDenominator == 1);
  assert(FileWinInitSharedParameters.field_00_ == args.field_00_);
  assert(FileWinInitSharedParameters.button1_bounds_.bottom
         == args.button1_bounds_.bottom);

  reset_controls(); prepare_result = 0; edit_result = 5;
  assert(win.init(0, &parent, &args) == 5);
  assert(event_count == 2 && event_log[0] == 1 && event_log[1] == 10);

  reset_controls(); button2_result = 6;
  win.flat_button2_.result = 6;
  assert(win.init(0, &parent, &args) == 6);
  assert(event_count == 4 && event_log[2] == 11 && event_log[3] == 20);
  assert(button_args[0][0] == -1 && button_args[0][1] == 31
         && button_args[0][3] == 100 && button_args[0][4] == 100);

  reset_controls(); win.flat_button2_.result = 0; win.flat_button1_.result = 7;
  assert(win.init(0, &parent, &args) == 7);
  assert(event_count == 5 && event_log[4] == 30);
  assert(button_args[1][0] == -2 && button_args[1][1] == 41
         && button_args[1][3] == 100 && button_args[1][4] == 100);

  reset_controls(); win.flat_button1_.result = 0; list_result = 8;
  assert(win.init(0, &parent, &args) == 8);
  assert(event_log[event_count - 2] == 39 && event_log[event_count - 1] == 40);

  reset_controls();
  assert(win.init(0, &parent, &args) == 0);
  assert(win.list_box_.field_8_ == 1);
  assert(args.field_44_ == 0x76543210);
  assert(win.list_box_.field_34_ == 13);
  assert(dialog_rect[0] == 11 && dialog_rect[1] == 12
         && dialog_rect[2] == 111 && dialog_rect[3] == 112);
  assert(edit_args[0] == 21 && edit_args[1] == 22
         && edit_args[2] == 100 && edit_args[3] == 100);
  assert(win.edit_box_.field_B14_ == 0x104);
  assert(window_name_seen == FileWinListWindowTitle);
  assert(window_args[0] == 0 && window_args[1] == 0x40000040
         && window_args[2] == 0 && window_args[3] == 0
         && window_args[4] == CW_USEDEFAULT && window_args[5] == CW_USEDEFAULT
         && window_args[6] && window_args[7] && window_args[8] && window_args[9]);

  reset_controls(); create_result = 1;
  assert(win.init(0, &parent, &args) == 1);
  assert(win.directory_list_handle_ == 0);
  return 0;
}

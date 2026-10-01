#include "stdafx.h"
#include "quaylewin.h"
#include "alpha.h"
#include "base.h"
#include "console.h"
#include "font.h"
#include "game.h"
#include "general.h"
#include "reportif.h"
#include "strings.h"
#include "netdaemon.h"
#include "tutwin.h"
#include "stringstruct.h"


// Independent caption cells passed by the original initializer. Their writers
// and string extent remain recovery obligations; do not infer a larger buffer.
// RECOVERY_OBJECT: v1 0x00876470 ?QuayleStandardCaption@@3PADA
char QuayleStandardCaption[4];
// RECOVERY_OBJECT: v1 0x00876474 ?QuayleWideCaption@@3PADA
char QuayleWideCaption[4];

// The constructor at496896/49689C installs these compiler-generated tables.
// The +444 table belongs to GraphicWin's inherited Buffer subobject.
// Primary Win/GraphicWin table has 90 entries; the next original table starts
// at66D700. The secondary Buffer table has its two inherited entries.
// RECOVERY_GENERATED: v1 vtable 0x0066D598 0x0066D700 ??_7QuayleWin@@6BWin@@@ 0x00496810
// RECOVERY_GENERATED: v1 vtable 0x0066D590 0x0066D598 ??_7QuayleWin@@6BBuffer@@@ 0x00496810
// RECOVERY_OBJECT: v1 0x00872CB0 ?QuayleWinGlobal@@3VQuayleWin@@A
QuayleWin QuayleWinGlobal;

// ORIGINAL: 0x00496810 ??0QuayleWin@@QAE@XZ 0x00496810-0x004968B4;0x00658CB0-0x00658D08 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496810; retained seed src/unrecovered/00496810.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
QuayleWin::QuayleWin() {}

// ORIGINAL: 0x00496710 ??1QuayleWin@@QAE@XZ 0x00496710-0x00496801;0x00658C40-0x00658CA8
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496710; retained seed src/recovered/units/00496710.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// symbol    ??1QuayleWin@@UAE@XZ
QuayleWin::~QuayleWin() {}

// ORIGINAL: 0x004951A0 ?init@QuayleWin@@QAEHXZ 0x004951A0-0x004956C1
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004951A0; retained seed src/unrecovered/004951a0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
int QuayleWin::init() {
  field_A1C_ = 0x320;
  field_A20_ = 0x258;
  monitor_mode_ = ReportIfGlobal.field_1E230_;
  spot_.init(0x20);

  int width;
  int height;
  char *caption;
  if (monitor_mode_ == 0) {
    caption = QuayleStandardCaption;
    field_A14_ = field_A18_ = window_rect_.left = window_rect_.top = 0;
    window_rect_.right = field_A1C_;
    window_rect_.bottom = field_A20_;
    width = 0x320;
    height = 0x258;
  } else {
    caption = QuayleWideCaption;
    field_A14_ = window_rect_.left = 0x77;
    field_A18_ = window_rect_.top = 0x50;
    window_rect_.right = field_A1C_ + 0x77;
    window_rect_.bottom = field_A20_ + 0x50;
    width = 0x400;
    height = 0x300;
  }
  GraphicWin::init(0, 0, width, height, caption, 0x1000000, 0, 0, 0);

  line_advance_ = static_cast<uint32_t>(ReportIfGlobal.field_1E184_->line_advance());
  if (buffer1_.load_pcx("frame.pcx", 0, 0xA, 0xEC) != 0) return 1;
  if (buffer2_.load_pcx("holobook.pcx", 0, 0xA, 0xEC) != 0) return 1;

  illustration_rect_.left = 0x14C; illustration_rect_.top = 0x26;
  illustration_rect_.right = 0x2F0; illustration_rect_.bottom = 0x22E;
  chapter_list_rect_.left = 0x13A; chapter_list_rect_.top = 0x19;
  chapter_list_rect_.right = 0x301; chapter_list_rect_.bottom = 0x235;
  navigation_button_rect_.left = 0x289; navigation_button_rect_.top = 0x239;
  navigation_button_rect_.right = 0x301; navigation_button_rect_.bottom = 0x24D;
  rating_rect_.left = 0x16C; rating_rect_.top = 0x239;
  rating_rect_.right = 0x27A; rating_rect_.bottom = 0x24D;
  title_rect_.left = 0x2A; title_rect_.top = 0x22;
  title_rect_.right = 0x116; title_rect_.bottom = 0x81;
  introduction_rect_.left = 0x1E; introduction_rect_.top = 0x13E;
  introduction_rect_.right = 0x11E; introduction_rect_.bottom = 0x1B0;

  RECT *translated[] = {&illustration_rect_, &chapter_list_rect_,
                        &navigation_button_rect_, &rating_rect_,
                        &title_rect_, &introduction_rect_};
  for (unsigned int i = 0; i < sizeof(translated) / sizeof(translated[0]); ++i) {
    translated[i]->left += window_rect_.left;
    translated[i]->right += window_rect_.left;
    translated[i]->top += window_rect_.top;
    translated[i]->bottom += window_rect_.top;
  }

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0xA64 / 4));
  chapter_button_.init(StringTemp, 999, navigation_button_rect_.left, navigation_button_rect_.top,
                       navigation_button_rect_.right - navigation_button_rect_.left,
                       navigation_button_rect_.bottom - navigation_button_rect_.top,
                       this, 0);
  static_cast<Buffer &>(chapter_button_).set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
  chapter_button_.set_border_styles(&InterfaceBoxStyles[47],
                                    &InterfaceBoxStyles[46],
                                    &InterfaceBoxStyles[45]);
  chapter_button_.redraw();
  chapter_button_.hide();

  StringTemp[0] = 0;
  strcat(StringTemp, label_get(0xB60 / 4));
  pages_button_.init(StringTemp, 999, navigation_button_rect_.left, navigation_button_rect_.top,
                     navigation_button_rect_.right - navigation_button_rect_.left,
                     navigation_button_rect_.bottom - navigation_button_rect_.top, this, 0);
  static_cast<Buffer &>(pages_button_).set_font(&TutWinBaseWindow.font1_, 0, 0, 0);
  pages_button_.set_border_styles(&InterfaceBoxStyles[47],
                                 &InterfaceBoxStyles[46],
                                 &InterfaceBoxStyles[45]);
  pages_button_.redraw();
  pages_button_.hide();
  fill(0);
  return 0;
}

// ORIGINAL: 0x004964A0 ?close@QuayleWin@@QAEXXZ 0x004964A0-0x004964C1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004964A0; retained seed src/recovered/004964a0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
void QuayleWin::close() {
  list_box_.close();
  release_modal();
  GraphicWin::close();
}

// ORIGINAL: 0x004964D0 ?on_left_click@QuayleWin@@QAEXHH@Z 0x004964D0-0x00496533 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004964D0; retained seed src/recovered/004964d0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
void QuayleWin::on_left_click(int x, int y) {
  const unsigned int faction_bit = 1u << ConsoleGlobal.faction_id_;
  if ((NetDaemonNet.lock_state_[0].dword_[12] & faction_bit) != 0) return;
  if (NetDaemonNet.lock_state_[2].dword_[4] != 0) return;
  if (NetDaemonNet.lock_state_[0].dword_[3] != 0) return;
  if (spot_.check(x, y, &y, &x) + 1 != 0) field_2144_ = y;
  redraw();
}

// ORIGINAL: 0x00496540 ?on_selected@QuayleWin@@QAEXHH@Z 0x00496540-0x00496558 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496540; retained seed src/recovered/00496540.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
void QuayleWin::on_selected(int selection, int) {
  field_2144_ = selection;
  redraw();
}

// ORIGINAL: 0x00496560 ?on_button_clicked@QuayleWin@@QAEXH@Z 0x00496560-0x004965B1
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496560; retained seed src/recovered/00496560.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// symbol    ?on_button_clicked@QuayleWin@@UAEXH@Z
void QuayleWin::on_button_clicked(int button_id) {
  if (button_id != 999) return;
  if (field_2144_ != -1) {
    field_2144_ = -1;
    redraw();
    return;
  }
  close();
}

// ORIGINAL: 0x004965C0 ?on_key_click@QuayleWin@@QAEHHH@Z 0x004965C0-0x0049660C
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004965C0; retained seed src/recovered/units/004965c0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
int QuayleWin::on_key_click(int, int) {
  if (field_2144_ != -1) {
    field_2144_ = -1;
    redraw();
  } else {
    close();
  }
  return 1;
}

// ORIGINAL: 0x004956D0 ?on_redraw@QuayleWin@@QAEXXZ 0x004956D0-0x0049649D;0x00658D10-0x00658D4F
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 004956D0; retained seed src/unrecovered/004956d0.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
// symbol    ?on_redraw@QuayleWin@@UAEXXZ
void QuayleWin::on_redraw() {
  Heap *saved_string_owner = StringAllocationHeap;
  StringAllocationHeap = 0;
  StringStruct strings;
  const int book = ConsoleGlobal.faction_id_;
  const int chapter = field_2144_;
  char temporary[0x100];
  char number[0x50];
  {
  Buffer local_buffer;
  Buffer *canvas = static_cast<Buffer *>(this);
  canvas->restore_clip();
  if (monitor_mode_ != 0) buffer1_.copy(canvas, 0, 0, 0, 0, 0x400, 0x300);
  buffer2_.copy(canvas, 0, 0, window_rect_.left, window_rect_.top, 0x320, 0x258);
  canvas->set_clip(&window_rect_);
  canvas->set_clip(&title_rect_);
  canvas->set_font(ReportIfGlobal.field_1E1AC_, 0, 0, 0);
  canvas->set_text_color(0x9B, -1, 1, 1);
  spot_.kill_type(0);

  parse_it("Holobook", "BOOKTITLE");
  canvas->wrap_cent(StringTemp, title_rect_.left, title_rect_.top,
                    title_rect_.right - title_rect_.left);
  canvas->set_font(ReportIfGlobal.field_1E1B0_, 0, 0, 0);
  canvas->set_text_color(0x63, -1, 1, 1);

  chapter_button_.hide();
  pages_button_.hide();
  if (chapter == -1) {
    canvas->set_clip(&introduction_rect_);
    parse_it("Holobook", "BOOKINTRO");
    canvas->wrap_cent(StringTemp, introduction_rect_.left, introduction_rect_.top,
                      introduction_rect_.right - introduction_rect_.left);

    if (Language == 0) {
    canvas->restore_clip();
      Font *font = ReportIfGlobal.field_1E1A8_;
      canvas->set_font(font, font, font, 0);
      canvas->set_text_color(0x63, -1, 1, 1);
      canvas->set_text_color2(0x9B, -1, 1, 1);
      canvas->set_text_color3(0x9B, -1, 1, 1);
      text_scratch_rect_.left = chapter_list_rect_.left;
      text_scratch_rect_.top = chapter_list_rect_.bottom + 6;
      text_scratch_rect_.right = chapter_list_rect_.right;
      text_scratch_rect_.bottom = chapter_list_rect_.bottom + 10;
      RECT &spot_rect = text_scratch_rect_;
      for (int page = 0; page <= field_2148_; ++page) {
        temporary[0] = 0;
        strcat(temporary, "BOOKLIST");
        _itoa(page, number, 10);
        strcat(temporary, number);
        GenderDefault = Players[book].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[book].title_leader,
                   Players[book].is_leader_female, -1);
        GenderDefault = Players[book].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[book].name_leader,
                   Players[book].is_leader_female, -1);
        parse_it("Holobook", temporary);
        if (page == field_2148_) {
          strcpy(temporary, StringTemp);
          StringTemp[0] = 0;
          strcat(StringTemp, "["); strcat(StringTemp, temporary);
          strcat(StringTemp, "]");
        }
        const int row_width = static_cast<int>(chapter_list_rect_.right - chapter_list_rect_.left);
        field_A14_ = chapter_list_rect_.left;
        field_A18_ = spot_rect.top;
        field_A1C_ = row_width;
        field_A20_ = canvas->wrap_height(StringTemp, row_width);
        spot_rect.left = chapter_list_rect_.left;
        spot_rect.right = chapter_list_rect_.left + row_width;
        spot_rect.top = field_A18_ - field_A20_ - 6;
        spot_rect.bottom = spot_rect.top + field_A20_;
        spot_.add(page, 0, spot_rect.left, spot_rect.top,
                  spot_rect.right - spot_rect.left,
                  spot_rect.bottom - spot_rect.top);
        canvas->wrap(StringTemp, &spot_rect);
      }
    } else {
      list_box_.close();
      list_box_.Dialog::init(&chapter_list_rect_, 0);
      list_box_.set_dialog_font(ReportIfGlobal.field_1E1A8_,
                                ReportIfGlobal.field_1E1A8_,
                                ReportIfGlobal.field_1E1A8_);
      list_box_.set_dialog_text_color(0x63, -1, 1, 1);
      list_box_.set_report_text_color(0x9B);
      for (int page = field_2148_; page >= 0; --page) {
        temporary[0] = 0;
        strcat(temporary, "BOOKLIST"); _itoa(page, number, 10);
        strcat(temporary, number);
        GenderDefault = Players[book].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[book].title_leader,
                   Players[book].is_leader_female, -1);
        GenderDefault = Players[book].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[book].name_leader,
                   Players[book].is_leader_female, -1);
        parse_it("Holobook", temporary);
        list_box_.item(StringTemp, page);
      }
      list_box_.reset_report_selection();
      list_box_.field_8_ = 1;
      list_box_.Dialog::field_B4_ = 0;
      list_box_.Dialog::attach_at_stored_position(this, 0x80020);
      list_box_.set_selected_pos(0, 1);
      list_box_.show(0);
    }

    field_2144_ = -1;
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(0x424));
    strcat(StringTemp, ":"); strcat(StringTemp, " ");
    strcat(StringTemp, " ");
    _itoa(get_rating(book, PlayersData[book].unk_107), number, 10);
    strcat(StringTemp, number);
    strcat(StringTemp, "%");
    canvas->set_font(ReportIfGlobal.field_1E1B0_, 0, 0, 0);
    canvas->set_text_color(0x9B, -1, 1, 1);
    canvas->restore_clip();
    canvas->write_l(StringTemp, &rating_rect_,
                    static_cast<unsigned int>(strlen(StringTemp)));
    pages_button_.hide();
    chapter_button_.show(1);
  } else {
    list_box_.close();
    canvas->set_clip(&introduction_rect_);
    if (Language == 0) {
      parse_it("Holobook", "BOOKINTRO");
    } else {
      temporary[0] = 0;
      strcat(temporary, "BOOKLIST"); _itoa(chapter, number, 10);
      strcat(temporary, number);
      GenderDefault = Players[book].is_leader_female;
      PluralityDefault = 0;
      parse_says(0, Players[book].title_leader,
                 Players[book].is_leader_female, -1);
      GenderDefault = Players[book].is_leader_female;
      PluralityDefault = 0;
      parse_says(1, Players[book].name_leader,
                 Players[book].is_leader_female, -1);
      parse_it("Holobook", temporary);
    }
    canvas->wrap_cent(StringTemp, introduction_rect_.left, introduction_rect_.top,
                      introduction_rect_.right - introduction_rect_.left);
      canvas->restore_clip();
    StringTemp[0] = 0;
    strcat(StringTemp, "artbox"); _itoa(chapter, number, 10);
    strcat(StringTemp, number); strcat(StringTemp, ".pcx");
    local_buffer.load_pcx(StringTemp, 0, 10, 0xEC);
    local_buffer.draw(canvas, 9, illustration_rect_.left, illustration_rect_.top);
    if (chapter == 1) {
      RECT &facts = text_scratch_rect_;
      facts.left = window_rect_.left + 0x15B; facts.top = window_rect_.top + 0x1F6;
      facts.right = window_rect_.left + 0x2E2; facts.bottom = window_rect_.top + 0x21C;
      canvas->set_clip(&illustration_rect_);
      StringTemp[0] = 0;
      strcat(StringTemp, Players[book].title_leader);
      strcat(StringTemp, " ");
      strcat(StringTemp, Players[book].name_leader);
      canvas->set_font(ReportIfGlobal.field_1E1AC_, 0, 0, 0);
      canvas->set_text_color(0x45, -1, 1, 1);
      canvas->write_cent_l(StringTemp, &facts, static_cast<unsigned int>(strlen(StringTemp)));
      canvas->restore_clip();
    } else if (chapter == 2) {
      RECT &facts = text_scratch_rect_;
      facts.left = window_rect_.left + 0x136; facts.top = window_rect_.top + 0x1A5;
      facts.right = window_rect_.left + 0x301; facts.bottom = window_rect_.top + 0x1D7;
      canvas->set_clip(&illustration_rect_);
      StringTemp[0] = 0;
      strcat(StringTemp, Players[book].title_leader);
      strcat(StringTemp, " ");
      strcat(StringTemp, Players[book].name_leader);
      canvas->set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
      canvas->set_text_color(0x45, -1, 1, 1);
      canvas->write_cent_l(StringTemp, &facts, static_cast<unsigned int>(strlen(StringTemp)));
      canvas->restore_clip();
    } else if (chapter == 8) {
      RECT &facts = text_scratch_rect_;
      facts.left = window_rect_.left + 0x13C; facts.top = window_rect_.top + 0x1DF;
      facts.right = window_rect_.left + 0x2FD; facts.bottom = window_rect_.top + 0x233;
      canvas->set_clip(&illustration_rect_);
      StringTemp[0] = 0;
      strcat(StringTemp, Players[book].title_leader);
      strcat(StringTemp, " ");
      strcat(StringTemp, Players[book].name_leader);
      canvas->set_font(ReportIfGlobal.field_1E1B8_, 0, 0, 0);
      canvas->set_text_color(0x5E, -1, 1, 1);
      canvas->wrap_cent(StringTemp, &facts);
      canvas->restore_clip();
    }
    chapter_button_.hide();
    pages_button_.show(1);
  }
  }
  strings.remove_all();
  StringAllocationHeap = saved_string_owner;
}

void QuayleWin::vslot_19(int x, int y) { on_left_click(x, y); }

int QuayleWin::vslot_25(int repeat, int key) {
  return on_key_click(repeat, key);
}

void QuayleWin::vslot_53(int selection, int unused) {
  on_selected(selection, unused);
}

// ORIGINAL: 0x00496670 ?quayle@@YAXH@Z 0x00496670-0x0049670F
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original entry 00496670; retained seed src/unrecovered/00496670.cpp", "recovery/tasks/quaylewin-owner-coverage-wave.json", "QuayleWin constructor layout and virtual-slot evidence; fresh isolated build and host controls"], "reason": "Rehome the retained QuayleWin seed into its real GraphicWin owner with typed subobjects; preserve original spans and historical seed obligations."}
void __cdecl quayle(int faction_id) {
  int score = PlayersData[faction_id].unk_107;
  int rating = get_rating(faction_id, score);
  int rank = 0;
  int step = 2;
  do {
    if (rating < (step * step) / 2) break;
    ++rank;
    ++step;
  } while (step - 2 < 0x18);
  if (IsMultiplayerNet) return;
  if (QuayleWinGlobal.init() != 0) return;
  if (rank < 0x19) {
    if (rank < -1) rank = -1;
  } else {
    rank = 0x18;
  }
  QuayleWinGlobal.set_page(rank);
  QuayleWinGlobal.set_chapter(-1);
  QuayleWinGlobal.set_modal(0, 0, 0);
}

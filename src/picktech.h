#pragma once
#include "stdafx.h"
#include "graphicwin.h"
#include "font.h"
#include "technology.h"

// Native484AC0/488690: real GraphicWin bases and four Font lifetimes.
// Primary original table66CFCC; secondary Buffer table66CFC4.
class PickTech : public GraphicWin {
public:
    PickTech();
    virtual ~PickTech();
    void close();

    void on_mouse_move(int x, int y);
    int on_key_click(int source, int key);
    // Native table slot19 directly owns 4860B0; no forwarding callback body.
    virtual void on_left_click_event(int x, int y);
    // Native66CFCC+22*4→486110. Win's inherited name is still neutral.
    virtual void vslot_22(int x, int y);
    void UNK1(char *text);
    int draw_item(int item, int selected);
    int draw_tech(int tech, RECT *rect, int flags);
    int pick(int faction, int mode, int count, char *section_id);

    // Constructor stores establish these four state members as zero. Other
    // labels remain neutral until multiple bodies/native accesses agree.
    int cell_width_;              // +0xA14, draw_item and draw_tech
    int cell_height_;             // +0xA18, draw_item and draw_tech
    int row_pitch_;               // +0xA1C, draw and caption pen advance
    int faction_id_;              // +0xA20, assigned by pick/used for preview
    int column_count_;            // +0xA24, ctor zero / mouse / key / draw
    int row_count_;               // +0xA28, ctor zero / key / draw
    int item_count_;              // +0xA2C, draw / key / pick
    int layout_state_30_;         // +0xA30, semantics unresolved
    int layout_state_34_;         // +0xA34, semantics unresolved
    int selected_index_;          // +0xA38, ctor zero / mouse / click / key
    int selection_active_;        // +0xA3C, ctor zero / mouse / click
    int page_x_;                  // +0xA40
    int page_bottom_;             // +0xA44
    int page_width_;              // +0xA48
    int title_baseline_;          // +0xA4C
    int page_pen_y_;              // +0xA50
    int page_state_;              // +0xA54, draw_tech and caption flow
    int technology_ids_[MaxTechnologyNum]; // +0xA58..+0xBB8, 89 native slots
    Font title_font_;             // +0xBBC
    Font body_font_primary_;      // +0xBE4
    Font body_font_secondary_;    // +0xC0C
    Font header_font_;            // +0xC34
};


static_assert(sizeof(PickTech) == 0xC5C, "PickTech native extent");


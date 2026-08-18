// ORIGINAL: 0x0041E790 ?on_redraw@BaseWin@@QAEXXZ 0x0041E790-0x0041E885 FILE
// RULED-OUT: caching `[this+0x40afc]` into one local reused for all three BaseButton::set checks (mnemonic similarity 0.945, 8 edit blocks) vs. re-reading it through a `volatile int *` for each check (0.959, 6 edit blocks) - the original re-reads the field three separate times (0x0041E7B9, 0x0041E7D2, 0x0041E7EC), so the volatile re-read is the more faithful form and scores closer. Neither reached BYTE_EXACT; first divergence is prologue/register scheduling around the button-set sequence.
// size      245 bytes
// prototype void (__thiscall ?on_redraw@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   ?   call targets   16
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0040C850 0x0040E360 0x0040F0F0 0x0040FCC0 0x00410870 0x00411240
//           0x004103B0 0x00412AA0 0x00414B40 0x00415080 0x00415730 0x00415B20
//           0x0041E560 0x0045C290 0x005D5890 0x00607C80

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.

class MainInterface {
public:
    void restore_back(int);
};

class GraphicWin {
public:
    void soft_update();
};

class BaseButton {
public:
    void set(int);
};

class BaseWin {
public:
    void update_gov_buttons();
    void draw_production(int);
    void draw_expenses(int);
    void draw_queue(int);
    void draw_info(int);
    void draw_misc(int);
    void draw_farm(int);
    void draw_pop(int);
    void draw_facilities(int);
    void draw_commerce(int);
    void draw_nutrients(int);
    void draw_supported(int);
    void draw_garrison(int);
    void on_redraw();
};

extern int g_7d392c;
extern int g_7ae778[];

static MainInterface *const g_main_interface = (MainInterface *)0x007AE820;
static GraphicWin *const g_main_interface_gw = (GraphicWin *)0x007AE820;

void BaseWin::on_redraw() {
    if (g_7ae778[g_7d392c] == 3) {
        g_main_interface->restore_back(1);
        update_gov_buttons();
        volatile int *const production =
            reinterpret_cast<volatile int *>(reinterpret_cast<char *>(this) + 0x40afc);
        char *self = reinterpret_cast<char *>(this);
        reinterpret_cast<BaseButton *>(self + 0x148b8)->set(*production == 0);
        reinterpret_cast<BaseButton *>(self + 0x15404)->set(*production == 1);
        reinterpret_cast<BaseButton *>(self + 0x13d6c)->set(*production == 2);
        draw_production(0);
        draw_expenses(0);
        draw_queue(0);
        draw_info(0);
        draw_misc(0);
        draw_farm(0);
        draw_pop(0);
        draw_facilities(0);
        draw_commerce(0);
        draw_nutrients(0);
        draw_supported(0);
        draw_garrison(0);
        reinterpret_cast<GraphicWin *>(this)->soft_update();
        g_main_interface_gw->soft_update();
    }
}

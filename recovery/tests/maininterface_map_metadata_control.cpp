// Actual draw_edit_pane host control, not native class layout or UI acceptance.
#include <cassert>
#include <climits>
#include <stdint.h>
#include <stddef.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
typedef int BOOL;
struct Map {};
// @@ACTUAL_MAP_LAYOUT@@
MapStorageWindow MapStorage;
struct RECT { int left, top, right, bottom; };
struct Font {};
struct BoxStyle {};
static std::vector<std::string> lines;
static std::vector<int> labels;
static int updates;
struct Buffer {
    void set_text_color(int,int,int,int) {}
    void set_font(Font*,Font*,Font*,int) {}
    void box_sprite(RECT*,BoxStyle*) {}
    void write_l(const char* s,int,int,size_t n) { lines.push_back(std::string(s,n)); }
    int text_line_height() { return 10; }
};
struct MainInterface : Buffer {
    struct Button { int field_84_; } button_group_[1];
    RECT editor_text_rect_;
    void draw_edit_pane();
    void soft_update(RECT* r) { assert(r == &editor_text_rect_); ++updates; }
};
static MainInterface MainInterfaceGlobal;
static int WinScreenWidth = 1024;
static int InfoTextColors[4][2];
static BoxStyle InterfaceBoxStyles[1];
static int MapEditorPaintId, MapEditorBrushId = 1;
static Font font;
static Font* find_font(int,int) { return &font; }
static const char* label_get(int id) {
    labels.push_back(id);
    switch (id) { case 20:return "dry"; case 21:return "medium"; case 22:return "wet"; default:return "label"; }
}
static const int VK_SCROLL=145;
static int scroll, locked;
static int GetKeyState(int) { return scroll; }
static struct Console { int edit_lock() { return locked; } } ConsoleGlobal;
static struct Strings { const char* get(int) { return "tool"; } } StringTable;
static struct NaturalRule { int name_short; } Natural[11];
static struct TerraformRule { int name; } Terraforming[24];
static void _itoa(int v,char* s,int radix) { assert(radix==10); sprintf(s,"%d",v); }
// @@ACTUAL_DRAW_EDIT_PANE@@
int main() {
    MainInterfaceGlobal.button_group_[0].field_84_=1002;
    MapLongitudeBounds=80; MapLatitudeBounds=40;
    strcpy(MapFilePath,"world.map");
    for (int cloud=0;cloud!=3;++cloud) {
        MapCloudCover=cloud; MapSizePlanet=(cloud+1)%3;
        lines.clear(); labels.clear();
        MainInterfaceGlobal.draw_edit_pane();
        assert(lines.size()==6);
        const char* expected=cloud==0?"label: {dry}":cloud==1?"label: {medium}":"label: {wet}";
        assert(lines[4]==expected);
        assert(lines[5]==lines[4]+"{label}"); // original append, no clear
        assert(labels[8]==20+cloud);
    }
    // Observe wrapped argument only. The mock does not establish safety of
    // the original unchecked label-table access for this malformed value.
    MapCloudCover=INT_MAX; labels.clear(); lines.clear();
    MainInterfaceGlobal.draw_edit_pane();
    assert(labels[8]==INT_MIN+19);
    int before=updates;
    WinScreenWidth=1023; MainInterfaceGlobal.draw_edit_pane();
    assert(updates==before);
    WinScreenWidth=1024; MainInterfaceGlobal.button_group_[0].field_84_=0;
    MainInterfaceGlobal.draw_edit_pane(); assert(updates==before);
}

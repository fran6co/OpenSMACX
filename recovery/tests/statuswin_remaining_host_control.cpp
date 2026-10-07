// Bounded host double for the extracted candidate bodies; not a target ABI or
// runtime proof. The actual candidate methods are included from the single
// private method packet below (rather than copied into the fixture).
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct RECT { long left, top, right, bottom; };
struct VOX_Vect;
struct VOX_Matrix;
static int g_union_calls;
static int g_base_at_result = -1;
static int g_veh_at_result = -1;
static int g_base_at_calls;
static int g_veh_at_calls;
static int g_rotation_calls;
static int g_font_init_calls[3];
static int g_font_last_height[3];
static int g_font_last_style[3];
static int g_caviar_init_calls;
static int g_caviar_direct_calls;
static int g_caviar_scaling_calls;
static VOX_Vect *g_caviar_camera_out;
static VOX_Matrix *g_caviar_matrix_out;
static float g_caviar_scaling;

int WinScreenWidth = 800;
const char *DefaultFontFace = "Arial";
uint32_t StatusFlags;

struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };

struct Veh { int16_t next_veh_id_stack, prev_veh_id_stack; };
Veh Vehs[4];

struct Font {
    int unk_1_;
    int slot_;
    char padding_[0x20];
    Font() : unk_1_(0), slot_(0) { memset(padding_, 0, sizeof(padding_)); }
    void init(const char *, int height, int style) {
        ++g_font_init_calls[slot_];
        g_font_last_height[slot_] = height;
        g_font_last_style[slot_] = style;
    }
};

struct Spot {
    int init_count;
    int last_size;
    Spot() : init_count(0), last_size(0) { }
    void init(int size) { ++init_count; last_size = size; }
};

struct Caviar {
    char prefix_[0x60];
    VOX_Vect light_vector_;
    VOX_Vect shadow_vector_;
    VOX_Vect &light_vector() { return light_vector_; }
    VOX_Vect &shadow_vector() { return shadow_vector_; }
    char through_A3_[0xA4 - 0x78];
    uint8_t field_A4_;
    char tail_[0x13D0 - 0xA5];
    Caviar() : field_A4_(0) {
        memset(prefix_, 0, sizeof(prefix_));
        memset(through_A3_, 0, sizeof(through_A3_));
        memset(tail_, 0, sizeof(tail_));
    }
    void init() { ++g_caviar_init_calls; }
    void set_field_A4(int v) { field_A4_ = static_cast<uint8_t>(v); }
    void set_camera_direct(const VOX_Vect *camera, const VOX_Matrix *matrix) {
        ++g_caviar_direct_calls;
        *g_caviar_camera_out = *camera;
        *g_caviar_matrix_out = *matrix;
    }
    void set_scaling(float value) { ++g_caviar_scaling_calls; g_caviar_scaling = value; }
};
typedef char CaviarSizeMustMatch[(sizeof(Caviar) == 0x13D0) ? 1 : -1];

struct MainInterfaceDouble {
    RECT hit, right, left, updated;
    int soft_update_count;
    MainInterfaceDouble() : soft_update_count(0) { }
    const RECT &status_hit_rect() const { return hit; }
    const RECT &status_right_rect() const { return right; }
    const RECT &status_left_rect() const { return left; }
    void soft_update(const RECT *rect) { ++soft_update_count; updated = *rect; }
} MainInterfaceGlobal;

int base_at(int, int) { ++g_base_at_calls; return g_base_at_result; }
int veh_at(int, int) { ++g_veh_at_calls; return g_veh_at_result; }

int UnionRect(RECT *out, const RECT *a, const RECT *b) {
    ++g_union_calls;
    out->left = a->left < b->left ? a->left : b->left;
    out->top = a->top < b->top ? a->top : b->top;
    out->right = a->right > b->right ? a->right : b->right;
    out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    return 1;
}

void caviar_apply_rotation(const float *angles, VOX_Matrix *matrix) {
    ++g_rotation_calls;
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            matrix->values[row][col] = (row == col) ? 1.0f : 0.0f;
    assert(angles[0] == 0.0f && angles[2] == 0.0f);
}

struct StatusWin {
    char prefix_[0x30];
    Caviar caviar_;
    char through_13FF_[0x100];
    VOX_Vect camera_angles_;
    VOX_Matrix camera_rotation_;
    Font font1_, font2_, font3_;
    int field_15A8_, field_15AC_;
    char field_15B0_[4];
    int iTileX_, iTileY_, iUnitIndex_, iCityIndex_;
    int field_15C4_, field_15C8_, field_15CC_, field_15D0_, field_15D4_;
    int fInOnClick_, field_15DC_;
    Spot spot_;
    int draw_count;
    StatusWin() : field_15A8_(0), field_15AC_(0), iTileX_(0), iTileY_(0),
                  iUnitIndex_(-1), iCityIndex_(-1), field_15C4_(0),
                  field_15C8_(0), field_15CC_(0), field_15D0_(0),
                  field_15D4_(0), fInOnClick_(0), field_15DC_(0),
                  draw_count(0) { }
    void draw_status(int, int, int, int, int) { ++draw_count; }
    void arrow_left();
    void arrow_right();
    void init();
};
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif
typedef char StatusAnglesMustBeAt1500[(offsetof(StatusWin, camera_angles_) == 0x1500) ? 1 : -1];
typedef char StatusMatrixMustBeAt150C[(offsetof(StatusWin, camera_rotation_) == 0x150C) ? 1 : -1];
typedef char StatusFontMustBeAt1530[(offsetof(StatusWin, font1_) == 0x1530) ? 1 : -1];
typedef char StatusUnitMustBeAt15BC[(offsetof(StatusWin, iUnitIndex_) == 0x15BC) ? 1 : -1];
typedef char StatusTailMustBeAt15DC[(offsetof(StatusWin, field_15DC_) == 0x15DC) ? 1 : -1];
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

/*__STATUSWIN_THREE_METHOD_BODIES__*/

int main() {
    StatusWin status;
    status.font1_.slot_ = 0; status.font2_.slot_ = 1; status.font3_.slot_ = 2;
    MainInterfaceGlobal.hit.left = 0; MainInterfaceGlobal.hit.top = 0;
    MainInterfaceGlobal.hit.right = 10; MainInterfaceGlobal.hit.bottom = 10;
    MainInterfaceGlobal.right.left = 10; MainInterfaceGlobal.right.top = 0;
    MainInterfaceGlobal.right.right = 20; MainInterfaceGlobal.right.bottom = 10;
    MainInterfaceGlobal.left.left = -5; MainInterfaceGlobal.left.top = 0;
    MainInterfaceGlobal.left.right = 0; MainInterfaceGlobal.left.bottom = 10;

    // Existing stack: previous changes selection, redraws, and unions all 3
    // UI rectangles, while same-size fonts remain untouched at 800px.
    status.iUnitIndex_ = 1;
    Vehs[1].prev_veh_id_stack = 0;
    status.field_15AC_ = 10;
    status.arrow_left();
    assert(status.iUnitIndex_ == 0 && status.iCityIndex_ == -1);
    assert(status.draw_count == 1 && MainInterfaceGlobal.soft_update_count == 1);
    assert(g_font_init_calls[0] == 0 && g_union_calls == 2);

    // End of stack selects the tile base only when one exists, then applies
    // the non-800 font size and the native +4 vertical adjustment.
    status.iUnitIndex_ = 2;
    Vehs[2].prev_veh_id_stack = -1;
    status.iTileX_ = 7; status.iTileY_ = 9;
    g_base_at_result = 6;
    WinScreenWidth = 1024;
    status.arrow_left();
    assert(status.iUnitIndex_ == -1 && status.iCityIndex_ == 6);
    assert(g_base_at_calls == 1 && g_font_last_height[0] == 12);
    assert(g_font_last_style[1] == 2 && g_font_last_style[2] == 1);
    assert(status.font1_.unk_1_ == 4 && status.draw_count == 2);

    // The city-to-unit route requires the separately mapped 15DC flag.
    g_veh_at_result = 3;
    const int before_city_draw = status.draw_count;
    status.field_15DC_ = 0;
    status.arrow_right();
    assert(status.iCityIndex_ == 6 && status.iUnitIndex_ == -1);
    assert(g_veh_at_calls == 1 && status.draw_count == before_city_draw);
    status.field_15DC_ = 1;
    status.arrow_right();
    assert(status.iCityIndex_ == -1 && status.iUnitIndex_ == 3);
    assert(g_veh_at_calls == 2 && status.draw_count == 3);

    // Unit-to-next route uses Veh's actual +0x30 link member.
    Vehs[3].next_veh_id_stack = 0;
    status.arrow_right();
    assert(status.iUnitIndex_ == 0 && status.draw_count == 4);

    // The full initializer resets state, creates owned subobjects, rotates
    // the camera, and seeds the actual Caviar vectors through typed members.
    StatusWin fresh;
    fresh.font1_.slot_ = 0; fresh.font2_.slot_ = 1; fresh.font3_.slot_ = 2;
    VOX_Vect captured_camera;
    VOX_Matrix captured_matrix;
    g_caviar_camera_out = &captured_camera;
    g_caviar_matrix_out = &captured_matrix;
    StatusFlags = 0;
    fresh.init();
    assert(StatusFlags == 0x20 && fresh.spot_.init_count == 1);
    assert(fresh.spot_.last_size == 0x18 && g_caviar_init_calls == 1);
    assert(fresh.caviar_.field_A4_ == 0 && g_rotation_calls == 1);
    assert(g_caviar_direct_calls == 1 && g_caviar_scaling_calls == 1);
    assert(g_caviar_scaling == 0.18f);
    assert(captured_camera.values[0] == 0.0f);
    assert(captured_camera.values[1] == 0.0f);
    assert(captured_camera.values[2] == 10000.0f);
    assert(captured_matrix.values[2][0] == 0.0f);
    assert(captured_matrix.values[2][1] == 0.0f);
    assert(captured_matrix.values[2][2] == -1.0f);
    assert(fresh.caviar_.light_vector_.values[0] == 0.0f);
    assert(fresh.caviar_.light_vector_.values[1] == 0.0f);
    assert(fresh.caviar_.light_vector_.values[2] == 1.0f);
    assert(fresh.caviar_.shadow_vector_.values[2] == 1.0f);
    assert(fresh.iUnitIndex_ == -1 && fresh.iCityIndex_ == -1);
    return 0;
}

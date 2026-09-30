// Actual Dialog declaration/close and Heap bodies; the list is a call-order
// double. Real entry/hook/allocator lifetimes have separate StringList tests.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <cstdlib>
typedef signed char int8_t;
typedef unsigned int uint32_t;
typedef void *LPVOID;
typedef int BOOL;
struct RECT { int left, top, right, bottom; };
struct Font { int tag; };
class GraphicWin; // Only pointer parameters are present in the retained declaration.
#define MEASURED
#define UNUSED(name)
#define nullptr 0
#define private public
// LIVE_HEAP_DECLARATION
int clears, list_destructors, frees, stage, expect_heap_release;
void *allocated;
void tracked_release(void *memory) {
    assert(stage == 1 && memory == allocated);
    ++frees;
    allocated = 0;
    free(memory);
    stage = 2;
}
namespace std { void tracked_free(void *p) { tracked_release(p); } }
// LIVE_HEAP_CONSTRUCTOR
#define free tracked_free
// LIVE_HEAP_DESTRUCTOR
// LIVE_HEAP_RESET
#undef free

struct StringList {
    void *head_, *current_;
    int entry_count_, current_position_;
    StringList() : head_(0), current_(0), entry_count_(0), current_position_(0) {}
    ~StringList() { ++list_destructors; }
    void remove_all() {
        assert(stage == 0 && (allocated != 0) == expect_heap_release);
        ++clears;
        stage = 1;
        if (head_) {
            head_ = 0;
            current_position_ = 0;
            entry_count_ = 0;
        }
    }
};
// LIVE_DIALOG_DECLARATION
#undef private
uint32_t DialogDefaults[37];
Font *DialogDefaultFonts[3];
uint32_t DialogField30Default = 301, DialogField34Default = 341;
uint32_t DialogField40Default = 401, DialogFieldACDefault = 1721;
uint32_t DialogFieldB0Default = 1761, DialogFieldB4Default = 1801;
uint32_t DialogFieldB8Default = 1841;
Dialog::Dialog() {}
Dialog::~Dialog() {}
// LIVE_DIALOG_CLOSE

void check_defaults(Dialog &dialog) {
    assert(!dialog.heap_ptr_ && !dialog.selected_position_);
    assert(dialog.field_1C_ == ~0u && dialog.field_F0_ == ~0u);
    assert(dialog.field_48_ == ~0u);
    assert(dialog.field_20_ == DialogDefaults[36]);
    assert(dialog.field_2C_ == DialogDefaults[19]);
    assert(dialog.field_44_ == DialogDefaults[20]);
    assert(dialog.field_4C_ == DialogDefaults[21]);
    assert(dialog.field_50_ == DialogDefaults[22]);
    assert(dialog.field_6C_ == DialogDefaults[23]);
    assert(dialog.field_30_ == DialogField30Default);
    assert(dialog.field_34_ == DialogField34Default);
    assert(dialog.field_40_ == DialogField40Default);
    assert(dialog.field_AC_ == DialogFieldACDefault);
    assert(dialog.field_B0_ == DialogFieldB0Default);
    assert(dialog.field_B4_ == DialogFieldB4Default);
    assert(dialog.field_B8_ == DialogFieldB8Default);
    uint32_t Dialog::*zero_fields[] = {
        &Dialog::field_24_, &Dialog::field_28_, &Dialog::field_38_,
        &Dialog::field_3C_, &Dialog::field_54_, &Dialog::field_58_,
        &Dialog::field_5C_, &Dialog::field_60_, &Dialog::field_64_,
        &Dialog::field_68_};
    for (int i = 0; i < 10; ++i) assert(dialog.*zero_fields[i] == 0);
    uint32_t Dialog::*colors[] = {
        &Dialog::text_color_a_, &Dialog::text_color_2a_, &Dialog::text_color_3a_,
        &Dialog::text_color_b_, &Dialog::text_color_2b_, &Dialog::text_color_3b_,
        &Dialog::text_color_c_, &Dialog::text_color_2c_, &Dialog::text_color_3c_,
        &Dialog::text_color_d_, &Dialog::text_color_2d_, &Dialog::text_color_3d_};
    for (int color = 0; color < 12; ++color)
        assert(dialog.*colors[color] == DialogDefaults[24 + color]);
    assert(dialog.font1_ == DialogDefaultFonts[0]);
    assert(dialog.font2_ == DialogDefaultFonts[1]);
    assert(dialog.font3_ == DialogDefaultFonts[2]);
}

int main() {
    Font fonts[3];
    for (int f = 0; f < 3; ++f) DialogDefaultFonts[f] = &fonts[f];
    {
        Dialog dialog;
        // Reuse the same live heap/list members after both nonempty and
        // empty-head resets, with changed defaults on every pass.
        for (int pass = 0; pass < 4; ++pass) {
            for (int i = 0; i < 37; ++i) DialogDefaults[i] = 1000 * pass + i;
            stage = 0;
            expect_heap_release = 1;
            allocated = malloc(16);
            assert(allocated);
            dialog.heap_.base_ = allocated;
            dialog.heap_.current_ = allocated;
            dialog.heap_.base_size_ = dialog.heap_.free_size_ = 16;
            dialog.heap_.err_flags_ = 7;
            dialog.heap_ptr_ = &dialog.heap_;
            dialog.item_list_.head_ = (pass & 1) ? 0 : &dialog;
            dialog.item_list_.current_ = &dialog;
            dialog.item_list_.entry_count_ = 3;
            dialog.item_list_.current_position_ = 19;
            dialog.close();
            assert(stage == 2 && !allocated && frees == pass + 1);
            assert(clears == 2 * pass + 1 && list_destructors == 0);
            assert(!dialog.heap_.base_ && !dialog.heap_.current_);
            assert(!dialog.heap_.base_size_ && !dialog.heap_.free_size_);
            assert(!dialog.heap_.err_flags_);
            assert(!dialog.item_list_.head_ && !dialog.item_list_.current_position_);
            assert(dialog.item_list_.current_ == &dialog);
            assert(dialog.item_list_.entry_count_ == ((pass & 1) ? 3 : 0));
            check_defaults(dialog);
            // Closing again without reinitialization must not free anything
            // or destroy either still-live member.
            stage = 0;
            expect_heap_release = 0;
            dialog.close();
            assert(stage == 1 && !allocated && frees == pass + 1);
            assert(clears == 2 * pass + 2 && list_destructors == 0);
            assert(!dialog.item_list_.current_position_);
            assert(dialog.item_list_.current_ == &dialog);
            assert(dialog.item_list_.entry_count_ == ((pass & 1) ? 3 : 0));
            check_defaults(dialog);
        }
    }
    assert(list_destructors == 1 && frees == 4);
    return 0;
}

// Actual Dialog declaration/close and Heap bodies; the list is a call-order
// double. Real entry/hook/allocator lifetimes have separate StringList tests.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <cstdlib>
#include <string.h>
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
void tracked_free(void *p) { tracked_release(p); }
namespace std { void tracked_free(void *p) { tracked_release(p); } }
// LIVE_HEAP_CONSTRUCTOR
#define free tracked_free
// LIVE_HEAP_DESTRUCTOR
// LIVE_HEAP_RESET
#undef free

struct StringList {
    void *head_, *current_;
    int entry_count_, current_position_;
    // Preserve the pinned 0x30-byte member extent while replacing its actual
    // list-node machinery with this harness's call-order double.
    unsigned char padding_[0x20];
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
// LIVE_DIALOG_DEFAULTS_HEADER
uint32_t DialogDefaults[37];
Font *DialogDefaultFonts[3];
uint32_t DialogField30Default = 301, DialogField34Default = 341;
uint32_t DialogField40Default = 401, DialogFieldACDefault = 1721;
uint32_t DialogFieldB0Default = 1761, DialogFieldB4Default = 1801;
uint32_t DialogFieldB8Default = 1841;
Dialog::Dialog() {}
Dialog::~Dialog() {}
// LIVE_DIALOG_CLOSE

// This harness checks private state as an ABI fixture. These offsets are
// independently pinned for 32-bit Dialog/Heap objects; memcpy accesses the
// object representation without declaring test-only production accessors.
template <typename T>
void store_abi_bytes(void *object, size_t offset, const T &value) {
    memcpy(static_cast<unsigned char *>(object) + offset,
           &value, sizeof(value));
}

template <typename T>
T load_abi_bytes(const void *object, size_t offset) {
    T value;
    memcpy(&value, static_cast<const unsigned char *>(object) + offset,
           sizeof(value));
    return value;
}

Heap *embedded_heap(Dialog &dialog) {
    // Dialog's real Heap subobject begins at +4 after its 32-bit vfptr.
    unsigned char *bytes = static_cast<unsigned char *>(static_cast<void *>(&dialog));
    return static_cast<Heap *>(static_cast<void *>(bytes + 4));
}

class DialogHarness : public Dialog {
 public:
    uint32_t color_value(unsigned int index) const {
        switch (index) {
        case 0: return text_color_a_;
        case 1: return text_color_2a_;
        case 2: return text_color_3a_;
        case 3: return text_color_b_;
        case 4: return text_color_2b_;
        case 5: return text_color_3b_;
        case 6: return text_color_c_;
        case 7: return text_color_2c_;
        case 8: return text_color_3c_;
        case 9: return text_color_d_;
        case 10: return text_color_2d_;
        default: return text_color_3d_;
        }
    }
    uint32_t field_ac() const { return field_AC_; }
    uint32_t field_b0() const { return field_B0_; }
    uint32_t field_b8() const { return field_B8_; }
};

void check_defaults(DialogHarness &dialog) {
    assert(load_abi_bytes<Heap *>(&dialog, 0x18) == 0);
    assert(!dialog.selected_position_);
    assert(load_abi_bytes<uint32_t>(&dialog, 0x1C) == ~0u && dialog.field_F0_ == ~0u);
    assert(dialog.field_48_ == ~0u);
    assert(dialog.field_20_ == dialog_default_style());
    assert(dialog.field_2C_ == DialogDefaults[19]);
    assert(dialog.field_44_ == DialogDefaults[20]);
    assert(dialog.field_4C_ == DialogDefaults[21]);
    assert(dialog.field_50_ == DialogDefaults[22]);
    assert(dialog.field_6C_ == DialogDefaults[23]);
    assert(dialog.field_30_ == DialogField30Default);
    assert(dialog.field_34_ == DialogField34Default);
    assert(dialog.field_40_ == DialogField40Default);
    assert(dialog.field_ac() == DialogFieldACDefault);
    assert(dialog.field_b0() == DialogFieldB0Default);
    assert(dialog.field_B4_ == DialogFieldB4Default);
    assert(dialog.field_b8() == DialogFieldB8Default);
    uint32_t Dialog::*zero_uint32_fields[] = {
        &Dialog::field_38_, &Dialog::field_3C_,
        &Dialog::field_5C_, &Dialog::field_60_};
    for (int u = 0; u < 4; ++u)
        assert(dialog.*zero_uint32_fields[u] == 0);
    int Dialog::*zero_int_fields[] = {
        &Dialog::window_x_, &Dialog::window_y_,
        &Dialog::column_count_, &Dialog::row_count_,
        &Dialog::field_64_, &Dialog::field_68_};
    for (int i = 0; i < 6; ++i)
        assert(dialog.*zero_int_fields[i] == 0);
    for (int color = 0; color < 12; ++color)
        assert(dialog.color_value(color) == DialogDefaults[24 + color]);
    assert(dialog.font1_ == DialogDefaultFonts[0]);
    assert(dialog.font2_ == DialogDefaultFonts[1]);
    assert(dialog.font3_ == DialogDefaultFonts[2]);
}

int main() {
    Font fonts[3];
    for (int f = 0; f < 3; ++f) DialogDefaultFonts[f] = &fonts[f];
    {
        assert(sizeof(void *) == 4);
        assert(sizeof(Heap) == 0x14);
        assert(sizeof(StringList) == 0x30);
        assert(sizeof(Dialog) == 0xF4);
        DialogHarness dialog;
        // Reuse the same live heap/list members after both nonempty and
        // empty-head resets, with changed defaults on every pass.
        for (int pass = 0; pass < 4; ++pass) {
            for (int i = 0; i < 37; ++i) DialogDefaults[i] = 1000 * pass + i;
            assert(&dialog_default_style() == &DialogDefaults[36]);
            stage = 0;
            expect_heap_release = 1;
            allocated = malloc(16);
            assert(allocated);
            Heap *const heap = embedded_heap(dialog);
            store_abi_bytes<LPVOID>(heap, 0x04, allocated);
            heap->current_ = allocated;
            store_abi_bytes<size_t>(heap, 0x0C, 16);
            heap->free_size_ = 16;
            store_abi_bytes<int8_t>(heap, 0x00, 7);
            store_abi_bytes<Heap *>(&dialog, 0x18, heap);
            dialog.item_list_.head_ = (pass & 1) ? 0 : &dialog;
            dialog.item_list_.current_ = &dialog;
            dialog.item_list_.entry_count_ = 3;
            dialog.item_list_.current_position_ = 19;
            dialog.close();
            assert(stage == 2 && !allocated && frees == pass + 1);
            assert(clears == 2 * pass + 1 && list_destructors == 0);
            assert(!heap->get_base() && !heap->current_);
            assert(!heap->get_base_size() && !heap->free_size_);
            assert(load_abi_bytes<int8_t>(heap, 0x00) == 0);
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

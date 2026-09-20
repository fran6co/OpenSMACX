// Actual BasePop text-block declaration and close PREFIX are inserted below.
// Later cleanup/dispatch is deliberately excluded; this is not full close or
// game acceptance. The prefix's two other stores retain their original offsets.
#include <assert.h>
#include <stddef.h>
#include <string.h>
typedef unsigned int uint32_t;

// LIVE_TEXT_BLOCK

struct BasePop {
    unsigned char before[0x2E64];
    BasePopTextBlock text_block_a_;
    BasePopTextBlock text_block_b_;
    void (__cdecl *field_3094_)(BasePopTextBlock *, int);
    unsigned char gap[12];
    uint32_t field_30A4;
    unsigned char after[32];
    void close();
};

// LIVE_DEFAULT_BLOCKS
uint32_t BasePopDefaultField30A4 = 0x12345678;
void __cdecl config_callback(BasePopTextBlock *, int) {}
void (__cdecl *BasePopDefaultField3094)(BasePopTextBlock *, int) = config_callback;

// LIVE_COPY_PREFIX
} // End only the extracted prefix, not a replacement product function.

int main() {
    assert(sizeof(BasePopTextBlock) == 280);
    assert(offsetof(BasePop, text_block_a_) == 0x2E64);
    assert(offsetof(BasePop, text_block_b_) == 0x2F7C);
    assert(offsetof(BasePop, field_3094_) == 0x3094);
    assert(offsetof(BasePop, field_30A4) == 0x30A4);
    for (int epoch = 0; epoch != 3; ++epoch) {
        for (int index = 0; index != 70; ++index) {
            BasePopDefaultBlock2E64[index] = 0x10203040U + index * 0x10203U + epoch;
            BasePopDefaultBlock2F7C[index] = 0xFEDCBA98U - index * 0x30405U - epoch;
        }
        BasePop value;
        memset(&value, 0xA5, sizeof(value));
        value.close();
        assert(memcmp(&value.text_block_a_, BasePopDefaultBlock2E64, 280) == 0);
        assert(memcmp(&value.text_block_b_, BasePopDefaultBlock2F7C, 280) == 0);
        assert(value.field_3094_ == BasePopDefaultField3094);
        assert(value.field_30A4 == BasePopDefaultField30A4);
        for (int n = 0; n != sizeof(value.before); ++n) assert(value.before[n] == 0xA5);
        for (int g = 0; g != sizeof(value.gap); ++g) assert(value.gap[g] == 0xA5);
        for (int a = 0; a != sizeof(value.after); ++a) assert(value.after[a] == 0xA5);
    }
    return 0;
}

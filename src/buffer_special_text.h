#pragma once
#include <new>
// PRIVATE real source-owned declaration, supported by original constructor/
// destructor callbacks5E35D0/35F0 and vector stride30/count10 at5D7170/71A0.
struct BufferSpecialText {
    BufferSpecialText();
    ~BufferSpecialText();
    RECT bounds[2]; // +0/+10, actual writes5DC172..183, not Sprite pixels
    uint32_t field_20_; // original constructor/destructor explicitly zero
    uint32_t field_24_; // extent known, behavioral meaning not established
    uint32_t field_28_; // extent known, behavioral meaning not established
    uint32_t color_;    // +2C, low16 consumed at5DC115
};

// One raw-storage slot per original 0x30-byte element. The active bytes
// provide aligned storage for one explicitly placement-constructed object;
// the trivial union itself never destroys that object implicitly.
union BufferSpecialTextStorage {
    char bytes[0x30];
    uint32_t alignment_[12];
};
static_assert(sizeof(BufferSpecialTextStorage) == 0x30,
              "special-text element storage stride");

// The original guard byte is a separate source-owned scalar at 0x9B37CC.
// Keep it separate from BufferGouraudLight/Pitch at 0x9B37D0/D4; those are
// real neighbouring globals, not padding for a larger owner.
extern uint8_t BufferSpecialTextsCleanupGuard;

// The owner starts exactly where the ten original 0x30-byte records start.
// Raw per-element slots keep the compiler from unconditionally destructing
// records after an early-returning/reentrant owner destructor.
struct BufferSpecialTextArrayOwner {
    BufferSpecialTextStorage elements_[10];

    BufferSpecialTextArrayOwner() {
        int constructed = 0;
        try {
            for (; constructed < 10; ++constructed) {
                new (&elements_[constructed].bytes[0]) BufferSpecialText();
            }
        } catch (...) {
            while (constructed != 0) {
                at(--constructed).~BufferSpecialText();
            }
            throw;
        }
    }

    ~BufferSpecialTextArrayOwner() {
        if ((BufferSpecialTextsCleanupGuard & 1) != 0) return;
        BufferSpecialTextsCleanupGuard |= 1;
        for (int index = 10; index != 0; --index) {
            at(index - 1).~BufferSpecialText();
        }
    }

    BufferSpecialText &at(int index) {
        return *reinterpret_cast<BufferSpecialText *>(elements_[index].bytes);
    }
    const BufferSpecialText &at(int index) const {
        return *reinterpret_cast<const BufferSpecialText *>(elements_[index].bytes);
    }
};
static_assert(sizeof(BufferSpecialTextArrayOwner) == 0x1E0,
              "native special-text array extent");
extern BufferSpecialTextArrayOwner BufferSpecialTextsOwner;
// Existing field-accessor ORIGINAL bindings5E35D0/35F0 need genuine rehome to
// ctor/dtor with source writer preserving all spans/claims/provenance. Existing
// vector init/destruction5D7170/71A0 mappings must be compiler generated FILE,
// not fabricated explicit vtables or fresh behavior credit.

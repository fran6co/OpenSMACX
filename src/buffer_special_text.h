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
static_assert(sizeof(BufferSpecialText) == 0x30,
              "special-text element stride");

// A real contiguous array subobject gives the compiler ownership of element
// construction, partial-construction unwind and reverse destruction.
struct BufferSpecialTextArray {
    BufferSpecialText elements_[10];
};
static_assert(sizeof(BufferSpecialTextArray) == 0x1E0,
              "ten special-text elements retain their original extent");

// One trivial, four-byte-aligned storage window for the original ten records.
// The owner explicitly starts/ends a BufferSpecialTextArray lifetime here so
// its separate once-guard can skip duplicate/reentrant teardown.
union BufferSpecialTextArrayStorage {
    char bytes[0x1E0];
    uint32_t alignment_[0x78];
};
static_assert(sizeof(BufferSpecialTextArrayStorage) == 0x1E0,
              "special-text array storage extent");

// The original guard byte is a separate source-owned scalar at 0x9B37CC.
// Keep it separate from BufferGouraudLight/Pitch at 0x9B37D0/D4; those are
// real neighbouring globals, not padding for a larger owner.
extern uint8_t BufferSpecialTextsCleanupGuard;

// The owner starts exactly where the ten original 0x30-byte records start.
// Its single trivial storage member lets the guard conditionally enter the
// natural array destructor without an unconditional member teardown.
struct BufferSpecialTextArrayOwner {
    BufferSpecialTextArrayStorage elements_;

    BufferSpecialTextArrayOwner() {
        // No parentheses: preserve the three originally untouched suffix
        // fields in each element while the compiler constructs the array.
        new (&elements_.bytes[0]) BufferSpecialTextArray;
    }

    ~BufferSpecialTextArrayOwner() {
        if ((BufferSpecialTextsCleanupGuard & 1) != 0) return;
        BufferSpecialTextsCleanupGuard |= 1;
        array().~BufferSpecialTextArray();
    }

    BufferSpecialText &at(int index) {
        return array().elements_[index];
    }
    const BufferSpecialText &at(int index) const {
        return array().elements_[index];
    }

private:
    BufferSpecialTextArray &array() {
        return *reinterpret_cast<BufferSpecialTextArray *>(&elements_.bytes[0]);
    }
    const BufferSpecialTextArray &array() const {
        return *reinterpret_cast<const BufferSpecialTextArray *>(&elements_.bytes[0]);
    }
};
static_assert(sizeof(BufferSpecialTextArrayOwner) == 0x1E0,
              "native special-text array extent");
extern BufferSpecialTextArrayOwner BufferSpecialTextsOwner;
// Existing constructor/destructor identities5E35D0/35F0 are implemented by
// BufferSpecialText's genuine owner methods in buffer.cpp. Array lifetime
// helper selection remains a compiler consequence, not a handwritten adapter.

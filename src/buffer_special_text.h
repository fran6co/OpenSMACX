#pragma once
// Element declaration supported by original constructor/destructor
// callbacks5E35D0/35F0 and vector stride30/count10 at5D7170/71A0.
// Buffer owns the current static ten-element array declaration in buffer.h;
// that source owner is not a claim about the original declaring class.
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
// The existing 5E35D0/35F0 constructor/destructor identities are implemented
// by BufferSpecialText's methods in buffer.cpp. The ten-element array lifetime
// is emitted by the compiler for Buffer's static array, not an adapter.

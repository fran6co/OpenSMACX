#pragma once
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
extern BufferSpecialText BufferSpecialTexts[10]; // original9B37E0..9B39C0
// Existing field-accessor ORIGINAL bindings5E35D0/35F0 need genuine rehome to
// ctor/dtor with source writer preserving all spans/claims/provenance. Existing
// vector init/destruction5D7170/71A0 mappings must be compiler generated FILE,
// not fabricated explicit vtables or fresh behavior credit.

// ORIGINAL: 0x0048FF60 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0048FF60
// name           ?on_group_clicked@PrefWin@@QAEXHHH@Z
// size           65 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048ff60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_group_clicked@PrefWin@@QAEXHHH@Z  at 0x0048FF60  (65 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

class PrefWin { public:
    void on_group_clicked(int, int, int);
};

// A shim used only to reach the vtable slot the body dispatches through
// (0xB0) via ordinary virtual-call codegen instead of a free `__thiscall`
// function pointer, which VC6 rejects outright (C4234). Never instantiated,
// never defined - only the LAYOUT (slot order) matters, so filler slots
// 0-43 exist purely to hold the real one at the right byte offset
// (0xB0 = slot 44 * 4).
class VCall { public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void slot26();
    virtual void slot27();
    virtual void slot28();
    virtual void slot29();
    virtual void slot30();
    virtual void slot31();
    virtual void slot32();
    virtual void slot33();
    virtual void slot34();
    virtual void slot35();
    virtual void slot36();
    virtual void slot37();
    virtual void slot38();
    virtual void slot39();
    virtual void slot40();
    virtual void slot41();
    virtual void slot42();
    virtual void slot43();
    virtual void slot44(int);
};

void PrefWin::on_group_clicked(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0xa1c) == 0) {
        if (a2 <= 5 && a2 >= 0) {
            *reinterpret_cast<int *>(self + 0xa18) = a2;
        }
    } else {
        *reinterpret_cast<int *>(self + 0xa1c) = 0;
    }

    *reinterpret_cast<int *>(self + 0xa14) = a3;
    reinterpret_cast<VCall *>(this)->slot44(a3);
}

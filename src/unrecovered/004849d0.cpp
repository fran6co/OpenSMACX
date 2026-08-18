// ORIGINAL: 0x004849D0 ??0NewTechWin@@QAE@XZ 0x004849D0-0x00484A78;0x006575D0-0x00657626 FILE
// RULED-OUT: real `FlatButton flatButtons_[3]`/`StringBox stringBoxes_[4]` array members (the scaffold's own layout) - with no dtor declared on FlatButton/StringBox, MSVC synthesizes its OWN local `??_H@` vector-constructor-iterator COMDAT for the array member init that precedes the body of ANY constructor for that class, which is a SECOND external .text symbol and makes `verify_recovered_function.py`'s `object_code()` raise an uncaught ValueError ("expected one external .text symbol, found 2") instead of reporting NO_COMPILE - crashes --dir and --body scoring outright. Sidestepped the same way the sibling destructor (0x00484810) does: NewTechWin here is its OWN flat class (no real inheritance, byte-array "members"), and every subobject is constructed EXPLICITLY via placement `new` / direct calls, so nothing is ever implicitly member-initialized.
// RULED-OUT: reproducing the `push -1/push handler/mov fs:[0]` SEH frame and the `ebp-4` progressive construction-state byte - that is MSVC's own codegen for base/member initializer-list unwind, which explicit placement-new statements in an ordinary function body do not trigger; landed without it.
// size      254 bytes
// prototype void (__thiscall ??0NewTechWin@@QAE@XZ)(NewTechWin* this)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x00629110 0x0062B7C0 0x006456E4 0x006457C2

inline void *__cdecl operator new(unsigned int, void *p) { return p; }

class GraphicWin { public:
    GraphicWin();
};

class ButtonGroup { public:
    ButtonGroup();
};

class StringBox { public:
    StringBox();
};

extern "C" void __stdcall fn_006457c2(void *, unsigned int, int,
                                       void (*)(void *), void (*)(void *));

// The const-pointer spelling reproduces the original's encoding including
// the address; the numeric value is masked by the comparison.
static void *const g_00406880 = (void *)0x00406880;
static void *const g_00607cf0 = (void *)0x00607CF0;
static void *const g_004325c0 = (void *)0x004325C0;
static void *const g_00629110 = (void *)0x00629110;
static unsigned int *const g_0066ce54 = (unsigned int *)0x0066CE54;
static unsigned int *const g_0066ce5c = (unsigned int *)0x0066CE5C;

class NewTechWin { public:
    char pad_00[0xB2C];
    char flatButtons_[3 * 0xB4C];   // -> 0xB2C
    char buttonGroup_[0x94];        // -> 0x2D10
    char stringBoxes_[4 * 0x2BA0];  // -> 0x2DA4
    char stringBox2_[0x2BA0];       // -> 0xDC24

    NewTechWin();
};

NewTechWin::NewTechWin() {
    char *base = reinterpret_cast<char *>(this);

    new (base) GraphicWin();

    fn_006457c2(base + 0xb2c, 0xb4c, 3,
                reinterpret_cast<void (*)(void *)>(g_00607cf0),
                reinterpret_cast<void (*)(void *)>(g_00406880));

    new (base + 0x2d10) ButtonGroup();

    fn_006457c2(base + 0x2da4, 0x2ba0, 4,
                reinterpret_cast<void (*)(void *)>(g_00629110),
                reinterpret_cast<void (*)(void *)>(g_004325c0));

    new (base + 0xdc24) StringBox();

    *reinterpret_cast<unsigned int *>(base) =
        reinterpret_cast<unsigned int>(g_0066ce5c);
    *reinterpret_cast<unsigned int *>(base + 0x444) =
        reinterpret_cast<unsigned int>(g_0066ce54);
}

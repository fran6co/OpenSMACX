// ORIGINAL: 0x0043D120 ?on_status@DesignWin@@QAEXH@Z 0x0043D120-0x0043D215 FILE BYTE_EXACT
// size      245 bytes
// prototype void (__thiscall ?on_status@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   ?   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D440 0x0045D970 0x004C6920 0x005D56B0 0x005ED9D0 0x005EDCD0
//           0x005FCBB0
// indirect  0x0043D142 0x0043D156 0x0043D175 0x0043D186 0x0043D18B 0x0043D19A
//           0x0043D1E5
// BYTE_EXACT under `/c /O2 /Gy /GR- /Oy- /GX`. Three source-form defects, in
// the order they were worth the most:
//
//  1. THE BASE POINTER MUST NOT BE HOISTED ABOVE THE `if`. Declaring
//     `base = this - 0xa14` at function top gives it a value live down both
//     arms, so VC6 pins it in a callee-saved register (a fourth one, `ebp`,
//     under `/Oy`) and the frame pointer disappears. Declared at FIRST USE
//     inside each arm it is recomputed per arm - `lea ecx, [edi - 0xa14]` in
//     the true arm, `add edi, 0xfffff5ec` in the false one - which is what
//     the original does, and `this` then stays in `edi` throughout.
//     Hoisted: MISMATCH #2, similarity 0.956. Per-arm: BYTE_EXACT.
//  2. The `0x40` arms were the wrong way round: `test al, 0x40; je` puts
//     `slot1(0)` on the FALL-THROUGH, so the flag being SET selects
//     `slot1(0)` and clear selects `slot2()`.
//  3. `(*(VCall **)record)->slot1(4)` dereferenced one level too many. The
//     original's `mov edx, [esi]; mov ecx, esi` says `record` IS the
//     receiver, not a pointer to it.
//
// The original allocates NO locals - no `sub esp`, no `push ecx`, and the
// epilogue is a bare `pop ebp` - yet it spills `base` to `[ebp+8]`. That is
// the dead parameter's own home slot being reused, which VC6 does by itself:
// spelling it in the source (assigning to the parameter) is also BYTE_EXACT
// but buys nothing.
//
// Also measured, none of them a match: locals-at-top with every receiver
// declared at the top (MISMATCH #2, sim 0.847 framed); the logic corrected
// but `base` still hoisted (MISMATCH #2, sim 0.956); `/O1` under either
// `/Oy` setting (MISMATCH #2, never above sim 0.842). `real-object-local`
// was not tried: the original prologue registers no SEH frame, so there is
// no unwind funclet for a local object to produce.
//
// The lesson for the next reader of a `push`/`mov` divergence: the brief's
// gloss for that pair says to MAKE A VALUE LIVE. Here the fix was the exact
// reverse - a value that was live across the branch had to stop being live,
// because the fourth callee-saved register it occupied WAS the frame
// pointer. `push`/`mov` at instruction #0 means "the original has a frame
// pointer and you do not", and one register too many is one way to lose it.

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.
//
// WHAT `this` IS HERE, and it is not the DesignWin. `src/designwin.h` says
// `class DesignWin : GraphicWin, SubInterface` with GraphicWin occupying
// [0, 0xA14) and DesignWin's own fields starting at 0xA14 - which is exactly
// where the SECOND base sits. ECX in this body is that second-base pointer:
// `this - 0xa14` is handed to slide_show/slide_hide (`GraphicWin *`) and to
// Win::show/Win::hide, so it is the object start, and every offset below is
// relative to DesignWin+0xA14 rather than to DesignWin+0.
//
// Which identifies all three field offsets against the header:
//
//     this + 0x2d8   ->  DesignWin+0xCEC   = `FlatButton flatButtons_[25]`
//     this + 0x1970  ->  DesignWin+0x2384  = flatButtons_[2]   (0xCEC + 2*0xB4C)
//     this + 0x24bc  ->  DesignWin+0x2ED0  = flatButtons_[3]   (0xCEC + 3*0xB4C)
//
// The loop's `add esi, 0xb4c` over exactly 0x19 iterations is that array's
// stride and extent, so sizeof(FlatButton) == 0xB4C. Slot +4 takes an int and
// slot +8 is nullary: the loop calls +4 with 4 to show the window's buttons
// and +8 to hide them, then buttons 2 and 3 are revisited - shown with 0 when
// bit 0x40 of the byte at 0x9A64C0 is set, hidden when it is clear.

class VCall {
public:
    virtual void slot0();
    virtual void slot1(int);
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
};

class GraphicWin {
public:
    void update(GraphicWin *);
};

class Win {
public:
    void show(int);
    void hide();
};

class Wave {
public:
    int play();
};

void __cdecl do_all_draws();
void __cdecl slide_show(GraphicWin *, int);
void __cdecl slide_hide(GraphicWin *, int);

extern unsigned char g_9a64c0;

static GraphicWin *const g_main_interface_gw = (GraphicWin *)0x7ae820;
static Wave *const g_wave = (Wave *)0x749c88;

class DesignWin {
public:
    void on_status(int visible);
};

void DesignWin::on_status(int visible) {
    char *self = reinterpret_cast<char *>(this);
    if (visible != 0) {
        // `base` MUST be declared here and again in the other arm rather than
        // once above the `if` - see the note at the top of the file.
        GraphicWin *base = reinterpret_cast<GraphicWin *>(self - 0xa14);
        reinterpret_cast<VCall *>(base)->slot12();
        char *button = self + 0x2d8;                 // flatButtons_[0]
        for (int i = 0; i < 0x19; ++i) {
            reinterpret_cast<VCall *>(button)->slot1(4);
            button += 0xb4c;
        }
        if (g_9a64c0 & 0x40) {
            reinterpret_cast<VCall *>(self + 0x1970)->slot1(0);   // [2]
            reinterpret_cast<VCall *>(self + 0x24bc)->slot1(0);   // [3]
        } else {
            reinterpret_cast<VCall *>(self + 0x1970)->slot2();
            reinterpret_cast<VCall *>(self + 0x24bc)->slot2();
        }
        g_main_interface_gw->update(0);
        do_all_draws();
        g_wave->play();
        slide_show(base, 0);
        reinterpret_cast<Win *>(base)->show(3);
        return;
    }
    char *button = self + 0x2d8;
    for (int i = 0; i < 0x19; ++i) {
        reinterpret_cast<VCall *>(button)->slot2();
        button += 0xb4c;
    }
    GraphicWin *base = reinterpret_cast<GraphicWin *>(self - 0xa14);
    slide_hide(base, 0);
    reinterpret_cast<Win *>(base)->hide();
    do_all_draws();
}

// ORIGINAL: 0x0043D120 FILE
// name      ?on_status@DesignWin@@QAEXH@Z
// size      245 bytes
// spans     0x0043D120-0x0043D215
// prototype void (__thiscall ?on_status@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   ?   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0045D440 0x0045D970 0x004C6920 0x005D56B0 0x005ED9D0 0x005EDCD0
//           0x005FCBB0
// indirect  0x0043D142 0x0043D156 0x0043D175 0x0043D186 0x0043D18B 0x0043D19A
//           0x0043D1E5
// RULED-OUT: nothing tried beyond the direct transcription below - landed
//            at first compiling candidate given batch size. Mnemonic
//            similarity 0.88, first divergence is prologue register-save
//            scheduling (no `ebp` frame in the rebuilt body; the original
//            keeps one).

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.
//
// `this - 0xa14` recurs three times (0x0043D133, 0x0043D1F1, and as the
// receiver for slide_show/slide_hide and Win::show/hide) - a base
// subobject at that fixed adjustor, reused as a GraphicWin*/Win*.

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
    void on_status(int a1);
};

void DesignWin::on_status(int a1) {
    char *self = reinterpret_cast<char *>(this);
    GraphicWin *base = reinterpret_cast<GraphicWin *>(self - 0xa14);
    if (a1 != 0) {
        reinterpret_cast<VCall *>(base)->slot12();
        char *record = self + 0x2d8;
        for (int i = 0; i < 0x19; ++i) {
            (*reinterpret_cast<VCall **>(record))->slot1(4);
            record += 0xb4c;
        }
        VCall *sub1 = reinterpret_cast<VCall *>(self + 0x1970);
        VCall *sub2 = reinterpret_cast<VCall *>(self + 0x24bc);
        if (g_9a64c0 & 0x40) {
            sub1->slot2();
            sub2->slot2();
        } else {
            sub1->slot1(0);
            sub2->slot1(0);
        }
        g_main_interface_gw->update(0);
        do_all_draws();
        g_wave->play();
        slide_show(base, 0);
        reinterpret_cast<Win *>(base)->show(3);
        return;
    }
    char *record = self + 0x2d8;
    for (int i = 0; i < 0x19; ++i) {
        (*reinterpret_cast<VCall **>(record))->slot2();
        record += 0xb4c;
    }
    slide_hide(base, 0);
    reinterpret_cast<Win *>(base)->hide();
    do_all_draws();
}

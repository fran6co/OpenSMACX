// ORIGINAL: 0x005ED880 ?resize@Win@@QAEHHHH@Z 0x005ED880-0x005ED9B5 FILE
// TRIED: three raw vtable calls (offsets 0xc/0x30/0x108, i.e. slots 3/12/66) - `__thiscall` function-pointer typedefs are rejected by VC6 (C4234), so each slot needs its own dummy-padded virtual class reinterpret_cast, one class per distinct slot index and signature (0xc takes 3 ints; 0x30 and 0x108 are nullary). That forces FILE mode with its own minimal Win: the scaffold's Win/VCall are already fully defined, and these extra classes' own braces would end body-mode's brace-counted extraction before reaching the function. Landed at MISMATCH #0 (best-scoring flag set is `/Oy-`, framed; a frameless manual build tracks the first ~15 instructions exactly).
// size      309 bytes
// prototype int (__thiscall ?resize@Win@@QAEHHHH@Z)(Win* this, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EE330 0x005EEE70 0x005EF110
// indirect  0x005ED965 0x005ED983 0x005ED9AB

struct RECT {
    int left;
    int top;
    int right;
    int bottom;
};

class Win {
public:
    char pad_0000_[0x9c];
    unsigned int iSomeFlag_;
    char pad_0a0_[0xa8 - 0xa0];
    unsigned int poWinBase_;
    char pad_0ac_[0x13c - 0xac];
    RECT outer_rect_;
    RECT client_rect_;

    void client_to_nonclient(int *, int *);
    void nonclient_to_client(RECT *);
    int resize_event(int, int);
    int resize(int, int, int);
};

class VCall3 {
public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003(int, int, int);
};

class VCall12 {
public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
};

class VCall66 {
public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
};

int Win::resize(int a2, int a3, int a4) {
    if (a4 != 0) {
        if ((iSomeFlag_ & 2) == 0) {
            int left = outer_rect_.left;
            int top = outer_rect_.top;
            outer_rect_.left = left;
            outer_rect_.right = left + a2;
            outer_rect_.top = top;
            outer_rect_.bottom = top + a3;
            goto shared;
        }
        client_to_nonclient(&a2, &a3);
    }
    {
        int cleft = client_rect_.left;
        int ctop = client_rect_.top;
        client_rect_.left = cleft;
        client_rect_.top = ctop;
        client_rect_.right = cleft + a2;
        client_rect_.bottom = ctop + a3;
        outer_rect_.left = client_rect_.left;
        outer_rect_.top = client_rect_.top;
        outer_rect_.right = client_rect_.right;
        outer_rect_.bottom = client_rect_.bottom;
        int negLeft = outer_rect_.left;
        outer_rect_.left -= negLeft;
        outer_rect_.right -= negLeft;
        outer_rect_.bottom -= outer_rect_.top;
        outer_rect_.top -= outer_rect_.top;
        nonclient_to_client(&outer_rect_);
    }
shared:
    reinterpret_cast<VCall66 *>(this)->slot066();
    if (reinterpret_cast<Win *>(poWinBase_) != this) {
        reinterpret_cast<VCall3 *>(poWinBase_)->slot003(a2, a3, 0);
    }
    resize_event(outer_rect_.right - outer_rect_.left, outer_rect_.bottom - outer_rect_.top);
    reinterpret_cast<VCall12 *>(this)->slot012();
    return 0;
}

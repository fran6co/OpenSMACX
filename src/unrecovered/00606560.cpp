// ORIGINAL: 0x00606560 ?on_key_click@Scroll@@QAEHHH@Z 0x00606560-0x006066EC
// TRIED: transcription (guard, Home/End vs PageUp/PageDown, vtable slot 48/23/7/62 dispatch, cdecl callback at field_2144_) diverges at #1 - the prologue's register-save set differs, structurally the same shape as the tile()/tile_mask() family in this batch. Tried caching win_parent_ in a local and reordering the branch tests; neither moved the divergence.
// size      396 bytes
// prototype int (__thiscall ?on_key_click@Scroll@@QAEHHH@Z)(Scroll* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00606645 0x0060667B 0x006066A9 0x006066C2 0x006066CD 0x006066DE

// Vtable shim - slot signatures set to match this body's calls.
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual int slot007();
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
    virtual int slot023();
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
    virtual int slot048(int, int);
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
    virtual int slot062();
};

typedef int (__cdecl *KeyCallbackFn)(int, int);

int Scroll::on_key_click(int a1, int a2) {
    if (win_parent_ == 0) {
        return 0;
    }

    *g_009b8ddc = 1;

    int width = GraphicWin::buffer_.width_;
    int height = -static_cast<int>(GraphicWin::buffer_.height_);

    if (width > height) {
        if (a2 == 0x23) {
            position_ = range_maximum_;
        } else if (a2 == 0x24) {
            position_ = range_minimum_;
        } else {
            return 0;
        }
    } else if (a2 == 0x21) {
        int newPos = position_ - paging_;
        position_ = newPos;
        if (newPos < range_minimum_) {
            position_ = range_minimum_;
        }
    } else if (a2 == 0x22) {
        int newPos = position_ + paging_;
        position_ = newPos;
        if (newPos > range_maximum_) {
            position_ = range_maximum_;
        }
    } else {
        return 0;
    }

    *g_009b7ab8 = reinterpret_cast<int>(win_parent_);

    int computed;
    if (range_reversed_ == 0) {
        computed = position_;
    } else {
        computed = (range_maximum_ - position_) + range_minimum_;
    }
    reinterpret_cast<VCall *>(win_parent_)->slot048(field_A18_, computed);

    if (field_2144_ != 0) {
        (reinterpret_cast<KeyCallbackFn>(field_2144_))(field_A18_, computed);
    }

    void *sub = *reinterpret_cast<void **>(reinterpret_cast<char *>(win_parent_) + 0x80);
    if (sub != 0) {
        int result = reinterpret_cast<VCall *>(sub)->slot023();
        if (result == 0) {
            reinterpret_cast<VCall *>(sub)->slot007();
        }
    }

    field_A3C_ = 0xffffffff;
    reinterpret_cast<VCall *>(this)->slot062();

    return 1;
}

// ORIGINAL: 0x00614FE0 ?init@EditBox@@QAEHHHHHPAUWin@@H@Z 0x00614FE0-0x00615179
// RULED-OUT: same as 0x005C9860 - the scaffold's GraphicWin::init is stale as `void` (src/graphicwin.cpp's real recovery returns `int`, and this disassembly tests the return value), so a same-layout `GraphicWinInitShim` with the correct `int` return gets the call.
// RULED-OUT: landed at 0.92 similarity, 410 vs 409 bytes; diverges at #45 where the trailing low-byte or/and-with-1 flag update compiles to a wider `not`/`and` sequence here, and VC6 inlines the `strlen` call as `repne scasb` where the original kept a real `call`. Tried an explicit `unsigned char` low-byte read/write for the flag update instead of a 32-bit read-modify-write; same divergence point.
// size      409 bytes
// prototype int (__thiscall ?init@EditBox@@QAEHHHHHPAUWin@@H@Z)(EditBox* this, int, int, int, int, Win*, int)
// callers   5   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005D4EF0 0x005DACB0 0x005EC7C0 0x00616260 0x00616780 0x006453E0
// indirect  0x00615125 0x00615143

class VCall { public:
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
};

class GraphicWinInitShim { public:
    int init(int, int, int, int, char *, int, Win *, Menu *, BorderSizing *);
};

int EditBox::init(int a1, int a2, int a3, int a4, Win * a5, int a6) {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<uint32_t *>(self + 0xb14) = 10;
    *reinterpret_cast<char *>(self + 0xa14) = 0;

    *reinterpret_cast<uint32_t *>(self + 0xb28) = *g_009b90b8;
    *reinterpret_cast<uint32_t *>(self + 0xb2c) = *g_009b90bc;
    *reinterpret_cast<uint32_t *>(self + 0xb30) = *g_006971e0;
    *reinterpret_cast<uint32_t *>(self + 0xb34) = *g_006971e4;
    *reinterpret_cast<uint32_t *>(self + 0xb1c) = *g_006971dc;
    *reinterpret_cast<uint32_t *>(self + 0xb20) = *g_006971f8;
    *reinterpret_cast<uint32_t *>(self + 0xb24) = *g_006971f4;
    *reinterpret_cast<uint32_t *>(self + 0xb38) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xb40) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xb44) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xb48) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xb18) = 0;
    *reinterpret_cast<uint32_t *>(self + 0xb3c) = 0;

    Time *timer = reinterpret_cast<Time *>(self + 0xb4c);
    timer->close();

    GraphicWin *gw = reinterpret_cast<GraphicWin *>(this);
    gw->close();

    if (a5 == 0) {
        return 3;
    }

    int styleFlags = ((a5->iFlags_ & 0x1000) != 0) ? 0x2020 : 0x102020;

    int result = reinterpret_cast<GraphicWinInitShim *>(this)->init(
        a1, a2, a3, a4, 0, styleFlags, a5, 0, 0);
    if (result != 0) {
        return result;
    }

    *reinterpret_cast<uint32_t *>(self + 0x964) = 0;
    gw->buffer_.set_text_color(*g_009b90c0, *g_006971e8, *g_006971ec, *g_006971f0);

    reinterpret_cast<Win *>(this)->set_cursor(0x7f01);

    *reinterpret_cast<uint32_t *>(self + 0xb40) = 0;
    int len = strlen(self + 0xa14);
    *reinterpret_cast<uint32_t *>(self + 0xb44) = len;

    reinterpret_cast<VCall *>(this)->slot062();

    timer->init(reinterpret_cast<void (__cdecl *)(int)>(g_00615e40),
                reinterpret_cast<int>(this), 0x1f4, 5);

    reinterpret_cast<VCall *>(this)->slot062();

    uint32_t flags = *reinterpret_cast<uint32_t *>(self + 0xb3c);
    if (a6 != 0) {
        flags |= 1;
    } else {
        flags &= 0xfffffffe;
    }
    *reinterpret_cast<uint32_t *>(self + 0xb3c) = flags;
    return 0;
}

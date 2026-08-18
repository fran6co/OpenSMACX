// ORIGINAL: 0x0063BD10 sub_63bd10 0x0063BD10-0x0063BEAD
// RULED-OUT: MISMATCH #0 push/sub - unknown containing class, receiver expressed as an opaque thiscall shim (no field names); stack frame shape differs from the original's spill choices.
// size      413 bytes
// prototype
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00609F20 0x00616780
// indirect  0x0063BD50 0x0063BD63 0x0063BD7F 0x0063BDBB 0x0063BDCD 0x0063BDDF
// To start: tools/decomp_status.py --work 0x0063BD10

class VCall { public:
    virtual void slot000(int);
    virtual void slot001(void *);
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
    virtual void slot067();
    virtual void slot068();
    virtual void slot069();
    virtual void slot070();
    virtual void slot071();
    virtual void slot072();
    virtual void slot073();
    virtual void slot074();
    virtual void slot075();
    virtual void slot076();
    virtual void slot077();
    virtual void slot078();
    virtual void slot079();
    virtual void slot080();
    virtual void slot081();
    virtual void slot082();
    virtual void slot083();
    virtual void slot084();
    virtual void slot085();
    virtual void slot086();
    virtual void slot087();
    virtual void slot088();
    virtual void slot089();
    virtual void slot090();
};

class Sub63BD10Target {
 public:
  void run();
};

void Sub63BD10Target::run() {
    char *const self = reinterpret_cast<char *>(this);

    reinterpret_cast<Time *>(self + 0xa14)->close();

    int *const list = reinterpret_cast<int *>(self + 0x36a8);
    if (list[2] != 0) {
        int index = 0;
        if (0 < list[4]) {
            do {
                int *const entry = reinterpret_cast<int *>(list[2]);
                list[3] = entry[3];
                void *const payload = reinterpret_cast<void *>(entry[2]);
                reinterpret_cast<VCall *>(list)->slot001(payload);
                if (payload) {
                    uint32_t *const payload_vtable = *reinterpret_cast<uint32_t *const *>(payload);
                    void *const subobject = reinterpret_cast<char *>(payload) + payload_vtable[1];
                    reinterpret_cast<VCall *>(subobject)->slot000(1);
                }
                entry[2] = 0;
                int *const head = reinterpret_cast<int *>(list[2]);
                if (head) {
                    uint32_t *const head_vtable = *reinterpret_cast<uint32_t *const *>(head);
                    void *const subobject = reinterpret_cast<char *>(head) + head_vtable[1];
                    reinterpret_cast<VCall *>(subobject)->slot000(1);
                }
                ++index;
                list[2] = list[3];
            } while (index < list[4]);
        }
        list[2] = 0;
        list[5] = 0;
        list[4] = 0;
    }
    list[5] = 0;

    reinterpret_cast<ListBox *>(self + 0xa50)->close();
    reinterpret_cast<VCall *>(self + 0x15a4)->slot090();
    reinterpret_cast<VCall *>(self + 0x20a4)->slot090();
    reinterpret_cast<VCall *>(self + 0x2ba4)->slot090();
    reinterpret_cast<GraphicWin *>(self)->close();

    int32_t *const s = reinterpret_cast<int32_t *>(self);
    s[0x3718 / 4] = 7;
    s[0x3770 / 4] = 7;
    s[0x36a4 / 4] = 0;
    s[0x37a4 / 4] = 0;
    s[0x3798 / 4] = 0;
    s[0x379c / 4] = 0;
    s[0x3708 / 4] = 0;
    s[0x370c / 4] = 0;
    s[0x3710 / 4] = 2;
    s[0x3714 / 4] = 0;
    s[0x372c / 4] = 0xff;
    s[0x3730 / 4] = 0;
    s[0x3734 / 4] = 0;
    s[0x3738 / 4] = 2;
    s[0x373c / 4] = 0;
    s[0x3740 / 4] = -1;
    s[0x3744 / 4] = 1;
    s[0x3748 / 4] = 1;
    s[0x3774 / 4] = 0;
    s[0x3778 / 4] = 0;
    s[0x377c / 4] = 2;
    s[0x3780 / 4] = 0;
    s[0x3784 / 4] = 0xf8;
    s[0x3788 / 4] = 0;
    s[0x378c / 4] = -1;
    s[0x3790 / 4] = 1;
    s[0x3794 / 4] = 1;
}

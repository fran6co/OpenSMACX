// ORIGINAL: 0x00446B40
// name      ?ambience@@YAHH@Z
// size      406 bytes
// spans     0x00446B40-0x00446CD6
// prototype
// callers   18   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00446A00
// indirect  0x00446BA3 0x00446BB0 0x00446BBD 0x00446BCA 0x00446BD7 0x00446BE4 0x00446BF1 0x00446BFE 0x00446C0B 0x00446C18 0x00446C25 0x00446C32 0x00446C3F 0x00446C4C 0x00446C79 0x00446C9A 0x00446CBA
// RULED-OUT: guard bit 0x800, no-op-if-unchanged, in-range switch dispatch
//            on *DAT_0074daa0's vtable (14 cases mapping to slots verified
//            against the raw jump table at 0x446CD8, not Ghidra's summary,
//            which collapsed two distinct case targets), else stop the old
//            FX entry (slot 0x28), start the new one (slot 0x5c), revert on
//            failure (slot 0x20), and FX::play() the result. Landed at 0.89
//            similarity; diverges at #108 where case 0xc8 and case 0xd0 -
//            same vtable slot, genuinely two separate blocks in the
//            original (`mov eax`/`call [eax+..]` vs `mov edx`/`call
//            [edx+..]`) - compile to one shared jump target here. Tried
//            plain int* stride (`g_00749cf8 + id*27`) against explicit
//            byte-offset char* arithmetic for the FX table addressing;
//            neither changed the divergence.

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
};

int __cdecl ambience(int a1) {
    if ((*g_009a6490 & 0x800) == 0) {
        return a1;
    }

    if (*g_00683bf0 == a1) {
        return *g_00683bf0;
    }

    if (a1 >= 0xc8 && a1 < 0xd6) {
        if (*g_0074daa0 == 0) {
            return a1;
        }
        VCall *obj = reinterpret_cast<VCall *>(*g_0074daa0);
        switch (a1) {
            case 0xc8: obj->slot079(); break;
            case 0xc9: obj->slot076(); break;
            case 0xca: obj->slot080(); break;
            case 0xcb: obj->slot081(); break;
            case 0xcc: obj->slot082(); break;
            case 0xcd: obj->slot083(); break;
            case 0xce: obj->slot084(); break;
            case 0xcf: obj->slot077(); break;
            case 0xd0: obj->slot079(); break;
            case 0xd1: obj->slot085(); break;
            case 0xd2: obj->slot086(); break;
            case 0xd3: obj->slot087(); break;
            case 0xd4: obj->slot088(); break;
            case 0xd5: obj->slot089(); break;
        }
        return a1;
    }

    *g_0074c5e4 = 0xffffffff;
    int oldId = *g_00683bf0;
    if (oldId >= 0) {
        (reinterpret_cast<VCall *>(g_00749cf8 + oldId * 27))->slot010();
    }

    *g_00683bf0 = a1;

    if (a1 >= 0) {
        int result = (reinterpret_cast<VCall *>(g_00749cf8 + a1 * 27))->slot023();
        if (result != 0) {
            int curId = *g_00683bf0;
            (reinterpret_cast<VCall *>(g_00749cf8 + curId * 27))->slot008();
        }
        (reinterpret_cast<FX *>(g_00749cf8))->play(*g_00683bf0);
    }

    return *g_00683bf0;
}

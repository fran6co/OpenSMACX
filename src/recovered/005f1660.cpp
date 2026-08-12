// ORIGINAL: 0x005F1660 BYTE_EXACT
// 0x005F1660  ?OnChar@Win@@QAAXPAXDH@Z  ->  ?OnChar@Win@@QAAXPAXDH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// Slot 79 (0x13c/4) is called with two args (a1, a2); the emitter's VCall
// shim declares every slot nullary, so a second shim carries the real
// arity for the one slot used.
class VCall2 { public:
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
    virtual void slot079(void *, char);  // <-- used
};

void __cdecl Win::OnChar(void * a1, char a2, int a3) {
    int result;
    if (*g_009b7ae0 != 0) {
        result = *g_009b8d7c;
        if (result == 0) {
            result = *g_009b7ae0;
        }
    } else {
        result = *g_009b7ac4;
        if (result == 0) {
            result = *g_009b7aec;
        }
    }
    *g_009b7ac8 = result;
    if (result != 0) {
        char *p = *reinterpret_cast<char **>(result + 0xa8);
        if (p != 0 && (*reinterpret_cast<unsigned char *>(p + 0x9c) & 1) != 0) {
            Win *visobj = *reinterpret_cast<Win **>(p + 0xc4);
            if (visobj == 0 || visobj->is_visible() != 0) {
                reinterpret_cast<VCall2 *>(p)->slot079(a1, a2);
            }
        }
    }
}

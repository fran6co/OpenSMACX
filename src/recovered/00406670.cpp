// 0x00406670  sub_406670  ->  _sub_406670
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

class Obj28 {
public:
    void *release(unsigned int flag);
};

class __ShimThis {};
typedef int (__ShimThis::*Nullary)();
union NullaryCaster {
    int (__cdecl *free_fn)();
    Nullary member_fn;
};

void *Obj28::release(unsigned int flag) {
    char *outer = reinterpret_cast<char *>(this) - 0x28;
    Obj28 *self = reinterpret_cast<Obj28 *>(outer + 0x28);
    NullaryCaster caster;
    caster.free_fn = &sub_4066c0;
    (reinterpret_cast<__ShimThis *>(self)->*caster.member_fn)();
    *reinterpret_cast<void **>(self) = g_006693ac;
    int *member = *reinterpret_cast<int **>(reinterpret_cast<char *>(self) + 4);
    *g_009b3374 = reinterpret_cast<int>(member);
    if (flag & 1) {
        if (outer != 0) {
            if (member == 0) {
                free(outer);
            }
            *g_009b3374 = 0;
        }
    }
    return outer;
}

// 0x005B5A70  ?unspot_stack@@YAXH@Z  ->  ?unspot_stack@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

static short *const g_00952858 = (short *)0x00952858;
static uint8_t *const g_0095284f = (uint8_t *)0x0095284F;

void __cdecl unspot_stack(int a1) {
    if (a1 >= 0) {
        short *field_b = reinterpret_cast<short *>(g_0095285a);
        short next = field_b[a1 * 0x1a];
        while (next >= 0) {
            a1 = next;
            next = field_b[a1 * 0x1a];
        }
        for (; a1 >= 0; a1 = g_00952858[a1 * 0x1a]) {
            g_0095284f[a1 * 0x34] = 0;
        }
    }
}

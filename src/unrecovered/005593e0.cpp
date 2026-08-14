// ORIGINAL: 0x005593E0
// name      ?pick_top_veh@@YAHH@Z
// size      267 bytes
// spans     0x005593E0-0x005594EB
// prototype int (__cdecl ?pick_top_veh@@YAHH@Z)(int vehID)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: 98.2% mnemonic agreement (272 vs 267 bytes), diverges at one
//            `lea`/`mov` pair - the parent-chain climb needs an extra
//            `mov ecx,eax` to hold `top` across the array-index computation
//            that the original does not need. Tried folding `top` back into
//            the `a1` parameter itself (mutating it in place, matching the
//            register reuse the disassembly implies) - that changed the
//            PROLOGUE instead (diverges at instruction 0), worse overall.
//            Landing the closer of the two.

static short *const g_0095285a = (short *)0x0095285A;
static short *const g_00952858 = (short *)0x00952858;
static short *const g_00952832 = (short *)0x00952832;
static unsigned char *const g_00952836 = (unsigned char *)0x00952836;
static int *const g_0095282c = (int *)0x0095282C;
static unsigned char *const g_009ab88c = (unsigned char *)0x009AB88C;
static unsigned char *const g_0094a379 = (unsigned char *)0x0094A379;

int __cdecl pick_top_veh(int a1) {
    int bestScore = -99;
    int best = a1;
    if (a1 < 0) {
        return best;
    }
    int top = a1;
    short link = g_0095285a[a1 * 0x1a];
    while (link >= 0) {
        top = link;
        link = g_0095285a[top * 0x1a];
    }
    if (top < 0) {
        return best;
    }
    int owner = *g_00939284;
    int idx = top;
    do {
        if (*g_00689f30 == -1 || idx != *g_00689f30) {
            if (idx == *g_0093928c
                && g_00952836[idx * 0x34] == owner
                && (g_0095282c[idx * 0xd] & 0x00420000) == 0
                && *g_00939288 != 0
                && *g_00939294 != 0) {
                return idx;
            }
            int category = g_00952832[idx * 0x1a];
            unsigned char sub = g_009ab88c[category * 0x34];
            unsigned char looked = g_0094a379[sub * 0x90];
            int score = looked + (g_00952836[idx * 0x34] == owner ? 4 : 0);
            if (score > bestScore) {
                bestScore = score;
                best = idx;
            }
        }
        idx = g_00952858[idx * 0x1a];
    } while (idx >= 0);
    return best;
}

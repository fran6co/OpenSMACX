// ORIGINAL: 0x004EFD50 FILE
// RULED-OUT: original establishes a true EBP frame (flags: frame) because it
//            spills the reassigned `protoIDInit` parameter to its own stack
//            slot; several transliterations (ternary vs if/else for the
//            augmentation, ternary vs branch for the three clamps, B-then-A
//            vs A-then-B table read order) all landed the value in a GP
//            register (ebp used as scratch, no `mov ebp, esp`) instead.
//            Landed at MISMATCH #3 (first diff is the missing frame
//            prologue); the rest of the instruction stream already lines up
//            closely (same table indices, same clamp-via-mask shape for two
//            of three diffs).
// name      ?upgrade_cost@@YAHHHH@Z
// size      303 bytes
// spans     0x004EFD50-0x004EFE7F
// prototype int (__cdecl ?upgrade_cost@@YAHHHH@Z)(int factionID, int protoIDInit, int protoIDUpg)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)

extern unsigned char g_9ab891[];
extern unsigned char g_9ab898[];
extern unsigned char g_9ab88c[];
extern unsigned char g_9ab88d[];
extern unsigned char g_9ab88e[];
extern unsigned char g_94a378[];
extern unsigned char g_94ae6b[];
extern unsigned char g_94f282[];
extern int g_9a656c;
extern unsigned char g_97d044[];

int __cdecl upgrade_cost(int factionID, int protoIDInit, int protoIDUpg) {
    int protoA_off = protoIDInit * 0x34;
    int cost0 = g_9ab891[protoA_off];
    protoIDInit = cost0;
    if (protoIDInit > 0x3f && (g_9ab898[protoA_off] & 4) == 0) {
        int half = (cost0 + 1) / 2;
        int base = cost0 + half;
        protoIDInit = (base + 1) * base;
    }
    int protoB_off = protoIDUpg * 0x34;

    int armorA = g_94ae6b[g_9ab88d[protoA_off] * 0x10];
    int armorB = g_94ae6b[g_9ab88d[protoB_off] * 0x10];
    int armorDiff = armorA - armorB;
    if (armorDiff < 0) armorDiff = 0;

    int statA = g_94f282[g_9ab88e[protoA_off] * 0x10];
    int statB = g_94f282[g_9ab88e[protoB_off] * 0x10];
    int statDiff = statA - statB;
    if (statDiff < 0) statDiff = 0;

    int weaponA = g_94a378[g_9ab88c[protoA_off] * 0x90];
    int weaponB = g_94a378[g_9ab88c[protoB_off] * 0x90];
    int weaponDiff = weaponA - weaponB;
    if (weaponDiff < 0) weaponDiff = 0;

    int result = protoIDInit + armorDiff + statDiff + weaponDiff;

    if (g_9a656c >= 0 && g_97d044[g_9a656c * 0x134] == factionID) {
        result /= 2;
    }
    return result;
}

// ORIGINAL: 0x0046FA20 FILE
// RULED-OUT: nothing yet - MISMATCH #5 xor/push (78% mnemonic sim). `this`
//            is unused throughout (thiscall receiver never touched).
//            RULED-OUT: 0x00915B74/0x00915B78/0x00939F48 are all used as
//            index-table bases (`[reg + addr]`), so declared as
//            `extern int Table[]`/`extern Rec4 OutputTable[]` rather than
//            fixed-address globals, matching the lever note.
// name      ?UNK5@MapWin@@QAEXH@Z
// size      240 bytes
// spans     0x0046FA20-0x0046FB10
// prototype void (__thiscall ?UNK5@MapWin@@QAEXH@Z)(MapWin* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x0046FA20

struct Rec4 { int a; int b; int c; int d; };
extern int Table_915b74[];
extern int Table_915b78[];
extern Rec4 OutputTable[];  // at 0x00939F48

void MapWin::UNK5(int a1) {
    static int **const g_009156b0 = (int **)0x009156B0;
    static int *const g_008ea0b0 = (int *)0x008EA0B0;
    static int *const g_008ea0b8 = (int *)0x008EA0B8;
    static int *const g_0093f660 = (int *)0x0093F660;
    static unsigned int *const g_00939edc = (unsigned int *)0x00939EDC;

    if (a1 != 0) {
        int idx = (*g_009156b0)[1];
        int iVar1 = *(int *)((char *)Table_915b74 + idx);
        int iVar2 = *g_008ea0b0 - *(int *)((char *)Table_915b78 + idx);
        OutputTable[a1].b = 0;
        int iVar4 = (iVar1 * 3) / 4;
        OutputTable[a1].a = iVar4;
        OutputTable[a1].c = iVar1 / 4 + iVar4;
        OutputTable[a1].d = (iVar2 - *g_008ea0b8) / 2;
    } else {
        int idx;
        if (*g_0093f660 == 0) {
            idx = (*g_009156b0)[1];
            OutputTable[0].c = *(int *)((char *)Table_915b74 + idx);
        } else {
            idx = (*g_009156b0)[1];
            int t = *(int *)((char *)Table_915b74 + idx);
            OutputTable[0].c = (t * 3) / 4;
        }
        OutputTable[0].d = (*g_008ea0b0 - *(int *)((char *)Table_915b78 + idx)) - *g_008ea0b8;
        OutputTable[0].a = 0;
        OutputTable[0].b = 0;
    }
    *g_00939edc |= (1u << (a1 & 0x1f));
}

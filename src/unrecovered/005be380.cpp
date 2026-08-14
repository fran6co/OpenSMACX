// ORIGINAL: 0x005BE380
// RULED-OUT: MISMATCH #5 xor/push - the two per-tech record globals
//            (0x9c5b2c-family index arithmetic and the 0x946a50 record
//            array) used as plain static-address casts rather than
//            typed `extern` arrays; the call to the still-unrecovered
//            0x5BEB70 declared locally since it is not a catalogued
//            callee of this address.
// name      ?tech_selection@@YAHH@Z
// size      426 bytes
// spans     0x005BE380-0x005BE52A
// prototype int (__cdecl ?tech_selection@@YAHH@Z)(int factionID)
// callers   3   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x00530360 0x005932B0 0x005BEB70 0x00625EC0
// To start: tools/decomp_status.py --work 0x005BE380

extern "C" int __cdecl sub_5beb70(int, int, int, int);
extern "C" char g_96cdac_table[];
extern "C" char g_946a50_table[];

int __cdecl tech_selection(int a1) {
    if (*g_0093f660 != 0 &&
        (*reinterpret_cast<unsigned char *>(g_009a64e8) & (1 << (a1 & 0x1f))) != 0 &&
        a1 != *g_00939284) {

        int *const slot = reinterpret_cast<int *>(g_96cdac_table + a1 * 0x20cc);
        bool asked = false;
        while (*g_009b2068 == 0 && *slot < 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->net_tasks();
            if (!asked) {
                asked = true;
                *g_009bbff0 = 0;
                *g_009bbfec = *reinterpret_cast<int *>(g_946a50_table + a1 * 0x59c);
                parse_says(0, g_946a50_table + a1 * 0x59c + 0x4c, -1, -1);
                *g_009bbfec = *reinterpret_cast<int *>(g_946a50_table + a1 * 0x59c);
                *g_009bbff0 = 0;
                parse_says(1, g_946a50_table + a1 * 0x59c + 0x34, -1, -1);
                reinterpret_cast<NetMsg *>(g_00805338)->pop(
                    reinterpret_cast<const char *>(g_0069180c), 0, 0, 0);
            }
        }
        reinterpret_cast<NetMsg *>(g_00805338)->close();
        return *slot;
    }

    int result = sub_5beb70(a1, 0, -1, 0);
    bool locked = *g_0093f660 != 0;
    *reinterpret_cast<int *>(g_96cdac_table + a1 * 0x20cc) = result;
    if (locked && (((1 << (a1 & 0x1f)) & *reinterpret_cast<unsigned char *>(g_009a64e8)) != 0)) {
        synch_researching(a1);
        int stop = *g_009b2068;
        int *const slot = reinterpret_cast<int *>(g_96cdac_table + a1 * 0x20cc);
        *slot = -1;
        while (stop == 0 && *slot < 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->net_tasks();
            stop = *g_009b2068;
        }
    }
    return result;
}

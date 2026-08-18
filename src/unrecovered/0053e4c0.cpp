// ORIGINAL: 0x0053E4C0 ?demands_withdrawal@@YAXHH@Z 0x0053E4C0-0x0053E5B6 FILE
// RULED-OUT: `(byte)(1 << (a2 & 0x1f))` (explicit mask before the shift, mnemonic similarity 0.896, 8 edit blocks - the mask compiles to a spurious `and` the original does not have, since `shl` already masks its count to 5 bits at the hardware level) vs. plain `(byte)(1 << a2)` (0.945, 2 edit blocks). Also: if/else-if chain over the 0/1/2 result vs. `switch` - the original lowers the three-way dispatch as a decrement chain (`sub eax,0 / je / dec / je / dec / jne`), which only the `switch` form reproduces. Landed the switch form. Neither reached BYTE_EXACT; first remaining divergence is at the record-base multiply by 0x59c (#35, `shl` vs `mov`).
// size      246 bytes
// prototype void (__cdecl ?demands_withdrawal@@YAXHH@Z)(int, int)
// callers   ?   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A890 0x00549270 0x005589E0 0x00625EC0

// GENERATED SKELETON (hand-written, following tools/emit_translation_unit.py
// conventions) - a VERIFICATION ARTIFACT, not product source: classes are
// opaque and globals are bound to fixed addresses.

class NetMsg {
public:
    void pop(const char *, int, int, const char *);
};

extern int g_93f7cc;
extern int g_93f660;
extern unsigned char g_9a64e8;
extern int g_9a6800;
extern int g_9bbfec;
extern int g_9bbff0;

static NetMsg *const g_netmsg = (NetMsg *)0x805338;

int __cdecl commlink_attempter(int, int);
int __cdecl demand_withdrawal(int, int);
int __cdecl parse_says(int, char *, int, int);

extern "C" void __cdecl demands_withdrawal(int a1, int a2) {
    g_93f7cc = a2;
    if (commlink_attempter(a1, a2) != 0) {
        return;
    }
    if (g_93f660 != 0) {
        if ((g_9a64e8 & (unsigned char)(1 << a2)) != 0) {
            return;
        }
    }
    int withdrawResult = demand_withdrawal(a1, a2);
    if (g_9a6800 <= 0) {
        return;
    }
    char *record = reinterpret_cast<char *>(0x946d34 + a2 * 0x59c);
    int field18 = *reinterpret_cast<int *>(record + 0x18);
    int field1c = *reinterpret_cast<int *>(record + 0x1c);
    g_9bbfec = field18;
    g_9bbff0 = field1c;
    parse_says(0, record, -1, -1);
    switch (withdrawResult) {
    case 0:
        g_netmsg->pop(reinterpret_cast<const char *>(0x68d68c), 5000, 0, 0);
        break;
    case 1:
        g_netmsg->pop(reinterpret_cast<const char *>(0x68d680), 5000, 0, 0);
        break;
    case 2:
        g_netmsg->pop(reinterpret_cast<const char *>(0x68d674), 5000, 0, 0);
        break;
    default:
        break;
    }
}

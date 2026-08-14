// ORIGINAL: 0x00637C30
// RULED-OUT: original pushes an extra callee-saved register (ecx-slot, ebx,
//            esi, edi - 4 total) this body's control flow only needed 3 of
//            (no ebx); not chased further. `sub_63e6e0` is called here as
//            the scaffold's plain __stdcall(int,int,int,int), dropping the
//            `lea ecx,[esi+0xa5c]` receiver the disassembly sets right
//            before the call - Ghidra's own decompile drops the same
//            thing (`the decompiler drops the this pointer on __thiscall
//            functions`), so it reads as a genuine 5th (thiscall) param
//            this body does not reproduce.
// name      ?read_frame@Video@@QAEHXZ
// size      306 bytes
// spans     0x00637C30-0x00637D62
// prototype int (__thiscall ?read_frame@Video@@QAEHXZ)(Video* this)
// callers   7   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00626250 0x006373A0 0x0063E6E0
// indirect  0x00637CB7 0x00637D03

typedef int (__stdcall *TimeGetTimeProc)();
typedef int (__stdcall *MmioReadProc)(int, char *, unsigned int);

static int *const g_00669368 = (int *)0x00669368;
static int *const g_0066936c = (int *)0x0066936C;

int Video::read_frame() {
    char *self = reinterpret_cast<char *>(this);
    char *pos = self;
    if (*reinterpret_cast<int *>(self + 0x4cc) == 0) {
        int n = sub_63e6e0(reinterpret_cast<int>(&pos), 0,
                            *reinterpret_cast<int *>(self + 0x1000), 1);
        if (n != 0) {
            return 0;
        }
        log_say(reinterpret_cast<char *>(g_00698918),
                reinterpret_cast<char *>(g_00698900),
                *reinterpret_cast<int *>(self + 0x1000),
                *reinterpret_cast<int *>(self + 0xa64), 0);
    } else {
        int n = sub_63e6e0(reinterpret_cast<int>(&pos),
                            *reinterpret_cast<int *>(self + 0x4cc),
                            *reinterpret_cast<int *>(self + 0x1000), 0);
        if (n == 0x14) {
            return 0;
        }
        if (pos == 0) {
            TimeGetTimeProc timeGetTime = reinterpret_cast<TimeGetTimeProc>(*g_00669368);
            int t = timeGetTime();
            log_say(reinterpret_cast<char *>(g_00698940),
                    reinterpret_cast<char *>(g_00698928), t, 0, 0);
            return 0;
        }
        for (unsigned int remaining = *reinterpret_cast<unsigned int *>(self + 0x4cc);
             remaining != 0; ) {
            unsigned int chunk = remaining > 0x10000 ? 0x10000 : remaining;
            MmioReadProc mmioRead = reinterpret_cast<MmioReadProc>(*g_0066936c);
            int got = mmioRead(*reinterpret_cast<int *>(self + 0x4b0), pos, chunk);
            if (got == 0) {
                *reinterpret_cast<unsigned int *>(self + 0x4ac) &= 0xffff7fff;
                return 0;
            }
            pos += got;
            remaining -= got;
        }
        log_say(reinterpret_cast<char *>(g_00698968),
                reinterpret_cast<char *>(g_00698950),
                *reinterpret_cast<int *>(self + 0x1000),
                *reinterpret_cast<int *>(self + 0xa64), 0);
    }
    advance_frame();
    return 1;
}

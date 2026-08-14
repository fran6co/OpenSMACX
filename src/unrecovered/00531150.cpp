// ORIGINAL: 0x00531150
// RULED-OUT: MISMATCH #26 mov/add - NetDaemon has no known field layout, so
//            the `locked` flag at this+0x1BB4 and the embedded Lock at
//            this+0x14A0 are raw offsets rather than named members; close
//            past instruction 26 but register allocation in the retry loop
//            still differs.
// name      ?automated_lock@NetDaemon@@QAEHPAHH@Z
// size      422 bytes
// spans     0x00531150-0x005312F6
// prototype int (__thiscall ?automated_lock@NetDaemon@@QAEHPAHH@Z)(NetDaemon* this, int*, int)
// callers   2   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00531020 0x00534400 0x005901D0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005311A4 0x005311ED 0x0053122E 0x0053127D 0x00531288
// To start: tools/decomp_status.py --work 0x00531150

int NetDaemon::automated_lock(int * a1, int a2) {
    char *const self = reinterpret_cast<char *>(this);
    int32_t *const locked_flag = reinterpret_cast<int32_t *>(self + 0x1bb4);

    if (*g_0093f660 == 0) {
        return 0;
    }

    log_say(reinterpret_cast<char *>(g_0068c34c), a1[0],
            static_cast<int>(reinterpret_cast<signed char *>(g_00952839)[a1[0] * 0x34]), 0);
    ++*g_0093f664;

    uint32_t (__stdcall *const time_get_time)() =
        reinterpret_cast<uint32_t (__stdcall *)()>(*g_00669368);

    uint32_t start_time = time_get_time();
    *locked_flag = 1;
    int awaiting_orders = 0;

    if (*g_009b2068 == 0) {
        do {
            if (lock_veh(a1, a2, -1, -1, 0) == 0) {
                break;
            }
            if (awaiting_orders == 0 && (time_get_time() - start_time) > 500) {
                awaiting_orders = 1;
                reinterpret_cast<NetMsg *>(g_00805338)->pop(
                    reinterpret_cast<const char *>(g_0068c360), 0, 2, 0);
            } else {
                log_say(reinterpret_cast<char *>(g_0068c36c), a1[0], 0, 0);
                uint32_t elapsed = time_get_time() - start_time;
                while (elapsed < 0xfa && *g_009b2068 == 0) {
                    wait_task();
                    unsigned long out_a;
                    unsigned long out_b;
                    int msg = reinterpret_cast<Net *>(g_0093cd90)->get(&out_a, &out_b);
                    if (msg != 0) {
                        process_message(reinterpret_cast<char *>(msg), out_b,
                                        static_cast<int>(out_a));
                    }
                    reinterpret_cast<Lock *>(self + 0x14a0)->check_global();
                    check_spock();
                    elapsed = time_get_time() - start_time;
                }
                start_time = time_get_time();
            }
            check_spock();
        } while (*g_009b2068 == 0);
        if (awaiting_orders != 0) {
            reinterpret_cast<NetMsg *>(g_00805338)->close();
        }
    }

    log_say(reinterpret_cast<char *>(g_0068c380), a1[0],
            static_cast<int>(reinterpret_cast<signed char *>(g_00952839)[a1[0] * 0x34]), 0);
    *locked_flag = 0;
    --*g_0093f664;
    return *g_009b2068;
}

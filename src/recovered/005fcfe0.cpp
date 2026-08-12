// ORIGINAL: 0x005FCFE0 BYTE_EXACT
// name      ?do_all_chars@@YAXXZ
// size      107 bytes
// spans     0x005FCFE0-0x005FD04B
// prototype 
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FD01F 0x005FD02A 0x005FD031
// 0x005FCFE0  ?do_all_chars@@YAXXZ  ->  ?do_all_chars@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *PeekMessageFn)(void *, void *, unsigned, unsigned,
                                      unsigned);
typedef int (__stdcall *TranslateMessageFn)(void *);
typedef int (__stdcall *DispatchMessageFn)(void *);

void __cdecl do_all_chars() {
    int msg[7];

    *g_009b7b9c = 4;

    TranslateMessageFn translate =
        *reinterpret_cast<TranslateMessageFn *>(g_0066935c);
    PeekMessageFn peek = *reinterpret_cast<PeekMessageFn *>(g_00669358);
    DispatchMessageFn dispatch =
        *reinterpret_cast<DispatchMessageFn *>(g_00669340);

    int has_message;
    do {
        do_video();
        check_net();
        has_message = peek(msg, 0, 0x102, 0x102, 1);
        if (has_message) {
            translate(msg);
            dispatch(msg);
        }
    } while (has_message);

    *g_009b7b9c = 0;
    do_net();
}

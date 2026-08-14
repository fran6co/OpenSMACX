// ORIGINAL: 0x00636300 FILE
// RULED-OUT: nothing yet - MISMATCH #2 mov/push, mnemonic similarity 0.908.
//            for-loop and do/while shaped identically; not chased further.
// name      ?do_video@@YAXXZ
// size      238 bytes
// spans     0x00636300-0x006363EE
// prototype
// callers   16   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00626250 0x00637910 0x00637C30 0x00637D70
// indirect  0x0063634E 0x00636385
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00636300

typedef unsigned long (__stdcall *TimeGetTimeFn)(void);

void __cdecl do_video() {
    int i;
    for (i = 0; i < *g_009be68c; ++i) {
        Video *video = ((Video **)g_009be640)[i];
        unsigned int flags = *(unsigned int *)((char *)video + 0x4AC);
        if (flags & 0x8000) {
            if ((flags & 0x10) == 0) {
                if (video->play_frame() == 0) {
                    int t = (*(TimeGetTimeFn *)g_00669368)();
                    log_say((char *)g_0069887c, (char *)g_00698870, t, 0, 0);
                    unsigned char low = *(unsigned char *)((char *)video + 0x4AC);
                    if ((low & 0x10) == 0) {
                        video->play_frame();
                        if (video->decode_frame(0) != 0) {
                            goto next;
                        }
                    }
                }
            }
            {
                int t = (*(TimeGetTimeFn *)g_00669368)();
                log_say((char *)g_006988a0, (char *)g_00698894, t, 0, 0);
                unsigned char low = *(unsigned char *)((char *)video + 0x4AC);
                if ((low & 0x10) == 0 && video->play_frame() != 0) {
                    while (video->decode_frame(0) != 0) {
                        if (video->play_frame() == 0) break;
                    }
                }
            }
            video->read_frame();
        }
next:   ;
    }
}

// ORIGINAL: 0x00642C20 FILE
// name      _inflate_blocks
// size      3368 bytes
// spans     0x00642C20-0x00643948
// prototype 
// callers   1   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00643990 0x00643EB0 0x00643FA0 0x00644100 0x00644140 0x00644180 0x00644910 0x00644930
// indirect  0x00642F98 0x006432E1
// SPAN CORRECTED 2026-08-14, and the evidence is a compile rather than a
//            disassembly. terranx.exe statically links zlib 1.0.2 - the
//            image carries ' inflate 1.0.2 Copyright 1995-1996' - and
//            compiling the upstream infblock.c under this VC6 with
//            `/c /O2 /Gy /GR- /GX /TC` reproduces this function EXACTLY at
//            3368 bytes. The catalogued span stopped 59 bytes short of
//            that, at an instruction boundary, so `verify_span_termination`
//            could not see it: the truncation test looks for a span that
//            stops mid-instruction, and this one stops cleanly and early.
//            The tail lies in the 67-byte gap before the next catalogued
//            function, and the corrected span still ends before it.
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00642C20

// BODY GOES HERE.

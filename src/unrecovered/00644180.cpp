// ORIGINAL: 0x00644180 FILE
// name      _inflate_codes
// size      1896 bytes
// spans     0x00644180-0x006448E8
// prototype 
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644930 0x00644A70
// SPAN CORRECTED 2026-08-14, and the evidence is a compile rather than a
//            disassembly. terranx.exe statically links zlib 1.0.2 - the
//            image carries ' inflate 1.0.2 Copyright 1995-1996' - and
//            compiling the upstream infcodes.c under this VC6 with
//            `/c /O2 /Gy /GR- /GX /TC` reproduces this function EXACTLY at
//            1896 bytes. The catalogued span stopped 2 bytes short of
//            that, at an instruction boundary, so `verify_span_termination`
//            could not see it: the truncation test looks for a span that
//            stops mid-instruction, and this one stops cleanly and early.
//            The tail lies in the 42-byte gap before the next catalogued
//            function, and the corrected span still ends before it.
// placeholder - not yet decompiled
// To start: tools/decomp_status.py --work 0x00644180

// BODY GOES HERE.

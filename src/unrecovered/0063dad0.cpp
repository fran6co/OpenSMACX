// ORIGINAL: 0x0063DAD0
// name      sub_63dad0
// size      367 bytes
// spans     0x0063DAD0-0x0063DC3F
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: this is zlib's `longest_match` (deflate.c) - the field
//            offsets match exactly (0x70 max_chain_length, 0x60 strstart,
//            0x6c prev_length, 0x2c window, 0x20 w_size, 0x80 good_match,
//            0x68 nice_match, 0x84 lookahead, 0x64 match_start, 0x34 prev,
//            0x28 w_mask), but the compiled shape differs from the
//            textbook source in two places kept literal here: the loop's
//            break threshold uses `min(lookahead, nice_match)` while the
//            FINAL return clamps against the UNCLAMPED `nice_match` field
//            (not `lookahead`, unlike the well-known zlib source), and the
//            return value itself is a plain match-length int (Ghidra's
//            `char *` typing of the scan/match locals is a mistype from
//            the pointer-arithmetic-as-integer idiom, not a real pointer
//            return - changed the signature to `int __cdecl(char*,
//            unsigned)` on that evidence). Ghidra's comma-operator
//            "(pcVar7 = ..., cond)" chain in the 8-byte unrolled compare
//            is transcribed as a `for(;;)` of unconditional-assign-then-
//            break steps, matching the short-circuit assignment order.
//            0.73 mnemonic similarity, first divergence at #0 (prologue
//            register-save order for this many live locals).

extern "C" int __cdecl sub_63dad0(char *s, unsigned int cur_match) {
    unsigned int chainLength = *reinterpret_cast<unsigned int *>(s + 0x70);
    unsigned int strstart = *reinterpret_cast<unsigned int *>(s + 0x60);
    int bestLen = *reinterpret_cast<int *>(s + 0x6c);
    char *window = *reinterpret_cast<char **>(s + 0x2c);
    char *scan = window + strstart;

    unsigned int limit;
    if (*reinterpret_cast<unsigned int *>(s + 0x20) - 0x106u < strstart) {
        limit = (strstart - *reinterpret_cast<unsigned int *>(s + 0x20)) + 0x106;
    } else {
        limit = 0;
    }

    char scanEnd1 = scan[bestLen - 1];
    char *strend = scan + 0x102;
    char scanEnd = scan[bestLen];

    if (*reinterpret_cast<int *>(s + 0x80) <= bestLen) {
        chainLength >>= 2;
    }

    int niceMatchField = *reinterpret_cast<int *>(s + 0x68);
    int lookahead = *reinterpret_cast<int *>(s + 0x84);
    int niceMatch = lookahead;
    if (niceMatchField < niceMatch) {
        niceMatch = niceMatchField;
    }

    char *match;
    char *scan2;
    char *pcVar7 = scan;
    int len;

    while (1) {
        match = window + cur_match;
        if (match[bestLen] == scanEnd && match[bestLen - 1] == scanEnd1 &&
            *match == *scan && match[1] == scan[1]) {
            match += 2;
            scan2 = scan + 2;
            for (;;) {
                pcVar7 = scan2 + 1;
                if (scan2[1] != match[1]) break;
                pcVar7 = scan2 + 2;
                if (scan2[2] != match[2]) break;
                pcVar7 = scan2 + 3;
                if (scan2[3] != match[3]) break;
                pcVar7 = scan2 + 4;
                if (scan2[4] != match[4]) break;
                pcVar7 = scan2 + 5;
                if (scan2[5] != match[5]) break;
                pcVar7 = scan2 + 6;
                if (scan2[6] != match[6]) break;
                pcVar7 = scan2 + 7;
                if (scan2[7] != match[7]) break;

                pcVar7 = scan2 + 8;
                match += 8;
                if (scan2[8] != *match) break;
                scan2 = pcVar7;
                if (strend <= pcVar7) break;
            }

            len = 0x102 - static_cast<int>(strend - pcVar7);
            if (bestLen < len) {
                *reinterpret_cast<unsigned int *>(s + 100) = cur_match;
                bestLen = len;
                if (niceMatch <= len) {
                    goto RETURN_CLAMP;
                }
                scanEnd1 = scan[pcVar7 - strend + 0x101];
                scanEnd = scan[len];
            }
        }

        cur_match = *reinterpret_cast<unsigned short *>(
            *reinterpret_cast<int *>(s + 0x34) +
            (*reinterpret_cast<unsigned int *>(s + 0x28) & cur_match) * 2);

        if (cur_match <= limit) {
            goto RETURN_CLAMP;
        }
        --chainLength;
        if (chainLength == 0) {
            goto RETURN_CLAMP;
        }
    }

RETURN_CLAMP:
    if (bestLen > niceMatchField) {
        return niceMatchField;
    }
    return bestLen;
}

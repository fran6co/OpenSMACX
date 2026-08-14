// ORIGINAL: 0x0063AF60
// name      sub_63af60
// size      274 bytes
// spans     0x0063AF60-0x0063B072
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// RULED-OUT: `unsigned short pattern` forces a `movzx` the original never
//            emits (it relies on the shl/and pair discarding the garbage
//            upper half of a 16-bit-loaded register instead of an explicit
//            zero-extend); switching the parameter to plain `int` and
//            writing the fold as `(pattern<<16)|(pattern&0xffff)` gets
//            closest. The 16/8/4/2/1 unrolled fill body mirrors a
//            hand-written negative-counter cascade (`n -= 16; if (n>=0)
//            ...`), matched structurally to the original's
//            sub/jl-then-add/jl chain; landing the closest form
//            (mnemonic_similarity 0.644, 282 vs 274 bytes).

extern "C" void __cdecl sub_63af60(unsigned char *dst, int pattern, int count) {
    unsigned int fill = (pattern << 16) | (pattern & 0xffff);

    if (((unsigned int)dst & 1) == 0) {
        if ((unsigned int)dst & 2) {
            *reinterpret_cast<unsigned short *>(dst) = (unsigned short)fill;
            count--;
            dst += 2;
        }
        int tailWord = count & 1;
        int dwordCount = count >> 1;
        if (dwordCount != 0) {
            unsigned int *p = reinterpret_cast<unsigned int *>(dst);
            int n = dwordCount - 16;
            if (n >= 0) {
                do {
                    p[0] = fill; p[1] = fill; p[2] = fill; p[3] = fill;
                    p[4] = fill; p[5] = fill; p[6] = fill; p[7] = fill;
                    p[8] = fill; p[9] = fill; p[10] = fill; p[11] = fill;
                    p[12] = fill; p[13] = fill; p[14] = fill; p[15] = fill;
                    p += 16;
                    n -= 16;
                } while (n >= 0);
            }
            n += 8;
            if (n >= 0) {
                p[0] = fill; p[1] = fill; p[2] = fill; p[3] = fill;
                p[4] = fill; p[5] = fill; p[6] = fill; p[7] = fill;
                p += 8;
                n -= 8;
            }
            n += 4;
            if (n >= 0) {
                p[0] = fill; p[1] = fill; p[2] = fill; p[3] = fill;
                p += 4;
                n -= 4;
            }
            n += 2;
            if (n >= 0) {
                p[0] = fill; p[1] = fill;
                p += 2;
                n -= 2;
            }
            n += 1;
            if (n >= 0) {
                p[0] = fill;
                p += 1;
            }
            dst = reinterpret_cast<unsigned char *>(p);
        }
        if (tailWord) {
            *reinterpret_cast<unsigned short *>(dst) = (unsigned short)fill;
        }
    }
    else {
        *dst = (unsigned char)fill;
        dst++;
        count = count * 2 - 1;
        unsigned int rotated = (fill >> 1) | (fill << 31);
        unsigned int *p = reinterpret_cast<unsigned int *>(dst);
        int dwordCount = count >> 2;
        for (int i = dwordCount; i != 0; i--) {
            *p = rotated;
            p++;
        }
        dst = reinterpret_cast<unsigned char *>(p);
        if (count & 2) {
            *reinterpret_cast<unsigned short *>(dst) = (unsigned short)rotated;
            dst += 2;
        }
        if (count & 1) {
            *dst = (unsigned char)rotated;
        }
    }
}

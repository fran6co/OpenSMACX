// ORIGINAL: 0x00644930 FILE
// RULED-OUT: 3 stack args (IDA agrees), no receiver - ring-buffer flush:
//            drain up to `avail` bytes from a1's [0x2c,0x30) window into
//            a2's buffer, optionally through a `decode` callback at a1+0x34,
//            then repeat once more if the read pointer wrapped to a1+0x28.
//            Landed at MISMATCH #4; not chased further.
// name      sub_644930
// size      306 bytes
// spans     0x00644930-0x00644A62
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0064498D 0x00644A19

extern "C" int __cdecl sub_644930(int a1, int a2, int a3) {
    char *p1 = reinterpret_cast<char *>(a1);
    char *p2 = reinterpret_cast<char *>(a2);

    char *readPtr = *reinterpret_cast<char **>(p1 + 0x2c);
    char *writePtr = *reinterpret_cast<char **>(p1 + 0x30);
    char *dst = *reinterpret_cast<char **>(p2 + 0xc);

    if (writePtr < readPtr) {
        writePtr = *reinterpret_cast<char **>(p1 + 0x28);
    }

    unsigned int avail = *reinterpret_cast<unsigned int *>(p2 + 0x10);
    unsigned int n = static_cast<unsigned int>(writePtr - readPtr);
    if (avail < n) {
        n = avail;
    }
    if (n != 0 && a3 == -5) {
        a3 = 0;
    }
    *reinterpret_cast<unsigned int *>(p2 + 0x10) = avail - n;
    *reinterpret_cast<unsigned int *>(p2 + 0x14) += n;

    typedef int (__cdecl *DecodeFn)(int, void *, unsigned int);
    DecodeFn decode = *reinterpret_cast<DecodeFn *>(p1 + 0x34);
    if (decode != 0) {
        int r = decode(*reinterpret_cast<int *>(p1 + 0x38), readPtr, n);
        *reinterpret_cast<int *>(p1 + 0x38) = r;
        *reinterpret_cast<int *>(p2 + 0x30) = r;
    }

    char *src = readPtr;
    char *out = dst;
    for (unsigned int words = n >> 2; words != 0; --words) {
        *reinterpret_cast<unsigned int *>(out) = *reinterpret_cast<unsigned int *>(src);
        src += 4;
        out += 4;
    }
    for (unsigned int bytes = n & 3; bytes != 0; --bytes) {
        *out = *src;
        ++src;
        ++out;
    }
    readPtr += n;
    dst += n;

    if (readPtr == *reinterpret_cast<char **>(p1 + 0x28)) {
        readPtr = *reinterpret_cast<char **>(p1 + 0x24);
        if (*reinterpret_cast<char **>(p1 + 0x30) == *reinterpret_cast<char **>(p1 + 0x28)) {
            *reinterpret_cast<char **>(p1 + 0x30) = readPtr;
        }
        unsigned int avail2 = *reinterpret_cast<unsigned int *>(p2 + 0x10);
        unsigned int n2 = static_cast<unsigned int>(*reinterpret_cast<char **>(p1 + 0x30) - readPtr);
        if (avail2 < n2) {
            n2 = avail2;
        }
        if (n2 != 0 && a3 == -5) {
            a3 = 0;
        }
        *reinterpret_cast<unsigned int *>(p2 + 0x10) = avail2 - n2;
        *reinterpret_cast<unsigned int *>(p2 + 0x14) += n2;

        if (decode != 0) {
            int r = decode(*reinterpret_cast<int *>(p1 + 0x38), readPtr, n2);
            *reinterpret_cast<int *>(p1 + 0x38) = r;
            *reinterpret_cast<int *>(p2 + 0x30) = r;
        }

        char *src2 = readPtr;
        char *out2 = dst;
        for (unsigned int words2 = n2 >> 2; words2 != 0; --words2) {
            *reinterpret_cast<unsigned int *>(out2) = *reinterpret_cast<unsigned int *>(src2);
            src2 += 4;
            out2 += 4;
        }
        dst += n2;
        readPtr += n2;
        for (unsigned int bytes2 = n2 & 3; bytes2 != 0; --bytes2) {
            *out2 = *src2;
            ++src2;
            ++out2;
        }
    }

    *reinterpret_cast<char **>(p2 + 0xc) = dst;
    *reinterpret_cast<char **>(p1 + 0x2c) = readPtr;
    return a3;
}

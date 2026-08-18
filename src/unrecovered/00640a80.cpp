// ORIGINAL: 0x00640A80 sub_640a80 0x00640A80-0x00640BAE FILE
// RULED-OUT: `if(len!=0){do}while` (jbe->je); ternary `len>0x15b0?0x15b0:len` (jb->ja); `n -= blocks<<4` (neg->shl). Landed at MNEMONIC_ONLY (127/127 mnemonics match) with `n = n + blocks * -16` mirroring Ghidra's `uVar1 + uVar18 * -0x10`; remaining diff is byte-level only (register/immediate encoding), not chased further.
// size      302 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

extern "C" unsigned int __cdecl sub_640a80(unsigned int adler, const unsigned char *buf, unsigned int len)
{
    unsigned int sum1 = adler & 0xffff;
    unsigned int sum2 = adler >> 16;
    if (buf == 0) {
        return 1;
    }
    while (len > 0) {
        unsigned int n = len < 0x15b0 ? len : 0x15b0;
        len -= n;
        if ((int)n >= 0x10) {
            unsigned int blocks = n >> 4;
            n = n + blocks * -16;
            do {
                sum1 += buf[0];  sum2 += sum1;
                sum1 += buf[1];  sum2 += sum1;
                sum1 += buf[2];  sum2 += sum1;
                sum1 += buf[3];  sum2 += sum1;
                sum1 += buf[4];  sum2 += sum1;
                sum1 += buf[5];  sum2 += sum1;
                sum1 += buf[6];  sum2 += sum1;
                sum1 += buf[7];  sum2 += sum1;
                sum1 += buf[8];  sum2 += sum1;
                sum1 += buf[9];  sum2 += sum1;
                sum1 += buf[10]; sum2 += sum1;
                sum1 += buf[11]; sum2 += sum1;
                sum1 += buf[12]; sum2 += sum1;
                sum1 += buf[13]; sum2 += sum1;
                sum1 += buf[14]; sum2 += sum1;
                sum1 += buf[15]; sum2 += sum1;
                buf += 16;
            } while (--blocks);
        }
        while (n != 0) {
            sum1 += *buf++;
            sum2 += sum1;
            --n;
        }
        sum1 %= 65521;
        sum2 %= 65521;
    }
    return (sum2 << 16) | sum1;
}

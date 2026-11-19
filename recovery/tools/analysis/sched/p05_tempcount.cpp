#include "probe.h"
struct Sp { char pad[0x2c]; void extract(void *, int, int, int, int, int, int); };
extern unsigned char wfc;
struct FW { int a14, a18; Sp sp[12]; void t0(); void t1(); void t2(); void t3(); void t4(); void t5(); void t6(); void t7(); void t8(); void t9(); void t10(); void t11(); };
void FW::t0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t1() {
    char sheet[64];
    f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t2() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t3() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t4() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t5() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t6() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5); f1(ga + 6);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t7() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5); f1(ga + 6); f1(ga + 7);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t8() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5); f1(ga + 6); f1(ga + 7); f1(ga + 8);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t9() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5); f1(ga + 6); f1(ga + 7); f1(ga + 8); f1(ga + 9);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t10() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5); f1(ga + 6); f1(ga + 7); f1(ga + 8); f1(ga + 9); f1(ga + 10);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::t11() {
    char sheet[64];
    f1(ga + 1); f1(ga + 2); f1(ga + 3); f1(ga + 4); f1(ga + 5); f1(ga + 6); f1(ga + 7); f1(ga + 8); f1(ga + 9); f1(ga + 10); f1(ga + 11);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
            a18 += 0x50;
        }
        a14 += 0x50;
    }
}

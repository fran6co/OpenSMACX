#include "probe.h"
struct Sp { char pad[0x2c]; int extract(void *, int, int, int, int, int, int); };
extern unsigned char wfc;
struct FW { int a14, a18; Sp sp[12]; void base0(); void base1(); void base2(); void clause0(); void clause1(); void clause2(); void tmp0(); void tmp1(); void tmp2(); void ret0(); void ret1(); void ret2(); void after0(); void after1(); void after2(); void pre0(); void pre1(); void pre2(); };
void FW::base0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            sprite++->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::base1() {
    char sheet[64];
    f1(ga + 0);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            sprite++->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::base2() {
    char sheet[64];
    f1(ga + 0); f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            sprite++->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::clause0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::clause1() {
    char sheet[64];
    f1(ga + 0);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::clause2() {
    char sheet[64];
    f1(ga + 0); f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r, ++sprite) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::tmp0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            Sp *s = sprite++; s->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::tmp1() {
    char sheet[64];
    f1(ga + 0);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            Sp *s = sprite++; s->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::tmp2() {
    char sheet[64];
    f1(ga + 0); f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            Sp *s = sprite++; s->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::ret0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            int r = sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50; sprite++;
        }
        a14 += 0x50;
    }
}
void FW::ret1() {
    char sheet[64];
    f1(ga + 0);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            int r = sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50; sprite++;
        }
        a14 += 0x50;
    }
}
void FW::ret2() {
    char sheet[64];
    f1(ga + 0); f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            int r = sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50; sprite++;
        }
        a14 += 0x50;
    }
}
void FW::after0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); sprite++; a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::after1() {
    char sheet[64];
    f1(ga + 0);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); sprite++; a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::after2() {
    char sheet[64];
    f1(ga + 0); f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); sprite++; a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::pre0() {
    char sheet[64];
    
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            (++sprite - 1)->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::pre1() {
    char sheet[64];
    f1(ga + 0);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            (++sprite - 1)->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}
void FW::pre2() {
    char sheet[64];
    f1(ga + 0); f1(ga + 1);
    f1((int)sheet);
    Sp *sprite = sp;
    a14 = 1;
    for (int c = 0; c < 6; ++c) {
        a18 = 1;
        for (int r = 0; r < 2; ++r) {
            (++sprite - 1)->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0); a18 += 0x50;
        }
        a14 += 0x50;
    }
}

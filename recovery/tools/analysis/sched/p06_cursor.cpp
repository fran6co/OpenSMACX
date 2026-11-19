#include "probe.h"
struct Sp { char pad[0x2c]; void extract(void *, int, int, int, int, int, int); };
extern unsigned char wfc;
void T__none(Sp *sprite, int a14, int a18) {
    char sheet[64];
    
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__ga_plus1(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(ga + 1);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__ga(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(ga);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__ga_gb(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(ga + gb);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__store(Sp *sprite, int a14, int a18) {
    char sheet[64];
    ga = gb;
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__store2(Sp *sprite, int a14, int a18) {
    char sheet[64];
    ga = gb; gc = gd;
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__g0(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(g0());
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__g0store(Sp *sprite, int a14, int a18) {
    char sheet[64];
    ga = g0();
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__call0(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f0();
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__const(Sp *sprite, int a14, int a18) {
    char sheet[64];
    ga = 5;
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__cmp(Sp *sprite, int a14, int a18) {
    char sheet[64];
    if (ga) f0();
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__cmp2(Sp *sprite, int a14, int a18) {
    char sheet[64];
    if (ga > gb) f0();
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__arr(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(garr[ga]);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__byte(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(gch);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__ubyte(Sp *sprite, int a14, int a18) {
    char sheet[64];
    f1(guc);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}
void T__lea(Sp *sprite, int a14, int a18) {
    char sheet[64];
    char b2[8]; f1((int)b2);
    f0();
    sprite->extract(sheet, wfc, a14, a18, 0x4f, 0x4f, 0);
}

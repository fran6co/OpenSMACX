// MATCHING_GUIDE 3.2 "A global load between the compare and the jcc is a head merge" (2026-11-19 par-1).
// python3 run.py p_headmerge.cpp --asm
struct LabelsT { char **strings_ptr; int count; };
extern LabelsT Labels;
struct Strings { char *get(char *); };
extern Strings StringTable;
extern "C" char *strcat(char *, const char *);
extern void colon(char *);
extern signed char rating[];
extern int g;
// A: both arms start with the label read (say_weapon)
void a_both(char *out, int i) {
    if (rating[i] != 0) { strcat(out, StringTable.get(Labels.strings_ptr[0x81])); colon(out); }
    else { strcat(out, StringTable.get(Labels.strings_ptr[0xA1])); }
}
// B: then arm has a call first
void b_then_call(char *out, int i) {
    if (rating[i] != 0) { colon(out); strcat(out, StringTable.get(Labels.strings_ptr[0x81])); }
    else { strcat(out, StringTable.get(Labels.strings_ptr[0xA1])); }
}
// C: named local per arm
void c_named(char *out, int i) {
    if (rating[i] != 0) { char *l = Labels.strings_ptr[0x81]; strcat(out, StringTable.get(l)); colon(out); }
    else { char *l = Labels.strings_ptr[0xA1]; strcat(out, StringTable.get(l)); }
}
// D: ternary
void d_ternary(char *out, int i) {
    strcat(out, StringTable.get(rating[i] != 0 ? Labels.strings_ptr[0x81] : Labels.strings_ptr[0xA1]));
}
// E: compare a byte against 3 (draw_unit shape), arms identical shape
void e_cmp3(char *out, int i) {
    if ((unsigned char)rating[i] < 3) strcat(out, StringTable.get(Labels.strings_ptr[0x81]));
    else strcat(out, StringTable.get(Labels.strings_ptr[0xA1]));
}
// F: then arm label first, else arm global store first
void f_else_store(char *out, int i) {
    if (rating[i] != 0) strcat(out, StringTable.get(Labels.strings_ptr[0x81]));
    else { g = 1; strcat(out, StringTable.get(Labels.strings_ptr[0xA1])); }
}
// G: arms each push another argument before (strcat out first) -> argument order
void g_push(char *out, int i, int k) {
    if (rating[i] != 0) colon(StringTable.get(Labels.strings_ptr[0x81]));
    else strcat(out, StringTable.get(Labels.strings_ptr[0xA1]));
}
struct W { char *n; char *s; signed char off; signed char icon; unsigned char mode; unsigned char cost; short p; short q; };
extern W Weapon[]; struct P { char x[0x30]; short weapon_id; }; extern P Protos[];
extern void f1(); extern void f2(); extern int g;
void m_plain(int id) { if (Weapon[Protos[id].weapon_id].mode < 3) f1(); else f2(); }
void m_le2(int id) { if (Weapon[Protos[id].weapon_id].mode <= 2) f1(); else f2(); }
void m_uc(int id) { if ((unsigned char)Weapon[Protos[id].weapon_id].mode < 3) f1(); else f2(); }
void m_u(int id) { if ((unsigned)Weapon[Protos[id].weapon_id].mode < 3) f1(); else f2(); }
void m_ge(int id) { if (!(Weapon[Protos[id].weapon_id].mode >= 3)) f1(); else f2(); }
void m_local_u8(int id) { unsigned char m = Weapon[Protos[id].weapon_id].mode; if (m < 3) f1(); else f2(); }
void m_local_int(int id) { int m = Weapon[Protos[id].weapon_id].mode; if (m < 3) f1(); else f2(); }
void m_local_uns(int id) { unsigned m = Weapon[Protos[id].weapon_id].mode; if (m < 3) f1(); else f2(); }
void m_local_used(int id) { unsigned char m = Weapon[Protos[id].weapon_id].mode; if (m < 3) f1(); else { if (m >= 7) f2(); } }
void m_sw(int id) { switch (Weapon[Protos[id].weapon_id].mode) { case 0: case 1: case 2: f1(); break; default: f2(); } }

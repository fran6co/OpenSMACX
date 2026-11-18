#include "probe.h"
// Shape of success_rates (0x0059EEE0): two parameters with equal weighted counts; m is clamped first.
// The image enregisters m (ebx), our source enregistered d.
extern char buf[];
// m clamped by if; d tested, copied in one arm, used twice in the other; m used twice there
int t_base(int id, int m, int d, int b) {
    buf[0] = 0;
    if (m < 1) { m = 1; }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f;
    }
    use_c(id);
    return s;
}
// same without the clamp
int t_noclamp(int id, int m, int d, int b) {
    buf[0] = 0;
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f;
    }
    use_c(id);
    return s;
}
// clamp as a conditional expression
int t_ternary(int id, int m, int d, int b) {
    buf[0] = 0;
    m = m < 1 ? 1 : m;
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f;
    }
    use_c(id);
    return s;
}
// clamp into a local copy
int t_local(int id, int m0, int d, int b) {
    buf[0] = 0;
    int m = m0;
    if (m < 1) { m = 1; }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f;
    }
    use_c(id);
    return s;
}
// s = d moved before the call
int t_sfirst(int id, int m, int d, int b) {
    buf[0] = 0;
    if (m < 1) { m = 1; }
    int s;
    if (d < 0) { s = d; call(); }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f;
    }
    use_c(id);
    return s;
}
// clamp via max-like helper
inline int clamp1(int v) { return v < 1 ? 1 : v; }
int t_helper(int id, int m, int d, int b) {
    buf[0] = 0;
    m = clamp1(m);
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f;
    }
    use_c(id);
    return s;
}

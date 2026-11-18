#include "probe.h"
// success_rates shape again: which kinds of reference count?
extern char buf[];
// top: if (m < 1) { call(); } | else 1: use_a((d * 100) / ((m / 2) - p + 1)); | else 2: use_b(((d + 1) * 100) / (m - q));
int k_base(int id, int m, int d, int b) {
    if (m < 1) { call(); }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_a((d * 100) / ((m / 2) - p + 1));
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_b(((d + 1) * 100) / (m - q));
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

// top: if (m < 1) { call(); } | else 1: use_a(d); use_b(m); | else 2: use_a(d); use_b(m);
int k_push(int id, int m, int d, int b) {
    if (m < 1) { call(); }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

// top: call(); | else 1: use_a(d); use_b(m); | else 2: use_a(d); use_b(m);
int k_push_nom(int id, int m, int d, int b) {
    call();
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

// top: if (m < 1) { call(); } | else 1: use_b(m); | else 2: use_b(m);
int k_push_nod(int id, int m, int d, int b) {
    if (m < 1) { call(); }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_b(m);
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

// top: if (m < 1) { call(); } | else 1: use_a(d); | else 2: use_a(d);
int k_push_m_only_top(int id, int m, int d, int b) {
    if (m < 1) { call(); }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_a(d);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d);
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

// top: use_b(m); | else 1: use_a(d); use_b(m); | else 2: use_a(d); use_b(m);
int k_mtop_push(int id, int m, int d, int b) {
    use_b(m);
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

// top: if (m < 1) { call(); } | else 1: use_a(d); | else 2: use_a(d); use_b(m);
int k_mtop_cmp_d_push(int id, int m, int d, int b) {
    if (m < 1) { call(); }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        use_a(d);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
        s = p;
    }
    use_c(id);
    return s;
}

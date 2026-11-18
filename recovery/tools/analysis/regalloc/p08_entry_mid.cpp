#include "probe.h"
// m: one entry-block reference + two else-arm pushes; d: one test after the first if + two else-arm pushes (+ if-arm).
extern char buf[];
//  | if (m < 1) { call(); } | if (d < 0) {  }
int j_param(int id, int m, int d, int b) {
    
    if (m < 1) { call(); }
    int s = 0;
    if (d < 0) { call();  }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

//  | if (m < 1) { call(); } | if (d < 0) { use_a(d); }
int j_param_dcall(int id, int m, int d, int b) {
    
    if (m < 1) { call(); }
    int s = 0;
    if (d < 0) { call(); use_a(d); }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

//  | if (m < 1) { call(); } | if (cond()) { use_a(d); }
int j_param_cond(int id, int m, int d, int b) {
    
    if (m < 1) { call(); }
    int s = 0;
    if (cond()) { call(); use_a(d); }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

//  | if (cond()) { call(); } use_b(m); | if (cond()) { use_a(d); }
int j_param_cond_m(int id, int m, int d, int b) {
    
    if (cond()) { call(); } use_b(m);
    int s = 0;
    if (cond()) { call(); use_a(d); }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

//  | use_b(m); if (cond()) { call(); } | if (cond()) { use_a(d); }
int j_param_cond_m2(int id, int m, int d, int b) {
    
    use_b(m); if (cond()) { call(); }
    int s = 0;
    if (cond()) { call(); use_a(d); }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

// D(m); D(d); | if (m < 1) { call(); } | if (d < 0) {  }
int j_local(int id, int b) {
    D(m); D(d);
    if (m < 1) { call(); }
    int s = 0;
    if (d < 0) { call();  }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

// D(m); D(d); | if (m < 1) { call(); } | if (d < 0) { use_a(d); }
int j_local_dcall(int id, int b) {
    D(m); D(d);
    if (m < 1) { call(); }
    int s = 0;
    if (d < 0) { call(); use_a(d); }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

// D(m); D(d); | if (m < 1) { call(); } | if (cond()) { use_a(d); }
int j_local_cond(int id, int b) {
    D(m); D(d);
    if (m < 1) { call(); }
    int s = 0;
    if (cond()) { call(); use_a(d); }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

// D(d); D(m); | if (m < 1) { call(); } | if (d < 0) {  }
int j_local_rev(int id, int b) {
    D(d); D(m);
    if (m < 1) { call(); }
    int s = 0;
    if (d < 0) { call();  }
    else {
        D(p); D(q);
        use_a(d); use_b(m);
        U(p); U(q); U(p); U(q); U(p); U(q);
        use_a(d); use_b(m);
        U(p); U(q);
    }
    use_c(id);
    return s;
}

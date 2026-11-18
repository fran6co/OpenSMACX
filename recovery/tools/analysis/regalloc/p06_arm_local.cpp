#include "probe.h"
// Else-arm locals p (N uses) against a parameter d (one top-level test, one use per arm): who gets esi?
extern char buf[];
int e_n1(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n2(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n3(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n4(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n5(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p); U(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n6(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n7(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e_n8(int id, int m, int d, int b) {
    int s;
    if (d < 0) { call(); s = d; }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  s = d + 1; }
    use_c(id);
    return s;
}
int e2_n1(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n2(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n3(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n4(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n5(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n6(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n7(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e2_n8(int id, int m, int d, int b) {
    int s = 0;
    if (d < 0) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n1(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n2(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n3(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n4(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n5(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n6(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n7(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e3_n8(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { call(); }
    else { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n1(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n2(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n3(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n4(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n5(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n6(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n7(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
int e4_n8(int id, int m, int d, int b) {
    int s = 0;
    use_c(d); use_c(d);
    if (cond()) { D(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p); U(p);  }
    use_c(d);
    return s;
}
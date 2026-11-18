#include "probe.h"
// success_rates shape: how many more references does the clamped parameter m need to beat d for ebx?
extern char buf[];
// m: if (m < 1) { m = 1; } | if-arm:  | else-arm: 
int v_clamp(int id, int m, int d, int b) {
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

// m: if (m < 1) { m = 1; } sink_a = m; | if-arm:  | else-arm: 
int v_clamp_top1(int id, int m, int d, int b) {
    buf[0] = 0;
    if (m < 1) { m = 1; } sink_a = m;
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

// m: if (m < 1) { m = 1; } | if-arm: sink_a = m; | else-arm: 
int v_clamp_if1(int id, int m, int d, int b) {
    buf[0] = 0;
    if (m < 1) { m = 1; }
    int s;
    if (d < 0) { call(); s = d; sink_a = m;}
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

// m: if (m < 1) { m = 1; } | if-arm:  | else-arm: sink_a = m;
int v_clamp_else1(int id, int m, int d, int b) {
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
        s = f; sink_a = m;
    }
    use_c(id);
    return s;
}

// m: if (m < 1) { m = 1; } | if-arm:  | else-arm: sink_a = m; call(); sink_b = m;
int v_clamp_else2(int id, int m, int d, int b) {
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
        s = f; sink_a = m; call(); sink_b = m;
    }
    use_c(id);
    return s;
}

// m: if (m < 1) { call(); } | if-arm:  | else-arm: 
int v_cmp_only(int id, int m, int d, int b) {
    buf[0] = 0;
    if (m < 1) { call(); }
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

// m: if (m < 1) { call(); } | if-arm:  | else-arm: sink_a = m;
int v_cmp_only_else1(int id, int m, int d, int b) {
    buf[0] = 0;
    if (m < 1) { call(); }
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f; sink_a = m;
    }
    use_c(id);
    return s;
}

// m: sink_a = m; | if-arm:  | else-arm: 
int v_top1(int id, int m, int d, int b) {
    buf[0] = 0;
    sink_a = m;
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

// m: sink_a = m; | if-arm:  | else-arm: sink_a = m;
int v_top1_else1(int id, int m, int d, int b) {
    buf[0] = 0;
    sink_a = m;
    int s;
    if (d < 0) { call(); s = d; }
    else {
        D(p); D(q);
        int f = (d * 100) / ((m / 2) - p + 1);
        use_a(f); U(p); U(q); U(p); U(q); U(p); U(q);
        int l = ((d + 1) * 100) / (m - q);
        use_b(l); U(p); U(q);
        s = f; sink_a = m;
    }
    use_c(id);
    return s;
}

// m: sink_a = m; call(); sink_b = m; | if-arm:  | else-arm: 
int v_top2(int id, int m, int d, int b) {
    buf[0] = 0;
    sink_a = m; call(); sink_b = m;
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

// m: sink_a = d; if (m < 1) { m = 1; } | if-arm:  | else-arm: 
int v_clamp_d_first(int id, int m, int d, int b) {
    buf[0] = 0;
    sink_a = d; if (m < 1) { m = 1; }
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

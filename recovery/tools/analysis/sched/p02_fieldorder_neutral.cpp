#include "probe.h"
void g(RECT *);
void tile(int,int,int,int);
void P__l_t_r_b(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.top += 2; t.right += 3; t.bottom += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__l_t_b_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.top += 2; t.bottom += 3; t.right += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__l_r_t_b(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.right += 2; t.top += 3; t.bottom += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__l_r_b_t(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.right += 2; t.bottom += 3; t.top += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__l_b_t_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.bottom += 2; t.top += 3; t.right += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__l_b_r_t(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.bottom += 2; t.right += 3; t.top += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__t_l_r_b(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.top += 1; t.left += 2; t.right += 3; t.bottom += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__t_l_b_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.top += 1; t.left += 2; t.bottom += 3; t.right += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__t_r_l_b(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.top += 1; t.right += 2; t.left += 3; t.bottom += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__t_r_b_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.top += 1; t.right += 2; t.bottom += 3; t.left += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__t_b_l_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.top += 1; t.bottom += 2; t.left += 3; t.right += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__t_b_r_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.top += 1; t.bottom += 2; t.right += 3; t.left += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__r_l_t_b(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.left += 2; t.top += 3; t.bottom += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__r_l_b_t(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.left += 2; t.bottom += 3; t.top += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__r_t_l_b(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.top += 2; t.left += 3; t.bottom += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__r_t_b_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.top += 2; t.bottom += 3; t.left += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__r_b_l_t(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.bottom += 2; t.left += 3; t.top += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__r_b_t_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.bottom += 2; t.top += 3; t.left += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__b_l_t_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.bottom += 1; t.left += 2; t.top += 3; t.right += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__b_l_r_t(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.bottom += 1; t.left += 2; t.right += 3; t.top += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__b_t_l_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.bottom += 1; t.top += 2; t.left += 3; t.right += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__b_t_r_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.bottom += 1; t.top += 2; t.right += 3; t.left += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__b_r_l_t(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.bottom += 1; t.right += 2; t.left += 3; t.top += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__b_r_t_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.bottom += 1; t.right += 2; t.top += 3; t.left += 4;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__none(RECT *src, int c) {
    RECT t = *src; g(&t);
    
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__only_l_r(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.left += 1; t.right += 2;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}
void P__only_r_l(RECT *src, int c) {
    RECT t = *src; g(&t);
    t.right += 1; t.left += 2;
    g(&t);
    if (c) f0();
    if (t.left < t.right) tile(t.left, t.top, t.right - t.left, t.bottom - t.top);
}

// Shared declarations for the scheduling probes (run.py).
struct RECT { long left, top, right, bottom; };
extern void f0();
extern void f1(int);
extern void f2(int, int);
extern void f3(int, int, int);
extern void f6(int, int, int, int, int, int);
extern int g0();
extern int ga, gb, gc, gd;
extern signed char gsc;
extern char gch;
extern unsigned char guc;
extern int garr[];
extern RECT grect;
struct Sprite { int w, h; void extract(int, int); };
struct Win {
    int a, b, c, d;
    int arr[8];
    RECT r1, r2;
    signed char sc;
    char ch;
    void tile(void *s, int x, int y, int sx, int sy, int w, int h);
    virtual void vf(int);
};
extern Win gwin;

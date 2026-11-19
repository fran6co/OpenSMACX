== flip
#include <string.h>
struct R { int l, t, r, b; };
extern R WR; extern char* WA; struct F { int h, u; int ph(); int lh(); }; extern F* WF;
extern void wct(char*, int, int, int);
void probe() {
    if (strchr(WA, '^')) {
        int y = WR.t + 1;
        char* line = WA;
        char* split;
        do {
            split = strchr(line, '^');
            if (split) *split = 0;
            wct(line, WR.l, y, WR.r - WR.l);
            y += WF->u >= 0 ? WF->ph() + WF->u : WF->lh();
            if (split) *split = '^';
            line = split + 1;
        } while (split);
    }
}

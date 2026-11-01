// Lazy map-site queries. Original translation-unit filename is unknown.
#include "stdafx.h"
#include "map.h"

// ORIGINAL: 0x005797D0 ?site_at@@YAHHHH@Z 0x005797D0-0x00579832 FILE BYTE_EXACT
// TRIED: single combined expression and explicit temp reads of the first two parameters both match 44 of 46 mnemonics at 98 of 98 bytes; VC6 interleaves the push/mov prologue differently (push, push, mov, mov instead) - register scheduling, not logic
// working copy - scaffold materialised by --work
// size      98 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B50 0x005C4FD0
int __cdecl site_at(int x, int y, int is_ocean_site) {
    Map *tile = map_loc(x, y);
    if (tile->bit & BIT_BASE_RADIUS) {
        return 0;
    }
    int site = tile->val2 >> 4;
    if (site != 0) {
        return site;
    }
    const int generated = world_site(x, y, is_ocean_site);
    site_set(x, y, generated);
    return generated;
}

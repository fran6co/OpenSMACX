// ORIGINAL: 0x0046B040 ?draw_tile_fixup@@YAXHHHHHH@Z 0x0046B040-0x0046B132 FILE BYTE_EXACT
// size      242 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0
/*
Purpose: Repaint one or two map tiles across every live map window, choosing
         between two very different redraws on bit 0 of the more-preferences
         word - MPREF_MAP_SHOW_FOG_WAR.

         Fog of war OFF (bit clear, the `je 0x46b0f2` at 0x0046B04D): one
         broadcast of (x_coord, y_coord) at radius 0 with the CALLER's
         draw_type - byte for byte the same walk as ?draw_tile@@YAXHHH@Z at
         0x0046AF40, inlined here.

         Fog of war ON: the same walk twice, at radius 1 or 2 depending on
         `wider_radius`, and with the local faction id in place of the
         caller's draw_type - a visibility refresh rather than a terrain
         redraw. The second tile is drawn only when second_x_coord is not -1,
         which is this function's "there is no second tile" sentinel
         (`cmp dword ptr [ebp+0x10], -1` at 0x0046B09B).

         The caller's draw_type reaches draw_radius ONLY on the fog-off path;
         on the fog-on path 0x00939284 supplies that argument instead.

FRAME SHAPE - what it took to make cl 12.00.8168 emit this prologue, since a
         previous attempt recorded MISMATCH #0 (`push` against `mov`, 60%
         mnemonic similarity) and blamed frame-pointer omission. FPO was not
         the problem; the /Oy- build already had `push ebp; mov ebp, esp` and
         was diverging at #7. Four source-form changes, each measured:

           #0  -> #7   branch polarity. The original falls THROUGH into the
                       fog-on path and jumps away to the fog-off one, so the
                       fog-on block has to be the `if` and not the `else`.
           #7  -> #12  the 0x00939284 read hoisted into a local per loop. Left
                       at the call site it cannot be hoisted across
                       draw_radius, and reloads every iteration.
           #12 -> #22  `extern int LocalFaction;` in place of
                       `*(int *)0x00939284`, and the same for the preferences
                       word. Fixed-address casts are folded by /O2 and cost
                       the register allocation; see the seam note in
                       tools/agent_brief.py.
           #22 -> BYTE_EXACT  a real `MapWin::draw_radius` declaration, so the
                       call is `call rel32` to an extern symbol. The
                       pointer-to-member seam spells it `mov reg, 0x46a2a0;
                       call reg` - two instructions where the original has
                       one - and the extra register that costs is what pinned
                       `wider_radius` in EBX and spilled the faction id.

         RULED OUT along the way, all measured, none of them the lever: which
         values get a named local (drawType only / plus y / plus x / all at
         top - identical 250-byte output every time), an index walk against a
         pointer walk, `a5 ? 2 : 1` against `(a5 != 0) + 1` against
         `1 + (a5 != 0)`, a `__inline` helper, a macro, and a volatile read of
         the flag parameter. Once the call form was right, all three radius
         spellings reached BYTE_EXACT.

Return Value: n/a; EAX on return is draw_radius's leftover, as on draw_tile.
Status: Complete with temporary MapWin::draw_radius original dependency
*/

// A verification unit, not product source: this file is not in the DLL's
// source list, so it declares the one method it calls and binds its globals
// by name. `MapWin::draw_radius` mangles to ?draw_radius@MapWin@@QAEXHHHH@Z,
// which is the catalogued name at 0x0046A2A0.
class MapWin {
public:
    void draw_radius(int, int, int, int);
};

// 0x007D3C3C, eight slots ending at 0x007D3C5C - the bound the original
// spells as `cmp esi, 0x7d3c5c` at 0x0046B093.
extern MapWin *MapWinTable[];
const unsigned long MapWinActiveOffset = 0x1DD74;
const unsigned int MapWinTableSlots = 8;

// 0x00939284, the faction the human is playing - src/game_recovery.cpp,
// src/netdaemon.cpp, src/scenario.cpp and src/spying_recovery.cpp all bind
// this address under that name.
extern int LocalFaction;

// 0x009A6494, src/game.cpp's GameMorePreferences word. The original reads it
// with `mov al, byte ptr [0x9a6494]` because bit 0 is the only bit it wants.
extern unsigned int GameMorePreferences;
const unsigned int MPREF_MAP_SHOW_FOG_WAR = 0x1;

void __cdecl draw_tile_fixup(int x_coord, int y_coord, int second_x_coord,
                             int second_y_coord, int wider_radius,
                             int draw_type) {
    if (GameMorePreferences & MPREF_MAP_SHOW_FOG_WAR) {
        // 0x0046B053: read once per loop into EDI, not once per window - the
        // call in the loop could store to it, so it cannot be hoisted further.
        int const faction = LocalFaction;
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            // Re-read every iteration, as `mov ecx, dword ptr [esi]` does: a
            // callee that rewrites the table is seen by later slots.
            MapWin *const window = MapWinTable[slot];
            // 0x0046B063 test / 0x0046B065 je - the null test comes first, so
            // an empty slot 0 draws nothing despite the exemption below.
            if (window == 0) {
                continue;
            }
            // 0x0046B067 `cmp esi, 0x7d3c3c` / 0x0046B06D je. The cursor only
            // walks forward from the table base, so comparing it against that
            // base is exactly "is this slot index 0?" - the primary map window
            // is exempt from the activity gate.
            if (slot != 0) {
                const unsigned int active = *(const volatile unsigned int *)(
                    (const char *)window + MapWinActiveOffset);
                if (active == 0) {
                    continue;
                }
            }
            // 0x0046B07C..0x0046B087: `xor eax,eax; test edx,edx; setne al;
            // inc eax` - radius 1 normally, 2 when wider_radius is set.
            window->draw_radius(x_coord, y_coord, (wider_radius != 0) + 1,
                                faction);
        }
        // 0x0046B09B: -1 is "no second tile".
        if (second_x_coord != -1) {
            // 0x0046B0A5, the same read again rather than a value carried
            // across the first loop.
            int const faction = LocalFaction;
            for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
                MapWin *const window = MapWinTable[slot];
                if (window == 0) {
                    continue;
                }
                if (slot != 0) {
                    const unsigned int active = *(const volatile unsigned int *)(
                        (const char *)window + MapWinActiveOffset);
                    if (active == 0) {
                        continue;
                    }
                }
                window->draw_radius(second_x_coord, second_y_coord,
                                    (wider_radius != 0) + 1, faction);
            }
        }
    } else {
        // 0x0046B0F2 onwards: draw_tile's body, with the caller's draw_type.
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0) {
                const unsigned int active = *(const volatile unsigned int *)(
                    (const char *)window + MapWinActiveOffset);
                if (active == 0) {
                    continue;
                }
            }
            // 0x0046B119 `push 0` - radius 0, the single tile.
            window->draw_radius(x_coord, y_coord, 0, draw_type);
        }
    }
}


#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
typedef unsigned char uchar;
typedef unsigned int uint32_t;
#define __cdecl

struct Map {
    uchar climate, contour, val2, region, visibility, val3, unk_1;
    signed char territory;
    uint32_t bit;
    uint32_t bit2;
    uint32_t bit_visible[7];
};
typedef char Map_layout_must_be_2c[(sizeof(Map) == 0x2c) ? 1 : -1];

struct MapWin {
    unsigned char prefix[8];
    uint32_t field_8_;
    unsigned char middle[0x1dd74 - 12];
    uint32_t field_1DD74_;
    int calls;
    int got_x, got_y, got_radius;
    uint32_t sentinel_seen_by_call;
    void clear_alt(int x, int y, int radius) {
        ++calls;
        got_x = x;
        got_y = y;
        got_radius = radius;
        sentinel_seen_by_call = field_8_;
    }
};
typedef char MapWin_context_must_be_at_8[(offsetof(MapWin, field_8_) == 8) ? 1 : -1];
typedef char MapWin_live_must_be_at_1dd74[(offsetof(MapWin, field_1DD74_) == 0x1dd74) ? 1 : -1];

static Map tile_store[8];
Map *MapTiles = tile_store;
uint32_t MapLongitude = 2;
inline Map *map_tiles() { return MapTiles; }
const unsigned char AltDirectionTable[245] = {
#include "alt_direction_table.inc"
};
unsigned char AltBonusArid[113], AltBonusMoist[113];
unsigned char AltBonusRainy[113], AltBonusFlat[113], AltBonusOcean[113];
unsigned char AltTemplateArid[113], AltTemplateMoist[113];
unsigned char AltTemplateRainy[113], AltTemplateFlat[113], AltTemplateOcean[113];
uint32_t UnkBitfield1;
int MapLatitudeBounds, MapLongitudeBounds;
enum { BIT2_CRATER = 1, BIT2_VOLCANO = 2 };
inline Map *map_loc(int x, int y) { return map_tiles() + (x >> 1) + y * MapLongitude; }
static const size_t MapWinTableSlots = 8;
MapWin windows[8];
MapWin *MapWinTable[8];

/* ACTUAL_BODIES */


static void reset() {
    memset(tile_store, 0, sizeof(tile_store));
    memset(AltBonusArid, 0, sizeof(AltBonusArid));
    memset(AltBonusMoist, 0, sizeof(AltBonusMoist));
    memset(AltBonusRainy, 0, sizeof(AltBonusRainy));
    memset(AltBonusFlat, 0, sizeof(AltBonusFlat));
    memset(AltBonusOcean, 0, sizeof(AltBonusOcean));
    memset(windows, 0, sizeof(windows));
    for (int i = 0; i < 8; ++i) MapWinTable[i] = &windows[i];
    UnkBitfield1 = 0;
}

int main() {
    reset();
    for (int i = 0; i < 113; ++i) {
        AltTemplateOcean[i] = (unsigned char)(i + 1);
        AltTemplateFlat[i] = (unsigned char)(i + 2);
        AltTemplateRainy[i] = (unsigned char)(i + 3);
        AltTemplateArid[i] = (unsigned char)(i + 4);
        AltTemplateMoist[i] = (unsigned char)(i + 5);
    }
    fixup_landmarks();
    for (int i = 0; i < 113; ++i) {
        assert(AltBonusOcean[i] == AltTemplateOcean[i]);
        assert(AltBonusFlat[i] == AltTemplateFlat[i]);
        assert(AltBonusRainy[i] == AltTemplateRainy[i]);
        assert(AltBonusArid[i] == AltTemplateArid[i]);
        assert(AltBonusMoist[i] == AltTemplateMoist[i]);
    }

    reset();
    MapLatitudeBounds = 1; MapLongitudeBounds = 2;
    tile_store[0].bit2 = 0xfe000003u; // encoded type1, volcano + crater
    fixup_landmarks();
    for (int i = 0; i < 5; ++i) {
        unsigned char index = AltDirectionTable[5 + i];
        assert(AltBonusArid[index] == 0);
        assert(AltBonusMoist[index] == 0);
    }
    reset();
    tile_store[0].bit2 = 0xfd000001u; // encoded type2, crater only
    fixup_landmarks();
    for (int i = 0; i < 5; ++i) {
        unsigned char index = AltDirectionTable[10 + i];
        assert(AltBonusMoist[index] == 0);
        assert(AltBonusArid[index] == AltTemplateArid[index]);
    }
    MapLatitudeBounds = MapLongitudeBounds = 0;

    const int x = 4, y = 1;
    const int tile_index = (x >> 1) + y * (int)MapLongitude;
    const unsigned int old = (2u << 24) | 0x00000b83u;
    reset();
    tile_store[tile_index].bit2 = old;
    for (int i = 0; i < 5; ++i) {
        const unsigned char index = AltDirectionTable[10 + i];
        AltBonusArid[index] = AltBonusMoist[index] = 7;
        AltBonusRainy[index] = AltBonusFlat[index] = 7;
        AltBonusOcean[index] = 7;
    }
    windows[0].field_1DD74_ = 1;
    windows[3].field_1DD74_ = 2;
    delete_landmark(x, y);
    assert(tile_store[tile_index].bit2 == 0xfd000b03u);
    assert(UnkBitfield1 == 4u);
    for (int i = 0; i < 5; ++i) {
        const unsigned char index = AltDirectionTable[10 + i];
        assert(AltBonusMoist[index] == 0);
        assert(AltBonusArid[index] == 0);
        assert(AltBonusRainy[index] == 0);
        assert(AltBonusFlat[index] == 0);
        assert(AltBonusOcean[index] == 7);
    }
    for (int i = 0; i < 8; ++i) {
        const int active = (i == 0 || i == 3);
        assert(windows[i].calls == active);
        if (active) {
            assert(windows[i].field_8_ == 0x0098967fu);
            assert(windows[i].sentinel_seen_by_call == 0x0098967fu);
            assert(windows[i].got_x == x && windows[i].got_y == y);
            assert(windows[i].got_radius == 1);
        }
    }

    // Zero bit2 is a strict no-op, including dirty state and window caches.
    reset();
    windows[0].field_1DD74_ = 1;
    delete_landmark(x, y);
    assert(UnkBitfield1 == 0 && windows[0].calls == 0);

    // A negative signed type only clears bit 0x80; no bonus or cache effects.
    reset();
    tile_store[tile_index].bit2 = 0xff000081u;
    windows[0].field_1DD74_ = 1;
    delete_landmark(x, y);
    assert(tile_store[tile_index].bit2 == 0xff000001u);
    assert(UnkBitfield1 == 0 && windows[0].calls == 0);

    // The 0x100 and 0x800 masks both address Flat; neither touches Ocean.
    reset();
    tile_store[tile_index].bit2 = (0u << 24) | 0x00000900u;
    for (int i = 0; i < 5; ++i) {
        const unsigned char index = AltDirectionTable[i];
        AltBonusFlat[index] = 9;
        AltBonusOcean[index] = 9;
    }
    delete_landmark(x, y);
    for (int i = 0; i < 5; ++i) {
        const unsigned char index = AltDirectionTable[i];
        assert(AltBonusFlat[index] == 0);
        assert(AltBonusOcean[index] == 9);
    }
    return 0;
}

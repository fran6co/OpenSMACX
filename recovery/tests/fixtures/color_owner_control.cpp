#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#ifndef __cdecl
#define __cdecl
#endif
#define MEASURED
#define nullptr 0
struct RGBQUAD { unsigned char rgbBlue,rgbGreen,rgbRed,rgbReserved; };
struct CvrStep;
struct CvrPart;
class Caviar;
class Palette {
public:
    int set_rgbquad(const RGBQUAD *,int,int);
};
#define private public
/* PRODUCT_DECLARATIONS */
#undef private
#undef nullptr
#undef MEASURED

static int palette_calls,first_seen,count_seen,seed_effects,palette_result=23;
static unsigned char colours_seen[256][3];
int Palette::set_rgbquad(const RGBQUAD *colours,int first,int count) {
    ++palette_calls;first_seen=first;count_seen=count;++seed_effects;
    // The existing callee reads only these channels, not rgbReserved.
    for(int i=0;i<count;++i) {
        colours_seen[i][0]=colours[i].rgbBlue;
        colours_seen[i][1]=colours[i].rgbGreen;
        colours_seen[i][2]=colours[i].rgbRed;
    }
    return palette_result;
}
static int free_calls;
static void *freed_pointer;
static CaviarData *free_callback_owner;
static CvrRecord *free_callback_new_record;
static void controlled_free(void *allocation) {
    ++free_calls;freed_pointer=allocation;
    if(free_callback_owner) free_callback_owner->record_=free_callback_new_record;
    free(allocation);
}
// Unrelated existing full-record destructor implementation is outside these
// bodies' control; keep test fixtures alive without introducing record cleanup.
void CaviarData::close() { }
#define free controlled_free
/* PRODUCT_CAVIAR_BODIES */
#undef free
/* PRODUCT_POPUP_BODIES */

int main() {
    assert(PopupSavedLocX==8192&&PopupSavedLocY==8192);
    CaviarData data;
    Palette palette;
    assert(data.apply_palette(0,31)==7&&palette_calls==0);
    assert(data.apply_palette(&palette,-31)==7&&palette_calls==0);
    CvrRecord record=CvrRecord();data.record_=&record;
    assert(data.apply_palette(0,31)==16&&palette_calls==0);
    assert(data.apply_palette(&palette,31)==16&&palette_calls==0);
    unsigned char table[770];
    for(unsigned i=0;i<sizeof(table);++i) table[i]=static_cast<unsigned char>(i*7+3);
    table[0]=2;table[1]=5;record.palette_=table;
    assert(data.apply_palette(&palette,INT_MIN)==23);
    assert(first_seen==2&&count_seen==3&&palette_calls==1&&seed_effects==1);
    for(int i=0;i<3;++i) {
        assert(colours_seen[i][0]==table[2*3+4+i*3]);
        assert(colours_seen[i][1]==table[2*3+3+i*3]);
        assert(colours_seen[i][2]==table[2*3+2+i*3]);
    }
    table[0]=200;table[1]=201;
    assert(data.apply_palette(&palette,INT_MAX)==23&&first_seen==200&&count_seen==1);
    assert(colours_seen[0][0]==table[604]&&colours_seen[0][1]==table[603]&&colours_seen[0][2]==table[602]);
    table[0]=5;table[1]=5;
    assert(data.apply_palette(&palette,0)==23&&count_seen==0&&seed_effects==3);
    table[0]=5;table[1]=3;
    assert(data.apply_palette(&palette,0)==23&&count_seen==-2&&seed_effects==4);
    data.record_=0;data.release_palette_chunk();assert(free_calls==0);
    data.record_=&record;data.release_palette_chunk();assert(free_calls==0&&data.record_==&record&&record.palette_==table);
    record.field_18_=static_cast<uint8_t *>(malloc(8));assert(record.field_18_!=0);
    void *original_allocation=record.field_18_;
    data.release_palette_chunk();
    assert(free_calls==1&&freed_pointer==original_allocation&&record.field_18_==0);
    assert(data.record_==&record&&record.palette_==table);
    CvrRecord replacement=CvrRecord();unsigned char retained_chunk[4];
    replacement.field_18_=retained_chunk;
    record.field_18_=static_cast<uint8_t *>(malloc(8));assert(record.field_18_!=0);
    original_allocation=record.field_18_;
    free_callback_owner=&data;free_callback_new_record=&replacement;
    data.release_palette_chunk();
    assert(free_calls==2&&freed_pointer==original_allocation&&data.record_==&replacement);
    assert(replacement.field_18_==0&&record.field_18_==original_allocation);
    assert(record.palette_==table);
    assert(set_popup_saved_location(INT_MIN,INT_MAX)==INT_MIN);
    assert(PopupSavedLocX==INT_MIN&&PopupSavedLocY==INT_MAX);
    assert(set_popup_saved_location(0x2000,-99)==0x2000);
    assert(PopupSavedLocX==0x2000&&PopupSavedLocY==-99);
    return 0;
}

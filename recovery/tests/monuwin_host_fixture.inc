// Service doubles and compact regression contracts; no recovered MonuWin bodies.
#include <array>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#define __cdecl
using LPSTR = char *;
struct RECT { int left, top, right, bottom; };
struct Heap {};
Heap heap_sentinel;
Heap *StringAllocationHeap = &heap_sentinel;
struct Call {
    std::string name, text;
    const void *self = nullptr, *pointer = nullptr;
    std::vector<int> args;
};
std::vector<Call> calls;
std::map<const void *, int> ids;
int pcx_calls, pcx_fail_at, flic_result, spot_position, spot_type;
int live_buffers, live_sprites, live_spots, live_buttons, live_palettes;
int flic_closes, GenderDefault, PluralityDefault;
void check(bool ok, const char *message) {
    if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
void record(const char *name, const void *self = nullptr,
            std::initializer_list<int> args = {}, const char *text = "",
            const void *pointer = nullptr) {
    calls.push_back(Call{name, text, self, pointer, args});
}
std::vector<Call> named(const char *name) {
    std::vector<Call> found;
    for (const auto &c : calls) if (c.name == name) found.push_back(c);
    return found;
}
int identity(const void *pointer) {
    auto found = ids.find(pointer);
    return found == ids.end() ? -1 : found->second;
}
bool rect_equal(const RECT &a, const RECT &b) {
    return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
char *_itoa(int value, char *buffer, int radix) {
    check(radix == 10, "decimal resource selector");
    std::sprintf(buffer, "%d", value); return buffer;
}
struct Font { int line_advance() const { return 20; } } font;
struct BoxSpriteParams {};
BoxSpriteParams InterfaceBoxStyles[51];
struct Buffer {
    RECT rect1_ = {3,4,99,109}, rect2_ = {0,0,800,600};
    Buffer() { ++live_buffers; record("buffer_ctor", this); }
    virtual ~Buffer() { --live_buffers; record("buffer_dtor", this); }
    const RECT &current_clip() const { return rect1_; }
    void restore_clip() { set_clip(&rect2_); }
    void set_clip(RECT *rect) { rect1_ = *rect; record("clip", this, {rect->left,rect->top,rect->right,rect->bottom}); }
    void init(int w,int h,int flags,void *owner) { record("buffer_init",this,{w,h,flags},"",owner); }
    void fill(int color) { record("fill",this,{color}); }
    int load_pcx(const char *path,void *palette,int first,int count) {
        record("load",this,{first,count},path,palette);
        return ++pcx_calls == pcx_fail_at;
    }
    int copy(Buffer *canvas,int sx,int sy,int dx,int dy,int w,int h) {
        check(StringAllocationHeap == nullptr, "painting runs under StringList heap scope");
        check(rect_equal(canvas->rect1_,canvas->rect2_), "image copy uses restored full clip");
        record("copy",this,{sx,sy,dx,dy,w,h},"",canvas); return 0;
    }
    void set_font(Font *f,int a,int b,int c) { record("font",this,{a,b,c},"",f); }
    void set_text_color(int a,int b,int c,int d) { record("color",this,{a,b,c,d}); }
    void write_strings(void *list,int x,int y,int width,int mode) {
        check(StringAllocationHeap == nullptr, "text uses StringList heap scope");
        record("write",this,{x,y,width,mode},"",list);
    }
};
struct GraphicWin : Buffer {
    virtual ~GraphicWin() {}
    virtual void on_redraw() {}
    virtual void on_button_clicked(int) {}
    virtual void on_mouse_move(int,int) {}
    virtual void on_left_click(int,int) {}
    virtual int on_key_click(int,int) { return 0; }
    int init(int x,int y,int w,int h,const char *title,int flags,void*,void*,void*) {
        record("window_init",this,{x,y,w,h,flags},title); return 0;
    }
    void redraw() { record("redraw",this); }
    void release_modal() { record("modal",this); }
    void close() { record("close",this); }
    void soft_update() { record("soft",this); }
};
struct FlatButton : GraphicWin {
    FlatButton() { ++live_buttons; }
    ~FlatButton() { --live_buttons; }
    void init(const char *text,int id,int x,int y,int w,int h,GraphicWin *owner,int flags) {
        record("button_init",this,{id,x,y,w,h,flags},text,owner);
    }
    void set_border_styles(BoxSpriteParams *a,BoxSpriteParams *b,BoxSpriteParams *c) {
        check(a==&InterfaceBoxStyles[47] && b==&InterfaceBoxStyles[46] && c==&InterfaceBoxStyles[45], "typed button border-style owners");
    }
    void hide() { record("hide",this); }
    void show(int value) { record("show",this,{value}); }
};
struct Spot {
    Spot() { ++live_spots; }
    ~Spot() { --live_spots; }
    void init(int count) { record("spot_init",this,{count}); }
    void kill_type(int type) { record("kill",this,{type}); }
    int check(int x,int y,int *position,int *type) {
        record("spot_check",this,{x,y}); *position=spot_position; *type=spot_type;
        return spot_position == -1 ? -1 : 1;
    }
    int add(int position,int type,int x,int y,int w,int h) {
        record("add",this,{position,type,x,y,w,h}); return 0;
    }
};
struct Sprite {
    // Only bounded representation copying in the real body uses these words.
    uint32_t words_[11] = {0,0,9};
    Sprite() { ++live_sprites; }
    ~Sprite() { --live_sprites; }
    int transparent_index() const { return words_[2]&255; }
    void extract(Buffer *source,int transparent,int x,int y,int w,int h,void *palette) {
        check(palette == nullptr, "extract uses no external palette");
        record("extract",this,{transparent,x,y,w,h},"",source);
    }
    void draw(Buffer *canvas,int transparent,int x,int y) { record("draw",this,{transparent,x,y},"",canvas); }
};
struct Palette { Palette() { ++live_palettes; } ~Palette() { --live_palettes; } };
struct Flic {
    Buffer buffer_; Palette palette_; int field_5A4_ = 0;
    ~Flic() { close(); } // Real Flic destructor's sole close, then member teardown.
    int init(Buffer *target,char *path,int x,int y,int transparent) {
        record("flic_init",this,{x,y,transparent},path,target); return flic_result;
    }
__FLIC_SETTER__
    void decode_stored_frame() { record("decode",this,{field_5A4_,0,0}); }
    void close() { ++flic_closes; record("flic_close",this); }
};
struct StringList {
    Heap *saved_;
    StringList() : saved_(StringAllocationHeap) { StringAllocationHeap=nullptr; record("list_ctor",this); }
    ~StringList() { StringAllocationHeap=saved_; record("list_dtor",this); }
    void load(char *file,char *section,int clear,void *callback) {
        check(std::strcmp(file,"monument")==0 && clear==1 && callback==nullptr, "description loader contract");
        record("section",this,{clear},section);
    }
};
struct ReportInfo { int field_1E230_ = 0; Font *field_1E184_ = &font; } ReportIfGlobal;
struct ConsoleInfo { int faction_id_ = 3; } ConsoleGlobal;
struct LockWord { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; };
struct NetDaemonDouble { LockWord lock_state_[24]; } NetDaemonNet;
struct PlayerInfo {
    int is_leader_female, noun_gender, is_noun_plural;
    char filename[8], title_leader[32], name_leader[32], noun_faction[32], adj_name_faction[32];
} Players[8];
__LAYOUT__
MonumentFaction FactionMonuments[8];
struct FacilityInfo { void *name; } Facility[134];
enum { MaxPlayerNum=8, FacilitySPStart=70 };
int WinScreenWidth=1280, WinScreenHeight=960, WinFillColour=9, MonumentWindowCount;
char StringTemp[1024], technology_name[]="technology", project_name[]="project";
struct TutInfo { Font font1_; } TutWinBaseWindow;
const char *label_get(int index) { check(index==0xA64/4 || index==0xB60/4, "caption label index"); return "caption"; }
int parse_num(int slot,int value) { record("number",nullptr,{slot,value}); return 0; }
int parse_says(int slot,const char *text,int gender,int plurality) {
    record("text",nullptr,{slot,gender,plurality,GenderDefault,PluralityDefault},text,text); return 0;
}
const char *tech_name(int id,int category) { record("technology",nullptr,{id,category}); return technology_name; }
struct StringTableDouble { const char *get(int id) { record("string_get",nullptr,{id}); return project_name; } } StringTable;
// Access policy only is projected for test setup; full live member declarations remain intact.
#define private public
__DECLARATION__
#undef private
__METHODS__

void reset(MonuWin &window) {
    calls.clear(); ids.clear(); pcx_calls=0; pcx_fail_at=-1; flic_result=0; flic_closes=0;
    spot_position=4; spot_type=0; GenderDefault=91; PluralityDefault=92;
    StringAllocationHeap=&heap_sentinel;
    ConsoleGlobal.faction_id_=3; ReportIfGlobal.field_1E230_=0; MonumentWindowCount=4;
    NetDaemonNet={}; std::memset(FactionMonuments,0,sizeof FactionMonuments);
    for (int f=0;f<8;++f) {
        auto &p=Players[f]; p.is_leader_female=10+f; p.noun_gender=30+f; p.is_noun_plural=50+f;
        std::sprintf(p.filename,"f%d",f); std::sprintf(p.title_leader,"title%d",f);
        std::sprintf(p.name_leader,"name%d",f); std::sprintf(p.noun_faction,"noun%d",f); std::sprintf(p.adj_name_faction,"adjective%d",f);
        auto &m=FactionMonuments[f]; m.latest_type_=4;
        std::sprintf(m.founding_base_name_,"founding%d",f); std::sprintf(m.facility_base_name_,"facility%d",f); std::sprintf(m.conquered_base_name_,"conquered%d",f);
        for (int t=0;t<21;++t) {
            auto &r=m.records_[t]; r.year_=2100+100*f+t; r.word_8_=2; r.word_4_=7;
            r.owner_faction_=5; r.achieved_=1; r.word_14_=1;
        }
    }
    for (int i=0;i<134;++i) Facility[i].name=reinterpret_cast<void *>(static_cast<intptr_t>(1000+i));
    int *fields[]={&window.origin_x_,&window.origin_y_,&window.window_width_,&window.window_height_,&window.client_left_,&window.client_top_,&window.client_right_,&window.client_bottom_,&window.field_A34_,&window.field_A38_,&window.field_A3C_,&window.field_A40_,&window.frame_or_page_,&window.field_A48_,&window.selection_pending_,&window.preview_faction_or_page_,&window.selected_monument_type_};
    for (auto *p:fields) *p=0;
    window.preview_faction_or_page_=3; window.selected_monument_type_=4;
    window.rect1_={3,4,99,109}; window.rect2_={0,0,800,600};
    ids[&window]=0; ids[static_cast<Buffer *>(&window)]=1; ids[&window.spot_]=2;
    for (int i=0;i<2;++i) { ids[&window.buttons_[i]]=10+i; ids[&window.buffers_[i]]=20+i; }
    auto sprites=[](Sprite *p,int count,int base) { for(int i=0;i<count;++i) ids[p+i]=base+i; };
    sprites(window.sprites_2C0C_,15,100); sprites(window.sprites_2EA0_,15,200);
    sprites(window.sprites_3244_,6,300); sprites(window.sprites_33AC_,7,400);
    sprites(window.sprites_34E0_,7,500); sprites(window.sprites_3614_,2,600);
    auto rects=[](RECT *p,int count) { for(int i=0;i<count;++i) p[i]={10+i*3,20+i*4,54+i*3,62+i*4}; };
    rects(window.rects_3134_,17); rects(window.icon_rects_,6); rects(window.rects_366C_,7);
}
void select(MonuWin &window,int type,int first=1) {
    window.selection_pending_=1; FactionMonuments[3].latest_type_=type;
    if(type<21) FactionMonuments[3].records_[type].word_14_=first;
    window.on_redraw();
    check(StringAllocationHeap==&heap_sentinel,"StringList restores borrowed heap");
    check(named("list_ctor").size()==1 && named("list_dtor").size()==1,"one live StringList scope");
}
const Call &text_slot(int slot) {
    for (const auto &c:calls) if(c.name=="text" && c.args[0]==slot) return c;
    check(false,"missing parser slot"); return calls.front();
}
bool has_number(int slot,int value) {
    for (const auto &c:calls) if(c.name=="number" && c.args==std::vector<int>{slot,value}) return true;
    return false;
}
void init_contracts(MonuWin &window) {
    for(int fs:{0,1}) for(int faction:{0,3,7}) for(int kind:{0,4,5,14,21}) {
        reset(window); ReportIfGlobal.field_1E230_=fs; ConsoleGlobal.faction_id_=faction; MonumentWindowCount=kind;
        int before_buffers=live_buffers; check(window.init()==0,"successful resources initialize");
        check(flic_closes==1 && live_buffers==before_buffers && live_palettes==0,"single automatic Flic close and member lifetimes");
        check(pcx_calls==9,"all nine PCX resources loaded");
        check(window.origin_x_==1 && window.origin_y_==431,"sprite extraction cursors retained");
        auto decoded=named("decode"); check(decoded.size()==static_cast<size_t>(kind==14?13:kind),"animation frame count");
        for(auto &c:decoded) check(c.args==std::vector<int>{10,0,0},"palette offset applied before decode");
        auto extracts=named("extract"); check(extracts.size()==static_cast<size_t>(faction==0?50:52),"scalar art extracted only for local sheet");
        for(const auto &c:extracts) {
            int owner=identity(c.self);
            if(owner>=400 && owner<407) check(c.args==std::vector<int>{9,10,178,58,50},"small faction marks use33AC owners");
            if(owner>=500 && owner<507) check(c.args==std::vector<int>{9,164,109,160,97},"large faction portraits use34E0 owners");
        }
        std::vector<int> icon_order;
        for(const auto &c:extracts) if(identity(c.self)>=100 && identity(c.self)<215) icon_order.push_back(identity(c.self));
        check(icon_order.size()==30,"both complete icon arrays");
        for(int i=0;i<15;++i) check(icon_order[i]==100+i && icon_order[i+15]==200+i,"separate small and large icon extraction loops");
        int offsetx=fs?112:0,offsety=fs?84:0;
        check(window.rects_3134_[0].left==319+offsetx && window.rects_3134_[0].top==202+offsety,"translated primary rectangle");
        check(window.rects_366C_[6].left==(kind==14?328:364)+offsetx,"monument14 special rectangle");
        auto loads=named("load"); check(loads.front().text==(kind<5?"monzoom1.pcx":"monzoom2.pcx"),"zoom-sheet selection");
        auto hide=named("hide"); check(hide.size()==2 && identity(hide[0].self)==10 && identity(hide[1].self)==11,"init hides both buttons");
    }
    for(int failure=-1;failure<=9;++failure) {
        if(failure==0) continue;
        reset(window); if(failure==-1) flic_result=1; else pcx_fail_at=failure;
        int before=live_buffers; check(window.init()==1,"resource failure returns1");
        check(flic_closes==1 && live_buffers==before && live_palettes==0,"resource failure closes through sole automatic lifetime");
        check(named("button_init").empty(),"failed resources do not initialize buttons");
    }
}
void callback_contracts(MonuWin &window) {
    reset(window); spot_position=7; spot_type=0; window.on_left_click(101,202);
    check(FactionMonuments[3].latest_type_==7 && window.selected_monument_type_==7 && window.selection_pending_==1,"click consumes spot POSITION not type");
    check(named("redraw").size()==1,"click redraws selected panel");
    calls.clear(); window.on_button_clicked(998); check(calls.empty(),"other button ids ignored");
    window.on_button_clicked(999); check(window.selection_pending_==0 && named("redraw").size()==1,"button exits selected panel first");
    calls.clear(); window.on_button_clicked(999); check(named("modal").size()==1 && named("close").size()==1,"button releases modal then closes");
    reset(window); window.selected_monument_type_=14; spot_position=69; window.on_left_click(3,4);
    check(FactionMonuments[3].latest_type_==14,"primary spot resolves current selection");
    for(int gate=0;gate<3;++gate) {
        reset(window); if(gate==0) NetDaemonNet.lock_state_[0].trailing_words_[7]=1u<<3;
        if(gate==1) NetDaemonNet.lock_state_[2].payload_.flags=1;
        if(gate==2) NetDaemonNet.lock_state_[0].leading_words_[3]=1;
        window.on_left_click(3,4); window.on_mouse_move(3,4); check(calls.empty(),"all three network gates suppress callbacks");
    }
    for(int type:{0,4,13,15,20,69,-1}) {
        reset(window); spot_position=type; window.on_mouse_move(10,20);
        if(type==69 || type==-1) check(named("draw").empty() && named("soft").empty(),"hover ignores primary/no-hit");
        else { auto draw=named("draw"); check(draw.size()==1 && named("soft").size()==1,"hover updates one preview");
            int expected=type==13?600:type<15?200+type:501;
            check(identity(draw[0].self)==expected && window.selected_monument_type_==type,"hover selects typed preview sprite"); }
    }
    reset(window); window.selection_pending_=1; check(window.on_key_click(999,777)==1 && window.selection_pending_==0,"key exits selected panel");
    calls.clear(); check(window.on_key_click(0,0)==1 && named("modal").size()==1 && named("close").size()==1,"key closes unselected window");
    reset(window); window.close(); check(named("modal").size()==1 && named("close").size()==1,"explicit owner close lifecycle");
}
void paint_contracts(MonuWin &window) {
    for(int type=0;type<=21;++type) {
        reset(window); select(window,type);
        auto copy=named("copy"); check(copy.size()==2,"selected panel paints background and image");
        check(identity(copy[0].self)==21,"selected background uses second owned Buffer");
        check(copy[1].args==std::vector<int>{type==13 || type>=15?341:0,type==13?11:type>=15?372:0,16,28,600,360},"selected image source and destination coordinates");
        auto hide=named("hide"),show=named("show");
        check(hide.size()==1 && identity(hide[0].self)==10 && show.size()==1 && identity(show[0].self)==11 && show[0].args[0]==0,"selected panel hides0 and shows1");
        if(type==21) check(named("section").empty(),"type21 skips parser loader while painting common tail");
    }
    reset(window); for(auto &r:FactionMonuments[3].records_) r.achieved_=0;
    FactionMonuments[3].records_[4].achieved_=1; FactionMonuments[3].records_[4].year_=2150;
    FactionMonuments[3].records_[8].achieved_=1; FactionMonuments[3].records_[8].year_=2140;
    window.on_redraw(); auto add=named("add");
    check(add.size()==3 && add[0].args[0]==8 && add[0].args[2]==19 && add[1].args[0]==4 && add[1].args[2]==22,"sort fills holes and orders by year");
    check(add.back().args[0]==69 && add.back().args[1]==0,"primary Spot position69/type0");
    for(int mask=0;mask<64;++mask) {
        reset(window); for(int t=0;t<21;++t) FactionMonuments[3].records_[t].achieved_=t>=15 && ((mask>>(t-15))&1);
        window.on_redraw(); add=named("add");
        check(!add.empty() && add.front().args[0]==69,"victory-only panel keeps primary spot");
        // Compact reviewed regression contract, not a claim of native execution.
        int count=0; for(int i=0;i<6;++i) count+=(mask>>i)&1;
        int selector=15,slot=0;
        std::vector<int> expected;
        for(int i=0;i<count;++i) { check(selector<=20,"stable masks remain in declared record range"); if((mask>>(selector-15))&1) expected.push_back(selector++); else selector+=2; }
        std::vector<int> actual; for(const auto &c:add) if(c.args[0]!=69) actual.push_back(c.args[0]);
        check(actual==expected,"sparse victory consumes count slots with double absent-selector advance");
        for(const auto &c:add) if(c.args[0]!=69) ++slot;
        check(slot==static_cast<int>(expected.size()),"no synthesized skipped-victory spots");
    }
}
void parser_contracts(MonuWin &window) {
    const char *first_sections[]={"FOUNDCOLONY1ST","TECHDISCOVER1ST","SECRETS1STC","PROTO1ST","FACILITY1ST","PROJECT1ST","BATTLEWIN1ST","BASECAP1ST","NAVY1ST","AIR1ST","NATIVE1ST","SPACE1ST","PRESERVE1ST","WINCO-OPUNIFY","WINCO-OPASCENT","KILLFACTION","KILLFACTION","KILLFACTION","KILLFACTION","KILLFACTION","KILLFACTION"};
    const char *repeat_sections[]={"FOUNDCOLONY2ND","TECHDISCOVER2ND","SECRETS2NDC","PROTO2ND","FACILITY2ND","PROJECT2ND","BATTLEWIN2ND","BASECAP2ND","NAVY2ND","AIR2ND","NATIVE2ND","SPACE2ND","PRESERVE2ND","WINCO-OPUNIFY","WINCO-OPASCENT","KILLFACTION","KILLFACTION","KILLFACTION","KILLFACTION","KILLFACTION","KILLFACTION"};
    for(int type=0;type<21;++type) for(int first:{0,1}) {
        reset(window); select(window,type,first); auto section=named("section");
        check(section.size()==1 && section[0].text==(first?first_sections[type]:repeat_sections[type]),"all21 parser section branches");
        check(has_number(0,2400+type),"current achievement year");
        if(!first && type<=12) check(has_number(1,2600+type),"prior achievement year from OWNER");
    }
    reset(window); select(window,1,0); const auto &noun=text_slot(2);
    check(noun.pointer==Players[5].noun_faction && noun.args==std::vector<int>{2,33,53,35,55},"owner noun pointer with current explicit grammar and owner globals");
    reset(window); select(window,0,0); const auto &title=text_slot(4);
    check(title.pointer==Players[5].title_leader && title.args==std::vector<int>{4,13,-1,15,0},"owner leader text/current explicit gender/owner default");
    for(int first:{0,1}) for(int p8:{-1,2}) for(int p4:{-1,7}) {
        reset(window); auto &r=FactionMonuments[3].records_[2]; r.word_8_=p8; r.word_4_=p4; select(window,2,first);
        char section[32]; std::sprintf(section,"SECRETS%s%c",first?"1ST":"2ND",p8==-1?'A':p4==-1?'B':'C');
        check(named("section")[0].text==section,"secret sentinel section precedence");
        check(!has_number(first?2:3,7) || p8!=-1,"absent first secret suppresses second payload");
    }
    reset(window); select(window,6,1); check(named("number").size()==1,"first battle has no prior-owner year");
    reset(window); select(window,7,0);
    check(text_slot(2).pointer==FactionMonuments[3].conquered_base_name_ && text_slot(4).pointer==Players[5].title_leader,"repeat conquest keeps current buffer and owner leader");
    for(int type:{13,14}) {
        reset(window); select(window,type); check(text_slot(2).pointer==Players[2].noun_faction && text_slot(3).pointer==Players[2].noun_faction,"co-op noun slots share counterpart text");
        reset(window); FactionMonuments[3].records_[type].word_8_=-1; select(window,type);
        check(named("text").size()==3 && named("section")[0].text==(type==13?"WINUNIFY":"WINTRANSCEND"),"single-faction victory parser has only three slots");
        if(type==14) check(text_slot(2).pointer==Players[3].noun_faction,"solitary transcend uses current noun");
    }
}
int main() {
    static_assert(sizeof(MonumentRecord)==0x38,"live record extent");
    static_assert(sizeof(MonumentFaction)==0x4F4,"live faction extent");
    static_assert(offsetof(MonumentFaction,latest_type_)==0x498,"live selector owner");
    {
        MonuWin window;
        check(live_sprites==52 && live_spots==1 && live_buttons==2,"all declared owner members constructed");
        init_contracts(window); callback_contracts(window); paint_contracts(window); parser_contracts(window);
    }
    check(live_sprites==0 && live_spots==0 && live_buttons==0 && live_buffers==0 && live_palettes==0,"all real owner members torn down automatically");
    std::cout << "PASS MonuWin host contracts\n";
}

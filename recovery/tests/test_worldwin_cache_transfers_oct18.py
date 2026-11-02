"""Whole actual WorldWin redraw with pixel-bearing cache/copy stand-ins.

Host controls verify bounded C++ behavior, not VC6 ABI or live game acceptance.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from test_caviar_scene_product_source import extract_definition

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"

HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <stdint.h>
static int MapWinCaptionScale=0,MapWinCaptionDivisor=1;
static int MapLongitudeBounds=8,MapLatitudeBounds=8;
struct Palette {};
static Palette palette;
static Palette *PaletteActive=&palette;
static std::vector<std::string> events;
struct WorldWin;
static WorldWin *active;
static bool callback_mutation=false;
static bool forbid_stretch=false;
static int result_code=0;
static void require(bool ok,const char *name) {if(!ok){std::fprintf(stderr,"witness: %s\n",name);std::exit(1);}}
struct Header { int biWidth,biHeight; };
struct Dib { Header bmiHeader; };
struct Copy { int source,destination,sx,sy,sw,sh,dx,dy,dw,dh;bool scaled; };
static std::vector<Copy> copies;
struct Buffer {
    Dib dib_;std::vector<int> pixels;int identity;
    Buffer(int id=0):identity(id){dib_.bmiHeader.biWidth=0;dib_.bmiHeader.biHeight=0;}
    int width()const{return dib_.bmiHeader.biWidth;} int height()const{return -dib_.bmiHeader.biHeight;}
    void allocate(int w,int h,int value){dib_.bmiHeader.biWidth=w;dib_.bmiHeader.biHeight=-h;pixels.assign(w*h,value);}
    int init(int w,int h,int a,int b){require(a==0&&b==0,"init-arguments");events.push_back(identity==2?"init2":"init1");allocate(w,h,0);return result_code;}
    void sync_to_palette(Palette *p){require(p==PaletteActive,"palette-provider");events.push_back(identity==2?"palette2":"palette1");}
    int fill(int value){events.push_back("fill-map");std::fill(pixels.begin(),pixels.end(),value);return result_code;}
    int copy(Buffer *destination,int sx,int sy,int dx,int dy,int w,int h) {
        Copy c={identity,destination->identity,sx,sy,w,h,dx,dy,w,h,false};copies.push_back(c);events.push_back("copy");
        std::vector<int> snapshot=pixels;
        for(int y=0;y<h;++y)for(int x=0;x<w;++x)
            if(sx+x>=0&&sx+x<width()&&sy+y>=0&&sy+y<height()&&dx+x>=0&&dx+x<destination->width()&&dy+y>=0&&dy+y<destination->height())
                destination->pixels[(dy+y)*destination->width()+dx+x]=snapshot[(sy+y)*width()+sx+x];
        return result_code;
    }
    int copy(Buffer *destination,int sx,int sy,int sw,int sh,int dx,int dy,int dw,int dh) {
        require(!forbid_stretch,"signed-dimension-guard");
        Copy c={identity,destination->identity,sx,sy,sw,sh,dx,dy,dw,dh,true};copies.push_back(c);events.push_back("stretch-copy");
        std::vector<int> snapshot=pixels;
        for(int y=0;y<dh;++y)for(int x=0;x<dw;++x)
            destination->pixels[(dy+y)*destination->width()+dx+x]=snapshot[(sy+y*sh/dh)*width()+sx+x*sw/dw];
        return result_code;
    }
    int get_hdc(){events.push_back("get-hdc");return 31;}
    void release_hdc(int mode){require(mode==1,"release-hdc-mode");events.push_back("release-hdc");}
};
struct GraphicWin:Buffer {GraphicWin(int id=0):Buffer(id){} };
struct MapWin:GraphicWin {
    int field_1DDAC_,field_1DDB0_,center_x_,zoom_offset_,center_y_;unsigned field_1DD70_;
    MapWin():GraphicWin(3),field_1DDAC_(19),field_1DDB0_(23),center_x_(0),zoom_offset_(0),center_y_(0),field_1DD70_(0){}
    virtual int resize(int w,int h,int repaint){require(repaint==1,"map-resize-mode");events.push_back("map-resize");allocate(w,h,44);return result_code;}
    int draw_map(int mode);
    int gen_bases_only(){require(field_1DD70_==0x80000080U,"base-draw-flags");events.push_back("bases");pixels[1]=77;return result_code;}
};
struct WorldWin:GraphicWin {
    unsigned field_239BC_;
    /* ACTUAL_DIMENSION_MEMBERS */
    int field_239C4_,field_239C8_;
    MapWin mapWin_;Buffer buffer1_,buffer2_;
    WorldWin():GraphicWin(4),field_239BC_(0),field_A14_(4),field_A18_(2),field_239C4_(-99),field_239C8_(-99),buffer1_(1),buffer2_(2) {
        mapWin_.allocate(4,2,44);buffer1_.allocate(4,2,22);buffer2_.allocate(4,2,33);allocate(40,16,55);
    }
    void draw_box(){events.push_back("outline");}
    void on_redraw();
};
static void set_dirty(){events.push_back("dirty");}
static void clear_dirty(){events.push_back("clean");}
static int SetStretchBltMode(int hdc,int mode){require(hdc==31&&mode==0,"original-stretch-mode-zero");events.push_back("stretch-mode");return result_code;}
int MapWin::draw_map(int mode){require(mode==0&&field_1DD70_==0x80000000U,"terrain-draw-flags");events.push_back("terrain");pixels[0]=11;pixels[pixels.size()-1]=12;if(callback_mutation)active->field_239BC_|=2;return result_code;}
/* ACTUAL_BODY */
static void expect_copy(const Copy &c,int source,int destination,int w,int h,bool scaled,int dx=0,int dy=0,int dw=0,int dh=0) {
    require(c.source==source&&c.destination==destination,"copy-source-destination");
    require(c.sx==0&&c.sy==0&&c.sw==w&&c.sh==h,"copy-source-extent");
    require(c.scaled==scaled&&c.dx==dx&&c.dy==dy,"copy-destination-origin");
    require(c.dw==(scaled?dw:w)&&c.dh==(scaled?dh:h),"copy-destination-extent");
}
static void exercise(int flags,bool stretched,bool fail_results) {
    WorldWin w;active=&w;events.clear();copies.clear();callback_mutation=false;result_code=fail_results?7:0;
    w.field_239BC_=flags;if(stretched)w.allocate(3,1,55);w.on_redraw();
    require(w.field_239BC_==0,"dirty-flags-cleared");require(events.back()=="outline","outline-last");
    int width=(flags&8)?32:4,height=(flags&8)?8:2;
    const bool terrain=(flags&13)!=0,bases=(flags&14)!=0,fill=(flags&12)!=0;
    std::vector<int> expected2(width*height,(flags&8)?0:33);
    if(terrain){if(fill)std::fill(expected2.begin(),expected2.end(),0);expected2[0]=11;expected2[expected2.size()-1]=12;}
    std::vector<int> expected1(width*height,(flags&8)?0:22);
    if(bases){expected1=expected2;expected1[1]=77;}
    require(w.buffer2_.pixels==expected2,"terrain-cache-pixels");
    require(w.buffer1_.pixels==expected1,"composite-cache-pixels");
    std::vector<int> expected_live(width*height,44);
    if(terrain)expected_live=expected2;
    if(bases)expected_live=expected1;
    require(w.mapWin_.pixels==expected_live,"restored-map-pixels");
    unsigned c=0;
    if(terrain){if(!fill)expect_copy(copies[c++],2,3,width,height,false);expect_copy(copies[c++],3,2,width,height,false);}
    if(bases){expect_copy(copies[c++],2,3,width,height,false);expect_copy(copies[c++],3,1,width,height,false);}
    int x=stretched?0:(w.width()-width)/2,y=stretched?0:(w.height()-height)/2;
    expect_copy(copies[c++],1,4,width,height,stretched,x,y,w.width(),w.height());require(c==copies.size(),"exact-copy-count");
    require(w.field_239C4_==x&&w.field_239C8_==y,"center-or-zero-offsets");
    if(stretched){for(int py=0;py<w.height();++py)for(int px=0;px<w.width();++px)
        require(w.pixels[py*w.width()+px]==expected1[(py*height/w.height())*width+px*width/w.width()],"stretched-cache-pixels");}
    else {for(int py=0;py<w.height();++py)for(int px=0;px<w.width();++px){int expected=55;
        if(px>=x&&px<x+width&&py>=y&&py<y+height)expected=expected1[(py-y)*width+px-x];
        require(w.pixels[py*w.width()+px]==expected,"centered-cache-pixels");}}
    if(flags&8){require(w.field_A14_==32&&w.field_A18_==8,"resize-dimensions");require(w.mapWin_.field_1DDAC_==0&&w.mapWin_.field_1DDB0_==0,"resize-view-reset");require(w.mapWin_.center_x_==4&&w.mapWin_.zoom_offset_==-14&&w.mapWin_.center_y_==4,"resize-view-configuration");}
    std::vector<std::string> expected;
    if(flags&8){expected.push_back("map-resize");expected.push_back("init2");expected.push_back("palette2");expected.push_back("init1");expected.push_back("palette1");}
    if(terrain){if(fill){expected.push_back("dirty");expected.push_back("fill-map");}else expected.push_back("copy");expected.push_back("terrain");expected.push_back("copy");expected.push_back("clean");}
    if(bases){expected.push_back("copy");expected.push_back("bases");expected.push_back("copy");}
    if(stretched){expected.push_back("get-hdc");expected.push_back("stretch-mode");expected.push_back("release-hdc");expected.push_back("stretch-copy");}else expected.push_back("copy");
    expected.push_back("outline");require(events==expected,"ordered-render-copy-events");
}
int main(){
    for(int flags=0;flags<16;++flags)for(int scaled=0;scaled<2;++scaled)for(int failures=0;failures<2;++failures)exercise(flags,scaled!=0,failures!=0);
    // draw_map is a callback boundary: live flags can request a bases pass.
    {WorldWin w;active=&w;events.clear();copies.clear();w.field_239BC_=1;callback_mutation=true;w.on_redraw();require(w.buffer1_.pixels[0]==11&&w.buffer1_.pixels[1]==77,"callback-live-flags");callback_mutation=false;}
    // Positive caption scaling and signed odd map extents use integer /2.
    {WorldWin w;active=&w;events.clear();copies.clear();MapWinCaptionScale=1;MapWinCaptionDivisor=5;MapLongitudeBounds=9;MapLatitudeBounds=9;w.field_239BC_=8;w.on_redraw();require(w.field_A14_==64&&w.field_A18_==16,"positive-caption-sizing");}
    // Native wrapped32-bit gaps are interpreted as signed before halving;
    // the vertical NEG follows that truncating division, rather than preceding it.
    {WorldWin w;active=&w;events.clear();copies.clear();w.field_A14_=static_cast<int32_t>(0x80000000u);w.field_A18_=static_cast<int32_t>(0x80000000u);forbid_stretch=true;w.on_redraw();forbid_stretch=false;
     require(w.field_239C4_==-1073741804,"wrapped-horizontal-center");
     require(w.field_239C8_==-1073741816,"wrapped-vertical-center");
     require(copies.size()==1&&!copies[0].scaled,"wrapped-centered-copy");}
}
'''


def _body():
    return extract_definition((ROOT/"src/worldwin.cpp").read_text(),"WorldWin::on_redraw")


def _dimension_members():
    header=(ROOT/"src/worldwin.h").read_text()
    declarations=[]
    for name in ("field_A14_","field_A18_"):
        matches=re.findall(r"^\s*(?:u?int32_t|int|unsigned)\s+"+name+r"\s*;",header,re.M)
        assert len(matches)==1
        declarations.append(matches[0].strip())
    return "\n".join(declarations)


def _run(tmp_path,compiler,body,members=None):
    cpp,exe=tmp_path/"world.cpp",tmp_path/"world"
    cpp.write_text(HARNESS.replace("/* ACTUAL_BODY */",body).replace("/* ACTUAL_DIMENSION_MEMBERS */",_dimension_members() if members is None else members))
    built=subprocess.run([compiler,"-std=c++98","-O0",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=40)
    assert built.returncode==0,built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10)


@pytest.mark.parametrize("name",("g++","clang++"))
def test_whole_actual_redraw_pixel_cache_transfers(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler:pytest.skip(name+" unavailable")
    result=_run(tmp_path,compiler,_body())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("damage,witness",(("terrain-post","terrain-cache-pixels"),("bases-post","composite-cache-pixels"),
    ("terrain-pre","terrain-cache-pixels"),("bases-pre","terrain-cache-pixels")))
def test_compiling_reversed_copy_damage_fails_pixel_witness(tmp_path,damage,witness):
    compiler=shutil.which("g++")or shutil.which("clang++")
    if not compiler:pytest.skip("host compiler unavailable")
    body=_body()
    if damage=="terrain-post":old="map_surface->copy(&buffer2_,";new="buffer2_.copy(map_surface,"
    elif damage=="bases-post":old="map_surface->copy(&buffer1_,";new="buffer1_.copy(map_surface,"
    else:
        old="buffer2_.copy(map_surface,";new="map_surface->copy(&buffer2_,"
    expected=2 if damage.endswith("pre")else 1
    assert body.count(old)==expected
    occurrence=body.index(old) if damage!="bases-pre"else body.rindex(old)
    body=body[:occurrence]+body[occurrence:].replace(old,new,1)
    result=_run(tmp_path,compiler,body)
    assert result.returncode!=0 and "witness: "+witness in result.stderr,result.stderr


@pytest.mark.parametrize("old,new,witness", (
    ("static_cast<int>(\n            static_cast<uint32_t>(client_width) - field_A14_) / 2",
     "(static_cast<uint32_t>(client_width) - field_A14_) / 2", "wrapped-horizontal-center"),
    ("-(static_cast<int>(\n            static_cast<uint32_t>(Buffer::dib_.bmiHeader.biHeight) + field_A18_) / 2)",
     "-(static_cast<uint32_t>(Buffer::dib_.bmiHeader.biHeight) + field_A18_) / 2", "wrapped-vertical-center"),
))
def test_center_signed_halving_mutants_are_detected(tmp_path,old,new,witness):
    compiler=shutil.which("g++")or shutil.which("clang++")
    if not compiler:pytest.skip("host compiler unavailable")
    body=_body()
    assert body.count(old)==1
    mutated=body.replace(old,new,1)
    assert mutated!=body
    result=_run(tmp_path,compiler,mutated)
    assert result.returncode!=0 and "witness: "+witness in result.stderr,result.stderr


def test_unsigned_dimension_member_damage_is_detected(tmp_path):
    compiler=shutil.which("g++")or shutil.which("clang++")
    if not compiler:pytest.skip("host compiler unavailable")
    members=_dimension_members()
    assert members.count("int32_t ")==2
    damaged=members.replace("int32_t ","uint32_t ")
    assert damaged!=members
    result=_run(tmp_path,compiler,_body(),damaged)
    assert result.returncode!=0 and "witness: signed-dimension-guard" in result.stderr,result.stderr


def test_pinned_full_redraw_copy_receivers_and_scaled_argument_anchors():
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),PIN);span=Span(0x4C3EC0,0x4C4227);decoded={};cursor=span.start
    for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(span),span.start):
        assert i.address==cursor;decoded[cursor]=(i.mnemonic,i.op_str);cursor+=i.size
    assert cursor==span.end
    expected={0x4C4056:("lea","ecx, [esi + 0xfa4]"),0x4C405C:("call","0x5dff00"),
        0x4C4089:("lea","ecx, [esi + 0xfa4]"),0x4C4090:("push","ecx"),0x4C4091:("lea","ecx, [eax + esi + 0x1970]"),0x4C4098:("call","0x5dff00"),
        0x4C40D8:("lea","ecx, [esi + 0xfa4]"),0x4C40DE:("call","0x5dff00"),
        0x4C410A:("lea","edx, [esi + 0xa1c]"),0x4C4111:("push","edx"),0x4C4112:("lea","ecx, [ecx + esi + 0x1970]"),0x4C4119:("call","0x5dff00"),
        0x4C419A:("lea","ecx, [esi + 0xa1c]"),0x4C41A0:("call","0x5dff00"),
        0x4C41E8:("push","ecx"),0x4C41EF:("push","edx"),0x4C41F8:("push","ecx"),0x4C41F9:("push","edx"),
        0x4C41FF:("lea","ecx, [esi + 0xa1c]"),0x4C4205:("call","0x5d9610")}
    for address,pair in expected.items():assert decoded[address]==pair,hex(address)
    # Independently anchor the copy API assumed by the pixel fixture: the
    # receiver is preserved in EDI, arg1 in ESI, and read clipping uses EDI's
    # bitmap width/height. This bounded prefix is not a full-copy equivalence
    # claim; the full WorldWin caller span above establishes the four roles.
    cursor=0x5DFF00
    copy_prefix={}
    for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(cursor,0x5DFF90)),cursor):
        assert i.address==cursor;copy_prefix[cursor]=(i.mnemonic,i.op_str);cursor+=i.size
    assert cursor==0x5DFF90
    for address,pair in {
        0x5DFF09:("mov","edi, ecx"),
        0x5DFF2A:("mov","esi, dword ptr [ebp + 8]"),
        0x5DFF6A:("mov","ecx, dword ptr [edi + 0x80]"),
        0x5DFF78:("mov","edx, dword ptr [edi + 0x84]"),
    }.items():assert copy_prefix[address]==pair,hex(address)

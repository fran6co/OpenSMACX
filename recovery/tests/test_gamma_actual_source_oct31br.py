"""Actual-source Gamma math-handler and UI callback sequencing controls.

Service doubles expose original reload/snapshot boundaries; host controls do not
prove x87 precision, the ordinary CRT pow implementation, or full UI/Palette ABI.
"""
from pathlib import Path
import shutil
import subprocess
import pytest
import test_gamma_return as returns

ROOT = Path(__file__).resolve().parents[2]


def body(signature):
    text = (ROOT / "src/gamma.cpp").read_text()
    start = text.index(signature)
    return text[start:text.index("\n}\n", start) + 3]


def run(tmp_path, compiler, text):
    executable = tmp_path / "gamma-control"
    source = executable.with_suffix(".cpp")
    source.write_text(text)
    cxx = shutil.which(compiler)
    if not cxx:
        pytest.skip(f"{compiler} unavailable")
    subprocess.run([cxx, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


PALETTE = r'''
#include <cassert>
#include <cmath>
#include <string>
#include <vector>
struct PALETTEENTRY { unsigned char peRed,peGreen,peBlue,peFlags; };
struct Palette;
void reseed(Palette *);
void publish(Palette *);
struct Palette {
 PALETTEENTRY entries_[256]{};
 void reseed(){::reseed(this);}
 void set(){publish(this);}
};
struct Scroll { int position() const {return 100;} };
std::vector<std::string> events;
std::vector<double> inputs,exponents;
const double GammaPositionScale=0.01, GammaComponentScale=1.0/235.0;
struct Gamma {
 Palette *target_palette_=nullptr; Palette palette_; Scroll scroll_; double gamma_=1;
 void adjust_pal(double); void adjust_palette();
 void on_redraw(){events.push_back("redraw");}
};
Gamma *active;
Palette original,alternate,third;
int mode;
double mock_pow(double value,double exponent) {
 inputs.push_back(value);exponents.push_back(exponent);
 if(inputs.size()==1 && mode==1){
  original.entries_[10].peGreen=235; original.entries_[10].peBlue=0;
  active->target_palette_=&alternate;
 }
 if(inputs.size()==1 && mode==2)active->gamma_=2;
 return value;
}
void reseed(Palette *palette){
 events.push_back("reseed");
 if(mode==1){assert(palette==&alternate);active->target_palette_=&third;}
 else assert(palette==&active->palette_);
}
void publish(Palette *palette){
 events.push_back("set");
 assert(palette==(mode==1?&third:&active->palette_));
}
#define pow mock_pow
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None, "interleaved_input", "cached_target", "stale_after_reseed"])
def test_palette_snapshots_inputs_then_reloads_target(tmp_path, compiler, mutation):
    source = body("void Gamma::adjust_pal(double gamma)")
    if mutation == "interleaved_input":
        old = '''        double red = entry.peRed * GammaComponentScale;
        double green = entry.peGreen * GammaComponentScale;
        double blue = entry.peBlue * GammaComponentScale;
        red = pow(red, inverse_gamma);
        green = pow(green, inverse_gamma);
        blue = pow(blue, inverse_gamma);'''
        assert source.count(old) == 1
        source = source.replace(old, '''        double red = pow(entry.peRed * GammaComponentScale, inverse_gamma);
        double green = pow(entry.peGreen * GammaComponentScale, inverse_gamma);
        double blue = pow(entry.peBlue * GammaComponentScale, inverse_gamma);''', 1)
    elif mutation == "cached_target":
        anchor = "    const double inverse_gamma = 1.0 / gamma;"
        assert source.count(anchor) == 1
        source = source.replace(anchor, "    Palette *cached = target_palette_;\n" + anchor, 1)
        source = source.replace("target_palette_->", "cached->")
    elif mutation == "stale_after_reseed":
        old = "    target_palette_->reseed();\n    target_palette_->set();"
        assert source.count(old) == 1
        source = source.replace(old, "    Palette *cached = target_palette_;\n    target_palette_->reseed();\n    cached->set();", 1)
    main = r'''
int main(){
 Gamma gamma;active=&gamma;mode=1;gamma.target_palette_=&original;
 for(int i=0;i<256;++i){original.entries_[i]={47,94,141,77};alternate.entries_[i]={17,18,19,77};}
 gamma.adjust_pal(1);
 assert(inputs.size()==236*3);
 assert(std::fabs(inputs[0]-0.2)<1e-12 && std::fabs(inputs[1]-0.4)<1e-12 && std::fabs(inputs[2]-0.6)<1e-12);
 assert(alternate.entries_[10].peRed==47 && alternate.entries_[10].peGreen==94 && alternate.entries_[10].peBlue==141);
 assert(alternate.entries_[9].peRed==17 && alternate.entries_[246].peRed==17);
 for(int i=0;i<256;++i)assert(alternate.entries_[i].peFlags==77);
 assert((events==std::vector<std::string>{"reseed","set"}));
}
'''
    result = run(tmp_path, compiler, PALETTE + body("void Gamma::adjust_palette()") + source + main)
    assert (result.returncode == 0) == (mutation is None), result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_ramp_bounds_flags_and_live_gamma_exponent(tmp_path, compiler):
    source = body("void Gamma::adjust_palette()")
    main = r'''
int main(){
 Gamma gamma;active=&gamma;mode=2;
 for(auto &entry:gamma.palette_.entries_)entry={41,42,43,77};
 gamma.adjust_palette();
 assert(inputs.size()==236 && exponents[0]==1 && exponents[1]==0.5);
 assert(inputs[0]==0 && inputs[235]==1);
 assert(gamma.palette_.entries_[10].peRed==0 && gamma.palette_.entries_[245].peBlue==235);
 assert(gamma.palette_.entries_[9].peRed==41 && gamma.palette_.entries_[246].peRed==41);
 for(auto &entry:gamma.palette_.entries_)assert(entry.peFlags==77);
 assert((events==std::vector<std::string>{"reseed","set","redraw"}));
}
'''
    assert run(tmp_path, compiler, PALETTE + body("void Gamma::adjust_pal(double gamma)") + source + main).returncode == 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [False, True])
def test_exec_reuses_screen_snapshot_across_font_callback(tmp_path, compiler, mutation):
    declaration, source = returns.actual_body()
    if mutation:
        old = "GraphicWin::init(0, 0, screen_width, screen_height, 0, 0,"
        assert source.count(old) == 1
        source = source.replace(old, "GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0,", 1)
    stubs = returns.STUBS.replace("struct Font { template<class... A> void init(A...) {} };", r'''
extern int WinScreenWidth,WinScreenHeight;
struct Font {template<class... A> void init(A...){WinScreenWidth=1200;WinScreenHeight=900;}};
''')
    stubs = stubs.replace("template<class... A> void init(A...) {}\n    void fill(int)",
                          "void init(int,int,int w,int h,int,int,int,int,int){assert(w==800&&h==600);}\n    void fill(int)", 1)
    assert "assert(w==800&&h==600)" in stubs
    main = r'''
int main(){Gamma gamma;Palette palette;modal_value=1;cleanup_value=1;gamma.exec(&palette,1,0);}
'''
    text = stubs.replace("DECLARATION", declaration) + source + main
    result = run(tmp_path, compiler, text)
    assert (result.returncode == 0) == (not mutation), result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [False, True])
def test_redraw_preserves_initial_nul_when_formatter_fails(tmp_path, compiler, mutation):
    source = body("void Gamma::on_redraw()")
    if mutation:
        assert source.count("    value_text[0] = 0;") == 1
        source = source.replace("    value_text[0] = 0;", "    value_text[0] = 'x';", 1)
    stubs = r'''
#include <cassert>
#include <string.h>
struct RECT {int left,top,right,bottom;};
struct Font{};
int lines,labels;
int mock_sprintf(char *text,const char *format,double){
 assert(text[0]==0 && strcmp(format,"%f")==0);
 return -1; // Leave the caller's original initial state untouched.
}
#define sprintf mock_sprintf
struct Buffer {
 void line(int,int,int,int,int){++lines;}
 void set_font(Font*,int,int,int){}
 void set_text_color(int,int,int,int){}
 void write_cent_l(char *text,RECT*,int len){assert(len==0&&text[0]==0);++labels;}
};
struct GraphicWin:Buffer{void fill(RECT*,int){}void soft_update(){}};
struct Gamma:GraphicWin{RECT rect1_{0,0,236,32};Font font_;double gamma_=1;void on_redraw();};
'''
    result = run(tmp_path, compiler, stubs + source +
                 'int main(){Gamma gamma;gamma.on_redraw();assert(lines==236&&labels==1);}')
    assert (result.returncode == 0) == (not mutation), result.stderr


def test_pinned_original_establishes_callback_boundaries():
    import os
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image.is_file():
        pytest.skip("pinned original unavailable")
    pe = PeImage(image.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    def decode(start, end):
        return {i.address:(i.mnemonic,i.op_str) for i in md.disasm(pe.read(Span(start,end)),start)}
    apply = decode(0x5c95e0, 0x5c97b1)
    assert apply[0x5c9614] == ("mov", "cl, byte ptr [esi + eax + 0x28]")
    assert apply[0x5c9618] == ("mov", "dl, byte ptr [esi + eax + 0x29]")
    assert apply[0x5c9627] == ("mov", "cl, byte ptr [esi + eax + 0x2a]")
    for address in (0x5c965c,0x5c9671,0x5c9686):
        assert apply[address] == ("call", "0x646a99")
    assert apply[0x5c9792] == ("mov", "ecx, dword ptr [edi + 0xa64]")
    assert apply[0x5c9798] == ("call", "0x5fe6d0")
    assert apply[0x5c979d] == ("mov", "ecx, dword ptr [edi + 0xa64]")
    assert apply[0x5c97a3] == ("call", "0x5fe460")
    exec_ = decode(0x5c8e60,0x5c905a)
    assert exec_[0x5c8e97] == ("mov", "edx, dword ptr [0x9b7b20]")
    assert exec_[0x5c8ea4] == ("mov", "ebx, dword ptr [0x9b7b1c]")
    assert exec_[0x5c8ecc] == ("mov", "dword ptr [ebp + 8], edx")
    assert exec_[0x5c8ed1] == ("mov", "dword ptr [ebp + 0x10], ebx")
    assert exec_[0x5c9036] == ("call", "0x618f40")
    assert exec_[0x5c903b] == ("mov", "edx, dword ptr [ebp + 8]")
    assert exec_[0x5c903e] == ("mov", "eax, dword ptr [ebp + 0x10]")
    redraw = decode(0x5c9420,0x5c951d)
    assert redraw[0x5c94e0] == ("mov", "byte ptr [ebp - 0x68], 0")
    assert redraw[0x5c94e4] == ("call", "0x646a25")

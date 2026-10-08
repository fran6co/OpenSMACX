"""Actual-source host controls for the typed SocialWin access group.

Bodies are extracted unchanged from src/socialwin.cpp. This is a bounded host
storage projection, not VC6 layout, lifetime, or whole-game proof.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(signature)


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static int current_visible, iface_set_count, iface_release_count;
struct RECT { int32_t left, top, right, bottom; };
struct Buffer {
    unsigned char prefix[0x30]; RECT rect2;
    unsigned char tail[0x588 - 0x40];
    int set_text_color(int,int,int,int) { return 0; }
    const RECT &clip_rect() const { return rect2; }
    int write_l(char *, RECT *, int) { return 0; }
    int write_right_l(char *, RECT *, int) { return 0; }
};
struct Win {
    unsigned char bytes[0x444];
    int is_visible() { return current_visible; }
};
struct GraphicWin : Win, Buffer { unsigned char tail[0xA14 - 0x9CC]; };
struct SubInterface { unsigned char bytes[8]; void set_iface_mode() { ++iface_set_count; } void release_iface_mode() { ++iface_release_count; } };
struct Sprite {
    unsigned char prefix[8]; unsigned char key; unsigned char p0[0x1C - 9];
    int height_value; unsigned char tail[0x2C - 0x20];
    int height() const { return height_value; }
    unsigned char transparent_index() const { return key; }
    int draw(Buffer *b, int key_arg, int x, int y, int n, int d) {
        drawn=this; drawn_buffer=b; drawn_key=key_arg; drawn_x=x; drawn_y=y;
        drawn_n=n; drawn_d=d; return 1;
    }
    static Sprite *drawn; static Buffer *drawn_buffer; static int drawn_key;
    static int drawn_x, drawn_y, drawn_n, drawn_d;
};
Sprite *Sprite::drawn; Buffer *Sprite::drawn_buffer; int Sprite::drawn_key;
int Sprite::drawn_x, Sprite::drawn_y, Sprite::drawn_n, Sprite::drawn_d;
struct CheckButton { unsigned char prefix[0xA14]; uint32_t checked_; unsigned char tail[0xA2C-0xA18]; };
struct FlatButton { unsigned char bytes[0xB4C]; };
struct SocialWin : GraphicWin, SubInterface {
    unsigned char pad_to_faction[0xCF4-0xA1C]; uint32_t factionID_;
    unsigned char pad_to_effect[0xD08-0xCF8]; int socEffect_[11];
    unsigned char pad_to_locks[0xD6C-0xD34]; CheckButton energyLockButtons_[3];
    unsigned char pad_to_pending[0x2C50-0x2BF0]; uint32_t pendingAlloc_[3];
    unsigned char pad_to_sprites[0x2E18-0x2C5C]; Sprite effectIcons_[90];
    unsigned char pad_to_buttons[0x6020-0x3D90]; FlatButton flatButtons_[35];
    int is_visible() { return Win::is_visible(); }
    void show(int);
    void hide();
    void allocate_energy(int, int);
    int on_button_text_draw(GraphicWin *, char *, int);
};
static_assert(sizeof(Win)==0x444, "Win fixture");
static_assert(sizeof(Buffer)==0x588, "Buffer fixture");
static_assert(__builtin_offsetof(Buffer, rect2)==0x30, "working rect in Buffer");
static_assert(__builtin_offsetof(SocialWin, factionID_)==0xCF4, "faction");
static_assert(__builtin_offsetof(SocialWin, energyLockButtons_)==0xD6C, "locks");
static_assert(__builtin_offsetof(SocialWin, pendingAlloc_)==0x2C50, "pending");
static_assert(__builtin_offsetof(SocialWin, effectIcons_)==0x2E18, "sprites");
static_assert(sizeof(Sprite)==0x2C, "Sprite stride");
static_assert(sizeof(CheckButton)==0xA2C, "CheckButton stride");
static_assert(__builtin_offsetof(CheckButton, checked_)==0xA14, "checked word");
static_assert(__builtin_offsetof(SocialWin, effectIcons_) - 0x13CC == 51*0x84,
              "raw source base/index relation");

int energy_limit(int) { return 12; }
static SocialWin panel;
struct PendingPlayerEffect { int values_[11]; };
struct PlayerData { PendingPlayerEffect soc_effect_pending; };
static PlayerData PlayersData[8];
static char StringTemp[128];
static char PlusText[]="+";
static int _itoa(int v, char *s, int) { return std::sprintf(s,"%d",v); }
typedef int (*ItoaFn)(int,char *,int);
BODY_ALLOC
BODY_PAINT
BODY_SHOW
BODY_HIDE
static void fail(const char *what) { std::fprintf(stderr,"FAIL: %s\n",what); std::exit(1); }
int main() {
    current_visible=0; iface_set_count=iface_release_count=0;
    panel.show(1); panel.show(0);
    if(iface_set_count!=2) fail("show ignores argument and follows visibility");
    current_visible=1; panel.hide();
    if(iface_release_count!=1) fail("hide follows visibility");
    // The exact allocation loops: with alternatives locked, candidate==a1 is
    // still exempt and grows the requested category to ten (original quirk).
    panel.factionID_=0;
    panel.pendingAlloc_[0]=panel.pendingAlloc_[1]=panel.pendingAlloc_[2]=0;
    for (int i=0;i<3;++i) panel.energyLockButtons_[i].checked_=1;
    panel.allocate_energy(0,2);
    if (panel.pendingAlloc_[0]!=10 || panel.pendingAlloc_[1]!=0 ||
        panel.pendingAlloc_[2]!=0) fail("locked redistribution below ten");
    // Over-allocation decrements the requested category despite its lock.
    panel.pendingAlloc_[0]=14; panel.pendingAlloc_[1]=panel.pendingAlloc_[2]=0;
    panel.allocate_energy(0,14);
    if (panel.pendingAlloc_[0]!=10 || panel.pendingAlloc_[1]!=0 ||
        panel.pendingAlloc_[2]!=0) fail("locked reduction above ten");

    GraphicWin window; window.Buffer::rect2={10,20,110,120};
    for (int id=0x11;id<=0x1A;++id) for (int flag=0;flag<=1;++flag) {
        std::memset(panel.flatButtons_,0,sizeof(panel.flatButtons_));
        unsigned char *button=reinterpret_cast<unsigned char *>(panel.flatButtons_)+id*0xB4C;
        *reinterpret_cast<int *>(button+0xAB8)=flag?0:1;
        *reinterpret_cast<int *>(button+0xA18)=0;
        *reinterpret_cast<int *>(button+0xA74)=flag?0:1;
        const int chosen_flag=flag;
        const int typed_index=(id-0x11)*9+chosen_flag*3;
        panel.effectIcons_[typed_index].key=static_cast<unsigned char>(0x40+id+flag);
        Sprite::drawn=nullptr;
        char label[]="label";
        if(panel.on_button_text_draw(&window,label,id)!=1) fail("valid paint id");
        if(Sprite::drawn != &panel.effectIcons_[typed_index]) fail("sprite receiver/index");
        if(Sprite::drawn_key != panel.effectIcons_[typed_index].key) fail("transparent byte");
        if(Sprite::drawn_buffer != static_cast<Buffer *>(&window)) fail("typed Buffer destination");
    }
    // Guard remains before all receiver/window use for out-of-domain ids.
    if(panel.on_button_text_draw(nullptr,nullptr,0x10)!=0 ||
       panel.on_button_text_draw(nullptr,nullptr,0x1B)!=0) fail("paint range guard");
    return 0;
}
'''


def actual_parts():
    text = (ROOT / 'src/socialwin.cpp').read_text()
    return {
        'BODY_SHOW': body(text, 'void SocialWin::show('),
        'BODY_HIDE': body(text, 'void SocialWin::hide('),
        'BODY_ALLOC': body(text, 'void SocialWin::allocate_energy('),
        'BODY_PAINT': body(text, 'int SocialWin::on_button_text_draw('),
    }


def run(tmp_path, parts, compiler):
    source = HARNESS
    for key, value in parts.items():
        source = source.replace(key, value)
    path = tmp_path / 'socialwin-typed-host.cpp'
    binary = tmp_path / 'socialwin-typed-host'
    path.write_text(source)
    # Preserve the product's redundant StringTemp-array null check. GCC and
    # Clang diagnose that original quirk under different warning names.
    pointer_warning = ('-Wno-tautological-pointer-compare'
                       if 'clang' in Path(compiler).name else '-Wno-address')
    build = subprocess.run([compiler, '-std=c++17', '-Wall', '-Wextra',
                            '-Werror', '-Wno-invalid-offsetof',
                            pointer_warning,
                            '-Wno-unused-parameter',
                            '-fsanitize=address,undefined',
                            '-fno-sanitize-recover=all', str(path), '-o', str(binary)],
                           capture_output=True, text=True, timeout=30)
    assert build.returncode == 0, build.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_socialwin_owner_bodies_host_control(tmp_path, compiler):
    result = run(tmp_path, actual_parts(), compiler)
    if result is None:
        import pytest
        pytest.skip('host C++ compiler required')
    assert result.returncode == 0, result.stderr


def test_sprite_stride_damage_control(tmp_path, compiler):
    parts = actual_parts()
    paint = parts['BODY_PAINT']
    old = 'sprite_index = flag + button_id * 3'
    new = 'sprite_index = flag + button_id * 3 + 1'
    if old in paint:
        parts['BODY_PAINT'] = paint.replace(old, new, 1)
    else:
        old = 'sprite_index = (button_id - 0x11) * 9 + flag * 3'
        assert old in paint, 'actual body lacks original or proposed index expression'
        parts['BODY_PAINT'] = paint.replace(old,
                                             'sprite_index = (button_id - 0x11) * 3 + flag', 1)
    result = run(tmp_path, parts, compiler)
    if result is None:
        import pytest
        pytest.skip('host C++ compiler required')
    assert result.returncode != 0 and 'sprite receiver/index' in result.stderr


def test_win_receiver_cast_damage_control(tmp_path, compiler):
    parts = actual_parts()
    show = parts['BODY_SHOW']
    old = 'if (!reinterpret_cast<Win *>(this)->is_visible())'
    if old in show:
        show = show.replace(old, 'if (visible && !reinterpret_cast<Win *>(this)->is_visible())', 1)
    else:
        old = 'if (!is_visible())'
        assert old in show
        show = show.replace(old, 'if (visible && !is_visible())', 1)
    parts['BODY_SHOW'] = show
    result = run(tmp_path, parts, compiler)
    if result is None:
        import pytest
        pytest.skip('host C++ compiler required')
    assert result.returncode != 0 and 'show ignores argument' in result.stderr


def test_checkbutton_receiver_state_damage_control(tmp_path, compiler):
    parts = actual_parts()
    allocation = parts['BODY_ALLOC']
    old = 'reinterpret_cast<int *>(self + 0x1780 + candidate * 0xA2C) == 0'
    if old in allocation:
        allocation = allocation.replace(old,
            'reinterpret_cast<int *>(self + 0x1780 + candidate * 0xA2C) != 0', 1)
    else:
        old = 'energyLockButtons_[candidate].checked_ == 0'
        assert old in allocation
        allocation = allocation.replace(old,
            'energyLockButtons_[candidate].checked_ != 0', 1)
    parts['BODY_ALLOC'] = allocation
    result = run(tmp_path, parts, compiler)
    if result is None:
        import pytest
        pytest.skip('host C++ compiler required')
    assert result.returncode != 0 and 'locked redistribution below ten' in result.stderr


@pytest.fixture(params=("g++", "clang++"))
def compiler(request):
    tool = shutil.which(request.param)
    if not tool:
        pytest.skip("host compiler unavailable")
    return tool

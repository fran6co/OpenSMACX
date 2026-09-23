"""Actual frame publication/minimum-size bodies, not native GUI acceptance.

Original 5D6F50..7021 uses the getter result only as a predicate, rereads the
registry receiver and its flags, and reloads the signed count each iteration.
5D7030..7140 chooses caption/noncaption corners with bit10, requires four
corners before either store, then takes independent signed maxima of wrapping
32-bit sums. The geometry oracle below uses mathematical int64 sums/modulo,
not the production helper. Class/member declarations and complete functions
are extracted unchanged; unrelated class services/storage are projected out.
Getter mutations are explicit boundary controls, not claimed original getter
effects. A replacement plain Win without style flags is valid and must not be
downcast; styled entries remain GraphicWins. No concurrent mutation/native
layout/ABI/real rendering proof is implied.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if not depth:
            return source[start:end + 1]
    raise AssertionError(signature)


def one(source, pattern):
    result, = re.findall(pattern, source, re.M)
    return result


def actual_source():
    gh = (ROOT / 'src/graphicwin.h').read_text()
    gc = (ROOT / 'src/graphicwin.cpp').read_text()
    wh = (ROOT / 'src/win.h').read_text()
    wc = (ROOT / 'src/win.cpp').read_text()
    sh = (ROOT / 'src/sprite.h').read_text()
    parts = {
        'NC': braced(gh, 'struct NCSprites {') + ';',
        'DEFAULTS': braced(gh, 'struct GraphicWinFrameDefaults {') + ';',
        'DEFAULT_OWNER': one(gc, r'^GraphicWinFrameDefaults GraphicWinInitDefaults;'),
        'REGISTRY': one(wc, r'^Win \*WinZOrderArray\[512\];') + '\n' +
                    one(wc, r'^int WinZOrderCount;'),
        'INHERITANCE': one(gh, r'^class GraphicWin :[^\{]+\{'),
        'SETTER': braced(gc, 'void __cdecl GraphicWin::set_def_nc_sprites('),
        'LOCAL': braced(gc, 'void GraphicWin::load_nc_sprites_from_defs('),
        'REFRESH': braced(gc, 'void GraphicWin::load_all_nc_sprites_from_defs('),
        'MINIMUM': braced(gc, 'void GraphicWin::compute_min_size('),
        'SUM': braced(gc, 'static int frame_dimension_sum('),
        'SPRITE': '\n'.join(one(sh, r'^  int ' + name + r';') for name in
                            ('iSpriteWidth_', 'iSpriteHeight_', 'iWidth_', 'iHeight_')) +
                  '\n' + braced(sh, 'int width() const') + '\n' + braced(sh, 'int height() const'),
        'WIN_FIELDS': '\n'.join(one(wh, r'^  uint32_t ' + name + r';') for name in
                                ('iFlags_', 'field_134_', 'field_138_')),
        'GETTER': one(wh, r'^  virtual GraphicWin \*get_graphic_win\(\)[^\n]+').split('//')[0],
        'FRAME_FIELDS': '\n'.join(re.findall(r'^  Sprite \*frame_\w+_;', gh, re.M)),
        'DECLARATIONS': '\n'.join(one(gh, pattern) for pattern in (
            r'^  static void(?: __cdecl)? load_all_nc_sprites_from_defs\(\);',
            r'^  void compute_min_size\(\);',
            r'^  void load_nc_sprites_from_defs\(\);',
            r'^  static void __cdecl set_def_nc_sprites\(NCSprites \*sprites\);')),
    }
    assert len(re.findall('Sprite \\*frame_', parts['FRAME_FIELDS'])) == 11
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <vector>
#include <limits>
#define __cdecl
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(109);}}
#define require(x) check((x),__LINE__)
class Sprite {public: @SPRITE@};
@NC@
@DEFAULTS@
class GraphicWin;
class Win {public: @WIN_FIELDS@ @GETTER@ virtual ~Win(){} };
class Buffer {};
@INHERITANCE@
public:
@FRAME_FIELDS@
@DECLARATIONS@
void redraw(){require(false);} void on_update(){require(false);}
};
@DEFAULT_OWNER@
@REGISTRY@
@SUM@
@SETTER@
@LOCAL@
@REFRESH@
@MINIMUM@
Sprite sprites[40];
using Member=Sprite* GraphicWin::*;
Member members[]={&GraphicWin::frame_top_left_,&GraphicWin::frame_top_right_,
 &GraphicWin::frame_bottom_left_,&GraphicWin::frame_bottom_right_,&GraphicWin::frame_top_,
 &GraphicWin::frame_left_,&GraphicWin::frame_right_,&GraphicWin::frame_bottom_,
 &GraphicWin::frame_noncaption_middle_,&GraphicWin::frame_noncaption_left_,&GraphicWin::frame_noncaption_right_};
void publish(int offset){
 NCSprites n={&sprites[offset],&sprites[offset+1],&sprites[offset+2],&sprites[offset+3],
 &sprites[offset+4],&sprites[offset+5],&sprites[offset+6],&sprites[offset+7],
 &sprites[offset+8],&sprites[offset+9],&sprites[offset+10]};
 GraphicWin::set_def_nc_sprites(&n);
}
void fill(GraphicWin&w,int offset){for(int i=0;i<11;++i)w.*members[i]=&sprites[offset+i];}
void equals(GraphicWin&w,int offset){for(int i=0;i<11;++i)require(w.*members[i]==&sprites[offset+i]);}
std::vector<int> calls;
struct Observed:GraphicWin{
 int id=0;GraphicWin* answer=nullptr;std::function<void()> action;
 GraphicWin* get_graphic_win() override{calls.push_back(id);if(action)action();return answer;}
};
void reset(Observed*w,int n){
 calls.clear();for(int i=0;i<512;++i)WinZOrderArray[i]=nullptr;
 for(int i=0;i<n;++i){w[i].id=i;w[i].answer=&w[i];w[i].action=nullptr;
 w[i].iFlags_=0x30000000;w[i].field_134_=123;w[i].field_138_=456;fill(w[i],20);WinZOrderArray[i]=&w[i];}
 publish(0);WinZOrderCount=n;
}
void registry_cases(){
 Observed w[5];
 for(int count:{-12,-1,0}){reset(w,5);WinZOrderCount=count;GraphicWin::load_all_nc_sprites_from_defs();
 require(calls.empty());for(auto &v:w)equals(v,20);}
 for(uint32_t flags:{0U,0x10U,0x08000000U,0x10000000U,0x20000000U,0x30000000U}){
 reset(w,2);WinZOrderCount=1;w[0].iFlags_=flags;w[0].answer=&w[1];
 GraphicWin::load_all_nc_sprites_from_defs();require(calls==std::vector<int>{0});
 equals(w[0],(flags&0x30000000)?0:20);equals(w[1],20);
 require(w[0].field_134_==123&&w[0].field_138_==456);
 fill(w[0],20);w[0].load_nc_sprites_from_defs();equals(w[0],(flags&0x30000000)?0:20);}
 reset(w,2);WinZOrderCount=1;w[0].answer=nullptr;w[0].action=[&]{WinZOrderArray[0]=&w[1];};
 GraphicWin::load_all_nc_sprites_from_defs();equals(w[0],20);equals(w[1],20);
 reset(w,3);WinZOrderCount=1;w[0].iFlags_=0;w[0].answer=&w[2];
 w[0].action=[&]{WinZOrderArray[0]=&w[1];publish(11);};
 GraphicWin::load_all_nc_sprites_from_defs();equals(w[0],20);equals(w[1],11);equals(w[2],20);
 require(calls==std::vector<int>{0});
 reset(w,3);WinZOrderCount=1;w[1].iFlags_=0;w[0].answer=&w[2];
 w[0].action=[&]{WinZOrderArray[0]=&w[1];};
 GraphicWin::load_all_nc_sprites_from_defs();for(auto&v:w)equals(v,20);
 reset(w,2);WinZOrderCount=1;Win plain{};plain.iFlags_=0;
 w[0].action=[&]{WinZOrderArray[0]=&plain;};
 GraphicWin::load_all_nc_sprites_from_defs();equals(w[0],20);equals(w[1],20);
 require(calls==std::vector<int>{0});
 reset(w,3);w[0].action=[&]{WinZOrderCount=-1;};
 GraphicWin::load_all_nc_sprites_from_defs();require(calls==std::vector<int>{0});equals(w[0],0);equals(w[1],20);
 reset(w,3);WinZOrderCount=1;w[0].action=[&]{WinZOrderCount=3;publish(11);};
 GraphicWin::load_all_nc_sprites_from_defs();require(calls==std::vector<int>({0,1,2}));
 for(int i=0;i<3;++i)equals(w[i],11);
 reset(w,3);w[0].action=[&]{publish(11);};w[1].action=[&]{publish(0);};
 GraphicWin::load_all_nc_sprites_from_defs();equals(w[0],11);equals(w[1],0);equals(w[2],0);
 // Single-window sibling must still obey the same real default mapping.
 reset(w,1);GraphicWin::set_def_nc_sprites(nullptr);w[0].load_nc_sprites_from_defs();equals(w[0],0);
 w[0].iFlags_=0;fill(w[0],20);w[0].load_nc_sprites_from_defs();equals(w[0],20);
}
int mathematical_sum(int a,int b){
 int64_t n=int64_t(a)+int64_t(b);n%=int64_t(4294967296ULL);
 if(n<0)n+=4294967296ULL;if(n>=2147483648LL)n-=4294967296ULL;return int(n);
}
void size_case(uint32_t flags,const int*v){
 GraphicWin w{};fill(w,0);w.iFlags_=flags;
 for(int i=0;i<11;++i){sprites[i].iWidth_=v[i*2];sprites[i].iHeight_=v[i*2+1];
 sprites[i].iSpriteWidth_=10000+i; sprites[i].iSpriteHeight_=-10000-i;}
 int left=(flags&0x10)?0:9,right=(flags&0x10)?1:10;
 int top=mathematical_sum(v[2*left],v[2*right]);int bottom=mathematical_sum(v[4],v[6]);
 int a=mathematical_sum(v[2*left+1],v[5]),b=mathematical_sum(v[2*right+1],v[7]);
 uint32_t ew=uint32_t(top>bottom?top:bottom),eh=uint32_t(a>b?a:b);
 w.field_134_=777;w.field_138_=888;w.compute_min_size();require(w.field_134_==ew&&w.field_138_==eh);
 Member required[]={members[left],members[right],members[2],members[3]};
 for(auto missing:required){fill(w,0);w.*missing=nullptr;w.field_134_=777;w.field_138_=888;
 w.compute_min_size();require(w.field_134_==777&&w.field_138_==888);}
 fill(w,0);for(int i=0;i<11;++i)if(i!=left&&i!=right&&i!=2&&i!=3)w.*members[i]=nullptr;
 w.compute_min_size();require(w.field_134_==ew&&w.field_138_==eh);
}
void minimum_cases(){
 int v[22];
 for(int trial=0;trial<48;++trial){for(int i=0;i<22;++i)v[i]=((i*43+trial*29)%97)-48;
 for(uint32_t flags:{0U,0x10U,0x20U,0x10000000U,0x20000010U,0xffffffffU})size_case(flags,v);}
 const int special[]={INT32_MIN,INT32_MAX,-1,0,1,101,-233};
 for(int trial=0;trial<14;++trial){for(int i=0;i<22;++i)v[i]=special[(i+trial*(i%3+1))%7];
 size_case(0,v);size_case(0x10,v);}
}
int main(){registry_cases();minimum_cases();}
'''


def run(tmp_path, parts, optimization='-O1'):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        pytest.skip('host C++ compiler required')
    source = HARNESS
    for key, value in parts.items():
        source = source.replace('@' + key + '@', value)
    assert not re.search(r'@[A-Z_]+@', source)
    path = tmp_path / 'frames.cpp'
    path.write_text(source)
    executable = tmp_path / 'frames'
    compiled = subprocess.run([compiler, '-std=c++17', optimization, '-g',
        '-fsanitize=address,undefined,vptr', '-fno-sanitize-recover=all',
        str(path), '-o', str(executable)], capture_output=True, text=True, timeout=40)
    assert compiled.returncode == 0, compiled.stderr
    environment = dict(os.environ, ASAN_OPTIONS='detect_leaks=0')
    # LeakSanitizer is disabled for sandbox execution, not ASan/UBSan.
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10, env=environment)


@pytest.mark.parametrize('optimization', ['-O0', '-O1'])
def test_actual_frame_publication_and_signed_minima(tmp_path, optimization):
    result = run(tmp_path, actual_source(), optimization)
    assert result.returncode == 0, result.stderr


MUTATIONS = [
    ('REFRESH', 'int index = 0;', 'int index = 1;', 'first-index'),
    ('REFRESH', 'if (0 < WinZOrderCount)', 'if (0 <= WinZOrderCount)', 'zero-count'),
    ('REFRESH', 'int index = 0;', 'int index = 0; const int saved_count = WinZOrderCount;', 'count-snapshot'),
    ('REFRESH', '++cursor;', '/* cursor not advanced */', 'registry-step'),
    ('REFRESH', 'GraphicWin *graphic = static_cast<GraphicWin *>(window);', 'GraphicWin *graphic = static_cast<GraphicWin *>(window); graphic->compute_min_size();', 'unexpected-minimum'),
    ('REFRESH', 'GraphicWin *graphic = static_cast<GraphicWin *>(window);', 'GraphicWin *graphic = static_cast<GraphicWin *>(window); graphic->redraw();', 'unexpected-redraw'),
    ('REFRESH', 'graphic->frame_top_right_ = GraphicWinInitDefaults.top_right;', 'graphic->frame_top_right_ = GraphicWinInitDefaults.bottom_left;', 'refresh-corner-permutation'),
    ('REFRESH', 'graphic->frame_left_ = GraphicWinInitDefaults.left;', 'graphic->frame_left_ = GraphicWinInitDefaults.top;', 'refresh-edge-permutation'),
    ('REFRESH', 'graphic->frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_middle;', 'graphic->frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_right;', 'refresh-noncaption-permutation'),
    ('REFRESH', 'graphic->frame_bottom_ = GraphicWinInitDefaults.bottom;', '/* missing bottom publication */', 'refresh-missing-field'),
    ('REFRESH', '(*cursor)->get_graphic_win() != nullptr', '(*cursor)->get_graphic_win() == nullptr', 'predicate-null'),
    ('REFRESH', 'if ((*cursor)->get_graphic_win() != nullptr)', 'Win *saved_window = *cursor; if ((*cursor)->get_graphic_win() != nullptr)', 'cached-receiver'),
    ('REFRESH', 'Win *window = *cursor;', 'Win *window = (*cursor)->get_graphic_win();', 'predicate-as-receiver'),
    ('REFRESH', 'window->iFlags_ & 0x30000000', 'window->iFlags_ & 0x10000000', 'outer-first-style-only'),
    ('REFRESH', 'window->iFlags_ & 0x30000000', 'window->iFlags_ & 0x20000000', 'outer-second-style-only'),
    ('LOCAL', 'iFlags_ & 0x30000000', 'iFlags_ & 0x10000000', 'first-style-only'),
    ('LOCAL', 'iFlags_ & 0x30000000', 'iFlags_ & 0x20000000', 'second-style-only'),
    ('LOCAL', 'iFlags_ & 0x30000000', 'iFlags_ & 0x10', 'caption-not-style'),
    ('LOCAL', 'frame_top_right_ = GraphicWinInitDefaults.top_right;', 'frame_top_right_ = GraphicWinInitDefaults.top_left;', 'top-right'),
    ('LOCAL', 'frame_bottom_left_ = GraphicWinInitDefaults.bottom_left;', 'frame_bottom_left_ = GraphicWinInitDefaults.bottom_right;', 'bottom-left'),
    ('LOCAL', 'frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_middle;', 'frame_noncaption_middle_ = GraphicWinInitDefaults.noncaption_left;', 'noncaption-middle'),
    ('LOCAL', 'frame_top_ = GraphicWinInitDefaults.top;', '/* missing top */', 'missing-field'),
    ('SETTER', 'GraphicWinInitDefaults.top_right = sprites->top_right;', 'GraphicWinInitDefaults.top_right = sprites->bottom_left;', 'publisher-permutation'),
    ('SETTER', 'GraphicWinInitDefaults.noncaption_left = sprites->noncaption_left;', 'GraphicWinInitDefaults.noncaption_left = sprites->noncaption_middle;', 'publisher-noncaption'),
    ('MINIMUM', 'iFlags_ & 0x10', 'iFlags_ & 0x20', 'caption-bit'),
    ('MINIMUM', '? frame_top_left_ : frame_noncaption_left_', '? frame_noncaption_left_ : frame_top_left_', 'left-branch'),
    ('MINIMUM', '? frame_top_right_ : frame_noncaption_right_', '? frame_noncaption_right_ : frame_top_right_', 'right-branch'),
    ('MINIMUM', 'Sprite *bottom_left = frame_bottom_left_;', 'Sprite *bottom_left = frame_bottom_right_;', 'bottom-receiver'),
    ('MINIMUM', 'return;', 'field_134_ = 0; return;', 'null-preservation'),
    ('MINIMUM', 'top_left->width(), top_right->width()', 'top_left->height(), top_right->height()', 'dimension-width'),
    ('MINIMUM', 'bottom_right->width(), bottom_left->width()', 'bottom_right->width(), top_left->width()', 'bottom-sum'),
    ('MINIMUM', 'top_left->height(), bottom_left->height()', 'top_left->height(), top_right->height()', 'left-height'),
    ('MINIMUM', 'bottom_right->height(), top_right->height()', 'bottom_left->height(), top_right->height()', 'right-height'),
    ('MINIMUM', 'bottom_width > width', 'bottom_width < width', 'width-maximum'),
    ('MINIMUM', 'right_height > height', 'right_height < height', 'height-maximum'),
    ('MINIMUM', 'bottom_width > width', 'uint32_t(bottom_width) > uint32_t(width)', 'unsigned-width'),
    ('MINIMUM', 'right_height > height', 'uint32_t(right_height) > uint32_t(height)', 'unsigned-height'),
    ('SUM', '+ static_cast<unsigned int>(second)', '- static_cast<unsigned int>(second)', 'sum-not-difference'),
]


@pytest.mark.parametrize('part,before,after,label', MUTATIONS, ids=[m[3] for m in MUTATIONS])
def test_compiling_behavior_damage(tmp_path, part, before, after, label):
    parts = actual_source()
    assert before in parts[part]
    parts[part] = parts[part].replace(before, after)
    if label == 'count-snapshot':
        parts[part] = parts[part].replace('while (index < WinZOrderCount)', 'while (index < saved_count)')
    if label == 'cached-receiver':
        parts[part] = parts[part].replace('Win *window = *cursor;', 'Win *window = saved_window;')
    result = run(tmp_path, parts)
    assert result.returncode == 109 and 'assertion line' in result.stderr, result.stderr


def test_unstyled_plain_win_rejects_early_downcast(tmp_path):
    parts = actual_source()
    before = 'Win *window = *cursor;'
    assert before in parts['REFRESH']
    parts['REFRESH'] = parts['REFRESH'].replace(before, 'GraphicWin *window = static_cast<GraphicWin *>(*cursor);')
    # At -O1 Clang can eliminate the unused derived conversion before vptr
    # instrumentation. -O0 retains the genuine cast; no source substitution
    # forces a derived access or invents a different runtime operation.
    result = run(tmp_path, parts, '-O0')
    # This defect is an actual C++ invalid downcast, not an oracle assertion.
    assert result.returncode != 0
    assert 'runtime error: downcast of address' in result.stderr, result.stderr

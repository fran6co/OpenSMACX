"""Actual button cleanup/init/lifetime bodies with bounded service doubles.

Original 607217 dispatches slot90 before parent/name guards; 607347 dispatches
show only after successful initialization. 6070C4 directly closes GraphicWin;
607DAA closes Time1 before the FlatButton resets/delegation; 625330 clears the
borrowed Sprite before BaseButton cleanup; 62C033 delegates before 18 defaults.
ImageButton init's early Sprite publication is consequently cleared by the
actual virtual close, including when BaseButton rejects the parent afterward.
Image/Push destructors do not introduce a derived-close call; Flat does.

Complete product bodies, relevant declaration/member runs and real default-font
and PushButton owner definitions are extracted. Access labels alone are relaxed
in the projected declarations. GraphicWin, Time, allocator/CRT and Buffer setters
are explicit doubles; no native offsets/vtable numbers, real OS teardown or game
acceptance are claimed. Legacy FlatButton default storage is a synthetic table;
the existing default-colour alias is bound to its real DialogDefaults+5 relation
in this fixture, not evidence of repaired product ownership. Callback mutations
and a null allocator exercise caller ordering, not the real CRT's behavior
(actual canonical mem_get exits instead of returning null). No bodies are rewritten
for the positive control. ASan/UBSan remain strict; sandbox LSan is disabled.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    match = re.search(r'(?m)^[ \t]*' + re.escape(signature), text)
    assert match, signature
    start = text.index(signature, match.start())
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return text[start:end + 1]
    raise AssertionError('unterminated source')


def one(text, pattern):
    value, = re.findall(pattern, text, re.M)
    return value


def actual():
    h = {n: (ROOT / f'src/{n}.h').read_text() for n in
         ('basebutton', 'flatbutton', 'imagebutton', 'pushbutton', 'graphicwin',
          'win', 'buffer', 'sprite', 'general', 'dialog')}
    c = {n: (ROOT / f'src/{n}.cpp').read_text() for n in
         ('basebutton', 'flatbutton', 'imagebutton', 'gamma')}
    parts = {}
    declarations = [
        one(h['win'], r'typedef void \(__cdecl \*WinNotificationCallback\)\(int selected_id\);'),
        'struct Win { virtual ~Win() {}', one(h['win'], r'virtual void show\(int visible\);'), '};',
        'struct Buffer {',
        '\n'.join(one(h['buffer'], rf'(?:int|void) {name}\([^;]+;') for name in
                  ('set_font', 'set_text_color', 'set_text_color2', 'set_text_color3')), '};',
        one(h['graphicwin'], r'^class GraphicWin :[^\n]+\{') + ' public:',
        'virtual ~GraphicWin();', one(h['graphicwin'], r'uint32_t close\(\);'),
        one(h['graphicwin'], r'int init\(int x, int y,[^;]+;'), '};',
    ]
    for name, unit in (('BaseButton', 'basebutton'), ('FlatButton', 'flatbutton'),
                       ('ImageButton', 'imagebutton'), ('PushButton', 'pushbutton')):
        whole = braced(h[unit], f'class {name} :')
        declarations += [one(h[unit], rf'^class {name} :[^\n]+\{{') + ' public:',
                         one(whole, rf'(?<!~){name}\(\);')]
        parts[f'{unit}_decl'] = one(whole, r'(?:virtual )?void close\(\);')
        declarations.append(f'{unit.upper()}_CLOSE_DECL')
        if name == 'FlatButton':
            parts['flat_dtor'] = braced(whole, '~FlatButton()')
            declarations.append('FLAT_DTOR')
        else:
            declarations.append(one(whole, rf'~{name}\(\);'))
        # Full actual storage run, not a hand-spelled offset simulation.
        start = whole.index('  uint32_t field_A14_;' if name == 'BaseButton' else
                            '  Sprite *sprite_;' if name == 'ImageButton' else
                            '  Sprite *field_AB8_;' if name == 'PushButton' else
                            '  uint32_t field_AB8_;')
        members = whole[start:-1]
        members = re.sub(r'\b(?:private|protected):', 'public:', members)
        declarations.append(members)
        if name == 'BaseButton':
            declarations += [one(whole, r'int init\(LPCSTR name,[^;]+;'),
                             one(whole, r'static int set_def_font\([^;]+;')]
        if name == 'ImageButton':
            declarations.extend(re.findall(r'int init\(Sprite \*sprite,[^;]+;', whole))
        declarations.append('};')
    parts['declarations'] = '\n'.join(declarations)
    parts['owners'] = '\n'.join((
        one(c['basebutton'], r'^uint32_t BaseButtonA94Default;'),
        one(c['basebutton'], r'^uint32_t BaseButtonA98Default;'),
        one(h['basebutton'], r'^extern Font \*BaseButtonDefaultFonts\[3\];'),
        one(c['basebutton'], r'^Font \*BaseButtonDefaultFonts\[3\];'),
        one(h['pushbutton'], r'^extern Sprite \*PushButtonDefaults\[18\];'),
        one(c['gamma'], r'^Sprite \*PushButtonDefaults\[18\];'),
        one(h['dialog'], r'^extern uint32_t DialogDefaults\[37\];'),
        'uint32_t DialogDefaults[37];',
        one(h['general'], r'^LPVOID __cdecl mem_get\(int size\);'),
    ))
    for unit, cls in (('basebutton', 'BaseButton'), ('flatbutton', 'FlatButton'),
                      ('imagebutton', 'ImageButton'), ('gamma', 'PushButton')):
        key = cls.lower()
        for kind, sig in (('ctor', f'{cls}::{cls}()'), ('close', f'void {cls}::close()')):
            parts[f'{key}_{kind}'] = braced(c[unit], sig)
        if cls != 'FlatButton':
            parts[f'{key}_dtor'] = braced(c[unit], f'{cls}::~{cls}()')
    parts['base_init'] = braced(c['basebutton'], 'int BaseButton::init(')
    parts['font_setter'] = braced(c['basebutton'], 'int BaseButton::set_def_font(')
    for sig in re.findall(r'int ImageButton::init\([^)]*\)\s*\{', c['imagebutton']):
        parts['image_explicit' if 'int width' in sig else 'image_automatic'] = braced(c['imagebutton'], sig)
    parts['sprite'] = '\n'.join(one(h['sprite'], rf'int {name};') for name in ('iWidth_', 'iHeight_'))
    parts['sprite'] += '\n' + braced(h['sprite'], 'int width() const') + '\n' + braced(h['sprite'], 'int height() const')
    # Original 18 consecutive destinations AB8..AFC; not parsed from candidate assignments.
    names = [f'field_{offset:X}_' for offset in range(0xAB8, 0xB00, 4)]
    parts['push_seed'] = '\n'.join(f'p.{name}=&push_seed_sprites[{i}];' for i, name in enumerate(names))
    parts['push_values'] = ','.join(f'p.{name}' for name in names)
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <functional>
#include <type_traits>
#define __cdecl
#define MEASURED
using LPSTR=char*;using LPCSTR=const char*;using LPVOID=void*;
struct Menu {};struct BorderSizing {};struct ButtonGroup {};
struct Font {bool ready;bool is_initialized(){return ready;}};
struct Sprite { SPRITE };
struct BoxSpriteParams { int tag; };
void require_at(bool good,int line){if(!good){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(108);}}
#define require(x) require_at((x),__LINE__)
std::string events;std::function<void(char)> hook;bool observing=false;
void event(char k){if(observing){require(events.size()<100);events+=k;if(hook)hook(k);}}
int next_time=0;std::vector<int> destroyed_times;
struct Time {int id;Time():id(++next_time){}void close();
    ~Time(){if(observing)destroyed_times.push_back(id);event('d');}};
DECLARATIONS
OWNERS
void __cdecl close_callback_probe(int) {}
Sprite flat_default_sprites[81];
Sprite push_default_sprites[36], push_seed_sprites[18];
Sprite *flat_defaults[27];Sprite **const FlatButtonDefaultsTable=flat_defaults;
BaseButton *active;Font fonts[6]={{true},{true},{true},{true},{false},{true}};
void Time::close(){require(!observing||this==&active->time1_);event('T');}
Win parent;ButtonGroup group;Sprite sprite{31,-17};
BoxSpriteParams border_styles[3];
int graphic_status=0,expected_style=0;bool allocation_fails=false;
int expected_width=19,expected_height=-13;std::vector<void*> frees;
char *last_allocation=nullptr;
void fixture_free(void*p){frees.push_back(p);event('F');std::free(p);}
char* text(const char*s){auto n=std::strlen(s)+1;auto p=static_cast<char*>(std::malloc(n));std::memcpy(p,s,n);return p;}
LPVOID __cdecl mem_get(int size){require(size==6);event('A');
    if(allocation_fails)return nullptr;last_allocation=static_cast<char*>(std::malloc(size));
    std::memset(last_allocation,0x5a,size);return last_allocation;}
GraphicWin::~GraphicWin(){event('D');}
uint32_t GraphicWin::close(){require(!observing||this==static_cast<GraphicWin*>(active));event('G');return 77;}
int GraphicWin::init(int x,int y,int w,int h,LPSTR caption,int style,Win*owner,Menu*menu,BorderSizing*border){
    require(this==static_cast<GraphicWin*>(active));
    require(x==-7&&y==11&&w==expected_width&&h==expected_height);
    require(caption==nullptr&&owner==&parent&&menu==nullptr&&border==nullptr&&style==expected_style);
    event('I');return graphic_status;
}
void Win::show(int value){require(value==0);event('W');}
void colors(Buffer*b,int tier,int a,int c,int d,int e){
    require(b==static_cast<Buffer*>(active));
    require(a==static_cast<int>(DialogDefaults[5+tier])&&c==static_cast<int>(DialogDefaults[8+tier]));
    require(d==static_cast<int>(DialogDefaults[11+tier])&&e==static_cast<int>(DialogDefaults[14+tier]));
    require(active->field_A78_==static_cast<uint32_t>(-23));event('1'+tier);
}
void Buffer::set_text_color(int a,int b,int c,int d){colors(this,0,a,b,c,d);}
void Buffer::set_text_color2(int a,int b,int c,int d){colors(this,1,a,b,c,d);}
void Buffer::set_text_color3(int a,int b,int c,int d){colors(this,2,a,b,c,d);}
int Buffer::set_font(Font*a,Font*b,Font*c,Font*d){
    require(this==static_cast<Buffer*>(active));require(a==BaseButtonDefaultFonts[0]&&b==BaseButtonDefaultFonts[1]);
    require(c==BaseButtonDefaultFonts[2]&&d==nullptr);event('N');return 3;
}
// Only service names are substituted; positive product bodies are unchanged.
#define free fixture_free
BODIES
#undef free
template<class T>struct Probe:T{
    void close(){event('Q');T::close();}
    void show(int value){require(value==0);event('S');}
};
void defaults(){
    for(int i=0;i<37;++i)DialogDefaults[i]=1000+i*17;
    for(int i=0;i<27;++i)flat_defaults[i]=&flat_default_sprites[i];
    for(int i=0;i<18;++i)PushButtonDefaults[i]=&push_default_sprites[i];
    BaseButtonA94Default=401;BaseButtonA98Default=402;
    BaseButtonDefaultFonts[0]=&fonts[0];BaseButtonDefaultFonts[1]=&fonts[1];BaseButtonDefaultFonts[2]=&fonts[2];
}
void begin(BaseButton&b){
    active=&b;observing=true;events.clear();hook=nullptr;frees.clear();destroyed_times.clear();
    graphic_status=0;allocation_fails=false;expected_width=19;expected_height=-13;last_allocation=nullptr;
}
void end(){observing=false;hook=nullptr;}
void poison(BaseButton&b){
    b.field_A14_=51;b.field_A18_=52;b.field_A74_=53;b.field_A78_=54;b.field_A9C_=55;
    b.field_A44_=56;b.field_A48_=57;b.field_AAC_=58;
    b.field_AB0_=close_callback_probe;b.field_AB4_=close_callback_probe;
    b.field_A94_=61;b.color_=62;b.bevel_upper_=63;b.bevel_lower_=64;b.bevel_thickness_=65;
    b.field_A98_=66;b.field_AA4_=67;b.field_AA0_=68;b.group_=&group;
}
void base_cleared(BaseButton&b){
    require(b.field_A74_==0&&b.field_A9C_==0&&b.field_A78_==0);
    require(b.field_A44_==0xffffffffU&&b.field_A48_==0xffffffffU);
    require(b.field_AAC_==0&&b.field_AB0_==0&&b.field_AB4_==0&&b.field_AA4_==0);
    require(b.field_A94_==BaseButtonA94Default&&b.field_A98_==BaseButtonA98Default);
    require(b.color_==DialogDefaults[0]&&b.bevel_upper_==DialogDefaults[1]&&b.bevel_lower_==DialogDefaults[2]);
    require(b.bevel_thickness_==DialogDefaults[3]&&b.field_AA0_==DialogDefaults[4]);
    require(!b.name_&&!b.bubble_text_&&!b.group_);
}
void flat_poison(FlatButton&f){poison(f);f.field_AB8_=88;
    for(int i=0;i<3;++i){f.field_ABC_[i]=91+i;f.border_styles_[i]=&border_styles[i];f.field_AD4_[i]=&sprite;}
    for(int i=0;i<27;++i)f.config[i]=&flat_default_sprites[27+i];
}
void flat_cleared(FlatButton&f){base_cleared(f);require(f.field_A14_==0&&f.field_A18_==0&&f.field_AB8_==0xffffffffU);
    for(int i=0;i<3;++i)require(f.field_ABC_[i]==0xffffffffU&&f.border_styles_[i]==0&&f.field_AD4_[i]==nullptr);
    for(int i=0;i<27;++i)require(f.config[i]==flat_defaults[i]);
}
void push_poison(PushButton&p){poison(p);PUSH_SEED}
std::vector<Sprite*> push_values(PushButton&p){return {PUSH_VALUES};}
void push_cleared(PushButton&p){base_cleared(p);auto values=push_values(p);
    for(int i=0;i<18;++i)require(values[i]==PushButtonDefaults[i]);}
void closes(){
    BaseButton b;Probe<FlatButton> f;Probe<ImageButton> image;Probe<PushButton> push;
    defaults();poison(b);begin(b);
    char*old_name=text("old");char*old_bubble=text("bubble");char*replacement=text("newbubble");
    b.name_=old_name;b.bubble_text_=old_bubble;
    hook=[&](char k){
        if(k=='G'){require(b.field_A74_==53&&b.name_==old_name&&b.group_==&group);
            DialogDefaults[0]=801;BaseButtonA94Default=802;}
        if(k=='F'&&frees.size()==1){require(b.name_==old_name&&b.bubble_text_==old_bubble&&b.group_==&group);
            require(b.color_==801&&b.field_A94_==802);b.bubble_text_=replacement;}
        if(k=='F'&&frees.size()==2){require(b.name_==nullptr&&b.bubble_text_==replacement&&b.group_==&group);}
    };
    static_cast<BaseButton*>(&b)->close();require(events=="GFF"&&frees==std::vector<void*>({old_name,replacement}));
    base_cleared(b);require(b.field_A14_==51&&b.field_A18_==52);std::free(old_bubble);
    hook=nullptr;events.clear();b.close();require(events=="G");end();
    defaults();flat_poison(f);begin(f);
    hook=[&](char k){if(k=='T'){require(f.field_A14_==51&&f.field_A18_==52&&f.field_AB8_==88);}
        if(k=='G'){require(f.field_A14_==0&&f.field_A18_==0&&f.field_AB8_==0xffffffffU);
            require(f.config[0]==&flat_default_sprites[27]&&f.field_AD4_[0]==&sprite);
            for(int i=0;i<27;++i)flat_defaults[i]=&flat_default_sprites[54+i];}};
    static_cast<BaseButton*>(&f)->close();require(events=="QTG");flat_cleared(f);
    events.clear();hook=nullptr;f.FlatButton::close();require(events=="TG");end();
    defaults();poison(image);image.sprite_=&sprite;begin(image);
    hook=[&](char k){if(k=='Q')require(image.sprite_==&sprite);if(k=='G')require(image.sprite_==nullptr);};
    static_cast<BaseButton*>(&image)->close();require(events=="QG");base_cleared(image);require(image.sprite_==nullptr);
    events.clear();Sprite replacement_sprite{7,9};
    hook=[&](char k){if(k=='G'){require(image.sprite_==nullptr);image.sprite_=&replacement_sprite;}};
    image.ImageButton::close();require(events=="G"&&image.sprite_==&replacement_sprite);end();
    defaults();push_poison(push);begin(push);const auto before=push_values(push);
    hook=[&](char k){if(k=='G'){require(push_values(push)==before);
        for(int i=0;i<18;++i)PushButtonDefaults[i]=&push_default_sprites[18+i];}};
    static_cast<BaseButton*>(&push)->close();require(events=="QG");push_cleared(push);
    events.clear();hook=nullptr;push.PushButton::close();require(events=="G");end();
}
void initialization(){
    Probe<BaseButton>b;char name[]="hello";
    for(int style:{0,1,-7,32768})for(int fail:{0,3,4,23})for(bool named:{false,true}){
        defaults();poison(b);begin(b);expected_style=0x01000320|(style?0x8000:0);
        if(fail==4&&!named){end();continue;}
        graphic_status=fail==23?23:0;allocation_fails=fail==4;
        hook=[&](char k){
            if(k=='Q')require(b.field_A74_==53);
            if(k=='I'){require(b.field_A78_==0);require(!named||std::strcmp(b.name_,"hello")==0);
                for(int i=5;i<17;++i)DialogDefaults[i]+=100;}
            if(k=='1')for(int i=0;i<4;++i)DialogDefaults[6+3*i]+=200;
            if(k=='2')for(int i=0;i<4;++i)DialogDefaults[7+3*i]+=300;
            if(k=='3')require(BaseButton::set_def_font(&fonts[3],&fonts[4],&fonts[5])==0);
            if(k=='N')b.field_A78_=77;
            if(k=='S')require(b.field_A78_==77);
        };
        require(b.init(named?name:nullptr,-23,-7,11,19,-13,fail==3?nullptr:&parent,style)==fail);
        std::string expected="QG";
        if(fail!=3){if(named)expected+="A";if(fail!=4){expected+='I';if(fail==0)expected+="123NS";}}
        require(events==expected);
        if(fail==0)require(b.field_A78_==77);else require(b.field_A78_==0);
        if(fail==3||fail==4)require(b.name_==nullptr);
        else if(named)require(b.name_==last_allocation&&std::strcmp(b.name_,"hello")==0);
        end();b.BaseButton::close();
    }
    // Real producer publishes the same font array consumed by init.
    defaults();require(BaseButton::set_def_font(nullptr,&fonts[4],&fonts[5])==3);
    require(BaseButtonDefaultFonts[0]==&fonts[0]&&BaseButtonDefaultFonts[1]==&fonts[1]);
    require(BaseButton::set_def_font(&fonts[4],&fonts[5],nullptr)==0);
    require(BaseButtonDefaultFonts[0]==&fonts[0]&&BaseButtonDefaultFonts[1]==&fonts[5]&&BaseButtonDefaultFonts[2]==nullptr);
    Probe<ImageButton> image;
    for(bool automatic:{false,true})for(bool valid_parent:{false,true}){
        defaults();begin(image);image.sprite_=nullptr;expected_style=0x01008320;
        expected_width=automatic?31:19;expected_height=automatic?-17:-13;
        hook=[&](char k){if(k=='Q')require(image.sprite_==&sprite);if(k=='G')require(image.sprite_==nullptr);};
        int r=automatic?image.init(&sprite,nullptr,-23,-7,11,valid_parent?&parent:nullptr,1):
            image.init(&sprite,nullptr,-23,-7,11,19,-13,valid_parent?&parent:nullptr,1);
        require(r==(valid_parent?0:3));require(image.sprite_==nullptr);
        require(events==(valid_parent?"QGI123NS":"QG"));end();
    }
    begin(image);image.sprite_=&sprite;
    require(image.init(nullptr,nullptr,1,2,3,&parent,0)==3&&image.sprite_==&sprite&&events.empty());end();
    defaults();begin(image);image.sprite_=&sprite;
    require(image.init(nullptr,nullptr,-23,-7,11,19,-13,nullptr,0)==3);
    require(image.sprite_==nullptr&&events=="QG");end();
    defaults();begin(image);image.sprite_=&sprite;expected_style=0x01000320;
    require(image.init(nullptr,nullptr,-23,-7,11,19,-13,&parent,0)==0);
    require(image.sprite_==nullptr&&events=="QGI123NS");end();
}
template<class T>void lifetime(const char*trace){
    defaults();auto p=new Probe<T>;begin(*p);int first=p->time1_.id,second=p->time2_.id;
    delete static_cast<BaseButton*>(p);
    require(events==trace&&destroyed_times==std::vector<int>({second,first}));end();
}
int main(){
    static_assert(std::is_same<decltype(FlatButton::field_AD4_),Sprite*[3]>::value,"actual borrowed pointers");
    defaults();closes();initialization();
    lifetime<BaseButton>("GddD");lifetime<FlatButton>("TGGddD");
    lifetime<ImageButton>("GddD");lifetime<PushButton>("GddD");
}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if compiler is None:
        pytest.skip('host C++ compiler unavailable')
    declarations = parts['declarations'].replace('FLAT_DTOR', parts['flat_dtor'])
    for unit in ('basebutton', 'flatbutton', 'imagebutton', 'pushbutton'):
        declarations = declarations.replace(f'{unit.upper()}_CLOSE_DECL', parts[f'{unit}_decl'])
    excluded = {'declarations', 'owners', 'sprite', 'push_seed', 'push_values', 'flat_dtor'}
    bodies = '\n'.join(value for key, value in parts.items() if key not in excluded and not key.endswith('_decl'))
    values = {'DECLARATIONS': declarations, 'OWNERS': parts['owners'], 'SPRITE': parts['sprite'],
              'BODIES': bodies, 'PUSH_SEED': parts['push_seed'], 'PUSH_VALUES': parts['push_values']}
    content = re.sub(r'\b(' + '|'.join(values) + r')\b', lambda m: values[m.group()], HARNESS)
    source, binary = tmp_path / 'buttons.cpp', tmp_path / 'buttons'
    source.write_text(content)
    built = subprocess.run([compiler, '-std=c++11', '-O1', '-fsanitize=address,undefined',
                            '-fno-sanitize-recover=all', str(source), '-o', str(binary)],
                           capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=20,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_button_close_dispatch(tmp_path):
    result = run_host(tmp_path, actual())
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ('basebutton_decl', 'virtual void close();', 'void close();'),
    ('base_init', '    close();', '    BaseButton::close();'),
    ('base_init', '    close();', '    if (parent) close();'),
    ('base_init', 'return 3;', 'return 0;'),
    ('base_init', 'return 4;', 'return 0;'),
    ('base_init', 'return status;', 'close(); return status;'),
    ('base_init', 'return status;', 'return 0;'),
    ('base_init', 'style_flag ? 0x8000 : 0', 'style_flag & 0x8000'),
    ('base_init', 'field_A78_ = static_cast<uint32_t>(id);', 'field_A78_ = 0;'),
    ('base_init', 'DialogDefaults[8]', 'DialogDefaults[6]'),
    ('base_init', 'BaseButtonDefaultFonts[2], nullptr', 'BaseButtonDefaultFonts[1], nullptr'),
    ('base_init', '    show(0);', '    Win::show(0);'),
    ('basebutton_close', 'GraphicWin::close();', ''),
    ('basebutton_close', 'free(name_);\n        name_ = nullptr;', 'LPSTR old=name_; name_=nullptr; free(old);'),
    ('basebutton_close', 'group_ = nullptr;', ''),
    ('basebutton_close', 'field_AA0_ = DialogDefaults[4];', 'field_AA0_ = DialogDefaults[3];'),
    ('flatbutton_close', 'time1_.close();', 'time2_.close();'),
    ('flatbutton_close', 'time1_.close();', ''),
    ('flatbutton_close', 'BaseButton::close();', 'GraphicWin::close();'),
    ('flatbutton_close', 'column < 3', 'column < 2'),
    ('flatbutton_close', 'group < 9', 'group < 8'),
    ('flatbutton_close', 'field_AD4_[column] = nullptr;', ''),
    ('flatbutton_close', 'border_styles_[column] = 0;', ''),
    ('imagebutton_close', 'sprite_ = 0;', ''),
    ('imagebutton_close', 'BaseButton::close();', 'GraphicWin::close();'),
    ('imagebutton_close', 'BaseButton::close();', 'BaseButton::close(); sprite_ = nullptr;'),
    ('image_explicit', 'sprite_ = sprite;', 'if (!sprite) return 3; sprite_ = sprite;'),
    ('pushbutton_close', 'BaseButton::close();', 'GraphicWin::close();'),
    ('pushbutton_close', 'PushButtonDefaults[17]', 'PushButtonDefaults[0]'),
    ('pushbutton_close', 'field_ADC_ = PushButtonDefaults[9];', ''),
    ('imagebutton_dtor', '{\n}', '{\n    close();\n}'),
    ('pushbutton_dtor', '{\n}', '{\n    close();\n}'),
    ('basebutton_dtor', 'close();', ''),
    ('flat_dtor', 'close();', 'BaseButton::close();'),
    ('font_setter', 'if (font1->is_initialized())', 'if (true)'),
]


@pytest.mark.parametrize('part,before,after', DAMAGE)
def test_compiling_button_damage_rejected(tmp_path, part, before, after):
    parts = actual()
    assert before in parts[part], (part, before)
    parts[part] = parts[part].replace(before, after)
    result = run_host(tmp_path, parts)
    assert result.returncode == 108 and 'assertion line' in result.stderr, result.stderr

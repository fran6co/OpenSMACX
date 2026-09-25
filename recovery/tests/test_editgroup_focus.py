"""Actual EditGroup focus and constructor, over a bounded host projection.

Pinned 612670..6126B6 focuses only first control for any nonzero argument;
zero walks ten live slots in order. Constructor6119E1..611A04 independently
initializes two ten-pointer arrays, ten limits and the process default. The
original complete-object vbtable puts GraphicWin at8C, so the focus body's
historical this-88 means control member4, not raw self arithmetic.

Actual declarations, inheritance and full bodies are extracted unchanged.
Unrelated base/member behavior and EditBox focus are explicit service doubles;
dispatch into recording derived EditBoxes is real C++. Host layout is NOT VC6
layout/vtordisp/native-table proof. Dialogs' own kind declaration is distinguished
from a falsely inherited EditGroup member, without pretending to execute the
full Dialogs/BasePop lifetime. No service failure/throw or GUI proof is claimed.
Unrelated members/methods (including Dialogs' own focus override) are omitted
from the projection; private recovered fields are public only for observation.
Leak checking is disabled for the sandbox; address/undefined checks remain on.
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
    value, = re.findall(pattern, source, re.M)
    return value


def actual_source():
    headers = {name: (ROOT / f'src/{name}.h').read_text() for name in
               ('win', 'graphicwin', 'editbox', 'editgroup', 'dialogs',
                'listbox', 'radiobutton', 'checkbox', 'spritebox')}
    source = (ROOT / 'src/editgroup.cpp').read_text()
    group = braced(headers['editgroup'], 'class EditGroup :')
    dialogs = braced(headers['dialogs'], 'class Dialogs :')
    # These type/ownership facts supplement behavior tests, not replace them.
    assert not re.search(r'^\s*(?:u?int32_t|int)\s+kind_;', group, re.M)
    assert not re.search(r'on_dialog_focus\(long', group)
    result = {
        'WIN_HOOK': braced(headers['win'], 'virtual void on_dialog_focus('),
        'GROUP_HOOK': one(group, r'^\s*(?:virtual )?void on_dialog_focus\(int[^;{}]*\);'),
        'BOX_HOOK': one(headers['editbox'], r'^\s*(?:virtual )?void on_dialog_focus\(int[^;{}]*\);'),
        'GROUP_MEMBERS': '\n'.join(one(group, r'^\s*' + pattern) for pattern in (
            r'EditBox \*boxes_\[10\];', r'char \*text_buffers_\[10\];',
            r'uint32_t limits_\[10\];', r'uint32_t field_84_;')),
        'KIND': one(dialogs, r'^\s*(?:int32_t|int) kind_;'),
        'DEFAULT': one(source, r'^uint32_t EditGroupDefault;'),
        'CTOR': braced(source, 'EditGroup::EditGroup()'),
        'BODY': braced(source, 'void EditGroup::on_dialog_focus(int'),
    }
    for cls, unit in [('Win','win'), ('GraphicWin','graphicwin'), ('EditBox','editbox'),
                      ('EditGroup','editgroup'), ('Dialogs','dialogs'),
                      ('ListBox','listbox'), ('RadioButton','radiobutton'),
                      ('CheckBox','checkbox'), ('SpriteBox','spritebox')]:
        result[cls.upper() + '_BASES'] = one(headers[unit], r'^class ' + cls + r' :[^\{]+\{')
    return result


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <vector>
#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(110);}}
#define require(x) check((x),__LINE__)
struct AutoSound {virtual ~AutoSound(){}};
@WIN_BASES@
public: @WIN_HOOK@
};
class Buffer {};
@GRAPHICWIN_BASES@
};
class Dialog {public: virtual ~Dialog(){};};
@EDITBOX_BASES@
public: @BOX_HOOK@
};
// A qualified call bypassing the recorded derived receiver is a defect here.
void EditBox::on_dialog_focus(int){require(false);}
@EDITGROUP_BASES@
public:
@GROUP_MEMBERS@
@GROUP_HOOK@
EditGroup();
};
@LISTBOX_BASES@
};
@RADIOBUTTON_BASES@
};
@CHECKBOX_BASES@
};
@SPRITEBOX_BASES@
};
@DIALOGS_BASES@
public: @KIND@
};
@DEFAULT@
@CTOR@
@BODY@
static_assert(std::is_same<decltype(EditGroup::boxes_),EditBox*[10]>::value,"ten actual controls");
static_assert(std::is_same<decltype(EditGroup::text_buffers_),char*[10]>::value,"ten owned texts");
static_assert(std::is_same<decltype(EditGroup::limits_),uint32_t[10]>::value,"ten original limits");
static_assert(std::is_same<decltype(&Dialogs::kind_),int32_t Dialogs::*>::value,"kind belongs to Dialogs");
struct Event {int id,value;bool operator==(const Event&o)const{return id==o.id&&value==o.value;}};
std::vector<Event> calls;
struct Box:EditBox {
 int id;std::function<void()> callback;
 void on_dialog_focus(int value) override {calls.push_back({id,value});if(callback)callback();}
};
void send(Win *receiver,int value){receiver->on_dialog_focus(value);}
void attach(EditGroup&g,Box*b){calls.clear();for(int i=0;i<12;++i){b[i].id=i;b[i].callback=nullptr;}
 for(int i=0;i<10;++i){g.boxes_[i]=&b[i];g.text_buffers_[i]=nullptr;g.limits_[i]=900+i;}}
void constructor_cases(){
 for(uint32_t value:{0U,1U,0xdeadbeefU,0xffffffffU}){
 EditGroupDefault=value;EditGroup g;
 for(int i=0;i<10;++i){require(g.boxes_[i]==nullptr);require(g.text_buffers_[i]==nullptr);require(g.limits_[i]==10);}
 require(g.field_84_==value);}
}
void focus_cases(){
 EditGroup g;Box b[12];Win*base=&g;
 attach(g,b);for(int i=0;i<10;++i)g.boxes_[i]=nullptr;
 for(int value:{0,1,-1,17,INT32_MIN,INT32_MAX}){send(base,value);require(calls.empty());}
 // Nonzero never searches past a missing first slot, even if later slots exist.
 attach(g,b);g.boxes_[0]=nullptr;send(base,7);require(calls.empty());
 for(int value:{1,-1,17,INT32_MIN,INT32_MAX}){
 attach(g,b);send(base,value);require(calls==std::vector<Event>({{0,value}}));}
 attach(g,b);send(base,0);std::vector<Event> all;for(int i=0;i<10;++i)all.push_back({i,0});require(calls==all);
 // Each possible sparse position, including final index9, contributes once.
 for(int keep=0;keep<10;++keep){attach(g,b);for(int i=0;i<10;++i)if(i!=keep)g.boxes_[i]=nullptr;
 send(base,0);require(calls==std::vector<Event>({{keep,0}}));}
 attach(g,b);for(int i:{1,3,4,7})g.boxes_[i]=nullptr;
 send(base,0);require(calls==std::vector<Event>({{0,0},{2,0},{5,0},{6,0},{8,0},{9,0}}));
 // Aliases remain separate slots; do not deduplicate identical receivers.
 attach(g,b);for(int i=0;i<10;++i)g.boxes_[i]=&b[2];send(base,0);
 require(calls==std::vector<Event>(10,Event{2,0}));
 // Future slots must reload: replace2, clear4, fill8, then replace9 again.
 attach(g,b);g.boxes_[8]=nullptr;
 b[0].callback=[&]{g.boxes_[0]=&b[11];g.boxes_[2]=&b[10];g.boxes_[4]=nullptr;g.boxes_[8]=&b[11];};
 b[3].callback=[&]{g.boxes_[9]=&b[10];};send(base,0);
 require(calls==std::vector<Event>({{0,0},{1,0},{10,0},{3,0},{5,0},{6,0},{7,0},{11,0},{10,0}}));
 // The first callback cannot turn a nonzero focus into a broadcast or retry.
 attach(g,b);b[0].callback=[&]{g.boxes_[0]=&b[10];};send(base,-37);
 require(calls==std::vector<Event>({{0,-37}}));send(base,2);
 require(calls==std::vector<Event>({{0,-37},{10,2}}));
 // Focus does not overwrite limits, texts or process default snapshots.
 char texts[10];for(int i=0;i<10;++i)g.text_buffers_[i]=&texts[i];
 uint32_t prior=g.field_84_;send(base,0);require(g.field_84_==prior);
 for(int i=0;i<10;++i){require(g.text_buffers_[i]==&texts[i]);require(g.limits_[i]==uint32_t(900+i));}
 // Same actual virtual base route in the projected real shared hierarchy.
 Dialogs composite;attach(composite,b);composite.kind_=4;send(static_cast<Win*>(&composite),3);
 require(calls==std::vector<Event>({{0,3}}));require(composite.kind_==4);
}
int main(){constructor_cases();focus_cases();}
'''


def run(tmp_path, parts):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if not compiler:
        pytest.skip('host C++ compiler required')
    source = HARNESS
    for key, value in parts.items():
        source = source.replace('@' + key + '@', value)
    assert not re.search(r'@[A-Z_]+@', source)
    path = tmp_path / 'focus.cpp'
    path.write_text(source)
    executable = tmp_path / 'focus'
    compiled = subprocess.run([compiler, '-std=c++17', '-O1', '-g',
        '-fsanitize=address,undefined', '-fno-sanitize-recover=all', str(path),
        '-o', str(executable)], capture_output=True, text=True, timeout=40)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_constructor_and_virtual_focus(tmp_path):
    result = run(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


MUTATIONS = {
    'first_receiver': ('BODY', 'EditBox *box = boxes_[0];', 'EditBox *box = boxes_[1];'),
    'negative_is_nonzero': ('BODY', 'focus_id != 0', 'focus_id > 0'),
    'first_argument': ('BODY', 'box->on_dialog_focus(focus_id);', 'box->on_dialog_focus(1);'),
    'first_qualified': ('BODY', 'box->on_dialog_focus(focus_id);', 'box->EditBox::on_dialog_focus(focus_id);'),
    'zero_argument': ('BODY', 'box->on_dialog_focus(0);', 'box->on_dialog_focus(1);'),
    'zero_qualified': ('BODY', 'box->on_dialog_focus(0);', 'box->EditBox::on_dialog_focus(0);'),
    'last_slot': ('BODY', 'int count = 10;', 'int count = 9;'),
    'zero_does_nothing': ('BODY', 'EditBox **slot = boxes_;', 'return; EditBox **slot = boxes_;'),
    'nonzero_broadcast': ('BODY', '        return;', '        /* missing early return */'),
    'zero_skips_first': ('BODY', 'EditBox **slot = boxes_;\n    int count = 10;',
                        'EditBox **slot = boxes_ + 1;\n    int count = 9;'),
    'future_slots_cached': ('BODY', 'EditBox **slot = boxes_;',
                          'EditBox *snapshot[10]; for(int i=0;i<10;++i) snapshot[i]=boxes_[i];\n    EditBox **slot = snapshot;'),
    'focus_overwrites_limit': ('BODY', 'EditBox **slot = boxes_;', 'limits_[9]=10; EditBox **slot = boxes_;'),
    'focus_overwrites_text': ('BODY', 'EditBox **slot = boxes_;', 'text_buffers_[9]=nullptr; EditBox **slot = boxes_;'),
    'constructor_limits': ('CTOR', 'limits_[i] = 10;', 'limits_[i] = 11;'),
    'constructor_default': ('CTOR', 'field_84_ = EditGroupDefault;', 'field_84_ = 0;'),
}


@pytest.mark.parametrize('name', tuple(MUTATIONS) + ('missing_override',))
def test_compiling_behavior_damage_is_rejected(tmp_path, name):
    parts = actual_source()
    if name == 'missing_override':
        # Removing only the redundant virtual keyword would leave the override.
        # Rename declaration and definition: Win dispatch must then use its no-op.
        for key in ('GROUP_HOOK', 'BODY'):
            assert 'on_dialog_focus' in parts[key]
            parts[key] = parts[key].replace('on_dialog_focus', 'damaged_focus', 1)
    else:
        key, before, after = MUTATIONS[name]
        assert parts[key].count(before) == 1, (name, before)
        parts[key] = parts[key].replace(before, after)
    result = run(tmp_path, parts)
    assert result.returncode == 110, (name, result.returncode, result.stderr)
    assert 'assertion line ' in result.stderr
    assert 'runtime error:' not in result.stderr and 'AddressSanitizer' not in result.stderr

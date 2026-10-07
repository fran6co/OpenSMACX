"""Actual tutorial reset and social click with instrumented virtual services.

Original 4BDDF0 clears arrow state before hide, 4BDDF9 reloads the owner,
4BDE01 dispatches its slot63, and 4BDE07 draws before the final resets.
The inactive branch performs the same stores but returns zero without calls.
Complete actual bodies and header member/declaration runs are extracted.
Popup/Win operations and do_all_draws are boundary doubles, not GUI or native
ABI proof. Callback mutation/throws stress sequencing; owner becoming null
during hide is outside the original valid-call domain. Final stores have no
intervening observable service, so their relative machine order is not proved.
The inactive owner=null store is redundant in this nonvolatile C++ domain;
its presence is an original/native instruction fact, not a host damage control.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if not depth:
            return text[start:end + 1]
    raise AssertionError("unterminated actual definition")


def one(text, pattern):
    result, = re.findall(pattern, text, re.M)
    return result


def actual_source(social=False):
    header = (ROOT / "src/tutwin.h").read_text()
    source = (ROOT / "src/tutwin.cpp").read_text()
    win = (ROOT / "src/win.h").read_text()
    popup = (ROOT / "src/popup.h").read_text()
    members = header[header.index("  int32_t field_537C_;"):header.index("\n};", header.index("  int32_t field_537C_;"))]
    declarations = "\n".join((
        "struct Win {", one(win, r"virtual void hide\(\);"),
        braced(win, "virtual void on_update()"), "};",
        "struct Popup : Win {", one(popup, r"void hide\(\);"), "};",
        one(header, r"class TutWin :[^\n]+\{") + " public:",
        one(header, r"int reset\(\);"), members, "};",
        one(header, r"^extern TutWin TutorialWindow;"),
        one(header, r"^extern uint32_t TutWinShownFlag;"),
        one((ROOT / "src/win.h").read_text(), r"void __cdecl do_all_draws\(\);")))
    body = braced(source, "int TutWin::reset()")
    reset = {"field_537C_", "field_5380_", "field_539C_", "field_53A4_", "field_53A8_", "field_53AC_", "field_53B8_", "field_53C4_"}
    scalars = re.findall(r"(?:int32_t|uint32_t) (field_[0-9A-F]+_);", members)
    assert len(scalars) == 21
    initialize = "\n".join(f"p.{name} = {i + 100};" for i, name in enumerate(scalars))
    untouched = [name for name in scalars if name not in reset]
    snapshot = ",".join("static_cast<uint32_t>(p." + name + ")" for name in untouched)
    social_declaration = social_body = ""
    if social:
        sh = (ROOT / "src/socialwin.h").read_text()
        social_declaration = "struct SocialWin { " + one(sh, r"void on_iface_left_click\(int[^;]*\);") + " };"
        social_body = braced((ROOT / "src/socialwin.cpp").read_text(), "void SocialWin::on_iface_left_click(")
    return {
        "DECLARATIONS": declarations,
        "OWNERS": one(source, r"^TutWin TutorialWindow;") + "\n" + one(source, r"^uint32_t TutWinShownFlag;"),
        "BODY": body, "INITIALIZE": initialize, "SNAPSHOT": snapshot,
        "SOCIAL_DECLARATION": social_declaration, "SOCIAL_BODY": social_body,
        "SOCIAL_TEST": "social_test();" if social else "",
        "SOCIAL_SCENARIO": SOCIAL_SCENARIO if social else "",
    }


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <functional>
#define __cdecl
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(106);}}
#define require(x) check((x),__LINE__)
DECLARATIONS
OWNERS
SOCIAL_DECLARATION
TutWin *active=nullptr;
std::string events;
std::function<void(char)> callback;
int update_id=0;
void service(char kind){events+=kind;if(callback)callback(kind);}
void Win::hide(){service('Q');}
void Popup::hide(){require(this==static_cast<Popup*>(active));service('H');}
struct DerivedTutorial : TutWin {void hide(){require(this==active);service('V');}};
struct Owner : Win {int id;explicit Owner(int v):id(v){}void on_update(){update_id=id;service('U');}};
Owner first(1),second(2),third(3);
void __cdecl do_all_draws(){service('D');}
BODY
SOCIAL_BODY
void seed(TutWin&p,bool owned){
    INITIALIZE
    for(int i=0;i<4;++i)p.field_53B0_[i]=static_cast<uint8_t>(0xa0+i);
    p.field_53D4_=owned?&first:nullptr;
    TutWinShownFlag=91;events.clear();callback=nullptr;update_id=0;active=&p;
}
std::vector<uint32_t> preserved(TutWin&p){return {SNAPSHOT,
    p.field_53B0_[0],p.field_53B0_[1],p.field_53B0_[2],p.field_53B0_[3]};}
void reset_state(TutWin&p){
    require(p.field_53D4_==nullptr&&p.field_53A4_==0);
    require(p.field_5380_==-1&&p.field_537C_==-1&&p.field_539C_==-1);
    require(p.field_53A8_==0&&p.field_53AC_==-1&&p.field_53B8_==0&&p.field_53C4_==0);
    require(TutWinShownFlag==0);
}
void boundary(TutWin&p,char hide,int throw_stage){
    callback=[&,hide,throw_stage](char kind){
        if(events.size()==1){
            require(kind==hide&&p.field_53D4_==&first&&p.field_53A4_==0);
            require(p.field_537C_==100&&p.field_5380_==101&&p.field_53AC_==112);
            require(TutWinShownFlag==91);
            p.field_53D4_=&second;p.field_53A4_=31;p.field_5380_=231;
        }else if(events.size()==2){
            require(kind=='U'&&update_id==2&&p.field_53D4_==&second);
            require(p.field_53A4_==31&&p.field_5380_==231&&TutWinShownFlag==91);
            p.field_53D4_=&third;p.field_53A4_=41;p.field_53AC_=241;TutWinShownFlag=92;
        }else if(events.size()==3){
            require(kind=='D'&&p.field_53D4_==&third&&p.field_53A4_==41);
            require(p.field_53AC_==241&&TutWinShownFlag==92);
            p.field_53D4_=&first;p.field_53A4_=51;p.field_53C4_=251;TutWinShownFlag=93;
        }else require(false);
        if(static_cast<int>(events.size())==throw_stage)throw throw_stage;
    };
}
void scenarios(TutWin&p,char hide){
    for(int owned=0;owned<2;++owned){
        seed(p,owned);const auto before=preserved(p);
        if(owned)boundary(p,hide,-1);
        require(p.reset()==owned);reset_state(p);require(preserved(p)==before);
        require(events==(owned?std::string(1,hide)+"UD":""));
        callback=nullptr;events.clear();require(p.reset()==0);reset_state(p);require(events.empty());
    }
    for(int stage=1;stage<=3;++stage){
        seed(p,true);const auto before=preserved(p);boundary(p,hide,stage);
        bool caught=false;try{p.reset();}catch(int n){require(n==stage);caught=true;}
        require(caught&&events==std::string(std::string(1,hide)+"UD").substr(0,stage));
        require(p.field_53D4_==(stage==1?&second:stage==2?&third:&first));
        require(p.field_53A4_==21+stage*10&&TutWinShownFlag==(stage==1?91:90+stage));
        require(preserved(p)==before);
    }
    callback=nullptr;
}
SOCIAL_SCENARIO
int main(){DerivedTutorial derived;TutWin plain;scenarios(derived,'V');scenarios(plain,'H');
    scenarios(TutorialWindow,'H');SOCIAL_TEST}
'''


SOCIAL_SCENARIO = r'''
void social_test(){SocialWin first_social,second_social;
    for(int pass=0;pass<4;++pass){
        seed(TutorialWindow,pass&1);const auto before=preserved(TutorialWindow);
        if(pass&1)boundary(TutorialWindow,'H',-1);
        SocialWin&social=pass&2?first_social:second_social;
        social.on_iface_left_click(pass==0?-2147483647:pass*13,pass==3?2147483647:-pass*7);
        reset_state(TutorialWindow);require(preserved(TutorialWindow)==before);
        require(events==(pass&1?"HUD":""));
    }
    callback=nullptr;
}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "tutorial.cpp", tmp_path / "tutorial"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                             "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                            capture_output=True, text=True, timeout=40)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_tutorial_reset(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


def test_actual_social_tutorial_click(tmp_path):
    result = run_host(tmp_path, actual_source(social=True))
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ("hide();", "Popup::hide();"),
    ("hide();", "Win *saved=field_53D4_; hide(); field_53D4_=saved;"),
    ("hide();", ""),
    ("field_53D4_->on_update();", "field_53D4_->Win::on_update();"),
    ("field_53D4_->on_update();\n        do_all_draws();", "do_all_draws();\n        field_53D4_->on_update();"),
    ("do_all_draws();", ""),
    ("field_53A4_ = 0;\n        hide();", "hide();\n        field_53A4_ = 0;"),
    ("hide();", "TutWinShownFlag=0; hide();"),
    ("return 1;", "return 0;"),
    ("return 0;", "return 1;"),
    ("if (field_53D4_ != 0)", "if (false)"),
    ("    field_53D4_ = 0;\n    field_53A4_", "    do_all_draws();\n    field_53D4_ = 0;\n    field_53A4_"),
    ("field_5380_ = -1;", "field_5384_ = -1;"),
]
for indentation in ("        ", "    "):
    for assignment in ("field_53D4_ = 0;", "field_53A4_ = 0;", "field_5380_ = -1;",
                       "field_537C_ = -1;", "field_539C_ = -1;", "field_53A8_ = 0;",
                       "field_53AC_ = -1;", "field_53B8_ = 0;", "field_53C4_ = 0;", "TutWinShownFlag = 0;"):
        if indentation == "    " and assignment == "field_53D4_ = 0;":
            continue  # Already null on this branch; deleting the store is equivalent.
        # For the active branch's A4, target its post-callback reset, not the
        # independently tested pre-hide store. All other reset stores are unique.
        before = "\n" + indentation + assignment
        if indentation == "        " and assignment == "field_53A4_ = 0;":
            before = "field_53D4_ = 0;\n        field_53A4_ = 0;"
            after = "field_53D4_ = 0;"
        else:
            after = ""
        DAMAGE.append((before, after))


@pytest.mark.parametrize("before,after", DAMAGE)
def test_compiling_reset_damage_rejected(tmp_path, before, after):
    parts = actual_source()
    assert before in parts["BODY"], before
    parts["BODY"] = parts["BODY"].replace(before, after, 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 106 and "assertion line" in result.stderr, result.stderr


@pytest.mark.parametrize("replacement", ["", "TutWinShownFlag = 0;", "TutWin other; other.field_53D4_ = 0; other.reset();"])
def test_compiling_social_click_damage_rejected(tmp_path, replacement):
    parts = actual_source(social=True)
    assert "TutorialWindow.reset();" in parts["SOCIAL_BODY"]
    parts["SOCIAL_BODY"] = parts["SOCIAL_BODY"].replace("TutorialWindow.reset();", replacement)
    result = run_host(tmp_path, parts)
    assert result.returncode == 106 and "assertion line" in result.stderr, result.stderr

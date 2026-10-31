"""Actual font policy/selector controls; no historical TU or linked proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def test_native_ordinary_owner_resolves_the_single_real_definition():
    from match_recovery.source import read_file
    from match_recovery.source_selection import compilation_context
    from match_recovery.measurement import Measurement
    from match_recovery.ownership_source import project_view
    from match_recovery.application_binary import select_bindings
    from match_recovery.model import Origin
    records = [r for name in ('popup.cpp', 'interface_fonts.cpp')
               for r in read_file(ROOT / 'src' / name) if r.address == 0x588AA0]
    assert len(records) == 1
    record = records[0]
    assert record.path == ROOT / 'src/interface_fonts.cpp'
    assert not record.body and not record.context
    assert compilation_context(ROOT, record, Measurement.read(ROOT)) == record.path
    assert record.image_spans == ((0x588AA0, 0x588F8E),)
    assert 'void __cdecl config_popups() {' not in (ROOT / 'src/popup.cpp').read_text()
    assert record.path.read_text().count('void __cdecl config_popups() {') == 1
    view, snapshot = project_view(ROOT)
    entity = Origin(snapshot['image_sha256'], 'pe:va', record.address).entity_id
    bindings, refused, _ = select_bindings(view, frozenset({entity}))
    assert not refused and len(bindings) == 1
    assert bindings[0].unit == 'src/interface_fonts.cpp'
    assert bindings[0].symbol == '?config_popups@@YAXXZ'


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['none', 'menu_size', 'tie_rule'])
def test_actual_policy_and_selector(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + ' unavailable')
    text = (ROOT / 'src/interface_fonts.cpp').read_text()
    selector = body(text, 'Font *__cdecl find_font(')
    policy = body(text, 'void __cdecl config_popups() {')
    if damage == 'menu_size':
        anchor = 'PopupMenuFont = find_font(12, 0);'
        assert anchor in policy
        policy = policy.replace(anchor, 'PopupMenuFont = find_font(16, 0);', 1)
    if damage == 'tie_rule':
        assert 'delta < best_delta' in selector
        selector = selector.replace('delta < best_delta', 'delta <= best_delta', 1)
    names = re.findall(r'extern Font \*(\w+);', (ROOT / 'src/popup_fonts.h').read_text())
    harness = r'''
#include <cstring>
#include <cstddef>
#include <cstdlib>
#define __cdecl
#define FALSE 0
static int init_count;
struct Font { void init(const char*, int, int) { ++init_count; } };
static const size_t FontSizeTableCount=12;
static const int FontSizeTable[]={40,36,32,24,22,20,18,16,14,12,10,8};
static const int FontStyleTable[]={0,1,2,3};
static Font FontTable[48];
static const char *DefaultFontFace="Arial", *DefaultLargeFontFace="Arial Narrow";
static int WinScreenWidth,WinScreenHeight=600,IfaceScreenWidth,IfaceScreenHeight,FastUnitAnim;
static char StringTemp[100]; static const char *ScriptPopupFile="script";
static Font *WinBubbleFont,*PullDownDefaultFont;
static struct { Font *default_font_; } WinDynamicDefaults;
struct BasePop { static void set_def_file(const char *s){if(std::strcmp(s,"script.txt"))std::abort();} };
static int main_size, interlude_size;
static int prefs_get(const char *s,int fallback,int){
 if(!std::strcmp(s,"MainFontSize"))return main_size;
 if(!std::strcmp(s,"InterludeFontSize"))return interlude_size;
 return fallback;
}
static void prefs_put(const char*,int,int){}
static int normal_calls; static void popups_normal(){++normal_calls;}
static Font *menu_font; struct Menu {static void set_def_font(Font *p){menu_font=p;}};
@@NAMES@@
@@SELECTOR@@
@@POLICY@@
static void check(bool b){if(!b)std::abort();}
int main(){
 // Independent pinned-size expected indices, including the strict first tie.
 check(find_font(15,0)==FontTable+28);
 check(find_font(16,2)==FontTable+30);
 check(find_font(12,1)==FontTable+37);
 check(find_font(10,1)==FontTable+41);
 check(find_font(20000,0)==FontTable);
 for(int width: {800,1024}){
  WinScreenWidth=width;main_size=18;interlude_size=16;init_count=0;
  config_popups();
  check(init_count==48 && IfaceScreenWidth==width && IfaceScreenHeight==600);
  check(PopupMainFont==FontTable+(width==800?28:24));
  check(InterludeNormalFont==FontTable+(width==800?32:28));
  check(PopupMainItalicFont==PopupMainFont+2 && PopupMainBoldFont==PopupMainFont+1);
  check(InterludeThirdFont==InterludeNormalFont+2 && InterludeSecondFont==InterludeNormalFont+1);
  check(PopupMediumFont==FontTable+28 && PopupLargeFont==FontTable+28);
  check(PopupMediumItalicFont==FontTable+30 && PopupMediumBoldFont==FontTable+29);
  check(PopupLargeItalicFont==FontTable+30 && PopupLargeBoldFont==FontTable+29);
  check(PopupMenuFont==FontTable+(width==800?36:28));
  check(PopupMenuItalicFont==PopupMenuFont+2 && PopupMenuBoldFont==PopupMenuFont+1);
  check(PopupSmallFont==FontTable+36 && PopupSmallItalicFont==FontTable+38 && PopupSmallBoldFont==FontTable+37);
  check(WinBubbleFont==PopupSmallFont && PopupDefaultWindowFont==FontTable+37 && PopupMapLabelFont==FontTable+41);
  check(menu_font==PopupMenuFont && PullDownDefaultFont==PopupMenuFont && WinDynamicDefaults.default_font_==PopupDefaultWindowFont);
 }
 check(normal_calls==2);
}
'''
    harness = '#include <initializer_list>\n' + harness
    code = harness.replace('@@NAMES@@', '\n'.join('static Font *' + n + ';' for n in names))
    code = code.replace('@@SELECTOR@@', selector).replace('@@POLICY@@', policy)
    cpp = tmp_path / 'control.cpp'
    exe = tmp_path / 'control'
    cpp.write_text(code)
    build = subprocess.run([compiler, '-std=c++17', '-O0', str(cpp), '-o', str(exe)], capture_output=True, text=True)
    assert build.returncode == 0, build.stderr
    run = subprocess.run([str(exe)], capture_output=True)
    assert (run.returncode == 0) == (damage == 'none')

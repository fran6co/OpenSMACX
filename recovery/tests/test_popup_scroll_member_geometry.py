"""Actual-source member geometry; host pointer-return width is not ABI proof."""
from pathlib import Path
import re,shutil,subprocess
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
ROOT=Path(__file__).resolve().parents[2]

def projected_control(damage=None):
    row=next(r for r in read_file(ROOT/'src/popup.cpp') if r.address==0x406260)
    body=_definition(row.region,'Scroll *Popup::on_listbox_scroll_create(')
    # The actual pointer return needs no host-width adaptation. Native
    # compilation separately verifies the original 32-bit virtual ABI.
    assert 'return &scroll_;' in body
    getter=_definition((ROOT/'src/maininterface.h').read_text(),'Sprite &scroll_frame_sprite(')
    capacity=re.search(r'int visible_item_capacity\(\) const \{[^\n]*\}',(ROOT/'src/listbox.h').read_text()).group(0)
    if damage:
        old,new=damage;assert old in body;body=body.replace(old,new,1)
    fixture=(Path(__file__).parent/'popup_scroll_member_geometry.cpp').read_text()
    return fixture.replace('// ACTUAL_MAIN_ACCESSOR',getter).replace('// ACTUAL_CAPACITY_ACCESSOR',capacity).replace('// ACTUAL_SOURCE_BODY',body)

def run(tmp_path,compiler,text):
    source=tmp_path/'control.cpp';source.write_text(text);binary=tmp_path/'control'
    subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-Werror','-pedantic',str(source),'-o',str(binary)],check=True,capture_output=True)
    return subprocess.run([str(binary)],capture_output=True)

@pytest.mark.parametrize('name',['g++','clang++'])
def test_real_member_geometry(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler:pytest.skip(name+' unavailable')
    result=run(tmp_path,compiler,projected_control())
    assert result.returncode==0,result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new',[
    ('scroll_frame_sprite(2)','scroll_frame_sprite(3)'),
    ('scroll_frame_sprite(1)','scroll_frame_sprite(0)'),
    ('scroll_frame_sprite(0)','scroll_frame_sprite(2)'),
    ('if (is_visible())','if (!is_visible())'),
])
def test_member_damage_detected(tmp_path,old,new):
    compiler=shutil.which('g++')
    if not compiler:pytest.skip('g++ unavailable')
    result=run(tmp_path,compiler,projected_control((old,new)))
    assert result.returncode<0,'damaged geometry must fail runtime control'

"""Actual-source controls, not native ABI or full game acceptance."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT=Path(__file__).resolve().parents[2]
ROWS=(('statuswin',0x50f630,'void __stdcall sub_50f630('),
      ('diplopop',0x539970,'void __cdecl sub_539970('),
      ('netdaemon',0x593370,'void __cdecl sub_593370('),
      ('netdaemon',0x593480,'void __cdecl sub_593480('),
      ('general',0x584d30,'char *__cdecl sub_584d30('))

def actual_bodies():
    bodies=[]
    for owner,address,signature in ROWS:
        record=next(r for r in read_file(ROOT/('src/'+owner+'.cpp')) if r.address==address)
        assert record.region and 'BODY GOES HERE.' not in record.region
        bodies.append(_definition(record.region,signature))
    return '\n'.join(bodies)

def control(tmp_path,compiler,bodies):
    fixture=(Path(__file__).parent/'short_owner_helpers.cpp').read_text()
    assert fixture.count('// ACTUAL_SOURCE_BODIES')==1
    source=tmp_path/'control.cpp'; source.write_text(fixture.replace('// ACTUAL_SOURCE_BODIES',bodies))
    binary=tmp_path/'control'
    subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-Werror','-pedantic',str(source),'-o',str(binary)],check=True,capture_output=True)
    return subprocess.run([str(binary)],capture_output=True)

@pytest.mark.parametrize('name',['g++','clang++'])
def test_actual_short_helpers(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler: pytest.skip(name+' unavailable')
    result=control(tmp_path,compiler,actual_bodies())
    assert result.returncode==0,result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new',[
    ('ConsoleStatusWin.redraw();','if (unused == 0) ConsoleStatusWin.redraw();'),
    ('if (static_cast<Win &>(popup).is_visible())','if (!static_cast<Win &>(popup).is_visible())'),
    ('popup.draw_info();\n        static_cast<GraphicWin &>(popup).soft_update(&popup.rect1_);',
     'static_cast<GraphicWin &>(popup).soft_update(&popup.rect1_);\n        popup.draw_info();'),
    ('synch(1, 0, 0, 0, 0, 1, 0x2101)','synch(2, 0, 0, 0, 0, 1, 0x2101)'),
    ('synch(0x17, value, 0, 0, 0, 1, 0x2101)','synch(0x17, value + 1, 0, 0, 0, 1, 0x2101)'),
    ('dest[count - 1] = 0;','dest[count] = 0;'),
])
def test_damage_detected(tmp_path,old,new):
    compiler=shutil.which('g++')
    if not compiler: pytest.skip('g++ unavailable')
    bodies=actual_bodies(); assert old in bodies
    result=control(tmp_path,compiler,bodies.replace(old,new,1))
    assert result.returncode<0,'damaged body must fail runtime controls'

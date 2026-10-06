"""Host controls compile production startup bodies, not retained proposals."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name('startup_owner_host.cpp')


def product_bodies():
    records = {r.address:r for r in read_file(ROOT / 'src/game.cpp')}
    bodies = '\n'.join(records[a].region for a in (0x58EE60,0x4595C0))
    assert all(records[a].state.value == 'implemented' and not records[a].extract_error
               for a in (0x58EE60,0x4595C0))
    # This tiny projection uses the real caller's expression, not a second
    # manually maintained startup wrapper or executable-address call.
    source = (ROOT / 'src/game.cpp').read_text()
    assert source.count('    InterfaceMessageWin.init();') == 1
    bodies += '\nvoid startup_owner_initialize_message_window() {\n' + \
              next(line for line in source.splitlines() if line == '    InterfaceMessageWin.init();') + '\n}\n'
    return bodies


@pytest.mark.parametrize('compiler_name',['g++','clang++'])
@pytest.mark.parametrize('mutation',['none','font','multiplayer_width','height','initial_store'])
def test_actual_startup_order_geometry_lifetime_and_mutants(tmp_path,compiler_name,mutation):
    compiler=shutil.which(compiler_name)
    if not compiler:pytest.skip(f'{compiler_name} unavailable')
    body=product_bodies()
    replacements={
        'font':('MainInterfaceGlobal.tertiary_font()', 'MainInterfaceGlobal.secondary_font()'),
        'multiplayer_width':('multi_client.left - multi_client.right','multi_client.right - multi_client.left'),
        'height':('-10 - static_cast<int>(ConsoleGlobal.dib_.bmiHeader.biHeight)',
                  '-10 + static_cast<int>(ConsoleGlobal.dib_.bmiHeader.biHeight)'),
        'initial_store':('ConsoleGlobal.field_23D80_ = 10;', ''),
    }
    if mutation!='none':
        old,new=replacements[mutation];assert body.count(old)==1;body=body.replace(old,new)
    fixture=FIXTURE.read_text();assert fixture.count('// ACTUAL_STARTUP_BODIES')==1
    cpp=tmp_path/'actual.cpp';binary=tmp_path/'actual'
    cpp.write_text(fixture.replace('// ACTUAL_STARTUP_BODIES',body))
    subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],
                   check=True,capture_output=True,text=True,timeout=30)
    result=subprocess.run([str(binary)],capture_output=True,timeout=30)
    assert (result.returncode==0)==(mutation=='none'),result.stderr


def test_original_identity_seed_retirement_and_real_members():
    records={r.address:r for r in read_file(ROOT/'src/game.cpp')}
    for address,name,end in ((0x58EE60,'?desktop_init@@YAHH@Z',0x58EFEB),
                             (0x4595C0,'?bung_momma@@YAXXZ',0x45967F)):
        row=records[address];assert row.name==name and row.image_spans==((address,end),)
        assert not (ROOT/f'src/unrecovered/{address:08x}.cpp').exists()
    assert 'Font *tertiary_font() { return &font3_; }' in (ROOT/'src/maininterface.h').read_text()
    assert 'const RECT &client_rect() const { return client_rect_; }' in (ROOT/'src/win.h').read_text()
    assert 'messagewin_init(' not in (ROOT/'src/game.cpp').read_text()


def test_actual_system_startup_call_order_and_mutant():
    row=next(r for r in read_file(ROOT/'src/game.cpp') if r.address==0x58F040)
    calls=('WinBubbleWindow.init(', 'InterfaceMessageWin.init();',
           'DatalinkMain.init();', 'bung_momma();')
    def ordered(body):
        positions=[body.index(call) for call in calls]
        return positions==sorted(positions)
    assert ordered(row.region)
    mutant=row.region.replace('    InterfaceMessageWin.init();\n','')
    mutant=mutant.replace('    DatalinkMain.init();','    DatalinkMain.init();\n    InterfaceMessageWin.init();')
    assert not ordered(mutant)

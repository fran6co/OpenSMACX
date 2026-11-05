"""Host controls compile production startup bodies, not retained proposals."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name('startup_owner_host.cpp')


def product_bodies(font_mutant=False):
    game_records = {r.address:r for r in read_file(ROOT / 'src/game.cpp')}
    interface_records = {r.address:r for r in read_file(ROOT / 'src/maininterface.cpp')}
    desktop = game_records[0x58EE60].region
    bung = interface_records[0x4595C0].region
    if font_mutant:
        definition = _definition(bung, 'void __cdecl bung_momma()')
        old = 'MainInterfaceGlobal.tertiary_font()'
        assert definition.count(old) == 1
        bung = bung.replace(definition, definition.replace(
            old, 'MainInterfaceGlobal.secondary_font()', 1), 1)
    bodies = '\n'.join((desktop, bung))
    selected = (game_records[0x58EE60], interface_records[0x4595C0])
    assert all(record.state.value == 'implemented' and not record.extract_error
               for record in selected)
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
    body=product_bodies(font_mutant=(mutation=='font'))
    replacements={
        'multiplayer_width':('static_cast<uint32_t>(multi_client.left) -\n            static_cast<uint32_t>(multi_client.right)',
                             'static_cast<uint32_t>(multi_client.right) -\n            static_cast<uint32_t>(multi_client.left)'),
        'height':('static_cast<uint32_t>(-10) -\n        static_cast<uint32_t>(ConsoleGlobal.dib_.bmiHeader.biHeight)',
                  'static_cast<uint32_t>(-10) +\n        static_cast<uint32_t>(ConsoleGlobal.dib_.bmiHeader.biHeight)'),
        'initial_store':('ConsoleGlobal.field_23D80_ = 10;', ''),
    }
    if mutation not in ('none', 'font'):
        old,new=replacements[mutation];assert body.count(old)==1;body=body.replace(old,new)
    fixture=FIXTURE.read_text();assert fixture.count('// ACTUAL_STARTUP_BODIES')==1
    cpp=tmp_path/'actual.cpp';binary=tmp_path/'actual'
    cpp.write_text(fixture.replace('// ACTUAL_STARTUP_BODIES',body))
    subprocess.run([compiler,'-std=c++98','-Wall','-Wextra','-Werror',str(cpp),'-o',str(binary)],
                   check=True,capture_output=True,text=True,timeout=30)
    result=subprocess.run([str(binary)],capture_output=True,timeout=30)
    assert (result.returncode==0)==(mutation=='none'),result.stderr


def test_original_identity_seed_retirement_and_real_members():
    game_records={r.address:r for r in read_file(ROOT/'src/game.cpp')}
    interface_records={r.address:r for r in read_file(ROOT/'src/maininterface.cpp')}
    for row,address,name,end in (
            (game_records[0x58EE60],0x58EE60,'?desktop_init@@YAHH@Z',0x58EFEB),
            (interface_records[0x4595C0],0x4595C0,'?bung_momma@@YAXXZ',0x45967F)):
        assert row.address==address and row.name==name and row.image_spans==((address,end),)
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

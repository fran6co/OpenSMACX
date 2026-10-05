"""Compile the actual complete order-text body and helper, not a candidate copy."""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name('say_orders_host.cpp')


def definition(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for cursor in range(opening, len(text)):
        if text[cursor] == '{':
            depth += 1
        elif text[cursor] == '}':
            depth -= 1
            if depth == 0:
                return text[start:cursor + 1]
    raise AssertionError('unterminated production function')


def product_bodies():
    source = (ROOT / 'src/statuswin.cpp').read_text()
    bodies = '\n'.join(definition(source, signature) for signature in (
        'static const char *vehicle_order_text_id(int id)',
        'static const char *vehicle_order_rule_text(LPSTR encoded_id)',
        'void __cdecl say_orders(char *text, int veh_id)',
    ))
    compass = definition((ROOT / 'src/alpha.cpp').read_text(),
                         'LPSTR __cdecl compass_name(int direction)')
    return bodies, compass


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize('mutation', ['none', 'prefix', 'base_name', 'blocked'])
def test_actual_complete_order_text_and_mutants(tmp_path, compiler_name, mutation):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f'{compiler_name} unavailable')
    body, compass = product_bodies()
    replacements = {
        'prefix': ('vehicle.waypoint_count <= vehicle.patrol_current_point',
                   'vehicle.waypoint_count >= vehicle.patrol_current_point'),
        'base_name': ('Bases[base_id].name_string', '&Bases[base_id].name_string[7]'),
        'blocked': ('::strcat(text, vehicle_order_rule_text(Order[26].order));\n        return;',
                    '::strcat(text, vehicle_order_rule_text(Order[26].order));'),
    }
    if mutation != 'none':
        old, new = replacements[mutation]
        assert body.count(old) == 1
        body = body.replace(old, new)
    fixture = FIXTURE.read_text()
    assert fixture.count('// ACTUAL_SAY_ORDERS_BODY') == 1
    assert fixture.count('// ACTUAL_COMPASS_BODY') == 1
    source = fixture.replace('// ACTUAL_SAY_ORDERS_BODY', body)
    source = source.replace('// ACTUAL_COMPASS_BODY', compass)
    cpp = tmp_path / 'actual.cpp'
    binary = tmp_path / 'actual'
    cpp.write_text(source)
    subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra', '-Werror',
                    str(cpp), '-o', str(binary)], check=True, timeout=30)
    result = subprocess.run([str(binary)], capture_output=True, timeout=30)
    assert (result.returncode == 0) == (mutation == 'none'), result.stderr


def test_real_caller_and_seed_identity_continuity():
    source = (ROOT / 'src/statuswin.cpp').read_text()
    caller = definition(source, 'void StatusWin::draw_active')
    assert 'say_orders(StringTemp, vehicle_id)' in caller
    assert 'say_orders(StringTemp, vehicle.proto_id)' not in caller
    wrapper = definition(source, 'void say_orders(int veh_id)')
    assert 'say_orders(StringTemp, veh_id)' in wrapper
    record, = [row for row in read_file(ROOT / 'src/statuswin.cpp')
               if row.address == 0x4B43E0]
    assert record.name == '?say_orders@@YAXPADH@Z'
    assert record.image_spans == ((0x4B43E0, 0x4B496D),)
    assert not (ROOT / 'src/recovered/units/004b43e0.cpp').exists()
    body, _ = product_bodies()
    assert '0x004B43E0' not in body
    assert 'Order[26].order' in body
    assert 'VehicleOrderUnavailableTextId' not in source

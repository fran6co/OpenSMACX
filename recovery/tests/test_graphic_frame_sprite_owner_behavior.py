"""Actual frame owner controls; host seams do not prove native hierarchy/ABI."""
from pathlib import Path
import shutil
import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
from test_popup_animation_owner_behavior import run_control

ROOT = Path(__file__).resolve().parents[2]

def projected(damage=None):
    path = ROOT / 'src/graphicwin.cpp'
    row = next(r for r in read_file(path) if r.address == 0x5D68C0)
    helper = _definition(path.read_text(), 'static int frame_dimension_sum(')
    body = _definition(row.region, 'void GraphicWin::apply_frame_sprites(')
    body = helper + '\n' + body
    if damage is not None:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    header = (ROOT / 'src/graphicwin.h').read_text()
    start = header.index('struct NCSprites {')
    types = header[start:header.index('};', start) + 2]
    fixture = Path(__file__).with_name('graphic_frame_sprite_owner_control.cpp').read_text()
    return fixture.replace('// ACTUAL_SOURCE_TYPES', types).replace('// ACTUAL_SOURCE_BODY', body)

@pytest.mark.parametrize('name', ['g++', 'clang++'])
def test_actual_frame_sprite_owner(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + ' unavailable')
    result = run_control(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors='replace')

@pytest.mark.parametrize('old,new', [
    ('frame_top_right_ = sprites->top_right;', 'frame_top_right_ = sprites->top_left;'),
    ('frame_noncaption_middle_ = sprites->noncaption_middle;', 'frame_noncaption_middle_ = sprites->top;'),
    ('iFlags_ & 0x10', 'iFlags_ & 0x20'),
    ('!top_left || !top_right || !bottom_left || !bottom_right', '!top_left || !top_right || !bottom_right'),
    ('width <= bottom_width', 'width >= bottom_width'),
    ('height <= right_height', 'height >= right_height'),
    ('bottom_right->height(), top_right->height()', 'bottom_right->height(), top_left->height()'),
    ('    field_138_ = height;\n}', '    field_138_ = height + 1;\n}'),
    ('width <= bottom_width', 'static_cast<unsigned int>(width) <= static_cast<unsigned int>(bottom_width)'),
])
def test_frame_damage_detected(tmp_path, old, new):
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('g++ unavailable')
    result = run_control(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0, 'damaged actual body must fail a runtime witness'

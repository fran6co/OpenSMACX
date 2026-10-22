"""Actual-source picker redraw order controls, not GUI/runtime acceptance."""
from pathlib import Path
import os
import resource
import shutil
import subprocess

import pytest
from test_win_zorder_pointer_flow_oct10 import extract


ROOT = Path(__file__).resolve().parents[2]
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
PICKERS = {
    'armor': ('void DesignWin::select_armor()', 'draw_armor', 'Armor',
              'MaxArmorNum'),
    'reactor': ('void DesignWin::select_reactor()', 'draw_reactor', 'Reactor',
                'MaxReactorNum'),
}


def run(tmp_path, compiler_name, source, should_pass):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip('host compiler unavailable')
    executable = tmp_path / 'picker-redraw'
    built = subprocess.run(
        [compiler, '-std=c++98', '-O0', '-x', 'c++', '-', '-o', str(executable)],
        input=source, text=True, capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    result = subprocess.run(
        [str(executable)], cwd=tmp_path, capture_output=True,
        preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)),
    )
    assert (result.returncode == 0) == should_pass, result.stderr


def source_parts(kind, mutation=''):
    text = (ROOT / 'src/designwin.cpp').read_text()
    signature, own_draw, _, _ = PICKERS[kind]
    body = extract(text, signature)
    helpers = text[text.index('template <typename Record>\nstatic int picker_row_before'):
                   text.index('static const int DesignSelectableChassisCount')]
    redraw_helper = extract(text, 'static void redraw_design_picker_changes(DesignWin *window)')
    tail = (
        '    check_abil();\n'
        f'    {own_draw}(0);\n'
        '    draw_vehicle(0);\n'
        '    draw_special_1(0);\n'
        '    draw_special_2(0);\n'
        '    draw_flash();\n'
        '    draw_info();\n'
        '    on_update();'
    )
    assert body.count(tail) == 1
    if mutation == 'old_generic_overdraw':
        body = body.replace(tail, '    redraw_design_picker_changes(this);', 1)
    elif mutation == 'omit_own_draw':
        body = body.replace(f'    {own_draw}(0);\n', '', 1)
    elif mutation == 'omit_common_draw':
        body = body.replace('    draw_vehicle(0);\n', '', 1)
    elif mutation == 'reorder_own_common':
        body = body.replace(
            f'    {own_draw}(0);\n    draw_vehicle(0);',
            f'    draw_vehicle(0);\n    {own_draw}(0);', 1)
    elif mutation == 'nonzero_draw_argument':
        body = body.replace(f'{own_draw}(0);', f'{own_draw}(1);', 1)
    elif mutation == 'bypass_virtual_update':
        body = body.replace('    on_update();', '    DesignWin::on_update();', 1)
    elif mutation:
        raise AssertionError(mutation)
    return helpers, redraw_helper, body


HARNESS = r'''
#include <cassert>
#include <cstring>

struct ArmorRecord { int preq_tech; int defense_rating; };
struct WeaponRecord { int preq_tech; int offense_rating; };
struct ReactorRecord { int preq_tech; };
ArmorRecord Armor[6] = {{-1,0},{-1,1},{-1,2},{-1,0},{-1,0},{-1,0}};
WeaponRecord Weapon[6] = {{-1,0},{-1,1},{-1,0},{-1,0},{-1,0},{-1,0}};
ReactorRecord Reactor[6] = {{-1},{-1},{-1},{-1},{-1},{-1}};
const int MaxArmorNum = 4;
const int MaxReactorNum = 4;
unsigned char SpotlightColorTable[256];

struct Event { const char *name; int arg; };
Event events[64];
int event_count = 0;
int picker_reply = 2;
void record(const char *name, int arg = -999) {
    assert(event_count < 64);
    events[event_count].name = name;
    events[event_count].arg = arg;
    ++event_count;
}
void expect(const Event *wanted, int count) {
    assert(event_count == count);
    for (int i = 0; i < count; ++i) {
        assert(std::strcmp(events[i].name, wanted[i].name) == 0);
        assert(events[i].arg == wanted[i].arg);
    }
}
void clear_events() { event_count = 0; }

struct SelectPartWin {
    int exec(int x, int y, int faction, int kind, int selected,
             int chassis, int weapon, int armor, int abilities, int reactor,
             unsigned char *palette) {
        assert(x == 111 && y == 222 && faction == 3);
        assert(chassis == 17 && weapon == 1 && armor == 1);
        assert(abilities == 0x40000 && reactor == 1);
        assert(palette == SpotlightColorTable);
        assert(kind == 2 || kind == 3);
        assert(selected == (kind == 2 ? 1 : 1));
        record("exec", kind);
        return picker_reply;
    }
    ~SelectPartWin() { record("destroy"); }
};

struct DesignWin {
    struct Rect { int left, top; } outer_rect_;
    int factionID_;
    int protoChassisType_, protoWeaponType_, protoArmorType_;
    int protoAbilityFlags_, protoReactorType_;
    DesignWin() : outer_rect_(), factionID_(3), protoChassisType_(17),
        protoWeaponType_(1), protoArmorType_(1), protoAbilityFlags_(0x40000),
        protoReactorType_(1) {
        outer_rect_.left = 111;
        outer_rect_.top = 222;
    }
    void suggest_abil(int bits) { record("suggest_abil", bits); }
    void check_abil() { record("check_abil"); }
    void draw_armor(int arg) { record("draw_armor", arg); }
    void draw_reactor(int arg) { record("draw_reactor", arg); }
    void draw_chassis(int arg) { record("draw_chassis", arg); }
    void draw_weapon(int arg) { record("draw_weapon", arg); }
    void draw_vehicle(int arg) { record("draw_vehicle", arg); }
    void draw_special_1(int arg) { record("draw_special_1", arg); }
    void draw_special_2(int arg) { record("draw_special_2", arg); }
    void draw_flash() { record("draw_flash"); }
    void draw_info() { record("draw_info"); }
    virtual void on_update() { record("update.base"); }
    void select_armor();
    void select_reactor();
};
struct SpyDesignWin : DesignWin {
    virtual void on_update() { record("update.virtual"); }
};

HELPERS
REDRAW_HELPER
BODY_ARMOR
BODY_REACTOR

static const Event armor_with_suggestion[] = {
    {"exec",2},{"suggest_abil",0x200},{"check_abil",-999},
    {"draw_armor",0},{"draw_vehicle",0},{"draw_special_1",0},
    {"draw_special_2",0},{"draw_flash",-999},{"draw_info",-999},
    {"update.virtual",-999},{"destroy",-999}
};
static const Event armor_without_suggestion[] = {
    {"exec",2},{"check_abil",-999},{"draw_armor",0},
    {"draw_vehicle",0},{"draw_special_1",0},{"draw_special_2",0},
    {"draw_flash",-999},{"draw_info",-999},{"update.virtual",-999},
    {"destroy",-999}
};
static const Event reactor_events[] = {
    {"exec",3},{"check_abil",-999},{"draw_reactor",0},
    {"draw_vehicle",0},{"draw_special_1",0},{"draw_special_2",0},
    {"draw_flash",-999},{"draw_info",-999},{"update.virtual",-999},
    {"destroy",-999}
};

int main() {
    SpyDesignWin armor;
    picker_reply = 2;
    Armor[2].defense_rating = 2;
    Weapon[1].offense_rating = 1;
    armor.select_armor();
    assert(armor.protoArmorType_ == 2);
    expect(armor_with_suggestion,
           sizeof(armor_with_suggestion)/sizeof(armor_with_suggestion[0]));

    clear_events();
    SpyDesignWin armor_no_suggestion;
    Armor[2].defense_rating = 1;
    armor_no_suggestion.select_armor();
    expect(armor_without_suggestion,
           sizeof(armor_without_suggestion)/sizeof(armor_without_suggestion[0]));

    clear_events();
    SpyDesignWin reactor;
    reactor.select_reactor();
    assert(reactor.protoReactorType_ == 2);
    expect(reactor_events,
           sizeof(reactor_events)/sizeof(reactor_events[0]));
}
'''


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
def test_actual_armor_and_reactor_source_order_with_virtual_update(tmp_path,
                                                                   compiler_name):
    armor_helpers, redraw_helper, armor = source_parts('armor')
    reactor_helpers, reactor_redraw_helper, reactor = source_parts('reactor')
    assert armor_helpers == reactor_helpers
    assert redraw_helper == reactor_redraw_helper
    source = (HARNESS.replace('HELPERS', armor_helpers)
              .replace('REDRAW_HELPER', redraw_helper)
              .replace('BODY_ARMOR', armor).replace('BODY_REACTOR', reactor))
    run(tmp_path, compiler_name, source, True)


@pytest.mark.parametrize('compiler_name', ['g++', 'clang++'])
@pytest.mark.parametrize(('kind', 'mutation'), [
    ('armor', 'old_generic_overdraw'),
    ('reactor', 'old_generic_overdraw'),
    ('armor', 'omit_own_draw'),
    ('reactor', 'omit_common_draw'),
    ('armor', 'reorder_own_common'),
    ('reactor', 'nonzero_draw_argument'),
    ('reactor', 'bypass_virtual_update'),
])
def test_redraw_order_controls_reject_compiling_mutants(tmp_path, compiler_name,
                                                        kind, mutation):
    helpers, redraw_helper, body = source_parts(kind, mutation)
    # Compile both bodies in each witness to ensure mutations interact with the
    # real shared ordinal templates and the other unmutated caller.
    _, _, armor = source_parts('armor', mutation if kind == 'armor' else '')
    _, _, reactor = source_parts('reactor', mutation if kind == 'reactor' else '')
    source = (HARNESS.replace('HELPERS', helpers)
              .replace('REDRAW_HELPER', redraw_helper)
              .replace('BODY_ARMOR', armor).replace('BODY_REACTOR', reactor))
    assert body in (armor, reactor)
    run(tmp_path, compiler_name, source, False)


def test_pinned_picker_call_tails_and_conditional_ability_suggestion():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get('OPENSMACX_IMAGE',
                               ROOT/'.opensmacx/game/terranx_original.exe'))
    assert path.is_file(), 'explicit pinned original image is required'
    image = PeImage(path.read_bytes(), PIN)
    disasm = Cs(CS_ARCH_X86, CS_MODE_32)
    expected = {
        0x43A854: ('push', '0x200'),
        0x43A85B: ('call', '0x439d30'),
        0x43A862: ('call', '0x439c00'),
        0x43A86A: ('call', '0x435400'),
        0x43A872: ('call', '0x436090'),
        0x43A87A: ('call', '0x435b30'),
        0x43A882: ('call', '0x435de0'),
        0x43A889: ('call', '0x43dd50'),
        0x43A890: ('call', '0x4346a0'),
        0x43A899: ('call', 'dword ptr [edx + 0xfc]'),
        0x43AADA: ('call', '0x439c00'),
        0x43AAE2: ('call', '0x435800'),
        0x43AAEA: ('call', '0x436090'),
        0x43AAF2: ('call', '0x435b30'),
        0x43AAFA: ('call', '0x435de0'),
        0x43AB01: ('call', '0x43dd50'),
        0x43AB08: ('call', '0x4346a0'),
        0x43AB11: ('call', 'dword ptr [eax + 0xfc]'),
    }
    instructions = {}
    for begin, end in ((0x43A854, 0x43A89F), (0x43AADA, 0x43AB17)):
        instructions.update({item.address: (item.mnemonic, item.op_str)
                             for item in disasm.disasm(image.read(Span(begin, end)), begin)})
    assert {address: instructions[address] for address in expected} == expected

    armor_tail_calls = [(address, op[1]) for address, op in instructions.items()
                        if 0x43A862 <= address <= 0x43A899 and op[0] == 'call']
    reactor_tail_calls = [(address, op[1]) for address, op in instructions.items()
                          if 0x43AADA <= address <= 0x43AB11 and op[0] == 'call']
    assert armor_tail_calls == [
        (0x43A862, '0x439c00'), (0x43A86A, '0x435400'),
        (0x43A872, '0x436090'), (0x43A87A, '0x435b30'),
        (0x43A882, '0x435de0'), (0x43A889, '0x43dd50'),
        (0x43A890, '0x4346a0'), (0x43A899, 'dword ptr [edx + 0xfc]'),
    ]
    assert reactor_tail_calls == [
        (0x43AADA, '0x439c00'), (0x43AAE2, '0x435800'),
        (0x43AAEA, '0x436090'), (0x43AAF2, '0x435b30'),
        (0x43AAFA, '0x435de0'), (0x43AB01, '0x43dd50'),
        (0x43AB08, '0x4346a0'), (0x43AB11, 'dword ptr [eax + 0xfc]'),
    ]

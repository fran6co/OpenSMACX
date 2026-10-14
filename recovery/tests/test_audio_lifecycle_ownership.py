"""Audio lifecycle migration conservation, not complete matching acceptance."""
from pathlib import Path
from dataclasses import replace
import re
import subprocess

from match_recovery.source import read_file, read_text
from match_recovery.source.model import Mode
from match_recovery.source.writer import write_bytes
from test_spritelist_base_lifetime import extract_definition

ROOT = Path(__file__).resolve().parents[2]
BASE = 'df3f94c33d5d8bb167fcfddb7570312398c8ec24'
SPECS = (
    ('sound', 'SoundFadeState::SoundFadeState()', '// ORIGINAL: 0x004C8410'),
    ('sound', 'Sound::Sound()', '// Construct the Sound-owned fields'),
    ('sound', 'Sound::~Sound()', '/*\nPurpose: Destroy the sound.'),
    ('wave', 'Wave::Wave()', '/*\nPurpose: Build the inherited Sound/Fade'),
    ('wave', 'Wave::~Wave()', "// The destructor's dependencies."),
    ('midi', 'Midi::Midi()', '// Superseded seed deleted;'),
    ('midi', 'Midi::~Midi()', '/*\nPurpose: Destroy an ambience.'),
    ('ambience', 'Ambience::Ambience()', '/*\nPurpose: Build the ambience.'),
)


def original(unit):
    return subprocess.check_output(['git', 'show', BASE+':src/'+unit+'.cpp'], cwd=ROOT, text=True)


def historical_block(source, signature, prefix):
    start = source.rfind(prefix, 0, source.index(signature+' {'))
    assert start >= 0
    definition = extract_definition(source, signature+' {')
    end = source.index(signature+' {') + len(definition)
    return source[start:end]


def normalize(source):
    return re.sub(r'\s+', ' ', source).strip()


def test_complete_lifecycle_blocks_preserve_behavior_and_seed_history():
    current = (ROOT/'src/sound.cpp').read_text()
    header = (ROOT/'src/sound.h').read_text()
    for unit, signature, prefix in SPECS:
        old = original(unit)
        # Exact body equality protects initialization, cleanup order, volatile
        # accesses and the original deliberately uninitialized fields.
        owner = header if signature == 'Sound::~Sound()' else current
        assert extract_definition(owner, signature+' {') == extract_definition(old, signature+' {')
        block = historical_block(old, signature, prefix)
        if signature == 'Sound::~Sound()':
            record, = read_text(block, ROOT/'src/sound.cpp')
            block = write_bytes(block.encode(), [replace(record,mode=Mode.FILE)]).decode()
            block = block.replace('// symbol    ??1Sound@@UAE@XZ\n',
                                  '// symbol    ??1Sound@@UAE@XZ\n// body      src/sound.h\n')
            block = block[:block.index(signature+' {')]
            assert 'Sound::~Sound() {' not in current
            assert 'inline Sound::~Sound() {' in header
        block = block.replace('// WALL (unchanged, 2026-09-03):',
            '// Historical WALL (superseded by the co-located real lifetimes below):')
        assert normalize(block) in normalize(current)
        if unit != 'sound':
            assert signature+' {' not in (ROOT/f'src/{unit}.cpp').read_text()
    assert current.count('#pragma function(memset)') == 1
    assert current.count('#pragma intrinsic(memset)') == 1
    assert current.index('#pragma function(memset)') < current.index('SoundFadeState::SoundFadeState()')
    assert current.index('Ambience::Ambience()') < current.index('#pragma intrinsic(memset)')
    assert current.index('#pragma intrinsic(memset)') < current.index('Sound *SoundChainHead;')


def test_all_original_audio_identities_and_claims_survive_once():
    units = ('sound','wave','midi','ambience')
    old = {r.address:r for unit in units for r in read_text(original(unit),ROOT/f'src/{unit}.cpp')}
    rows = [r for unit in units for r in read_file(ROOT/f'src/{unit}.cpp')]
    assert len(rows) == len(old)
    current = {r.address:r for r in rows}
    assert current.keys() == old.keys()
    for address, before in old.items():
        after = current[address]
        for field in ('name','symbol','image_spans','byte_exact','semantic','kind'):
            assert getattr(after,field) == getattr(before,field), (hex(address),field)
        assert after.mode == (Mode.FILE if address == 0x4C6120 else before.mode)
        if address == 0x4C6120:
            assert after.body == 'src/sound.h'
    for address in (0x4C8410,0x4C6080,0x4C6120,0x4C66E0,0x4C67C0,
                    0x4C75B0,0x4C7670,0x4C8460,0x4C9300):
        assert current[address].path == ROOT/'src/sound.cpp'

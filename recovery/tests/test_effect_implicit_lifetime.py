"""Genuine Effect array lifetime and a compiler-backed explicit-dtor damage case."""
import os
from pathlib import Path
import subprocess

import pytest
from match_recovery.compiler import Compiler
from match_recovery.binding_source import parse
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin

ROOT = Path(__file__).resolve().parents[2]
BASE = '8593c5ff0d777b55f72a28196385f1e7f44505a7'
SYMBOL = '??1Effect@@UAE@XZ'


def original(name):
    return subprocess.check_output(['git', 'show', BASE + ':' + name], cwd=ROOT, text=True)


def test_only_effect_lifetime_moves_and_its_identity_survives():
    before = original('src/wave.cpp')
    start = before.index('/*\nPurpose: Tear down an Effect, which is a Wave and nothing more.')
    end = before.index('// ===== MANAGED GLOBALS', start)
    assert (ROOT/'src/wave.cpp').read_text() == before[:start] + before[end:]
    current = (ROOT/'src/fx.cpp').read_text()
    start = current.index('/*\nPurpose: Tear down an Effect, which is a Wave and nothing more.')
    end = current.index('// ORIGINAL: 0x00448350', start)
    assert current[:start] + current[end:] == original('src/fx.cpp')
    assert 'Effect::~Effect()' not in current + (ROOT/'src/wave.cpp').read_text()
    row, = (r for r in read_file(ROOT/'src/fx.cpp') if r.address == 0x4482C0)
    assert row.line in parse(current, read_file(ROOT/'src/fx.cpp'))
    assert row.image_spans == ((0x4482C0, 0x4482C5),)
    assert row.mode.value == 'file' and row.symbol == SYMBOL
    assert row.name == '??1Effect@@QAE@XZ'
    header = (ROOT/'src/wave.h').read_text()
    assert '~Effect(' not in header
    expected = original('src/wave.h').replace('  ~Effect();\n','',1).replace(
        '// at the same 0x6C stride. The definitions and original identities live in\n// wave.cpp, with the bank\'s construction evidence in fx.cpp.',
        '// at the same 0x6C stride. FX owns the initialization policy and the implicit\n// lifetime mappings in fx.cpp; no user-defined destructor stage is needed.')
    assert header == expected


def base_tail(obj):
    code, relocs, _ = obj.subject(obj.function(SYMBOL))
    return (len(relocs) == 1 and relocs[0].offset == 1
            and obj.symbols[int(relocs[0].target)].name == '??1Wave@@UAE@XZ'
            and code[0] == 0xE9 and all(b == 0x90 for b in code[5:]))


@pytest.mark.skipif(os.environ.get('OPENSMACX_LIVE_EFFECT_LIFETIME') != '1',
                    reason='requires coordinator-owned isolated compiler lease')
def test_compiler_owned_tail_and_explicit_destructor_damage():
    image_path = Path(os.environ['OPENSMACX_IMAGE'])
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    code = image.read(Span(0x4482C0,0x4482C5))
    assert code[0] == 0xE9
    assert 0x4482C5 + int.from_bytes(code[1:], 'little', signed=True) == 0x4C67C0
    for start,end in ((0x448290,0x44829F),(0x4482A0,0x4482BB)):
        assert b'\x68\xc0\x82\x44\x00' in image.read(Span(start,end))
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT,profile=settings.compiler_profile)
    path = ROOT/'src/fx.cpp'
    header = (ROOT/'src/wave.h').read_bytes()
    damaged = header.replace(b'class Effect : public Wave {\n public:\n',
                             b'class Effect : public Wave {\n public:\n  ~Effect() {}\n',1)
    assert damaged != header
    for flags in settings.flags_for('src/fx.cpp'):
        obj = Coff(compiler(path,(),flags))
        assert base_tail(obj)
        # The deleting entry must still call the derived complete destructor;
        # masked byte equality alone cannot protect this symbolic call edge.
        _, deleting_refs, _ = obj.subject(obj.function('??_GEffect@@UAEPAXI@Z'))
        assert [obj.symbols[int(v.target)].name for v in deleting_refs] == [
            SYMBOL, '??3@YAXPAX@Z']
        for symbol in ('??0FX@@QAE@XZ','??1FX@@QAE@XZ'):
            _, relocs, _ = obj.subject(obj.function(symbol))
            assert SYMBOL in [obj.symbols[int(v.target)].name for v in relocs]
        mutant = Coff(compiler.overlay(path,{'src/wave.h':damaged},(),flags))
        assert not base_tail(mutant), 'explicit derived destructor stage escaped control'

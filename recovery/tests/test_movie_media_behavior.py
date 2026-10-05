"""Actual-source media storage controls, not complete codec/game acceptance."""
from pathlib import Path
import json
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def original_adpcm_bytes():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    image = PeImage((ROOT / '.opensmacx/game/terranx_original.exe').read_bytes(), image_pin(ROOT))
    return image.read_initial(Span(0x693794, 0x694E24))


def adpcm_source_projection():
    header = (ROOT / 'src/general.h').read_text()
    start = header.index('struct MediaAdpcmStorage {')
    declaration = header[start:header.index('};', start) + 2]
    source = (ROOT / 'src/general.cpp').read_text()
    start = source.index('#define MEDIA_ADPCM_MAG')
    end = source.index('#undef MEDIA_ADPCM_MAG', start) + len('#undef MEDIA_ADPCM_MAG')
    return declaration + '\n' + source[start:end]


def test_actual_source_adpcm_initial_bytes_and_mutation(tmp_path):
    compiler = shutil.which('g++') or shutil.which('clang++')
    if not compiler:
        pytest.skip('host C++ compiler required')
    projected = adpcm_source_projection()
    expected = original_adpcm_bytes()
    assert len(expected) == 0x1690
    for label, source in (
        ('actual', projected),
        ('wrong_final_step', projected.replace('MEDIA_ADPCM_ROW(32767)', 'MEDIA_ADPCM_ROW(32766)')),
        ('wrong_overlap_prefix', projected.replace('0, 240, 460, 392', '0, 241, 460, 392')),
    ):
        unit = tmp_path / (label + '.cpp')
        unit.write_text('#include <cstdio>\n' + source + '\nint main() {\n'
                        'return fwrite(&MediaAdpcmTables, 1, sizeof(MediaAdpcmTables), stdout) '
                        '== sizeof(MediaAdpcmTables) ? 0 : 1; }\n')
        binary = tmp_path / label
        subprocess.run([compiler, '-std=c++98', str(unit), '-o', str(binary)], check=True)
        observed = subprocess.check_output([str(binary)])
        if label == 'actual':
            assert observed == expected
        else:
            assert observed != expected, 'control must detect damaged initializer'


def test_native_adpcm_storage_layout_and_all_initial_bytes():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    from match_recovery.codeview import class_layout
    from match_recovery.codeview_data import data_layout
    from match_recovery.object_source import read_objects, object_targets
    from match_recovery.object_data import check_initial_data
    from match_recovery.coff_replica import function_signature
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('VC6 and Wine32 required')
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    expected = original_adpcm_bytes()
    image = PeImage((ROOT / '.opensmacx/game/terranx_original.exe').read_bytes(), image_pin(ROOT))
    base = json.loads((ROOT / 'recovery/tasks/movie-media-owner-wave.json').read_text())['base_revision']
    baseline_sources = {
        path: subprocess.check_output(['git', 'show', base + ':' + path], cwd=ROOT)
        for path in ('src/general.cpp', 'src/general.h')
    }
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / 'src/general.cpp', (), flags + settings.layout_flags))
        baseline = Coff(compiler.overlay(ROOT / 'src/general.cpp', baseline_sources, (),
                                         flags + settings.layout_flags))
        changed = {'?sub_5ce640@@YAXHHHHPAH0@Z', '?sub_5ce6e3@@YAXPAEPAFHPAH2@Z'}
        compared = 0
        for old_symbol in baseline.symbols.values():
            if old_symbol.section <= 0 or not old_symbol.type & 0x20 or old_symbol.storage not in (2, 3):
                continue
            if old_symbol.name in changed:
                continue
            if not baseline.sections[old_symbol.section - 1].flags & 0x1000:
                continue
            old_function = baseline.function(old_symbol.name)
            current_function = obj.function(old_symbol.name)
            assert function_signature(baseline, old_function.name) == function_signature(obj, current_function.name), old_symbol.name
            compared += 1
        assert compared > 100, 'bounded TU preservation must cover the actual compiled functions'
        layout = class_layout(obj, 'MediaAdpcmStorage')
        assert layout['size'] == 0x1690
        assert layout['members']['lookup_and_delta_words'] == 0
        assert layout['members']['index_adjust'] == 0x1660
        assert layout['members']['predictor_left'] == 0x1680
        assert layout['members']['predictor_right'] == 0x1684
        assert layout['members']['step_index_left'] == 0x1688
        assert layout['members']['step_index_right'] == 0x168C
        symbol, = (s.name for s in obj.symbols.values()
                   if s.name.startswith('?MediaAdpcmTables@@') and s.section > 0)
        data = data_layout(obj, symbol)
        assert data['size'] == len(expected)
        section = obj.sections[data['section'] - 1]
        start, end = data['offset'], data['offset'] + data['size']
        assert not any(start <= offset < end for offset, _, _ in section.relocations)
        assert section.data[start:end] == expected
        bindings = tuple(b for b in read_objects(ROOT, image_pin(ROOT)) if b.symbol == symbol)
        assert len(bindings) == 1
        targets, _ = object_targets(bindings, {'src/general.cpp': obj})
        row, = check_initial_data(bindings, {'src/general.cpp': obj}, image, targets)
        assert row['definition'] == row['content'] == 'pass'
        assert row['lifetime'] == 'unknown'
        # VC6 may retain the small delta reader rather than inline it.
        helper = '?media_adpcm_delta@@YAHHI@Z'
        _, helper_relocs, _ = obj.subject(obj.function(helper))
        assert symbol in {obj.symbols[int(r.target)].name for r in helper_relocs}
        for function in ('?sub_5ce640@@YAXHHHHPAH0@Z', '?sub_5ce6e3@@YAXPAEPAFHPAH2@Z'):
            body, relocs, _ = obj.subject(obj.function(function))
            assert body
            targets = {obj.symbols[int(r.target)].name for r in relocs}
            assert symbol in targets or helper in targets

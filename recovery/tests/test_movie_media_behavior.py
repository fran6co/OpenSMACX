"""Actual-source media storage controls, not complete codec/game acceptance."""
from pathlib import Path
import json
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get('OPENSMACX_IMAGE', str(ROOT / '.opensmacx/game/terranx_original.exe')))


def original_adpcm_bytes():
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    image = PeImage(IMAGE.read_bytes(), image_pin(ROOT))
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
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('VC6 and Wine32 required')
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    expected = original_adpcm_bytes()
    image = PeImage(IMAGE.read_bytes(), image_pin(ROOT))
    # This recurring control verifies current ADPCM storage and references.
    # The historical media-wave General comparison remains checkpoint evidence;
    # its two-file overlay no longer supplies the retired header closure. This
    # test does not re-certify that comparison or replace preservation checks.
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / 'src/general.cpp', (), flags + settings.layout_flags))
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


"""Host controls execute definitions extracted verbatim from movie_media.cpp."""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


def root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src" / "IMAGE_PIN").is_file():
            return parent
    raise AssertionError("cannot locate OpenSMACX root")


ROOT = root()
SOURCE = ROOT / "src" / "movie_media.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/movie_media_host.cpp"


def definition(source: str, signature: str) -> str:
    # Some source functions are forward-declared just above their definition.
    start = source.rindex(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError("unterminated source definition: " + signature)


def initializer(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    closing = None
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                closing = position
                break
    assert closing is not None
    end = source.index(";", closing) + 1
    return source[start:end]


def declaration(source: str, signature: str) -> str:
    start = source.index(signature)
    return source[start : source.index(";", start) + 1]


def generated_source(source: str, edits=()) -> str:
    tables = "\n".join(
        initializer(source, name)
        for name in (
            "static const DWORD media_block_transform_weights[64]",
            "static const DWORD media_block_quant_order[64]",
        )
    )
    bodies = {
        "// ACTUAL_HELPERS": "\n".join(
            definition(source, signature)
            for signature in (
                "static int media_scale_signed_component(",
                "static void media_write_16bit_block(",
            )
        ),
        "// ACTUAL_SUB_5CD2D0": definition(
            source, 'extern "C" int __cdecl sub_5cd2d0('
        ),
        "// ACTUAL_SUB_5CC860": definition(
            source, 'extern "C" int __fastcall sub_5cc860('
        ),
        "// ACTUAL_SUB_5CD560": definition(
            source, 'extern "C" void __cdecl sub_5cd560('
        ),
        "// ACTUAL_SUB_5CF120": definition(
            source, 'extern "C" char *__fastcall sub_5cf120('
        ),
        "// ACTUAL_PARSE_FORMAT": definition(
            source, "int __fastcall MediaRuntime::parse_initial_format("
        ),
        "// ACTUAL_CLEANUP": definition(
            source, "int MediaRuntime::sub_5cae00()"
        ),
        "// ACTUAL_SOUND_SETUP": definition(
            source, "int MediaRuntime::setup_sound_device()"
        ),
        "// ACTUAL_SOUND_CREATE": definition(
            source, "int MediaRuntime::create_playback_buffer()"
        ),
        "// ACTUAL_SOUND_RELEASE": "\n".join(
            (
                definition(source, "int MediaRuntime::release_session_interfaces()"),
                definition(source, "int MediaRuntime::release_playback_buffer()"),
            )
        ),
    }
    template = FIXTURE.read_text()
    bodies["// ACTUAL_TABLES"] = tables
    bodies["// ACTUAL_HRESULT_TABLE"] = initializer(
        source, "static const MediaHResultText media_hresult_texts[]"
    )
    bodies["// ACTUAL_HRESULT_FALLBACK"] = declaration(
        source, "static const char media_hresult_unknown[]"
    )
    for old, new in edits:
        changed = False
        for key, body in bodies.items():
            if old in body:
                bodies[key] = body.replace(old, new, 1)
                changed = True
                break
        assert changed, "mutation anchor absent: " + old
    for marker, body in bodies.items():
        assert template.count(marker) == 1
        template = template.replace(marker, body)
    return template


def build_and_run(edits=()) -> subprocess.CompletedProcess:
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    current = generated_source(SOURCE.read_text(), edits)
    with tempfile.TemporaryDirectory() as temp:
        cpp = Path(temp) / "media_host_fixture.cpp"
        executable = Path(temp) / "media_host_fixture"
        cpp.write_text(current)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(executable)],
            capture_output=True,
            text=True,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(executable)], capture_output=True, text=True)


def test_current_actual_media_bodies() -> None:
    result = build_and_run()
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    "old,new",
    [
        ("created->Release();", "queried->Release();"),
        ("base + pitch * 8", "base + pitch * 7"),
        (
            "destination[0] = media_block_transform_weights[0] * 8;",
            "destination[0] = media_block_transform_weights[0];",
        ),
        ("if (read == 0) return -1;", "if (read == -1) return -1;"),
        (
            "if (d.stream_gate_7c != 0)",
            "if (d.stream_gate_7c == 0)",
        ),
        ("\"DDERR_ALREADYINITIALIZED\"", "\"DDERR_CHANGED\""),
        (
            "format.nSamplesPerSec = playback_frequency;",
            "format.nSamplesPerSec = 44100;",
        ),
        (
            "d.play_buffer->SetFrequency(playback_frequency);",
            "d.play_buffer->SetVolume(d.volume_argument);",
        ),
    ],
)
def test_controls_reject_actual_body_mutants(old: str, new: str) -> None:
    result = build_and_run(((old, new),))
    assert result.returncode != 0, result.stdout + result.stderr


def test_pinned_sound_import_contradicts_retained_directdraw_note():
    import struct
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.coff_archive import CoffArchive
    from match_recovery.source_inventory import image_pin
    from recovery.environment.wine import VC6_ROOT
    image = PeImage(IMAGE.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    call, = decoder.disasm(image.read(Span(0x5CE376, 0x5CE37B)), 0x5CE376)
    assert call.mnemonic == 'call' and call.op_str == '0x6501f0'
    thunk, = decoder.disasm(image.read(Span(0x6501F0, 0x6501F6)), 0x6501F0)
    assert thunk.mnemonic == 'jmp' and thunk.op_str == 'dword ptr [0x669034]'
    slot, = (entry for entry in image.imports() if entry.address == 0x669034)
    assert slot.module.lower() == 'dsound.dll' and slot.ordinal == 1
    library = VC6_ROOT / 'LIB/DSOUND.LIB'
    if not library.is_file():
        pytest.skip('SDK import library required to correlate the original ordinal')
    archive = CoffArchive(library.read_bytes())
    member, = (body for bodies in archive.members.values() for body in bodies
               if body[20:] == b'_DirectSoundCreate@12\0DSOUND.dll\0')
    sig1, sig2, version, machine, _, size, ordinal, type_info = struct.unpack('<HHHHIIHH', member[:20])
    assert (sig1, sig2, version, machine, size, ordinal, type_info) == (0, 0xFFFF, 0, 0x14C, 33, 1, 0)


def test_native_media_layout_storage_and_uv2_preservation():
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff, PeImage
    from match_recovery.codeview import class_layout
    from match_recovery.object_source import read_objects, object_targets
    from match_recovery.object_data import check_initial_data
    from match_recovery.source_inventory import image_pin
    from match_recovery.coff_replica import function_signature
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / 'BIN/CL.EXE').is_file() or not shutil.which('wine32'):
        pytest.skip('VC6 and Wine32 required')
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    image = PeImage(IMAGE.read_bytes(), image_pin(ROOT))
    # Check this owner correction against its immutable, compiling predecessor;
    # the earlier seed-era tree requires headers retired by later migrations.
    base = json.loads((ROOT / 'recovery/decisions/media-audio-owner-oct11.json').read_text())['base_revision']
    baseline_sources = {path: subprocess.check_output(['git', 'show', base + ':' + path], cwd=ROOT)
                        for path in ('src/uv2player.cpp', 'src/uv2player.h', 'src/movie_media.h')}
    for flags in settings.flag_sets:
        media = Coff(compiler(ROOT / 'src/movie_media.cpp', (), flags + settings.layout_flags))
        for name, size in (('MediaRuntime', 0x660), ('MediaDescriptor', 0x7B4),
                           ('MediaQueueRecord', 0x10), ('MediaPixelFormat', 0x20),
                           ('MediaConversionTableStorage', 0x3D74)):
            assert class_layout(media, name)['size'] == size
        descriptor = class_layout(media, 'MediaDescriptor')['members']
        assert descriptor['runtime'] == 0 and descriptor['pixel_format'] == 0x2C
        assert descriptor['source_buffer_58'] == 0x58
        bindings = tuple(b for b in read_objects(ROOT, image_pin(ROOT))
                         if b.symbol.startswith('?movie_media_conversion_tables@@'))
        assert len(bindings) == 1
        objects = {'src/movie_media.cpp': media}
        targets, _ = object_targets(bindings, objects)
        row, = check_initial_data(bindings, objects, image, targets)
        assert row['definition'] == row['content'] == 'pass'
        assert row['lifetime'] == 'unknown'
        uv2 = Coff(compiler(ROOT / 'src/uv2player.cpp', (), flags + settings.layout_flags))
        layout = class_layout(uv2, 'UV2Player')
        assert layout['size'] == 0xE68
        for member, offset in (('media_session_', 0x124), ('media_', 0x128),
                               ('buffer_', 0x8DC), ('write_delay_', 0xE64)):
            assert layout['members'][member] == offset
        baseline = Coff(compiler.overlay(ROOT / 'src/uv2player.cpp', baseline_sources, (),
                                         flags + settings.layout_flags))
        compared = 0
        for old in baseline.symbols.values():
            if old.section <= 0 or not old.type & 0x20 or old.storage not in (2, 3):
                continue
            if 'UV2Player@@' not in old.name and old.name != '_uv2_player_writer_redirect':
                continue
            assert function_signature(baseline, old.name) == function_signature(uv2, old.name), old.name
            compared += 1
        assert compared >= 9


@pytest.fixture(scope='module')
def historical_media_records():
    from match_recovery.source import read_text
    task = json.loads((ROOT / 'recovery/tasks/movie-media-owner-wave.json').read_text())
    records = []
    paths = [path for path in task['editable_files']
             if path.startswith(('src/recovered/', 'src/unrecovered/'))]
    paths.append('src/leaf_recoveries.cpp')
    for path in paths:
        old = subprocess.check_output(['git', 'show', task['base_revision'] + ':' + path], cwd=ROOT)
        records.extend(read_text(old.decode(), ROOT / path))
    return records


@pytest.mark.parametrize('address', (
    0x5CF120,0x5CBC00,0x5CC150,0x5CC2B0,0x5CC450,0x5CD2D0,0x5CC860,
    0x5CD560,0x5CC940,0x5CBE30,0x5CE2B0,0x5CE2F0,0x5CDFA0,0x5CB050,
    0x5CA6C0,0x5C9EC0,0x5CDB90,0x5CDD10,0x5CA310,0x5CA970,0x5CE340,
    0x5CE4B0,0x5CE450,0x5CE620,0x5CAE00,0x5CC710,0x5CB6C0,0x5CB220,
    0x5CAE80,0x5CB610,0x5CB660,0x5CAAB0,
))
def test_media_owner_retains_original_identity_and_findings(address, historical_media_records):
    from match_recovery.source import read_file, Mode, State
    current, = (record for record in read_file(ROOT / 'src/movie_media.cpp') if record.address == address)
    old = [record for record in historical_media_records if record.address == address]
    assert old
    assert current.mode == Mode.BODY and current.state == State.IMPLEMENTED
    assert current.symbol and not current.extract_error
    for record in old:
        assert record.name == current.name
        if record.image_spans != current.image_spans:
            assert address == 0x5C9EC0
            assert len(record.image_spans) == len(current.image_spans) == 1
            (start, end), = record.image_spans
            (owner_start, owner_end), = current.image_spans
            assert owner_start == start and owner_end >= end
            assert 'historical extent discrepancy' in current.region
        assert not record.byte_exact or current.byte_exact
        assert not record.semantic or current.semantic
        for field in ('levers', 'ruled_out', 'unrecoverable', 'deferred'):
            assert set(getattr(record, field)) <= set(getattr(current, field)), field


@pytest.mark.parametrize('address', (0x5CD750,0x5CD8D0,0x5CD980,0x5CDA30,0x5CFAED,0x5D02CC,0x5D35B0))
def test_completed_media_converters_retain_seed_identity_and_findings(address):
    from match_recovery.source import read_file, State
    from match_recovery.source import read_text
    task = json.loads((ROOT / 'recovery/tasks/movie-media-owner-wave.json').read_text())
    seed_path = 'src/unrecovered/%08x.cpp' % address
    historical = subprocess.check_output(
        ['git', 'show', task['base_revision'] + ':' + seed_path], cwd=ROOT)
    old, = read_text(historical.decode(), ROOT / seed_path)
    current, = (record for path in (ROOT / 'src/movie_media.cpp',
                                   ROOT / 'src/movie_media_decoder.cpp')
                for record in read_file(path) if record.address == address)
    assert current.state == State.IMPLEMENTED and '// BODY GOES HERE.' not in current.region
    assert current.symbol and not current.extract_error
    assert old.image_spans == current.image_spans and old.name == current.name
    assert not old.byte_exact or current.byte_exact
    assert not old.semantic or current.semantic
    for field in ('levers', 'ruled_out', 'unrecoverable', 'deferred'):
        assert set(getattr(old, field)) <= set(getattr(current, field)), field
    assert not (ROOT / seed_path).exists(), 'superseded seed must stay retired'

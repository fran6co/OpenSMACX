"""Configure-time terrain data; not a recovery CLI or runtime loader.

The source owns the original binding and C++ type. The installed package owns
source and PE parsing. CMake supplies an ignored build output; never emit code,
pointer values or an unpinned fallback, and never retain extracted data here.
"""
from pathlib import Path

from match_recovery.object_source import read_objects
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from match_recovery.source_replica import artifact_parent


_MOVIE_DECODER_DATA = (
    ('movie_decoder_static.inc', 0x00695B58, 0x00696A3C, 'words',
     'accessed_window'),
    ('movie_decoder_workspace.inc', 0x009C5000, 0x009C581C, 'words',
     'accessed_window'),
    # This selected initializer/access window begins at a source-owned v1
    # start-only binding. 325 DWORDs reach the highest proven access; that
    # does not establish the original object's allocation upper bound.
    ('movie_decoder_constants.inc', 0x009C9594, 0x009C9AA8, 'words',
     'accessed_window'),
    ('movie_decoder_tables.inc', 0x009C314C, 0x009C4658, 'words',
     'accessed_window'),
    ('movie_decoder_process.inc', 0x009B2D64, 0x009B3370, 'zero',
     'accessed_window'),
    ('movie_decoder_idct_storage.inc', 0x009C3000, 0x009C314C, 'words',
     'accessed_window'),
)


_LANDMARK_TABLES = {
    'terrain_mount_planet.inc': '_MountPlanetTable',
    'terrain_sunny_mesa.inc': '_SunnyMesaTable',
    'terrain_garland_crater.inc': '_GarlandCraterTable',
}

_ALTITUDE_TABLES = {
    'terrain_altitude_default.inc': '?AltNaturalDefault@@3PAIA',
    'terrain_altitude_contour.inc': '?AltContour@@3PAIA',
    'terrain_altitude_natural.inc': '?AltNatural@@3PAIA',
}

_LANDMARK_BONUS_TABLES = {
    'terrain_landmark_arid.inc': '?AltTemplateArid@@3QBEB',
    'terrain_landmark_moist.inc': '?AltTemplateMoist@@3QBEB',
    'terrain_landmark_rainy.inc': '?AltTemplateRainy@@3QBEB',
    'terrain_landmark_flat.inc': '?AltTemplateFlat@@3QBEB',
    'terrain_landmark_ocean.inc': '?AltTemplateOcean@@3QBEB',
}


def _terrain_initializer(root: Path, original: Path, output: Path, *,
                         output_name: str, unit: str, symbol: str,
                         size: int, signed_pairs: bool = False,
                         unsigned_words: bool = False,
                         accessed_window: bool = False,
                         require_start_only: bool = False) -> None:
    if unsigned_words and (signed_pairs or size % 4):
        raise ValueError('altitude initializer requires complete unsigned words')
    root = root.resolve()
    original = original.resolve(strict=True)
    output = output.absolute()
    # This hook is deliberately project-specific, not an arbitrary exporter.
    # Only reviewed terrain tables are selected by the callers below.
    if (output.resolve() != output or output.name != output_name
            or not output.is_relative_to(root / 'build')):
        raise ValueError('terrain initializer must remain in ignored project build context')
    if any(p.is_symlink() for p in (output, *output.parents) if p.is_relative_to(root)):
        raise ValueError('terrain initializer output cannot use aliases')
    pin = image_pin(root)
    binding, = [b for b in read_objects(root, pin)
                if b.unit == unit and b.symbol == symbol]
    if require_start_only and binding.span is not None:
        raise ValueError('accessed-window initializer requires a start-only source binding')
    if binding.span is None and accessed_window:
        # A reviewed memcpy window is not an original allocation upper bound.
        # Only the named landmark template hook opts into this start-only view.
        span = Span(binding.origin.address, binding.origin.address + size)
    elif binding.span is not None and binding.span.end - binding.span.start == size:
        span = binding.span
    else:
        raise ValueError(f'terrain table requires its bounded {size}-byte source interpretation')
    data = original.read_bytes()
    image = PeImage(data, pin)
    # read_initial validates data-section ownership; read independently refuses
    # virtual zero fill. Both must describe the same fully file-backed bytes.
    values = image.read(span)
    if any(flags & 0x20000000 for (region, _, _), flags in
           zip(image.sections, image.section_flags)
           if region.start <= span.start and span.end <= region.end):
        raise ValueError('terrain remap cannot use executable storage')
    if image.read_initial(span) != values or image.absolute_sites(span):
        raise ValueError('terrain remap requires file-backed non-relocated byte data')
    if unsigned_words:
        words = [int.from_bytes(values[i:i + 4], 'little')
                 for i in range(0, len(values), 4)]
        rows = [', '.join(f'{word}u' for word in words) + ',']
    elif signed_pairs:
        signed = [value if value < 128 else value - 256 for value in values]
        rows = ['{%d, %d},' % (signed[i], signed[i + 1])
                for i in range(0, len(signed), 2)]
    else:
        rows = [', '.join(str(value) for value in values[i:i + 16]) + ','
                for i in range(0, len(values), 16)]
    text = '// Generated from pinned source-owned terrain data; do not edit.\n' + '\n'.join(rows) + '\n'
    # Recheck authoritative inputs before publication. This is a cache, not an
    # acceptance record; native data verification separately checks the object.
    if original.read_bytes() != data or image_pin(root) != pin or binding not in read_objects(root, pin):
        raise ValueError('terrain initializer inputs changed during generation')
    artifact_parent(root, output.parent.relative_to(root).as_posix(),
                    excluded_roots=('src', 'recovery', 'cmake'))
    if output.exists() and (not output.is_file() or output.stat().st_nlink != 1):
        raise ValueError('terrain initializer output must be an unlinked regular file')
    if not output.exists() or output.read_text() != text:
        output.write_text(text)


def terrain_coast_initializer(root: Path, original: Path, output: Path) -> None:
    _terrain_initializer(root, original, output,
                         output_name='terrain_coast_remap.inc', unit='src/game.cpp',
                         symbol='_TerrainCoastRemapSource', size=53)


def terrain_landmark_initializer(root: Path, original: Path, output: Path) -> None:
    symbol = _LANDMARK_TABLES.get(output.name)
    if symbol is None:
        raise ValueError('unknown terrain landmark initializer')
    _terrain_initializer(root, original, output, output_name=output.name,
                         unit='src/mapwin.cpp', symbol=symbol, size=98,
                         signed_pairs=True)


def terrain_initializers(root: Path, original: Path, directory: Path) -> None:
    terrain_coast_initializer(root, original, directory / 'terrain_coast_remap.inc')
    for filename in _LANDMARK_TABLES:
        terrain_landmark_initializer(root, original, directory / filename)
    for filename in _ALTITUDE_TABLES:
        terrain_altitude_initializer(root, original, directory / filename)
    terrain_direction_initializer(root, original, directory / 'terrain_alt_direction.inc')
    for filename in _LANDMARK_BONUS_TABLES:
        terrain_landmark_bonus_initializer(root, original, directory / filename)


def movie_decoder_initializers(root: Path, original: Path,
                               directory: Path) -> None:
    """Project selected decoder access windows into the ignored cache.

    Each new decoder storage owner is anchored by a v1 start-only mapping. Its
    reviewed initializer window is selected separately and is not an
    original-object layout or native data-acceptance result.
    """
    root = root.resolve()
    original = original.resolve(strict=True)
    directory = directory.absolute()
    if (directory.resolve() != directory or directory.name != 'terrain-data'
            or not directory.is_relative_to(root / 'build')):
        raise ValueError('decoder derivatives must remain in ignored build context')
    if any(path.is_symlink() for path in (directory, *directory.parents)
           if path.is_relative_to(root)):
        raise ValueError('decoder derivative directory cannot use aliases')

    pin = image_pin(root)
    original_bytes = original.read_bytes()
    image = PeImage(original_bytes, pin)
    objects = read_objects(root, pin)
    for entry in _MOVIE_DECODER_DATA:
        if len(entry) == 5 and entry[4] == 'accessed_window':
            filename, start, end, representation, _ = entry
            accessed_window = True
        else:
            raise ValueError('invalid decoder initializer selection policy')
        span = Span(start, end)
        if accessed_window:
            # The native mapping anchors the source-owned storage start only.
            # Select this reviewed window from accesses, never from a claimed
            # original object extent.
            bindings = [obj for obj in objects
                        if obj.unit == 'src/movie_media_decoder.cpp'
                        and obj.origin.address == start and obj.span is None]
        else:
            bindings = [obj for obj in objects
                        if obj.unit == 'src/movie_media_decoder.cpp'
                        and obj.span == span]
        if len(bindings) != 1:
            expectation = 'one v1 start-only source binding' if accessed_window \
                else 'one exact source-owned object'
            raise ValueError(f'{filename} requires {expectation}')
        initial = image.read_initial(span)
        if image.absolute_sites(span):
            raise ValueError(f'{filename} must be initialized reloc-free image data')
        if representation == 'zero':
            # Zero-initialized storage can live in a PE BSS section and thus
            # have no unique file-backed representation. Its pinned initial
            # bytes are still reviewable through read_initial; do not demand
            # PeImage.read() for this representation.
            if any(initial):
                raise ValueError(f'{filename} must be zero-initialized source storage')
            continue
        raw = image.read(span)
        if initial != raw:
            raise ValueError(f'{filename} must be initialized reloc-free image data')
        if len(raw) % 4:
            raise ValueError(f'{filename} must contain whole DWORDs')
        words = [int.from_bytes(raw[i:i + 4], 'little')
                 for i in range(0, len(raw), 4)]
        text = ('// Generated from pinned, source-owned data; do not edit.\n' +
                '\n'.join(', '.join(f'0x{word:08x}u'
                                      for word in words[i:i + 8]) + ','
                          for i in range(0, len(words), 8)) + '\n')
        artifact_parent(root, directory.relative_to(root).as_posix(),
                        excluded_roots=('src', 'recovery', 'cmake'))
        output = directory / filename
        if output.exists() and (not output.is_file() or output.stat().st_nlink != 1):
            raise ValueError('decoder generated include must be an unlinked regular file')
        if not output.exists() or output.read_text() != text:
            output.write_text(text)
    if original.read_bytes() != original_bytes or image_pin(root) != pin:
        raise ValueError('decoder initializer inputs changed during generation')


def graphicwin_palette_initializer(root: Path, original: Path, output: Path) -> None:
    _terrain_initializer(root, original, output,
                         output_name='graphicwin_palette_polynomials.inc',
                         unit='src/graphicwin.cpp',
                         symbol='?GraphicWinBlitPolynomialWords@@3PAIA', size=132,
                         unsigned_words=True, accessed_window=True,
                         require_start_only=True)

def project_initializers(root: Path, original: Path, directory: Path) -> None:
    terrain_initializers(root, original, directory)
    movie_decoder_initializers(root, original, directory)
    graphicwin_palette_initializer(
        root, original, directory / 'graphicwin_palette_polynomials.inc')


def terrain_direction_initializer(root: Path, original: Path, output: Path) -> None:
    _terrain_initializer(root, original, output,
                         output_name='terrain_alt_direction.inc', unit='src/map.cpp',
                         symbol='?AltDirectionTable@@3QBEB', size=245)


def terrain_landmark_bonus_initializer(root: Path, original: Path, output: Path) -> None:
    symbol = _LANDMARK_BONUS_TABLES.get(output.name)
    if symbol is None:
        raise ValueError('unknown terrain landmark bonus initializer')
    _terrain_initializer(root, original, output, output_name=output.name,
                         unit='src/map.cpp', symbol=symbol, size=113,
                         accessed_window=True)


def terrain_altitude_initializer(root: Path, original: Path, output: Path) -> None:
    symbol = _ALTITUDE_TABLES.get(output.name)
    if symbol is None:
        raise ValueError('unknown terrain altitude initializer')
    _terrain_initializer(root, original, output, output_name=output.name,
                         unit='src/map.cpp', symbol=symbol, size=44,
                         unsigned_words=True)

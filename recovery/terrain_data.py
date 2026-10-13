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

_LANDMARK_GAPS = {
    'terrain_landmark_gap_mount_mesa.inc': ('_MountPlanetTable', '_SunnyMesaTable', 2),
    'terrain_landmark_gap_mesa_point_light.inc': ('_SunnyMesaTable', '_AltLightBit2', 2),
    'terrain_landmark_gap_point_light_crater.inc': ('_AltLightBit2', '_GarlandCraterTable', 3),
    'terrain_landmark_gap_crater_ocean.inc': ('_GarlandCraterTable', '_OceanCornerUVs', 0x72),
}

_LANDMARK_WINDOW_PARENT = (
    '?TerrainLandmarkStorageWindowData@@3UTerrainLandmarkStorageWindow@@A')
_POINT_LIGHT_ACCESS_WINDOW_SIZE = 113
_LANDMARK_WINDOW_MEMBERS = (
    ('_MountPlanetTable', 'mount_planet', 98, 0x000),
    ('_SunnyMesaTable', 'sunny_mesa', 98, 0x064),
    ('_AltLightBit2', 'alt_light_bit2', None, 0x0C8),
    ('_GarlandCraterTable', 'garland_crater', 98, 0x13C),
    ('_OceanCornerUVs', 'ocean_corner_uvs', 128, 0x210),
)

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
                         require_start_only: bool = False,
                         signed_bytes: bool = False) -> None:
    if signed_bytes and (signed_pairs or unsigned_words):
        raise ValueError('signed byte initializer cannot use word or pair representation')
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
    elif signed_bytes:
        signed = [value if value < 128 else value - 256 for value in values]
        rows = [', '.join(str(value) for value in signed[i:i + 16]) + ','
                for i in range(0, len(signed), 16)]
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


def terrain_landmark_gap_initializer(root: Path, original: Path,
                                      output: Path) -> None:
    """Materialize only the four unnamed byte gaps in the chosen window.

    These bytes preserve the observed pinned representation between existing
    independently mapped objects. They do not create original object bounds or
    ownership claims for the gaps or for the current aggregate.
    """
    entry = _LANDMARK_GAPS.get(output.name)
    if entry is None:
        raise ValueError('unknown terrain landmark gap initializer')
    left_symbol, right_symbol, expected_size = entry
    root = root.resolve()
    original = original.resolve(strict=True)
    output = output.absolute()
    if (output.resolve() != output or output.name not in _LANDMARK_GAPS
            or not output.is_relative_to(root / 'build')):
        raise ValueError('terrain gap initializer must remain in ignored project build context')
    if any(path.is_symlink() for path in (output, *output.parents)
           if path.is_relative_to(root)):
        raise ValueError('terrain gap initializer cannot use aliases')

    pin = image_pin(root)
    bindings = _landmark_window_bindings(root, pin)
    by_symbol = {binding.symbol: binding for binding in bindings}
    left = by_symbol[left_symbol]
    right = by_symbol[right_symbol]
    left_size = (left.span.end - left.span.start if left.span is not None
                 else _POINT_LIGHT_ACCESS_WINDOW_SIZE)
    start = left.origin.address + left_size
    end = right.origin.address
    size = end - start
    if size != expected_size:
        raise ValueError('terrain gap differs from the reviewed member layout')
    original_bytes = original.read_bytes()
    image = PeImage(original_bytes, pin)
    span = Span(start, start + size)
    values = image.read(span)
    if image.read_initial(span) != values or image.absolute_sites(span):
        raise ValueError('terrain gap bytes must be file-backed and relocation-free')
    if any(flags & 0x20000000 for (region, _, _), flags in
           zip(image.sections, image.section_flags)
           if region.start <= span.start and span.end <= region.end):
        raise ValueError('terrain gap bytes cannot come from executable storage')
    text = ('// Generated from pinned bytes between mapped terrain objects; do not edit.\n'
            + ', '.join(f'0x{value:02x}' for value in values) + ',\n')

    if (original.read_bytes() != original_bytes or image_pin(root) != pin
            or _landmark_window_bindings(root, pin) != bindings):
        raise ValueError('terrain gap initializer inputs changed during generation')
    artifact_parent(root, output.parent.relative_to(root).as_posix(),
                    excluded_roots=('src', 'recovery', 'cmake'))
    if output.exists() and (not output.is_file() or output.stat().st_nlink != 1):
        raise ValueError('terrain gap initializer must be an unlinked regular file')
    if not output.exists() or output.read_text() != text:
        output.write_text(text)


def _landmark_window_bindings(root: Path, pin: str):
    """Read and validate the five existing identities backing the chosen view."""
    objects = read_objects(root, pin)
    bindings = []
    base = None
    for symbol, member, size, offset in _LANDMARK_WINDOW_MEMBERS:
        matches = [binding for binding in objects
                   if binding.unit == 'src/mapwin.cpp'
                   and binding.symbol == symbol]
        if len(matches) != 1:
            raise ValueError(f'{symbol} requires one source-owned window member binding')
        binding, = matches
        if (binding.parent_symbol != _LANDMARK_WINDOW_PARENT
                or binding.member_path != (member,)):
            raise ValueError(f'{symbol} no longer binds the expected window member')
        if size is None:
            if binding.span is not None:
                raise ValueError(f'{symbol} must retain its start-only identity')
        else:
            if binding.span is None or binding.span.end - binding.span.start != size:
                raise ValueError(f'{symbol} requires its existing bounded extent')
        if base is None:
            base = binding.origin.address
        if binding.origin.address - base != offset:
            raise ValueError(f'{symbol} differs from the reviewed window member offset')
        bindings.append(binding)
    return tuple(bindings)


def terrain_initializers(root: Path, original: Path, directory: Path) -> None:
    terrain_coast_initializer(root, original, directory / 'terrain_coast_remap.inc')
    for filename in _LANDMARK_TABLES:
        terrain_landmark_initializer(root, original, directory / filename)
    for filename in _LANDMARK_GAPS:
        terrain_landmark_gap_initializer(root, original, directory / filename)
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
    caviar_projection_rows_initializer(
        root, original, directory / 'caviar_angle_rows.inc')
    for filename in _POINT_LIGHT_TABLES:
        point_light_initializer(root, original, directory / filename)


_POINT_LIGHT_TABLES = {
    'terrain_point_light_bit1.inc': '_AltLightBit1',
    'terrain_point_light_bit2.inc': '_AltLightBit2',
}


def point_light_initializer(root: Path, original: Path, output: Path) -> None:
    symbol = _POINT_LIGHT_TABLES.get(output.name)
    if symbol is None:
        raise ValueError('unknown point-light initializer')
    _terrain_initializer(root, original, output, output_name=output.name,
                         unit='src/mapwin.cpp', symbol=symbol, size=113,
                         accessed_window=True, require_start_only=True,
                         signed_bytes=True)


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


_CAVIAR_PROJECTION_TABLE = Span(0x006A5A00, 0x006A5A60)
_CAVIAR_PROJECTION_ROWS = Span(0x00699100, 0x006A5A00)
_CAVIAR_PROJECTION_ROW_SIZE = 0x860
_CAVIAR_PROJECTION_ROW_COUNT = 24
_CAVIAR_PROJECTION_ROW_SYMBOL = '?CaviarProjectionAngleData@@3QAY0IGA@$$CBEA'


def caviar_projection_rows_initializer(root: Path, original: Path,
                                        output: Path) -> None:
    root = root.resolve()
    original = original.resolve(strict=True)
    output = output.absolute()
    expected_parent = root / "build/recovery/preparation/terrain-data"
    if (output.resolve() != output or output.name != "caviar_angle_rows.inc"
            or output.parent != expected_parent):
        raise ValueError("Caviar row initializer must remain in the existing ignored preparation directory")
    if any(path.is_symlink() for path in (output, *output.parents)
           if path.is_relative_to(root)):
        raise ValueError("Caviar row initializer cannot use path aliases")

    pin = image_pin(root)
    bindings = [item for item in read_objects(root, pin)
                if item.unit == "src/caviar.cpp"
                and item.origin.address == _CAVIAR_PROJECTION_ROWS.start and item.span is None
                and item.symbol == _CAVIAR_PROJECTION_ROW_SYMBOL]
    if len(bindings) != 1:
        raise ValueError("Caviar rows require exactly one v1 start-only source binding")

    original_bytes = original.read_bytes()
    image = PeImage(original_bytes, pin)
    table = image.read_initial(_CAVIAR_PROJECTION_TABLE)
    pointers = [int.from_bytes(table[index:index + 4], "little")
                for index in range(0, len(table), 4)]
    expected = [_CAVIAR_PROJECTION_ROWS.start + row * _CAVIAR_PROJECTION_ROW_SIZE for row in range(_CAVIAR_PROJECTION_ROW_COUNT)]
    if pointers != expected or len(pointers) != _CAVIAR_PROJECTION_ROW_COUNT:
        raise ValueError("Caviar pointer table no longer describes the reviewed 24-row window")
    if any(address not in image.relocations
           for address in range(_CAVIAR_PROJECTION_TABLE.start, _CAVIAR_PROJECTION_TABLE.end, 4)):
        raise ValueError("each Caviar row pointer must retain its original relocation")
    if len(image.absolute_sites(_CAVIAR_PROJECTION_TABLE)) != _CAVIAR_PROJECTION_ROW_COUNT:
        raise ValueError("unexpected relocation set in Caviar pointer table")

    rows = image.read(_CAVIAR_PROJECTION_ROWS)
    if image.read_initial(_CAVIAR_PROJECTION_ROWS) != rows or image.absolute_sites(_CAVIAR_PROJECTION_ROWS):
        raise ValueError("Caviar row bytes must remain file-backed and relocation-free")
    if len(rows) != _CAVIAR_PROJECTION_ROW_SIZE * _CAVIAR_PROJECTION_ROW_COUNT:
        raise ValueError("Caviar row accessed window changed size")
    text = "// Generated from pinned Caviar row data; do not edit.\n"
    text += "\n".join(
        ", ".join(f"0x{byte:02x}" for byte in rows[offset:offset + 16]) + ","
        for offset in range(0, len(rows), 16)
    ) + "\n"

    if (original.read_bytes() != original_bytes or image_pin(root) != pin
            or bindings[0] not in read_objects(root, pin)):
        raise ValueError("pinned inputs changed during Caviar row generation")
    artifact_parent(root, output.parent.relative_to(root).as_posix(),
                    excluded_roots=("src", "recovery", "cmake"))
    if output.exists() and (not output.is_file() or output.stat().st_nlink != 1):
        raise ValueError("Caviar generated include must be an unlinked regular file")
    if not output.exists() or output.read_text() != text:
        output.write_text(text)

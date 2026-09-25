"""Configure-time terrain data; not a recovery CLI or runtime loader.

The source owns the original binding and C++ type. The installed package owns
source and PE parsing. CMake supplies an ignored build output; never emit code,
pointer values or an unpinned fallback, and never retain extracted data here.
"""
from pathlib import Path

from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from match_recovery.source_replica import artifact_parent


_LANDMARK_TABLES = {
    'terrain_mount_planet.inc': '_MountPlanetTable',
    'terrain_sunny_mesa.inc': '_SunnyMesaTable',
    'terrain_garland_crater.inc': '_GarlandCraterTable',
}


def _terrain_initializer(root: Path, original: Path, output: Path, *,
                         output_name: str, unit: str, symbol: str,
                         size: int, signed_pairs: bool = False) -> None:
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
    if binding.span is None or binding.span.end - binding.span.start != size:
        raise ValueError(f'terrain table requires its bounded {size}-byte source interpretation')
    data = original.read_bytes()
    image = PeImage(data, pin)
    # read_initial validates data-section ownership; read independently refuses
    # virtual zero fill. Both must describe the same fully file-backed bytes.
    values = image.read(binding.span)
    if any(flags & 0x20000000 for (region, _, _), flags in
           zip(image.sections, image.section_flags)
           if region.start <= binding.span.start and binding.span.end <= region.end):
        raise ValueError('terrain remap cannot use executable storage')
    if image.read_initial(binding.span) != values or image.absolute_sites(binding.span):
        raise ValueError('terrain remap requires file-backed non-relocated byte data')
    if signed_pairs:
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

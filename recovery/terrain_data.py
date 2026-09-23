"""Configure-time terrain palette data; not a recovery CLI or runtime loader.

The source owns the original binding and C++ type. The installed package owns
source and PE parsing. CMake supplies an ignored build output; never emit code,
pointer values or an unpinned fallback, and never retain extracted data here.
"""
from pathlib import Path

from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from match_recovery.source_replica import artifact_parent


def terrain_coast_initializer(root: Path, original: Path, output: Path) -> None:
    root = root.resolve()
    original = original.resolve(strict=True)
    output = output.absolute()
    # This hook is deliberately project-specific, not an arbitrary exporter.
    # Only the one uchar table required by load_terrain is materialized.
    if (output.resolve() != output or output.name != 'terrain_coast_remap.inc'
            or not output.is_relative_to(root / 'build')):
        raise ValueError('terrain initializer must remain in ignored project build context')
    if any(p.is_symlink() for p in (output, *output.parents) if p.is_relative_to(root)):
        raise ValueError('terrain initializer output cannot use aliases')
    pin = image_pin(root)
    binding, = [b for b in read_objects(root, pin)
                if b.unit == 'src/game.cpp' and b.symbol == '_TerrainCoastRemapSource']
    if binding.span is None or binding.span.end - binding.span.start != 53:
        raise ValueError('terrain remap requires its bounded 53-byte source interpretation')
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
    rows = [', '.join(str(value) for value in values[i:i + 16]) + ','
            for i in range(0, len(values), 16)]
    text = '// Generated from the pinned source-owned terrain remap; do not edit.\n' + '\n'.join(rows) + '\n'
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

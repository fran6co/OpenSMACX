"""Apply the declared post-link image patches to a freshly linked executable.

The shipped image carries bytes written over the linked file after the build
(`RECOVERY_IMAGE_PATCH`, see `source/image_patch.py`). The source is the code
as it was compiled, so a faithful build has the same second step: this module
writes each declared patch form over the bytes the linker placed.

The source declarations are the only list. For every declaration:

- its function is the one mapped record whose span holds it; the record's
  emitted symbol must be exactly one function row of the link's own MAP
  (which is validated against the executable), giving the built address;
- the built bytes from the function's start must decode to complete
  instructions that begin at the span's first byte and end at its last (the
  comparator's `covering_run` rule), no base relocation field may cross a
  span edge, and the bytes must not already be the patch;
- a `nop-jmp` destination must lie inside the same function and land on a
  decoded instruction start; it moves with the function.

Then the declared bytes are written at `built address + span offset`. The
relocation table is left alone, as the original patcher left it: an absolute
field of the replaced instructions keeps its stale HIGHLOW entry. Nothing
else in the file changes (no checksum, timestamp or size change).

Any refusal writes no output and removes a previous one: a half-patched or
stale file must never stand in for the reproduced image. The linked
executable itself is never modified; the patched file is a separate output,
because the MAP, the link observation and the debugger sidecar describe the
linker's product, and a second run must still see the compiled instructions
it has to verify.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

from .model import Span
from .msvc_link_map import parse_link_map
from .pe_coff import PeImage
from .source.image_patch import (FORM_NOP_JMP, PREFIX, covering_run, parse_image_patches,
                                 patches_for_spans)
from .source.reader import read_file
from .source_config import SourceConfiguration


def declared_patches(root: Path) -> list[tuple]:
    """Every `(record, patch)` the configured, non-seed source declares.

    A declaration outside every mapped span, or inside more than one, refuses:
    each patch belongs to exactly one function.
    """
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    found = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        raw = path.read_bytes()
        if configuration.is_seed(relative) or PREFIX.encode() not in raw:
            continue
        patches = parse_image_patches(raw.decode("utf-8", errors="replace"))
        owners = {patch: [] for patch in patches}
        for record in read_file(path):
            spans = tuple(getattr(record, "image_spans", ()) or ())
            for patch in patches_for_spans(patches, spans) if spans else ():
                owners[patch].append(record)
        for patch, records in owners.items():
            if len(records) != 1:
                raise ValueError(f"{relative}: image patch {patch.describe()} is inside "
                                 f"{len(records)} mapped functions, expected exactly one")
            found.append((records[0], patch))
    return sorted(found, key=lambda item: item[1].start)


def _file_offset(image: PeImage, span: Span) -> int:
    matches = [(region, offset) for region, offset, size in image.sections
               if region.start <= span.start and span.end <= region.start + size]
    if len(matches) != 1:
        raise ValueError("built span is not uniquely backed by file bytes")
    region, offset = matches[0]
    return offset + span.start - region.start


def _built_address(record, mapping) -> tuple[str, int]:
    names = [name for name in dict.fromkeys((getattr(record, "symbol", None), record.name)) if name]
    rows = [row for row in (*mapping.publics, *(mapping.statics or ()))
            if row.symbol in names and row.function and row.segment]
    addresses = {row.address for row in rows}
    if len(addresses) != 1:
        raise ValueError(f"MAP places {' / '.join(names) or '?'} at {len(addresses)} function addresses, "
                         "expected exactly one")
    return rows[0].symbol, addresses.pop()


def plan_patch(record, patch, image: PeImage, mapping) -> dict:
    """One verified write: `{"offset", "bytes", ...}`; raises ValueError to refuse."""
    from .source.asm import _decode, _render

    spans = [span for span in record.image_spans if span[0] <= patch.start and patch.end <= span[1]]
    if len(spans) != 1 or spans[0][0] != record.address:
        raise ValueError("the patched span is not in the function's entry span")
    original_start, original_end = spans[0]
    symbol, built = _built_address(record, mapping)
    start, end = patch.start - original_start, patch.end - original_start
    reach = end
    target = None
    if patch.form == FORM_NOP_JMP:
        if not original_start <= patch.jmp_target < original_end:
            raise ValueError("the declared jump leaves its function; only an in-function "
                             "destination moves with the built function")
        target = patch.jmp_target - original_start
        reach = max(reach, target + 1)
    code = image.read(Span(built, built + reach))
    instructions = _decode(code, built)
    extents = [(instruction.address - built, instruction.size) for instruction in instructions]
    run = covering_run(extents, start, end)
    if run is None:
        raise ValueError("no run of complete built instructions covers exactly that span")
    if target is not None and target not in {offset for offset, _size in extents}:
        raise ValueError("the jump destination is not a built instruction start")
    current = code[start:end]
    expected = patch.expected  # position-independent: nops, or E9 with a relative displacement
    if current == expected or set(current) == {0x90}:
        raise ValueError("the built bytes already hold the patch; the input is not the linker's output")
    crossing = sorted(site for site in image.relocations
                      if site < built + start < site + 4 or site < built + end < site + 4)
    if crossing:
        raise ValueError("a base relocation field crosses the span edge at "
                         + ", ".join(f"0x{site:08X}" for site in crossing))
    stale = sorted(site for site in image.relocations if built + start <= site < built + end)
    return {"function": f"0x{record.address:08X}", "symbol": symbol,
            "source": f"{Path(record.path).name}:{patch.line}",
            "declared": patch.describe(),
            "built_start": f"0x{built + start:08X}", "built_end": f"0x{built + end:08X}",
            "offset": _file_offset(image, Span(built + start, built + end)),
            "replaced_bytes": current.hex(),
            "replaced_instructions": [_render(instructions[index]) for index in run],
            "written_bytes": expected.hex(),
            "base_relocations_left_in_span": [f"0x{site:08X}" for site in stale],
            "bytes": expected}


def apply_image_patches(root: Path, exe: Path, map_path: Path, out: Path) -> dict:
    """Write `out` = `exe` with every declared patch applied; never touches `exe`."""
    exe, out = Path(exe).resolve(), Path(out).resolve()
    if out == exe:
        raise ValueError("the patched image is a separate output; it cannot replace the linked executable")
    data = exe.read_bytes()
    report = {"operation": "apply-image-patches", "input": str(exe), "output": str(out),
              "input_sha256": hashlib.sha256(data).hexdigest(), "patches": [], "refused": []}
    patched = bytearray(data)
    try:
        declared = declared_patches(Path(root))
        if declared:
            image = PeImage(data, report["input_sha256"])
            mapping = parse_link_map(Path(map_path).read_bytes())
            mapping.validate(image)
        for record, patch in declared:
            try:
                row = plan_patch(record, patch, image, mapping)
            except ValueError as error:
                report["refused"].append({"function": f"0x{record.address:08X}",
                                          "declared": patch.describe(), "reason": str(error)})
                continue
            raw = row.pop("bytes")
            patched[row["offset"]:row["offset"] + len(raw)] = raw
            report["patches"].append(row)
    except (OSError, ValueError) as error:
        report["refused"].append({"reason": str(error)})
    if report["refused"] or len(patched) != len(data):
        out.unlink(missing_ok=True)
        report.update(result="fail", written=False)
        return report
    temporary = out.with_name(out.name + ".tmp")
    temporary.write_bytes(bytes(patched))
    temporary.replace(out)
    report.update(result="pass", written=True, output_sha256=hashlib.sha256(bytes(patched)).hexdigest(),
                  changed_bytes=sum(a != b for a, b in zip(data, patched)))
    return report

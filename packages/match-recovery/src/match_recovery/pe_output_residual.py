"""Literal same-layout PE accounting, not source authority or acceptance.

The caller must independently establish that supplied bodies are authorized,
complete and have no aliases/companions. No timestamp, directory, padding or
overlay normalization is performed. PE geometry fields absent from PeImage use
Microsoft's PE32 optional/section header definitions:
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
"""

from hashlib import sha256
import struct

from .model import Result, Span, require_text
from .pe_coff import PeImage


MAX_FILE_BYTES = 512 * 1024 * 1024
MAX_BODIES = 100000


def _geometry(image):
    if (type(image) is not PeImage or type(image.data) is not bytes
            or not 0 < len(image.data) <= MAX_FILE_BYTES):
        raise ValueError("PE accounting requires bounded immutable PeImage bytes")
    fresh = PeImage(image.data, sha256(image.data).hexdigest())
    if image.__dict__ != fresh.__dict__:
        raise ValueError("parsed PE state differs from its physical bytes")
    data = image.data
    pe, = struct.unpack_from("<I", data, 0x3C)
    count, = struct.unpack_from("<H", data, pe + 6)
    optional, = struct.unpack_from("<H", data, pe + 20)
    section_alignment, file_alignment = struct.unpack_from("<II", data, pe + 24 + 32)
    image_size, headers_size = struct.unpack_from("<II", data, pe + 24 + 56)
    table = pe + 24 + optional
    power = lambda value: value > 0 and value & (value - 1) == 0
    if (not 1 <= count <= 96 or not power(section_alignment) or not power(file_alignment)
            or section_alignment < file_alignment
            or section_alignment < 0x1000 and section_alignment != file_alignment
            or not table + count * 40 <= headers_size <= len(data)
            or headers_size % file_alignment or image_size % section_alignment
            or image.base + image_size > 0x100000000):
        raise ValueError("unsupported or unbounded PE header geometry")
    sections, raw_ranges, zeros = [], [], []
    for index, ((span, offset, size), flags) in enumerate(zip(image.sections, image.section_flags)):
        virtual, rva = struct.unpack_from("<II", data, table + index * 40 + 8)
        if (rva < headers_size or rva % section_alignment or rva + max(virtual, size) > image_size
                or size and (offset < headers_size or offset % file_alignment or size % file_alignment)):
            raise ValueError("PE section aliases headers or has unsupported geometry")
        if size:
            raw_ranges.append((offset, offset + size))
        if virtual > size:
            zeros.append({"address": span.start + size, "size": virtual - size})
        sections.append((virtual, rva, size, offset, flags))
    raw_ranges.sort()
    if any(a[1] > b[0] for a, b in zip(raw_ranges, raw_ranges[1:])):
        raise ValueError("PE sections have aliased physical backing")
    return (pe, optional, image.base, section_alignment, file_alignment, image_size,
            headers_size, tuple(sections)), zeros


def _bodies(image, rows):
    if not isinstance(rows, (list, tuple)) or len(rows) > MAX_BODIES:
        raise ValueError("PE scope requires a bounded body list")
    result, identities = [], set()
    for row in rows:
        if not isinstance(row, dict) or set(row) != {"entity_id", "address", "size"}:
            raise ValueError("invalid PE body identity/extent fields")
        require_text(row["entity_id"], "PE body identity")
        address, size = row["address"], row["size"]
        if (row["entity_id"] in identities or type(address) is not int or type(size) is not int
                or size <= 0 or address < image.base or address + size > 0x100000000):
            raise ValueError("duplicate, invalid or unbounded PE body")
        identities.add(row["entity_id"])
        matches = [(span, offset) for (span, offset, raw), flags in zip(image.sections, image.section_flags)
                   if span.start <= address and address + size <= span.start + raw
                   and flags & 0x20 and flags & 0x20000000]
        if len(matches) != 1:
            raise ValueError("PE body lacks unique backed executable storage")
        image.read(Span(address, address + size))
        span, offset = matches[0]
        result.append({**row, "file_offset": offset + address - span.start})
    result.sort(key=lambda row: row["file_offset"])
    if any(a["file_offset"] + a["size"] > b["file_offset"] for a, b in zip(result, result[1:])):
        raise ValueError("PE scoped bodies overlap")
    return result


def compare_same_layout_pe(baseline_image, current_image, baseline_bodies, current_bodies, *, max_difference_ranges=256):
    """Count every outside byte; bound diagnostics, never the comparison scan.

    Both root lists must identify exactly the same bodies at identical places
    and extents. Different geometry or invalid scope stays unknown. File-length
    changes outside sections are literal added/removed overlay differences.
    Virtual zero-fill has identical extent and initial zero contents under the
    supported geometry; this says nothing about later loader/runtime effects.
    """
    report = {"check": "same_layout_pe_output_residual", "result": Result.UNKNOWN,
              "same_layout": Result.UNKNOWN, "outside_bytes": Result.UNKNOWN,
              "outside_scope_verification": Result.UNKNOWN, "ready_for_collection": False,
              "persisted": False}
    try:
        if type(max_difference_ranges) is not int or not 0 <= max_difference_ranges <= 100000:
            raise ValueError("invalid PE difference diagnostic bound")
        geometry, zeros = _geometry(baseline_image)
        current_geometry, current_zeros = _geometry(current_image)
        if geometry != current_geometry or zeros != current_zeros:
            raise ValueError("PE virtual/raw geometry differs")
        before = _bodies(baseline_image, baseline_bodies)
        after = _bodies(current_image, current_bodies)
        if before != after:
            raise ValueError("PE body identities, placements or extents differ")
    except (ValueError, TypeError, struct.error) as error:
        return {**report, "reason": str(error)}

    a, b = baseline_image.data, current_image.data
    outside, cursor = [], 0
    for row in before:
        outside.append((cursor, row["file_offset"]))
        cursor = row["file_offset"] + row["size"]
    outside.append((cursor, max(len(a), len(b))))
    differences, ranges, total_ranges = 0, [], 0
    for begin, end in outside:
        opened = None

        def close(at):
            nonlocal opened, total_ranges
            if opened is not None:
                total_ranges += 1
                if len(ranges) < max_difference_ranges:
                    ranges.append({"offset": opened, "size": at - opened})
                opened = None

        for block in range(begin, end, 65536):
            stop = min(end, block + 65536)
            if a[block:stop] == b[block:stop]:
                close(block)
                continue
            for at in range(block, stop):
                if at >= len(a) or at >= len(b) or a[at] != b[at]:
                    differences += 1
                    if opened is None:
                        opened = at
                else:
                    close(at)
        close(end)
    return {**report, "same_layout": Result.PASS,
            "outside_bytes": Result.FAIL if differences else Result.PASS,
            "baseline_sha256": sha256(a).hexdigest(), "current_sha256": sha256(b).hexdigest(),
            "baseline_file_bytes": len(a), "current_file_bytes": len(b),
            "scoped_bodies": before, "scoped_file_bytes": sum(row["size"] for row in before),
            "compared_file_bytes": sum(end - begin for begin, end in outside),
            "different_file_bytes": differences, "difference_range_count": total_ranges,
            "difference_ranges": ranges, "difference_ranges_truncated": total_ranges > len(ranges),
            "zero_fill_regions": zeros, "unchanged_zero_fill_bytes": sum(row["size"] for row in zeros)}

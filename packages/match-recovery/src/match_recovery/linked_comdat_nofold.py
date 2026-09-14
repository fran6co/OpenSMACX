"""Conservative fixed-prefix witnesses, not ICF or selection authority.

Microsoft describes ICF over whole COMDAT code segments and the native linker
as operating on OBJ sections, not arbitrary instruction subsequences:
https://devblogs.microsoft.com/oldnewthing/20161024-00/?p=94575
https://devblogs.microsoft.com/cppblog/shrink-my-program-database-pdb-file/
The documented i386 relocation fields are listed at:
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#intel-386-processors

These sources do not establish VC6-specific padding behavior. Consequently this
witness ignores every trailing zero/NOP/INT3 byte and never distinguishes solely
by size. The caller must establish a complete physical population, i386 machine,
whole-section offset-zero roots, and a relocation-only linker transformation
domain. True only rules out this pair; False does not prove fold equivalence.
"""

from .pe_coff import Section


# Complete containing-byte widths, not masks for individual relocated bits.
# ABSOLUTE is a documented no-op. Unsupported DIR16/REL16/SEG12 and CLR TOKEN
# deliberately refuse, as do unknown extensions. SECREL7 masks its entire byte.
_WIDTHS = {0x0000: 0, 0x0006: 4, 0x0007: 4, 0x000A: 2,
           0x000B: 4, 0x000D: 1, 0x0014: 4}
_PADDING = frozenset((0x00, 0x90, 0xCC))


def _observed(section, declared_size):
    if (type(section) is not Section or type(section.data) is not bytes
            or type(declared_size) is not int or declared_size < 0
            or len(section.data) != declared_size
            or type(section.relocations) is not tuple):
        raise ValueError("no-fold witness requires the exact immutable physical raw extent")
    writes = set()
    for row in section.relocations:
        if (type(row) is not tuple or len(row) != 3
                or any(type(value) is not int for value in row)):
            raise ValueError("no-fold witness has malformed i386 relocation metadata")
        offset, target, kind = row
        if kind not in _WIDTHS:
            raise ValueError("no-fold witness has an unsupported i386 relocation kind")
        width = _WIDTHS[kind]
        if offset < 0 or target < 0 or offset + width > declared_size:
            raise ValueError("no-fold witness relocation is outside its physical raw extent")
        writes.update(range(offset, offset + width))
    end = declared_size
    while end and section.data[end - 1] in _PADDING:
        end -= 1
    return end, writes


def sections_distinct(left_section, left_declared_size, right_section, right_declared_size) -> bool:
    """Find an unchanged byte mismatch at the same section-local offset.

    Validate both complete relocation tables before returning, even if an early
    byte differs or either section is empty. Relocation targets/addends, section
    labels/flags, association shapes and unequal sizes grant no distinction.
    No bytes or metadata are modified, and no instruction sliding is performed.
    """
    left_end, left_writes = _observed(left_section, left_declared_size)
    right_end, right_writes = _observed(right_section, right_declared_size)
    writes = left_writes | right_writes
    return any(left_section.data[offset] != right_section.data[offset]
               for offset in range(min(left_end, right_end)) if offset not in writes)

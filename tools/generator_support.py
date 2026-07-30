#!/usr/bin/env python3
"""Helpers shared by the bulk source generators.

A leaf module on purpose: it imports nothing from `tools/` beyond the third
party PE reader, so any generator can import it without risking the cycle that
kept `tools/jump_tables.py` carrying its own copy of `read_bytes`.

What belongs here is the machinery every generator needs identically - the
licence block its output opens with, reading bytes at a virtual address, and
turning a catalogued name into a C identifier. What does NOT belong here is
anything a generator decides for itself: the shapes it accepts, the seams it
binds, the spelling of the code it emits.

`tools/disasm.py` keeps its own `read_range`, deliberately. It is the fail-LOUD
reader - it raises for an address outside every section, and
`tools/test_disasm.py` pins that - where `read_bytes` below is the fail-EMPTY
one a shape-matching generator wants, because a generator walks thousands of
catalogued rows and an unreadable one is a row to refuse, not a crash.
"""

from __future__ import annotations

import re

import pefile

# The block every generated file opens with. It was previously six mechanisms
# for one string: three literals and three import-time reads of
# src/vector_teardown.h, which made a constant depend on a committed source
# file continuing to exist and to keep its licence in the same place. All six
# resolved to the identical 756 bytes when measured, so this is the same text,
# not a new one.
LICENSE = """/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
"""


def read_bytes(pe: pefile.PE, address: int, length: int) -> bytes:
    """`length` bytes at a virtual address, or b"" where the file has none.

    A section's virtual size can exceed its raw size, and here one does by a
    lot: `.data` is 155,648 bytes in the file and 3,408,376 in memory, so
    3,252,728 bytes of zero-initialised globals have no bytes on disk at all.

    Reading there returns EMPTY rather than whatever sits at the computed file
    offset, because that offset is not this section's data. Measured on the
    pinned executable: probing the `.data` tail a page at a time, 60 of 795
    probes came back non-zero - `.rsrc` and `.reloc` content presented as
    globals - and the offsets run to 6,038,000 in a 3,084,288-byte file, so
    most of the tail is past EOF. A generator handed b"" refuses the row; one
    handed foreign bytes can decode them into a shape it accepts.

    The length is clamped to the section's raw data for the same reason: a read
    that starts in one section must not run into the next one's bytes.
    """
    base = pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = base + section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= address < end:
            delta = address - begin
            if delta >= section.SizeOfRawData:
                return b""
            offset = section.PointerToRawData + delta
            available = section.SizeOfRawData - delta
            return pe.__data__[offset:offset + min(length, available)]
    return b""


def snake(name: str) -> str:
    """CamelCase or Camel_Case to snake_case, leaving snake_case alone.

    Splits on word boundaries, so an acronym stays whole: `HTTPServer` becomes
    `http_server`, not `h_t_t_p_server`. Use this on CLASS and SEAM names.
    For a global's catalogued name - `g_BASEWIN_WAVE` and its kind - use
    `identifier_of_global`, which is a different operation.
    """
    text = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    text = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", text)
    return re.sub(r"__+", "_", text).strip("_").lower()


def camel(name: str) -> str:
    """snake_case to CamelCase, dropping the separators."""
    return "".join(part.capitalize() for part in name.split("_") if part)


def identifier_of_global(name: str) -> str:
    """A global's catalogued name as the identifier the emitted twin uses.

    NOT a case converter, which is why it does not share `snake`'s body: the
    input is an already-upper SCREAMING_CASE global name, so the work is to
    drop the `g_` marker, lower it, and collapse the doubled underscores some
    catalogued names carry. `g_BASEWIN_WAVE` becomes `basewin_wave`.

    Applying `snake` here instead would rewrite every emitted symbol - the
    generated trampolines really are spelled `ambienceinitctor_default`,
    because this runs over seam names too - so the two must stay separate even
    though both return lowercase text with underscores.
    """
    text = name[2:] if name.startswith("g_") else name
    return re.sub(r"__+", "_", text.lower())

"""Strict public-address evidence from an MSVC PE32 executable link map.

A map supplies candidate placements, not original bindings, byte agreement,
archive provenance or object extents. Validate its coordinates against the PE
and compare actual linked bytes before using it for a bounded link check.
Debugger sidecar hints deliberately have a different, permissive parser.
"""

from dataclasses import dataclass
import re


@dataclass(frozen=True)
class Public:
    symbol: str
    segment: int
    offset: int
    address: int
    function: bool
    owner: str


@dataclass(frozen=True)
class LinkMap:
    base: int
    entry: tuple[int, int]
    publics: tuple[Public, ...]

    def validate(self, image):
        if self.base != image.base:
            raise ValueError("link map preferred base disagrees with executable")
        def address(segment, offset):
            if not 1 <= segment <= len(image.sections):
                raise ValueError("unsupported link map segment")
            region, _, _ = image.sections[segment - 1]
            value = region.start + offset
            if value >= region.end:
                raise ValueError("link map offset is outside its PE section")
            return value
        if address(*self.entry) != image.entrypoint:
            raise ValueError("link map entry point disagrees with executable")
        for public in self.publics:
            # Segment zero is an absolute linker symbol, never a placed image
            # target. Preserve it but do not invent a PE section or HIGHLOW.
            if public.segment and address(public.segment, public.offset) != public.address:
                raise ValueError(f"link map coordinates disagree for {public.symbol}")


def parse_link_map(data: bytes) -> LinkMap:
    try:
        text = data.decode("ascii")
    except UnicodeDecodeError as error:
        raise ValueError("unsupported non-ASCII MSVC link map") from error
    if any(ord(char) < 32 and char not in "\r\n\t" for char in text):
        raise ValueError("invalid control character in MSVC link map")
    base, entry, publics, names = None, None, [], set()
    active = False
    for raw in text.splitlines():
        line = raw.strip()
        found = re.fullmatch(r"Preferred load address is ([0-9a-fA-F]{8})", line)
        if found:
            if base is not None or active:
                raise ValueError("duplicate or misplaced link map preferred base")
            base = int(found[1], 16)
            continue
        if re.fullmatch(r"Address\s+Publics by Value\s+Rva\+Base\s+Lib:Object", line):
            if active or entry is not None or base is None:
                raise ValueError("duplicate or misplaced link map public table")
            active = True
            continue
        found = re.fullmatch(r"entry point at\s+([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", line)
        if found:
            if not active or entry is not None:
                raise ValueError("misplaced link map entry point")
            entry = (int(found[1], 16), int(found[2], 16))
            active = False
            continue
        if not active or not line:
            continue
        found = re.fullmatch(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+"
                            r"([0-9a-fA-F]{8})\s+(.+)", line)
        if found is None:
            raise ValueError("malformed MSVC link map public row")
        rest = found[5].split()
        flags, owner = rest[:-1], rest[-1]
        if owner in ("f", "i"):
            raise ValueError("missing MSVC link map public owner")
        if any(flag not in ("f", "i") for flag in flags) or len(set(flags)) != len(flags):
            raise ValueError("unsupported MSVC link map public flags")
        if found[3] in names:
            raise ValueError(f"duplicate MSVC link map public symbol: {found[3]}")
        names.add(found[3])
        publics.append(Public(found[3], int(found[1], 16), int(found[2], 16),
                              int(found[4], 16), "f" in flags, owner))
    if base is None or entry is None or not publics or active:
        raise ValueError("incomplete MSVC executable link map")
    return LinkMap(base, entry, tuple(publics))

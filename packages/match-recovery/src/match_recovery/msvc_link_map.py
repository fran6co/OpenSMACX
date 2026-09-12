"""Strict placement evidence from an MSVC PE32 executable link map.

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
class SectionContribution:
    segment: int
    offset: int
    size: int
    name: str
    kind: str


@dataclass(frozen=True)
class LinkMap:
    base: int
    entry: tuple[int, int]
    publics: tuple[Public, ...]
    # None distinguishes an unobserved table from an observed empty table.
    contributions: tuple[SectionContribution, ...] | None = None
    statics: tuple[Public, ...] | None = None

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
        for public in (*self.publics, *(self.statics or ())):
            # Segment zero is an absolute linker symbol, never a placed image
            # target. Preserve it but do not invent a PE section or HIGHLOW.
            if public.segment and address(public.segment, public.offset) != public.address:
                raise ValueError(f"link map coordinates disagree for {public.symbol}")
        placed = []
        for item in self.contributions or ():
            if not 1 <= item.segment <= len(image.sections):
                raise ValueError("unsupported link map contribution segment")
            region, _, _ = image.sections[item.segment - 1]
            if item.offset < 0 or item.size < 0 or item.offset + item.size > region.end - region.start:
                raise ValueError("link map contribution is outside its PE section")
            if any(old.segment == item.segment and max(old.offset, item.offset) < min(old.offset + old.size, item.offset + item.size)
                   for old in placed):
                raise ValueError("overlapping MSVC link map contributions")
            placed.append(item)


def _symbol(line, kind):
    found = re.fullmatch(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+"
                        r"([0-9a-fA-F]{8})\s+(.+)", line)
    if found is None:
        raise ValueError("malformed MSVC link map " + kind + " row")
    rest = found[5].split()
    flags, owner = rest[:-1], rest[-1]
    if owner in ("f", "i"):
        raise ValueError("missing MSVC link map " + kind + " owner")
    if any(flag not in ("f", "i") for flag in flags) or len(set(flags)) != len(flags):
        raise ValueError("unsupported MSVC link map " + kind + " flags")
    return Public(found[3], int(found[1], 16), int(found[2], 16), int(found[4], 16), "f" in flags, owner)


def parse_link_map(data: bytes) -> LinkMap:
    try:
        text = data.decode("ascii")
    except UnicodeDecodeError as error:
        raise ValueError("unsupported non-ASCII MSVC link map") from error
    if any(ord(char) < 32 and char not in "\r\n\t" for char in text):
        raise ValueError("invalid control character in MSVC link map")
    base, entry, publics, names = None, None, [], set()
    contributions, statics = None, None
    contribution_keys, static_names, line_tables, line_rows = set(), set(), set(), set()
    state, public_seen, icf_seen = "preamble", False, False
    for raw in text.splitlines():
        line = raw.strip()
        if not line:
            continue
        found = re.fullmatch(r"Preferred load address is ([0-9a-fA-F]{8})", line)
        if found:
            if base is not None or state != "preamble":
                raise ValueError("duplicate or misplaced link map preferred base")
            base = int(found[1], 16)
            continue
        if line.startswith("Preferred load"):
            raise ValueError("malformed MSVC link map preferred base")
        if re.fullmatch(r"Start\s+Length\s+Name\s+Class", line):
            if base is None or contributions is not None or state != "preamble":
                raise ValueError("duplicate or misplaced link map contribution table")
            contributions, state = [], "contributions"
            continue
        if line.startswith("Start"):
            raise ValueError("malformed MSVC link map contribution table")
        if re.fullmatch(r"Address\s+Publics by Value\s+Rva\+Base\s+Lib:Object", line):
            if public_seen or entry is not None or base is None or state not in ("preamble", "contributions"):
                raise ValueError("duplicate or misplaced link map public table")
            state, public_seen = "publics", True
            continue
        found = re.fullmatch(r"entry point at\s+([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", line)
        if found:
            if state != "publics" or entry is not None:
                raise ValueError("misplaced link map entry point")
            entry = (int(found[1], 16), int(found[2], 16))
            state = "after_entry"
            continue
        if line == "Static symbols":
            if entry is None or statics is not None or state == "trailer":
                raise ValueError("duplicate or misplaced link map static table")
            statics, state = [], "statics"
            continue
        found = re.fullmatch(r"Line numbers for (\S+)\((.+)\) segment (\S+)", line)
        if found:
            if entry is None or state == "trailer" or found.groups() in line_tables:
                raise ValueError("duplicate or misplaced MSVC link map line-number table")
            line_tables.add(found.groups())
            state, line_rows = "lines", set()
            continue
        if re.fullmatch(r"[0-9]+ bytes saved through ICF", line):
            if entry is None or icf_seen:
                raise ValueError("duplicate or misplaced MSVC link map ICF summary")
            state, icf_seen = "trailer", True
            continue
        if line.startswith(("Static", "Line numbers", "entry point", "Address")):
            raise ValueError("malformed MSVC link map table or entry point")
        if state == "contributions":
            found = re.fullmatch(r"([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+([0-9a-fA-F]{8})H\s+(\S+)\s+(\S+)", line)
            if found is None:
                raise ValueError("malformed MSVC link map contribution row")
            item = SectionContribution(int(found[1], 16), int(found[2], 16), int(found[3], 16), found[4], found[5])
            identity = item.segment, item.offset, item.name
            if identity in contribution_keys:
                raise ValueError("duplicate MSVC link map contribution row")
            contribution_keys.add(identity)
            contributions.append(item)
        elif state in ("publics", "statics"):
            item = _symbol(line, "public" if state == "publics" else "static")
            identity = item.symbol if state == "publics" else (item.owner.casefold(), item.symbol)
            seen = names if state == "publics" else static_names
            if identity in seen:
                raise ValueError("duplicate MSVC link map " + state + " symbol: " + item.symbol)
            seen.add(identity)
            (publics if state == "publics" else statics).append(item)
        elif state == "lines":
            # Line tables are not placement coverage. Validate their complete
            # syntax so a transition never swallows malformed symbol evidence.
            if not re.fullmatch(r"(?:[0-9]+\s+[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s*)+", line):
                raise ValueError("malformed MSVC link map line-number row")
            for row in re.findall(r"([0-9]+)\s+([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", line):
                key = tuple(int(value, 10 if index == 0 else 16) for index, value in enumerate(row))
                if key in line_rows:
                    raise ValueError("duplicate MSVC link map line-number row")
                line_rows.add(key)
        elif state != "preamble" or base is not None or re.match(r"[0-9a-fA-F]{4}:", line):
            raise ValueError("unexpected row outside MSVC link map tables")
    if base is None or entry is None or not publics or state == "publics":
        raise ValueError("incomplete MSVC executable link map")
    return LinkMap(base, entry, tuple(publics), None if contributions is None else tuple(contributions),
                   None if statics is None else tuple(statics))

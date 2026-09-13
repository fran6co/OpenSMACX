"""Image-bound ICF groups with every original physical owner retained.

This binds complete verbose-linker groups, not code equivalence or frame
acceptance. Native callers keep build epochs live and independently verify all
members; a folded map owner never replaces an original COFF owner.
"""

from dataclasses import dataclass
from hashlib import sha256
from pathlib import PureWindowsPath

from .linked_fixups import check_image
from .linked_objects import SelectedObjects
from .model import Span
from .msvc_icf import parse_icf_log
from .pe_coff import Coff, Symbol


@dataclass(frozen=True)
class FoldMember:
    owner: str
    symbol: str
    reported_symbol: str
    input: tuple
    object_sha256: str
    raw: bytes
    root: Symbol
    address: int
    fpo_sections: tuple[int, ...]


@dataclass(frozen=True)
class FoldGroup:
    image_sha256: str
    log_sha256: str
    address: int
    members: tuple[FoldMember, ...]

    @property
    def selected(self):
        return self.members[0]


def _owner(row):
    owner = row["owner"]
    member = PureWindowsPath(owner["member"]).name
    return (PureWindowsPath(owner["library"]).stem + ":" + member
            if owner["library"] is not None else member)


def _roots(obj, name):
    definitions = [item for item in obj.symbols.values() if item.name == name and item.section > 0]
    sections = {number for number, section in enumerate(obj.sections, 1) if section.name == name}
    if (not sections and len(definitions) != 1) or not definitions:
        raise ValueError("ICF member lacks a unique defined original symbol")
    if sections and (len(definitions) != len(sections)
                     or {item.section for item in definitions} != sections):
        raise ValueError("ICF section member lacks exact unique section definitions")
    result = []
    for symbol in definitions:
        section = obj.sections[symbol.section - 1]
        if sections:
            if (symbol.storage != 3 or symbol.type or symbol.value
                    or len(obj.auxiliary.get(symbol.index, b"")) != 18):
                raise ValueError("ICF section member lacks its exact COFF definition")
            roots = [item for item in obj.symbols.values() if item.section == symbol.section
                     and item.type == 0x20 and item.storage in (2, 3) and item.value == 0]
            if len(roots) != 1:
                raise ValueError("ICF section member lacks one exact defined code root")
            symbol, = roots
        root = obj.function(symbol.name)
        if (root != symbol or root.section in obj.associations or section.flags & 0x80
                or not section.flags & 0x20000000 or not section.data
                or len(section.data) != obj.section_sizes[root.section - 1]):
            raise ValueError("unsupported ICF original code root")
        result.append(root)
    return tuple(result)


@dataclass(frozen=True)
class Folding:
    """Lazy complete-group binding inside the caller's live build epoch."""

    _objects: SelectedObjects
    _mapping: object
    _image: object
    _fixups: object
    _groups: tuple
    image_sha256: str
    log_sha256: str

    def check_current(self):
        check_image(self._fixups, self._image)
        if sha256(self._image.data).hexdigest() != self.image_sha256:
            raise ValueError("ICF binding image changed")
        if self._objects._constant_evidence.get("link_log_sha256") != self.log_sha256:
            raise ValueError("ICF log differs from selected-object input evidence")
        _, fresh = self._objects._constant_population()
        fresh()

    def candidate_keys(self):
        """Advisory lookup superset, never group selection or acceptance.

        Native graph callers can avoid expensive negative lookups. Every
        positive still needs complete group binding; section-only observations
        retain all actual candidate names, including unsupported definitions.
        """
        self.check_current()
        try:
            result, parsed = set(), {}
            for rows in self._groups:
                winner = rows[0][0].casefold()
                for owner, reported in rows:
                    names = {reported}
                    if reported.startswith("."):
                        selected = self._objects.resolve(owner)
                        if selected["sha256"] not in parsed:
                            parsed[selected["sha256"]] = Coff(selected["raw"])
                        obj = parsed[selected["sha256"]]
                        sections = {number for number, section in enumerate(obj.sections, 1)
                                    if section.name == reported}
                        if not sections:
                            raise ValueError("ICF section observation lacks its physical section candidates")
                        names.update(symbol.name for symbol in obj.symbols.values() if symbol.section in sections)
                    result.update((spelling, name) for spelling in (owner.casefold(), winner) for name in names)
            return frozenset(result)
        finally:
            self.check_current()

    def group(self, owner, symbol, *, optional=False):
        """Find by original owner and reported symbol; absence is not no-fold proof."""
        self.check_current()
        indices = [index for index, rows in enumerate(self._groups)
                   if any(original.casefold() == owner.casefold() and self._reported_match(original, name, symbol)
                          for original, name in rows)]
        if not indices and optional:
            self.check_current()
            return None
        if not indices or (len(indices) != 1 and any(name == symbol for index in indices
                                                   for original, name in self._groups[index]
                                                   if original.casefold() == owner.casefold())):
            raise ValueError("ICF member has no unique complete reported group")
        groups = [self._bind(index) for index in indices]
        groups = [group for group in groups if any(member.owner.casefold() == owner.casefold()
                   and symbol in (member.symbol, member.reported_symbol) for member in group.members)]
        if not groups:
            if optional:
                return None
            raise ValueError("original symbol is not the independently bound ICF section member")
        if len(groups) != 1:
            raise ValueError("ICF member has no unique complete reported group")
        return groups[0]

    def for_map(self, owner, symbol, *, optional=False):
        """Resolve a rewritten selected-owner map row without losing origin."""
        self.check_current()
        candidates = []
        for index, rows in enumerate(self._groups):
            if rows[0][0].casefold() == owner.casefold() and any(
                    self._reported_match(original, name, symbol) for original, name in rows):
                candidates.append(index)
        if not candidates and optional:
            self.check_current()
            return None
        if not candidates:
            raise ValueError("folded map symbol has no unique complete ICF group")
        groups = [self._bind(index) for index in candidates]
        groups = [group for group in groups if any(member.symbol == symbol for member in group.members)]
        if not groups and optional:
            return None
        if len(groups) != 1 or sum(member.symbol == symbol for member in groups[0].members) != 1:
            raise ValueError("folded map symbol has ambiguous original members")
        return groups[0]

    def _reported_match(self, owner, reported, symbol):
        if reported == symbol:
            return True
        # First establish relevance from exact original symbol/section names.
        # An unrelated unsupported section-only group grants no absence proof,
        # but must not prevent looking up an ordinary entry elsewhere.
        if not reported.startswith("."):
            return False
        self.check_current()
        try:
            obj = Coff(self._objects.resolve(owner)["raw"])
            return any(item.name == symbol and item.section > 0
                       and obj.sections[item.section - 1].name == reported
                       for item in obj.symbols.values())
        finally:
            self.check_current()

    def _bind(self, index):
        self.check_current()
        try:
            rows = self._groups[index]
            if len({(owner.casefold(), name) for owner, name in rows}) != len(rows):
                raise ValueError("ICF group repeats an original member")
            originals = []
            for owner, name in rows:
                selected = self._objects.resolve(owner)
                raw = selected["raw"]
                if sha256(raw).hexdigest() != selected["sha256"]:
                    raise ValueError("ICF member differs from physical object evidence")
                obj = Coff(raw)
                roots = _roots(obj, name)
                originals.append((selected, obj, roots, name))
            winner = originals[0][0]["owner"]
            placed = []
            for selected, obj, roots, reported in originals:
                candidates = []
                for root in roots:
                    pool = self._mapping.publics if root.storage == 2 else self._mapping.statics or ()
                    matches = [row for row in pool if row.symbol == root.name
                               and (root.storage == 2 or row.owner.casefold() in
                                    (winner.casefold(), selected["owner"].casefold()))]
                    if len(matches) != 1 or not matches[0].function or not matches[0].segment:
                        raise ValueError("ICF member lacks one consistent selected-owner map placement")
                    row, = matches
                    if row.owner.casefold() == winner.casefold():
                        candidates.append((root, row.address))
                    elif len(roots) == 1:
                        raise ValueError("ICF member lacks one consistent selected-owner map placement")
                placed.append(candidates)
            addresses = set.intersection(*(set(address for _, address in rows) for rows in placed))
            if len(addresses) != 1:
                raise ValueError("ICF group map members disagree on one unambiguous common linked address")
            address, = addresses
            members = []
            physical_roots = set()
            for (selected, obj, _, reported), candidates in zip(originals, placed, strict=True):
                roots = [root for root, candidate in candidates if candidate == address]
                if len(roots) != 1:
                    raise ValueError("ICF member has multiple original section candidates at the common address")
                root, = roots
                key = (tuple(sorted(selected["identity"].items())), root.index)
                if key in physical_roots:
                    raise ValueError("ICF group aliases the same original physical root")
                physical_roots.add(key)
                section = obj.sections[root.section - 1]
                span = Span(address, address + len(section.data))
                self._image.read(span)
                regions = [flags for (region, _, _), flags in zip(self._image.sections, self._image.section_flags)
                           if region.start <= span.start and span.end <= region.end]
                if len(regions) != 1 or not regions[0] & 0x20 or not regions[0] & 0x20000000:
                    raise ValueError("ICF member is not inside one linked code region")
                members.append(FoldMember(selected["owner"], root.name, reported,
                    tuple(sorted(selected["identity"].items())), selected["sha256"], selected["raw"], root, address,
                    tuple(number for number, parent in obj.associations.items()
                          if parent == root.section and obj.sections[number - 1].name == ".debug$F")))
            return FoldGroup(self.image_sha256, self.log_sha256, address, tuple(members))
        finally:
            self.check_current()


def bind_folding(objects, mapping, image, fixups, log_bytes):
    """Parse one actual pinned log; unsupported groups refuse upon selection."""
    if not isinstance(objects, SelectedObjects):
        raise ValueError("ICF binding requires selected physical input evidence")
    check_image(fixups, image)
    observations = parse_icf_log(log_bytes)
    if objects._constant_evidence.get("link_log_sha256") != observations["sha256"]:
        raise ValueError("ICF log differs from selected-object input evidence")
    mapping.validate(image)
    groups = tuple(tuple((_owner(row), row["symbol"]) for row in (group["selected"], *group["replaced"]))
                   for group in observations["groups"])
    result = Folding(objects, mapping, image, fixups, groups,
                     sha256(image.data).hexdigest(), observations["sha256"])
    result.check_current()
    return result

"""Complete frame correspondence for a physically bound ICF group.

Every original member remains distinct, including members without frame data.
This evidence does not establish member code; graph consumers must verify every
member against the PE and keep their dependencies and shared placement visible.
"""

from collections import Counter
from dataclasses import dataclass, replace
import struct

from .linked_fixups import check_image
from .linked_folding import FoldGroup
from .linked_fpo import validate_fpo_source
from .pe_coff import Coff


@dataclass(frozen=True)
class FoldFrames:
    group: FoldGroup
    pdb_sha256: str
    frames: tuple

    def verify(self, obj, number, root, image, address, fixups):
        check_image(fixups, image)
        if self.group.image_sha256 != fixups.image_sha256 or self.pdb_sha256 != fixups.pdb_sha256:
            raise ValueError("folded frames belong to another image/PDB pair")
        matches = []
        for index, member in enumerate(self.group.members):
            if member.root != root or number not in member.fpo_sections:
                continue
            original = Coff(member.raw)
            if all(getattr(original, key) == getattr(obj, key)
                   for key in ("symbols", "sections", "auxiliary", "associations", "section_sizes")):
                matches.append(index)
        if len(matches) != 1 or address != self.group.address:
            raise ValueError("folded frame does not identify one complete original member")
        frame = validate_fpo_source(obj, number, root, image, address, fixups)
        if tuple(sorted(frame.items())) != self.frames[matches[0]]:
            raise ValueError("folded source frame differs from complete group evidence")
        return frame

    def canonical(self):
        return [{"symbol": member.symbol, "frame": dict(frame) if frame is not None else None}
                for member, frame in zip(self.group.members, self.frames)]


def verify_fold_frames(group, image, fixups):
    """Retain the exact multiset, never select any matching duplicate record."""
    check_image(fixups, image)
    if not isinstance(group, FoldGroup) or group.image_sha256 != fixups.image_sha256:
        raise ValueError("folded metadata requires an image-bound physical group")
    frames, expected = [], []
    for member in group.members:
        if len(member.fpo_sections) > 1:
            raise ValueError("folded member has ambiguous source frame children")
        if not member.fpo_sections:
            frames.append(None)
            continue
        frame = validate_fpo_source(Coff(member.raw), member.fpo_sections[0], member.root,
                                    image, group.address, fixups)
        expected.append(struct.pack("<I", group.address - image.base) + bytes.fromhex(frame["data"]))
        frames.append(tuple(sorted(frame.items())))
    actual = [row for row in fixups.fpo_records
              if struct.unpack_from("<I", row)[0] == group.address - image.base]
    if Counter(actual) != Counter(expected):
        raise ValueError("folded FPO records do not account for every original member exactly")
    return FoldFrames(group, fixups.pdb_sha256, tuple(frames))


def original_owner_view(mapping, group):
    """A derived map interpretation backed by all original ICF members.

Only winner-owned rows for these exact members change interpretation; addresses,
coordinates, kinds and all unrelated rows remain the original map evidence.
The enclosing binder already verifies each such coordinate and physical owner.
"""
    members = {(member.root.storage, member.symbol): member for member in group.members}
    if len(members) != len(group.members):
        raise ValueError("folded map interpretation has ambiguous original names")
    seen = set()

    def rows(values, storage):
        result = []
        for row in values:
            member = members.get((storage, row.symbol))
            if member is not None and row.owner.casefold() == group.selected.owner.casefold():
                key = storage, row.symbol
                if key in seen or row.address != group.address or not row.function or not row.segment:
                    raise ValueError("folded map interpretation contradicts physical group")
                seen.add(key)
                row = replace(row, owner=member.owner)
            result.append(row)
        return tuple(result)

    result = replace(mapping, publics=rows(mapping.publics, 2),
                     statics=None if mapping.statics is None else rows(mapping.statics, 3))
    if seen != set(members):
        raise ValueError("folded map interpretation omits an original member")
    return result

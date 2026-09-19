"""Strict byte/reference proof over independently bounded original code.

Target adapters supply relocation records and explicit source-to-image bindings.
An unresolved target is unknown, never a byte mask that can yield success.
"""

from dataclasses import dataclass
from enum import StrEnum
from typing import Mapping

from .model import Result, Span, require_text


class RelocationKind(StrEnum):
    ABSOLUTE32 = "absolute32"
    RELATIVE32 = "relative32"


@dataclass(frozen=True)
class AbsoluteConstant:
    """An independently resolved COFF constant, not a rebased image address.

    This type carries a binding supplied by the caller, not its provenance or
    link-selection proof. The comparator supports only ABSOLUTE32 uses.
    """

    value: int

    def __post_init__(self):
        if type(self.value) is not int or not 0 <= self.value < 2**32:
            raise ValueError("absolute constant must be an unsigned 32-bit integer")


@dataclass(frozen=True)
class CompilerArtifact:
    """A diagnostic hint for an unplaced compiler label, not a target binding.

    Neither a compiler-looking name nor a definition in the same object proves
    its original destination. Such references remain unresolved until supplied
    with independent placement or constant evidence through the ordinary types.
    """

    symbol: str

    def __post_init__(self):
        if type(self.symbol) is not str or not (self.symbol.startswith("$")
                                                or self.symbol.startswith("??_C@")):
            raise ValueError("compiler artifact names follow the VC6 reserved-label conventions")


@dataclass(frozen=True)
class Relocation:
    offset: int
    kind: RelocationKind
    target: str

    def __post_init__(self):
        if type(self.offset) is not int or self.offset < 0:
            raise ValueError("relocation offset must be nonnegative")
        if not isinstance(self.kind, RelocationKind):
            raise ValueError("unsupported relocation kind")
        require_text(self.target, "relocation target")


def compare_bound_code(span: Span, original: bytes, candidate: bytes,
                       relocations: tuple[Relocation, ...],
                       targets: Mapping[str, int | AbsoluteConstant | CompilerArtifact], *,
                       original_absolute_sites: frozenset[int] = frozenset(),
                       alignment: int = 1) -> dict:
    """Apply known relocations, then compare every byte; never enlarge `span`.

    A target's address must come from a source binding, not the operand being
    tested. An explicit AbsoluteConstant instead requires no loader HIGHLOW at
    its ABSOLUTE32 field. Its relative uses are unsupported, not image bindings.
    The i386 addend is the candidate's own four-byte relocation field.
    Object-section alignment may account for NOP/INT3 fill *after* the declared
    original extent; it may not account for additional code or jump-table data.
    Relocations must fit the candidate itself. A valid field past the original
    extent is a boundary mismatch, not malformed evidence or alignment fill.
    This is a byte/reference check, not a proof of C++ object ownership or ABI.
    """
    size = span.end - span.start
    if len(original) != size:
        raise ValueError("original bytes do not cover exactly the declared span")
    if type(alignment) is not int or alignment < 1 or alignment & (alignment - 1):
        raise ValueError("section alignment must be a positive power of two")
    for target in targets.values():
        if type(target) is AbsoluteConstant:
            # Revalidate at consumption even if a caller bypassed frozen
            # dataclass construction or modified the instance deliberately.
            AbsoluteConstant.__post_init__(target)
        elif type(target) is CompilerArtifact:
            CompilerArtifact.__post_init__(target)
        elif type(target) is not int or not 0 <= target < 2**32:
            raise ValueError("resolved i386 target must be a 32-bit address")
    rows, occupied, outside_original = [], set(), []
    for relocation in relocations:
        sites = set(range(relocation.offset, relocation.offset + 4))
        if relocation.offset + 4 > len(candidate) or occupied & sites:
            raise ValueError("overlapping or out-of-bound relocation")
        occupied.update(sites)
        if relocation.offset + 4 > size:
            outside_original.append(relocation.offset)
    if any(type(site) is not int or not 0 <= site <= size - 4 for site in original_absolute_sites):
        raise ValueError("original relocation lies outside its declared span")
    tail = candidate[size:]
    extent_ok = (len(candidate) == size or
                 (len(candidate) == size + (-size % alignment) and bool(tail)
                  and all(byte in (0x90, 0xCC) for byte in tail)))
    if not extent_ok or outside_original:
        return {"result": Result.FAIL, "boundary": Result.FAIL,
                "bytes": Result.UNKNOWN, "references": Result.UNKNOWN,
                "original_size": size, "candidate_size": len(candidate),
                "candidate_relocations_outside_original": sorted(outside_original),
                "reason": "candidate exceeds or misses the independently recorded extent"}
    linked = bytearray(candidate[:size])
    unresolved, constant_absolute, artifacts = set(), set(), set()
    for relocation in relocations:
        offset = relocation.offset
        target = targets.get(relocation.target)
        if target is None:
            unresolved.update(range(offset, offset + 4))
            rows.append({"offset": offset, "target": relocation.target, "result": Result.UNKNOWN})
            continue
        if type(target) is CompilerArtifact:
            # Preserve the diagnostic without granting reference correctness.
            # Even byte-identical operands cannot establish an unplaced target.
            artifacts.update(range(offset, offset + 4))
            unresolved.update(range(offset, offset + 4))
            rows.append({"offset": offset, "target": relocation.target,
                         "binding": {"kind": "compiler_artifact", "symbol": target.symbol},
                         "result": Result.UNKNOWN,
                         "reason": "compiler artifact has no independently established target"})
            continue
        binding = {}
        if type(target) is AbsoluteConstant:
            binding = {"binding": {"kind": "absolute_constant", "value": target.value}}
            if relocation.kind != RelocationKind.ABSOLUTE32:
                unresolved.update(range(offset, offset + 4))
                rows.append({"offset": offset, "target": relocation.target, **binding,
                             "result": Result.UNKNOWN, "reason": "relative absolute-constant binding is unsupported"})
                continue
            constant_absolute.add(offset)
            target = target.value
        addend = int.from_bytes(linked[offset:offset + 4], "little", signed=True)
        value = target + addend
        if relocation.kind == RelocationKind.RELATIVE32:
            value -= span.start + offset + 4
        encoded = (value & 0xFFFFFFFF).to_bytes(4, "little")
        linked[offset:offset + 4] = encoded
        rows.append({"offset": offset, "target": relocation.target, **binding,
                     "result": Result.PASS if encoded == original[offset:offset + 4] else Result.FAIL})
    absolute_sites = {relocation.offset for relocation in relocations
                      if relocation.kind == RelocationKind.ABSOLUTE32}
    missing = sorted(original_absolute_sites - absolute_sites)
    # An unresolved COFF symbol may ultimately be an absolute linker constant
    # (for example a runtime segment offset), not an image address. Until its
    # binding is established, absence of a PE base relocation is not a proven
    # rebase mismatch. Known image-address targets still require HIGHLOW sites.
    unresolved_absolute = {relocation.offset for relocation in relocations
                           if relocation.kind == RelocationKind.ABSOLUTE32
                           and (targets.get(relocation.target) is None
                                or type(targets.get(relocation.target)) is CompilerArtifact)}
    extra = sorted(absolute_sites - original_absolute_sites - unresolved_absolute - constant_absolute)
    constant_rebased = sorted(constant_absolute & original_absolute_sites)
    differences = [index for index, (left, right) in enumerate(zip(original, linked, strict=True))
                   if left != right and index not in unresolved]
    byte_result = Result.FAIL if differences else Result.UNKNOWN if unresolved else Result.PASS
    reference_results = [row["result"] for row in rows]
    references = (Result.FAIL if missing or extra or constant_rebased or Result.FAIL in reference_results else
                  Result.UNKNOWN if Result.UNKNOWN in reference_results else Result.PASS)
    result = (Result.FAIL if Result.FAIL in (byte_result, references) else
              Result.UNKNOWN if Result.UNKNOWN in (byte_result, references) else Result.PASS)
    return {"result": result, "boundary": Result.PASS, "bytes": byte_result,
            "references": references, "relocations": rows,
            "missing_absolute_relocations": missing, "differing_offsets": differences,
            "extra_absolute_relocations": extra,
            "unresolved_absolute_relocations": sorted(unresolved_absolute - original_absolute_sites),
            "constant_absolute_relocations": sorted(constant_absolute),
            "unexpected_constant_base_relocations": constant_rebased,
            "compiler_artifact_relocations": sorted(artifacts),
            "alignment_fill": len(tail)}

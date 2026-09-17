"""Shared application/dependency checks over already compiled COFF objects.

Placement comes from source bindings, never from the operands under test. This
checks bounded code and references, not ownership, class layout or runtime.
"""

from dataclasses import dataclass
import hashlib
from typing import Mapping, Sequence

from .binary import AbsoluteConstant, CompilerArtifact, compare_bound_code
from .components import relative_path
from .model import Origin, Result, Span, require_text
from .pe_coff import Coff, PeImage


MAX_REFERENCE_DIAGNOSTICS = 128


def _pooled_string_constant(obj, symbol, image):
    """Return (literal, address) for a TU-private pooled string constant.

    The literal is the NUL-terminated bytes at the symbol's own section
    offset; its address is the unique occurrence of those bytes, terminator
    included, inside the image's read-only initialized data. VC6 /GF pools
    identical literals to one address, so content is the identity; anything
    absent, ambiguous or writable-backed stays unresolved.
    """
    section = obj.sections[symbol.section - 1]
    data = section.data
    if symbol.value >= len(data):
        return None, None
    end = data.find(b"\0", symbol.value)
    if end < 0 or end == symbol.value:
        return None, None
    literal = bytes(data[symbol.value:end + 1])
    occurrences = []
    for (span, offset, size), flags in zip(image.sections, image.section_flags):
        if not size or flags & 0x80000000 or not flags & 0x40000000:
            continue
        raw = image.data[offset:offset + size]
        at = raw.find(literal)
        while at >= 0:
            occurrences.append(span.start + at)
            at = raw.find(literal, at + 1)
    if len(occurrences) == 1:
        return literal, occurrences[0]
    return literal, None


def _binding_description(target):
    if target is None:
        return None
    if type(target) is int:
        return f"0x{target:08X}"
    if type(target) is AbsoluteConstant:
        return f"0x{target.value:08X}"
    return "compiler-artifact"


def _emitted_references(obj, subject, targets):
    entries, count = [], 0
    for number in obj.code_sections(subject):
        section = obj.sections[number - 1]
        for offset, index, kind in section.relocations:
            if kind == 0:  # COFF's explicit no-op is not a reference.
                continue
            count += 1
            if len(entries) >= MAX_REFERENCE_DIAGNOSTICS:
                continue
            entries.append({"section": number, "section_name": section.name,
                            "offset": offset, "coff_type": kind,
                            "symbol": obj.symbols[index].name,
                            "storage_class": obj.symbols[index].storage,
                            "binding_address": _binding_description(targets.get(str(index)))})
    return {"emitted_references": entries, "omitted_references": count - len(entries),
            "limitations": ["candidate section offsets, not aligned original instruction positions",
                            "binding addresses exclude operand addends and do not prove target agreement or linker selection",
                            "diagnostic limit never narrows whole-function verification"]}


@dataclass(frozen=True)
class FunctionBinding:
    origin: Origin
    spans: tuple[Span, ...]
    symbol: str
    unit: str

    def __post_init__(self):
        require_text(self.symbol, "emitted symbol")
        relative_path(self.unit)
        if not self.spans or self.spans[0].start != self.origin.address:
            raise ValueError("function requires an independently mapped entry span")
        ordered = sorted(self.spans)
        if any(left.end > right.start for left, right in zip(ordered, ordered[1:])):
            raise ValueError("function image spans overlap")


def check_objects(bindings: Sequence[FunctionBinding], objects: Mapping[str, Coff],
                  image: PeImage, *, external_targets: Mapping[str, int | AbsoluteConstant] | None = None,
                  import_targets: Mapping[str, int] | None = None,
                  errors: Mapping[str, str] | None = None, member_functions: bool = False) -> list[dict]:
    """One report per selected binding, including missing objects and symbols.

    External targets must be unambiguous source-owned mappings supplied by the
    adapter. Static symbol names never leak across translation units. Defined
    symbols resolve through their own mappings too, not only undefined imports.
    Pinned archive members may name plain code sections; their binding carries
    the identity that a product object's COMDAT boundary would prove.
    """
    if len({item.origin.entity_id for item in bindings}) != len(bindings):
        raise ValueError("ambiguous original function selection")
    pin = hashlib.sha256(image.data).hexdigest()
    if any(item.origin.image_sha256 != pin or item.origin.address_space != "pe:va" for item in bindings):
        raise ValueError("function binding refers to a different image/address space")
    externals = dict(external_targets or {})
    imports = dict(import_targets or {})
    if any(not isinstance(key, str) or not key or type(value) not in (int, AbsoluteConstant)
           for key, value in externals.items()) or any(
            not isinstance(key, str) or not key or type(value) is not int
            or not 0 <= value <= 0xFFFFFFFF for key, value in imports.items()):
        raise ValueError("invalid explicit external target binding")
    if imports.keys() & (externals.keys() | {binding.symbol for binding in bindings}):
        raise ValueError("conflicting source and import symbol bindings")
    sections, subjects, layouts = {}, {}, {}
    for binding in bindings:
        obj = objects.get(binding.unit)
        if obj is None:
            continue
        try:
            symbol = obj.function(binding.symbol, comdat=not member_functions)
        except ValueError as error:
            subjects[binding.origin.entity_id] = str(error)
            continue
        key = (binding.unit, symbol.section)
        base = binding.origin.address - symbol.value
        if key in sections and sections[key] != base:
            raise ValueError("conflicting original placement of a COFF section")
        sections[key] = base
        code_sections = obj.code_sections(symbol)
        if len(binding.spans) == len(code_sections) and len(code_sections) <= 2:
            # A unique companion has a unique recorded tail. More than one
            # would require explicit per-fragment identities, not zip/order.
            layout = tuple(zip(code_sections, binding.spans, strict=True))
            for number, span in layout:
                key = (binding.unit, number)
                if key in sections and sections[key] != span.start:
                    raise ValueError("conflicting original placement of a COFF section")
                sections[key] = span.start
            layouts[binding.origin.entity_id] = layout
        if symbol.storage == 2:
            if symbol.name in externals and externals[symbol.name] != binding.origin.address:
                raise ValueError("conflicting external symbol bindings")
            externals[symbol.name] = binding.origin.address
        subjects[binding.origin.entity_id] = symbol
    rows = []
    for binding in bindings:
        row = {"entity_id": binding.origin.entity_id, "address": f"0x{binding.origin.address:08X}",
               "symbol": binding.symbol, "unit": binding.unit}
        try:
            if binding.unit not in objects:
                raise ValueError((errors or {}).get(binding.unit, "translation unit was not compiled"))
            subject = subjects[binding.origin.entity_id]
            if isinstance(subject, str):
                raise ValueError(subject)
            layout = layouts.get(binding.origin.entity_id)
            obj, targets = objects[binding.unit], {}
            for index, symbol in obj.symbols.items():
                if symbol.section > 0:
                    base = sections.get((binding.unit, symbol.section))
                    if base is not None:
                        targets[str(index)] = base + symbol.value
                    elif symbol.storage == 2 and symbol.name in externals:
                        # A global defined in this TU still needs its own source
                        # binding. Never place an entire data section from it.
                        targets[str(index)] = externals[symbol.name]
                    elif symbol.name.startswith("$") and symbol.storage != 2:
                        # A VC6 compiler label (try targets, unwind-state
                        # storage, guards) defined in an associated section the
                        # binding does not place: its reference is structural
                        # to this translation unit, not a product address.
                        targets[str(index)] = CompilerArtifact(symbol.name)
                    elif symbol.name.startswith("??_C@"):
                        # A TU-private pooled string constant (VC6 emits these
                        # as ??_C@-mangled definitions in the object's own
                        # data sections, storage class varying by emission):
                        # /GF pools identical literals to one address, so the
                        # literal's own bytes identify it in the original. The
                        # content must occur exactly once in the image's
                        # read-only data; anything ambiguous or absent stays
                        # unresolved.
                        literal, address = _pooled_string_constant(obj, symbol, image)
                        if address is not None:
                            targets[str(index)] = address
                            row.setdefault("pooled_string_constants", []).append(
                                {"symbol": symbol.name, "literal": literal, "address": f"0x{address:08X}"})
                elif symbol.section == 0 and symbol.value == 0 and symbol.storage == 2:
                    if symbol.name in externals:
                        targets[str(index)] = externals[symbol.name]
                    elif symbol.name in imports:
                        targets[str(index)] = imports[symbol.name]
            row["diagnostic"] = _emitted_references(obj, subject, targets)
            if layout is None:
                raise ValueError("code sections require one entry and at most one uniquely associated recorded tail")
            proofs = []
            for number, span in layout:
                code, relocations, alignment = obj.section_subject(number)
                proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                                           original_absolute_sites=image.absolute_sites(span), alignment=alignment)
                for relocation in proof.get("relocations", []):
                    relocation["symbol"] = obj.symbols[int(relocation["target"])].name
                proofs.append({**proof, "span": {"start": span.start, "end": span.end},
                               "section": number, "section_name": obj.sections[number - 1].name})
            if len(proofs) == 1:
                row.update(proofs[0])
            else:
                for dimension in ("result", "boundary", "bytes", "references"):
                    results = {proof[dimension] for proof in proofs}
                    row[dimension] = (Result.FAIL if Result.FAIL in results else Result.UNKNOWN
                                      if Result.UNKNOWN in results else Result.PASS)
                row["spans"] = proofs
                row["relocations"] = [{**item, "span_start": proof["span"]["start"]}
                                      for proof in proofs for item in proof.get("relocations", [])]
        except ValueError as error:
            row.update(result=Result.UNKNOWN, reason=str(error))
        rows.append(row)
    return rows

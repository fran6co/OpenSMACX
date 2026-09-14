"""Exact native ordinary-data producers, not contents or ownership permission.

Compiled sections correspond by source TU and unique section name. Dependency
sections additionally retain the full physical member/content pin and ordinal.
No address, symbol spelling, bytes or neighboring section supplies an identity.
"""

from contextlib import contextmanager
from dataclasses import dataclass
from pathlib import Path

from .linked_code_ownership import _contents, _portable, _target
from .linked_input_epoch import input_epoch, _borrow_epoch
from .linked_objects import _owner
from .model import require_text
from .pe_coff import Coff


def _section(obj, number):
    if type(number) is not int or not 1 <= number <= len(obj.sections):
        raise ValueError("data producer requires an exact positive section number")
    section, size = obj.sections[number - 1], obj.section_sizes[number - 1]
    if (obj.machine != 0x14C or obj.optional_header_size or section.flags & ~0xC0F000C0
            or not section.flags & 0x40000000 or section.flags & 0xC0 not in (0x40, 0x80)
            or number in obj.associations or number in obj.associations.values()
            or not section.name or section.name.startswith("/")
            or section.name.lower().startswith((".tls", ".debug", ".idata", ".edata", ".rsrc"))
            or not 0 < size <= 16 * 1024 * 1024):
        raise ValueError("data producer requires an ordinary non-COMDAT data section")
    if sum(other.name == section.name for other in obj.sections) != 1:
        raise ValueError("data producer section name is ambiguous within its object")
    if ((section.flags & 0x80 and (section.data or section.relocations))
            or section.flags & 0x40 and len(section.data) != size):
        raise ValueError("data producer section extent disagrees with its physical representation")
    return section.name



def _selected(objects, native, owner, inputs):
    require_text(owner, "native data owner")
    actual = native.resolve(owner)
    supplied = objects.resolve(owner)
    if actual != supplied:
        raise ValueError("data producer differs from its independent selected physical input")
    raw = _contents(actual["identity"], actual["sha256"], inputs)
    if actual["raw"] != raw:
        raise ValueError("data producer selected bytes differ from its physical input")
    return actual, Coff(raw)


def _identify(scope, objects, owner, number):
    with scope() as (root, directory, compiled, _, native, inputs):
        selected, obj = _selected(objects, native, owner, inputs)
        name = _section(obj, number)
        identity = selected["identity"]
        by_output = {row["path"]: unit for unit, row in compiled.items()}
        if set(identity) == {"path"} and identity["path"] in by_output:
            unit = by_output[identity["path"]]
            if compiled[unit]["sha256"] != selected["sha256"]:
                raise ValueError("data producer differs from its exact compilation receipt")
            return {"kind": "compiled", "unit": unit, "section": name}
        return {"kind": "dependency", "input": _portable(Path(identity["path"]), root, directory),
                "object_sha256": selected["sha256"], "section": name, "object_section": number,
                **{key: identity[key] for key in ("member", "occurrence", "generated") if key in identity}}


def _resolve(scope, objects, descriptor):
    with scope() as (root, directory, compiled, coverage, native, inputs):
        if not isinstance(descriptor, dict):
            raise ValueError("invalid data producer descriptor")
        descriptor = dict(descriptor)
        name = descriptor.pop("section", None)
        require_text(name, "data producer section name")
        has_number = "object_section" in descriptor
        number = descriptor.pop("object_section", None)
        if descriptor.get("kind") == "compiled":
            if has_number:
                raise ValueError("compiled data correspondence cannot depend on a section ordinal")
        elif type(number) is not int or number <= 0:
            raise ValueError("dependency data producer requires its exact section ordinal")
        # The shared portable-path parser has exact compiled/dependency schemas;
        # its symbol is a local parsing token, never source or linked identity.
        if "symbol" in descriptor:
            raise ValueError("data producer descriptors do not contain symbols")
        target, expected, _ = _target({**descriptor, "symbol": "data-section"}, root, directory, compiled)
        if (descriptor["kind"] == "dependency" and set(target) == {"path"}
                and any(row["path"] == target["path"] for row in compiled.values())):
            raise ValueError("compiled data producer cannot be reclassified as a dependency")
        raw = _contents(target, expected, inputs)
        obj = Coff(raw)
        if number is None:
            numbers = [index for index, section in enumerate(obj.sections, 1) if section.name == name]
            if len(numbers) != 1:
                raise ValueError("compiled data producer has no unique current section name")
            number, = numbers
        if _section(obj, number) != name:
            raise ValueError("dependency section name differs from its exact ordinal")
        candidates = []
        for row in coverage["inputs"]:
            if target["path"] not in row["paths"]:
                continue
            owner = _owner(row)
            try:
                selected = native.resolve(owner)
            except ValueError:
                continue  # An ambiguous family can never supply an identity.
            if selected["identity"] == target and selected["sha256"] == expected:
                _selected(objects, native, owner, inputs)
                candidates.append({"owner": selected["owner"], "section": number})
        if len(candidates) != 1:
            raise ValueError("data producer lacks one exact native selected owner")
        return candidates[0]


@dataclass(frozen=True)
class _SectionProducers:
    identify: object
    resolve: object
    check_current: object


@contextmanager
def section_producers(context, objects, *, epoch=None):
    """Share physical parsing within one live native section-producer epoch.

    Every call checks context, receipts, all captured files and the independently
    witnessed native indices before and after use. The handle expires at exit;
    returned descriptors remain ordinary correspondence inputs, not coverage.
    """
    if epoch is None:
        with input_epoch(context, objects) as owned:
            with section_producers(context, objects, epoch=owned) as handle:
                yield handle
        return
    with _borrow_epoch(epoch, context, objects) as native:
        state = (native.root, native.directory, native.compiled, native.coverage, native.native, native.inputs)
        fresh = epoch.check_current
        live = True

        def check_current():
            if not live:
                raise ValueError("native data section producer epoch has expired")
            fresh()

        @contextmanager
        def scope():
            try:
                check_current()
                yield state
            except (KeyError, TypeError, AttributeError) as error:
                raise ValueError("incomplete native data producer evidence") from error
            finally:
                check_current()

        handle = _SectionProducers(lambda owner, number: _identify(scope, objects, owner, number),
                                   lambda descriptor: _resolve(scope, objects, descriptor), check_current)
        try:
            check_current()
            yield handle
        finally:
            try:
                check_current()
            finally:
                live = False


def identify_section(context, objects, owner, number):
    """Describe one exact selected ordinary data producer in a live build."""
    with section_producers(context, objects) as batch:
        return batch.identify(owner, number)


def resolve_section(context, objects, descriptor):
    """Resolve correspondence independently; this grants no retention/contents proof."""
    with section_producers(context, objects) as batch:
        return batch.resolve(descriptor)

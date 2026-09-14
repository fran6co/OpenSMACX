"""Exact native ordinary-data producers, not contents or ownership permission.

Compiled sections correspond by source TU and unique section name. Dependency
sections additionally retain the full physical member/content pin and ordinal.
No address, symbol spelling, bytes or neighboring section supplies an identity.
"""

from contextlib import contextmanager
from copy import deepcopy
from dataclasses import dataclass
from pathlib import Path

from .build_graph_commands import _context
from .components import relative_path
from .linked_code_ownership import _compilations, _contents, _Inputs, _portable, _target
from .link_file_evidence import parse_link_files
from .link_input_coverage import _input_inventory
from .linked_objects import SelectedObjects, _owner
from .model import digest, require_text
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


@contextmanager
def _native(context, objects):
    inputs = _Inputs()
    report = digest(context.report)
    context_identity = (context.root, context.directory, context.check_current)
    population_fresh = None
    selected_fresh = None

    def fresh():
        try:
            if selected_fresh is not None:
                selected_fresh()
            elif population_fresh is not None:
                population_fresh()
        finally:
            try:
                inputs.check_current()
            finally:
                try:
                    context_identity[2]()
                finally:
                    if ((context.root, context.directory, context.check_current) != context_identity
                            or digest(context.report) != report):
                        raise ValueError("data producer native context report changed")

    try:
        context.check_current()
        if type(objects) is not SelectedObjects:
            raise ValueError("data producer requires native selected physical inputs")
        root, directory, compiled = _compilations(context)
        _, _, _, command = _context(context)
        artifacts = {}
        for kind in ("link-input-files", "msvc-link-log"):
            rows = [row for row in context.report.get("outputs", ()) if row.get("format") == kind]
            if len(rows) != 1:
                raise ValueError("data producer requires its native link input and final-pass artifacts")
            row, = rows
            path = directory / relative_path(row["path"])
            raw = inputs.read(path)
            if inputs.files[path][1] != {key: row[key] for key in ("sha256", "size")}:
                raise ValueError("data producer native artifact changed")
            artifacts[kind] = raw
        evidence = parse_link_files(artifacts["link-input-files"])
        if (digest(evidence) != digest(objects._constant_evidence)
                or list(command.arguments) != evidence["arguments"] or str(command.cwd) != evidence["cwd"]):
            raise ValueError("data producer objects differ from this native command/input epoch")
        coverage, records = _input_inventory(artifacts["link-input-files"], artifacts["msvc-link-log"])
        native = SelectedObjects(evidence, coverage, records)
        population, population_fresh = objects._constant_population()
        if digest(sorted(population, key=digest)) != digest(sorted(records, key=digest)):
            raise ValueError("data producer population differs from the native physical input inventory")
        # Read actual compiler/source/header inputs, not merely receipt fields.
        for row in context.report["compile_link"]["compilations"]:
            for item in row["compilation"]["inputs"]:
                path = Path(item["path"])
                inputs.read(path)
                if (item["before"] != item["after"] or inputs.files[path][1] != item["after"]):
                    raise ValueError("data producer compiler/source input changed from its receipt")
        from .coff_archive import CoffArchive
        from .common_allocation import _policy
        from .linked_context import _same
        # Initialize legitimate lazy policy state before pinning the immutable
        # native indices. Growing byte/archive/owner caches remain independently
        # checked against actual physical files, not copied verdicts.
        policy = _policy(evidence, records)
        if objects._constant_policy_gaps is None:
            objects._constant_policy_gaps = policy
        elif not _same(objects._constant_policy_gaps, policy):
            raise ValueError("data producer selected policy differs from native physical inputs")
        mutable = {"_files", "_archives", "_resolved"}
        pinned = deepcopy({key: value for key, value in objects.__dict__.items() if key not in mutable})
        archives = {}

        def selected_fresh():
            if not _same({key: value for key, value in objects.__dict__.items() if key not in mutable}, pinned):
                raise ValueError("data producer selected-object indices changed")
            population_fresh()
            if set(objects._files) - objects._stamps.keys() or set(objects._archives) - objects._files.keys():
                raise ValueError("data producer has an unobserved native cache key")
            for name, raw in objects._files.items():
                path = Path(name)
                actual = inputs.read(path)
                if inputs.files[path][1] != objects._stamps[name] or not _same(actual, raw):
                    raise ValueError("data producer physical file cache changed")
            for path, archive in objects._archives.items():
                if path not in archives:
                    archives[path] = deepcopy(CoffArchive(objects._files[path]).__dict__)
                if type(archive) is not CoffArchive or not _same(archive.__dict__, archives[path]):
                    raise ValueError("data producer physical archive cache changed")
            for owner, value in objects._resolved.items():
                if not _same(value, native.resolve(owner)):
                    raise ValueError("data producer selected owner cache changed")

        fresh()
        yield (root, directory, compiled, coverage, native, inputs), fresh
    except (KeyError, TypeError, AttributeError) as error:
        raise ValueError("incomplete native data producer evidence") from error
    finally:
        fresh()


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
def section_producers(context, objects):
    """Share physical parsing within one live native section-producer epoch.

    Every call checks context, receipts, all captured files and the independently
    witnessed native indices before and after use. The handle expires at exit;
    returned descriptors remain ordinary correspondence inputs, not coverage.
    """
    with _native(context, objects) as (state, fresh):
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

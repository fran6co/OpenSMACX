"""Compile each upstream TU once and verify its source-owned image obligations."""

from collections import Counter
from pathlib import Path
from typing import Callable

from .bound_objects import FunctionBinding, check_objects
from .binary import AbsoluteConstant, compare_bound_code
from .components import ToolchainManifest, VendorManifest, local_path
from .fingerprint import input_snapshot
from .import_source import import_targets
from .model import Result
from .pe_coff import Coff, PeImage


def _check_functions(manifest, objects, image, errors, import_bindings, section_bases=None):
    if {item.symbol for item in manifest.bindings} & {item.symbol for item in import_bindings}:
        raise ValueError("component symbol is bound as both function and import")
    imports, checked_imports = import_targets(import_bindings, objects, image)
    rows = check_objects(tuple(FunctionBinding(item.origin, item.spans, item.symbol, item.unit)
                               for item in manifest.bindings), objects, image, errors=errors,
                         import_targets=imports, member_functions=isinstance(manifest, ToolchainManifest),
                         section_bases=section_bases)
    return rows, checked_imports


def _verify_data_sections(manifest, objects, image, errors):
    """Verify full archive section contributions; symbol anchors carry no extents."""
    rows, bases = [], {}
    for item in manifest.data_sections:
        row = {"unit": item.unit, "section": item.section, "section_name": item.section_name,
               "address": f"0x{item.span.start:08X}",
               "span": {"start": item.span.start, "end": item.span.end},
               "symbol_offsets": {anchor.name: anchor.offset for anchor in item.symbols},
               "result": Result.UNKNOWN}
        obj = objects.get(item.unit)
        try:
            if obj is None:
                raise ValueError(errors.get(item.unit, "archive member was not read"))
            if not 1 <= item.section <= len(obj.sections):
                raise ValueError("COFF data section ordinal is absent")
            section = obj.sections[item.section - 1]
            if section.name != item.section_name:
                raise ValueError("COFF data section name differs from the source binding")
            flags = section.flags
            if (not flags & 0x40 or flags & (0x200 | 0x800 | 0x80 | 0x20 | 0x1000 | 0x02000000 | 0x20000000)
                    or section.name.startswith(".tls")):
                raise ValueError("COFF section is not ordinary initialized non-executable data")
            if len(section.data) != item.span.end - item.span.start:
                raise ValueError("COFF data section extent differs from its full original span")
            if section.relocations:
                raise ValueError("COFF data section carries unsupported relocation records")
            if image.absolute_sites(item.span):
                row["result"] = Result.FAIL
                row["reason"] = "original data contribution contains a PE base relocation"
                rows.append(row)
                continue
            if image.read_initial(item.span) != section.data:
                row["result"] = Result.FAIL
                row["reason"] = "full COFF data section differs from the pinned original image span"
                rows.append(row)
                continue
            expected = {anchor.name: anchor for anchor in item.symbols}
            actual = [symbol for symbol in obj.symbols.values()
                      if symbol.section == item.section and symbol.index not in obj.auxiliary]
            if {symbol.name for symbol in actual} != set(expected) or len(actual) != len(expected):
                raise ValueError("COFF data section symbol inventory differs from explicit anchors")
            for name, anchor in expected.items():
                matches = [symbol for symbol in actual if symbol.name == name]
                if (len(matches) != 1 or matches[0].storage != anchor.storage
                        or matches[0].value != anchor.offset):
                    raise ValueError(f"COFF data anchor {name!r} differs from its section-relative identity")
            row["result"] = Result.PASS
            bases[(item.unit, item.section)] = item.span.start
        except ValueError as error:
            row["reason"] = str(error)
        rows.append(row)
    return rows, bases


def _ordinary_code_section(obj, item):
    if not 1 <= item.section <= len(obj.sections):
        raise ValueError("COFF code section ordinal is absent")
    section = obj.sections[item.section - 1]
    if section.name != item.section_name:
        raise ValueError("COFF code section name differs from the source binding")
    if (not section.flags & 0x20 or not section.flags & 0x20000000
            or section.flags & (0x40 | 0x80 | 0x200 | 0x800 | 0x1000 | 0x02000000)):
        raise ValueError("COFF section is not an ordinary executable code contribution")
    if item.section in obj.associations:
        raise ValueError("associative COMDAT code sections are not whole ordinary contributions")
    if not section.data:
        raise ValueError("COFF code contribution is empty")
    if len(section.data) != item.span.end - item.span.start:
        raise ValueError("COFF code contribution extent differs from its full original span")
    # Parsing every relocation is mandatory even when its target is unknown.
    obj.section_subject(item.section)
    return section


def _code_section_preflight(manifest, objects, image, errors):
    rows, bases, sections = [], {}, {}
    for item in manifest.code_sections:
        row = {"unit": item.unit, "section": item.section, "section_name": item.section_name,
               "address": f"0x{item.span.start:08X}",
               "span": {"start": item.span.start, "end": item.span.end},
               "result": Result.UNKNOWN, "boundary": Result.UNKNOWN,
               "bytes": Result.UNKNOWN, "references": Result.UNKNOWN,
               "callable": False, "link_target": False}
        obj = objects.get(item.unit)
        try:
            if obj is None:
                raise ValueError(errors.get(item.unit, "archive member was not read"))
            section = _ordinary_code_section(obj, item)
            mapped = [(index, region, flags) for index, (region, _, _) in enumerate(image.sections)
                      if region.start <= item.span.start and item.span.end <= region.end
                      for flags in (image.section_flags[index],)]
            if len(mapped) != 1 or mapped[0][2] & 0x20000020 != 0x20000020:
                raise ValueError("original full code contribution is not uniquely backed by executable image bytes")
            raw = image.read(item.span)
            sites = image.absolute_sites(item.span)
            code, relocations, alignment = obj.section_subject(item.section)
            proof = compare_bound_code(item.span, raw, code, relocations, {},
                                       original_absolute_sites=sites, alignment=alignment)
            row.update({key: proof[key] for key in ("boundary", "bytes", "references", "result")})
            row["relocations"] = proof.get("relocations", [])
            row["differing_offsets"] = proof.get("differing_offsets", [])
            row["nonrelocation_bytes"] = (Result.FAIL if proof.get("differing_offsets")
                                           else Result.PASS if proof["boundary"] == Result.PASS
                                           else Result.UNKNOWN)
            row["highlow_offsets"] = sorted(sites)
            row["reason"] = "preflight establishes whole-section placement and nonrelocation bytes only"
            sections[(item.unit, item.section)] = (obj, section, item)
            if (proof["boundary"] == Result.PASS and proof["bytes"] != Result.FAIL
                    and proof["references"] != Result.FAIL):
                bases[(item.unit, item.section)] = item.span.start
        except ValueError as error:
            row["reason"] = str(error)
        rows.append(row)
    return rows, bases, sections


def _source_function_bases(manifest, objects, function_rows):
    """Only already-declared exclusive function identities can supply targets."""
    by_identity = {row.get("entity_id"): row for row in function_rows}
    bases = {}
    for binding in manifest.bindings:
        row = by_identity.get(binding.origin.entity_id, {})
        if row.get("boundary") != Result.PASS or row.get("bytes") == Result.FAIL:
            continue
        obj = objects.get(binding.unit)
        if obj is None:
            continue
        try:
            symbol = obj.function(binding.symbol, comdat=False)
        except ValueError:
            continue
        base = binding.origin.address - symbol.value
        key = (binding.unit, symbol.section)
        if key in bases and bases[key] != base:
            raise ValueError("conflicting original placement of a COFF section")
        bases[key] = base
    return bases


def _code_section_targets(manifest, objects, section_bases, function_rows, import_rows, absolute_rows):
    function_addresses = {}
    for row in function_rows:
        if row.get("boundary") == Result.PASS and row.get("bytes") != Result.FAIL:
            function_addresses.setdefault(row["symbol"], []).append(int(row["address"], 16))
    import_addresses = {}
    for row in import_rows:
        if row.get("identity") == Result.PASS and row.get("address") is not None:
            import_addresses.setdefault(row["symbol"], []).append(int(row["address"], 16))
    absolute_values = {}
    for item, row in zip(manifest.absolute_symbols, absolute_rows, strict=True):
        if row["result"] == Result.PASS:
            absolute_values.setdefault(item.symbol, []).append(AbsoluteConstant(item.value))
    targets_by_member = {}
    for unit, obj in objects.items():
        targets = {}
        for index, symbol in obj.symbols.items():
            if symbol.section > 0:
                base = section_bases.get((unit, symbol.section))
                section = obj.sections[symbol.section - 1]
                if base is not None and 0 <= symbol.value < len(section.data):
                    targets[str(index)] = base + symbol.value
            elif symbol.section < 0:
                values = absolute_values.get(symbol.name, [])
                if (symbol.section == -1 and len(values) == 1 and symbol.storage == 2
                        and symbol.value == values[0].value):
                    targets[str(index)] = values[0]
            elif symbol.section == 0 and symbol.value == 0 and symbol.storage == 2:
                choices = function_addresses.get(symbol.name, []) + import_addresses.get(symbol.name, [])
                choices += absolute_values.get(symbol.name, [])
                if len(choices) == 1:
                    targets[str(index)] = choices[0]
        targets_by_member[unit] = targets
    return targets_by_member


def _verify_code_section_references(manifest, objects, image, rows, bases, function_rows,
                                    import_rows, absolute_rows):
    targets_by_member = _code_section_targets(manifest, objects, bases, function_rows,
                                              import_rows, absolute_rows)
    for item, row in zip(manifest.code_sections, rows, strict=True):
        obj = objects.get(item.unit)
        if obj is None or row.get("boundary") != Result.PASS or row.get("bytes") == Result.FAIL:
            continue
        try:
            code, relocations, alignment = obj.section_subject(item.section)
            proof = compare_bound_code(item.span, image.read(item.span), code, relocations,
                                       targets_by_member.get(item.unit, {}),
                                       original_absolute_sites=image.absolute_sites(item.span),
                                       alignment=alignment)
            for relocation in proof.get("relocations", []):
                relocation["symbol"] = obj.symbols[int(relocation["target"])].name
            row.update({key: proof[key] for key in ("boundary", "bytes", "references", "result")})
            row["relocations"] = proof.get("relocations", [])
            row["nonrelocation_bytes"] = (Result.FAIL if proof.get("differing_offsets")
                                           else Result.PASS if proof["boundary"] == Result.PASS
                                           else Result.UNKNOWN)
            row.pop("reason", None)
        except ValueError as error:
            row.update(result=Result.UNKNOWN, reason=str(error))
    return rows


def verify_component(root: Path, manifest: VendorManifest, image_data: bytes,
                     compile_source: Callable[[Path, tuple[str, ...]], bytes], *, import_bindings=()) -> dict:
    pins = {binding.origin.image_sha256 for binding in manifest.bindings}
    spaces = {binding.origin.address_space for binding in manifest.bindings}
    if len(pins) != 1 or spaces != {"pe:va"}:
        raise ValueError("i386 component verification requires one PE virtual-address image")
    image = PeImage(image_data, pins.pop())
    source = local_path(root, manifest.source_root)
    inputs = tuple(manifest.source_root + "/" + item.path for item in manifest.files)
    before = input_snapshot(root, inputs, manifest.to_dict())
    objects, errors = {}, {}
    for unit in manifest.units:
        try:
            objects[unit] = Coff(compile_source(local_path(source, unit), manifest.flags))
        except (OSError, ValueError) as error:
            errors[unit] = str(error)
    rows, checked_imports = _check_functions(manifest, objects, image, errors, import_bindings)
    results = [row["result"] for row in rows]
    if input_snapshot(root, inputs, manifest.to_dict()) != before:
        raise ValueError("component sources changed during measurement; retry at a stable checkpoint")
    overall = (Result.FAIL if Result.FAIL in results else
               Result.UNKNOWN if not results or Result.UNKNOWN in results else Result.PASS)
    return {"component_id": manifest.component.component_id, "check": "binary_references",
            "result": overall, "functions": rows, "import_bindings": checked_imports,
            "compile_attempts": len(manifest.units),
            "counts": dict(sorted(Counter(results).items())),
            "unresolved_symbols": sorted({relocation["symbol"] for row in rows
                                           for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "unmeasurable": [{"address": row["address"], "symbol": row["symbol"], "reason": row["reason"]}
                              for row in rows if "reason" in row],
            "application_recoveries": 0, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN}


def verify_toolchain(manifest: ToolchainManifest, image_data: bytes, archive_data: bytes, *, import_bindings=()) -> dict:
    """Verify explicitly bounded functions in a pinned precompiled library.

    Archive directory indexes are not a final-link witness. Only the selected
    member bytes are read; no object is extracted, compiled, or executed.
    Missing dependency bindings remain unknown, never operand-derived aliases.
    """
    import hashlib
    from .coff_archive import CoffArchive
    pins = {binding.origin.image_sha256 for binding in manifest.bindings}
    pins.update(item.origin.image_sha256 for item in manifest.data_sections)
    pins.update(item.origin.image_sha256 for item in manifest.code_sections)
    spaces = {binding.origin.address_space for binding in manifest.bindings}
    spaces.update(item.origin.address_space for item in manifest.data_sections)
    spaces.update(item.origin.address_space for item in manifest.code_sections)
    if len(pins) != 1 or spaces != {"pe:va"}:
        raise ValueError("i386 toolchain verification requires one PE virtual-address image")
    image = PeImage(image_data, pins.pop())
    actual = hashlib.sha256(archive_data).hexdigest()
    archive_result = Result.PASS if actual == manifest.archive_sha256 else Result.FAIL
    objects, errors, rows, checked_imports = {}, {}, [], []
    members = sorted({binding.unit for binding in manifest.bindings}
                     | {item.unit for item in manifest.absolute_symbols}
                     | {item.unit for item in manifest.data_sections}
                     | {item.unit for item in manifest.code_sections})
    data_rows, section_bases = [], {}
    code_rows = []
    if archive_result == Result.PASS:
        archive = CoffArchive(archive_data)
        for member in members:
            try:
                objects[member] = Coff(archive.member(member))
            except ValueError as error:
                errors[member] = str(error)
        data_rows, section_bases = _verify_data_sections(manifest, objects, image, errors)
        section_bases = {key: base for key, base in section_bases.items()
                         if next(row for row in data_rows
                                 if (row["unit"], row["section"]) == key)["result"] == Result.PASS}
    code_rows, code_bases, _ = _code_section_preflight(manifest, objects, image, errors)
    if archive_result == Result.PASS:
        rows, checked_imports = _check_functions(manifest, objects, image, errors, import_bindings,
                                                 section_bases=section_bases)
    else:
        rows, checked_imports = [], []
    absolute_rows = []
    for item in manifest.absolute_symbols:
        row = {"symbol": item.symbol, "unit": item.unit, "value": f"0x{item.value:08X}",
               "result": Result.UNKNOWN}
        obj = objects.get(item.unit)
        if obj is None:
            row["reason"] = errors.get(item.unit, "archive member was not read")
        else:
            matches = [symbol for symbol in obj.symbols.values() if symbol.name == item.symbol]
            absolutes = [symbol for symbol in matches if symbol.section == -1]
            if len(matches) != 1 or not absolutes:
                row["reason"] = "member does not define exactly one absolute symbol"
            elif absolutes[0].value != item.value or absolutes[0].storage != 2:
                row["reason"] = "member defines a different absolute value"
            else:
                row["result"] = Result.PASS
        absolute_rows.append(row)
    function_bases = _source_function_bases(manifest, objects, rows)
    qualified_bases = {**section_bases, **code_bases}
    for key, base in function_bases.items():
        if key in qualified_bases and qualified_bases[key] != base:
            raise ValueError("conflicting original placement of a COFF section")
        qualified_bases[key] = base
    code_rows = _verify_code_section_references(manifest, objects, image, code_rows,
                                                qualified_bases, rows, checked_imports,
                                                absolute_rows)
    outcomes = ({row["result"] for row in rows} | {archive_result}
                | {row["result"] for row in absolute_rows} | {row["result"] for row in data_rows}
                | {row["result"] for row in code_rows})
    required_rows = bool(rows or data_rows or code_rows)
    overall = (Result.FAIL if Result.FAIL in outcomes else Result.UNKNOWN
               if Result.UNKNOWN in outcomes or not required_rows else Result.PASS)
    return {"component_id": manifest.component.component_id, "check": "toolchain_binary_references",
            "result": overall, "archive": {"result": archive_result, "sha256": actual},
            "functions": rows, "absolute_symbols": absolute_rows, "data_sections": data_rows,
            "code_sections": code_rows,
            "import_bindings": checked_imports,
            "counts": dict(sorted(Counter(row["result"] for row in rows).items())),
            "data_section_counts": dict(sorted(Counter(row["result"] for row in data_rows).items())),
            "code_section_counts": dict(sorted(Counter(row["result"] for row in code_rows).items())),
            "selected_members": members, "member_errors": errors,
            "member_reads": len(members) if archive_result == Result.PASS else 0, "compile_attempts": 0,
            "unresolved_symbols": sorted({relocation["symbol"] for row in rows + code_rows
                                           for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "application_recoveries": 0, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN, "link_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["selected original bindings only, not whole-library identification or coverage",
                            "original extents and symbol interpretations require independent source evidence",
                            "archive indexes, linker member selection, ABI and runtime behavior remain unproved",
                            "import identities use explicit source DLL/export declarations, not original operands or candidate link selection",
                            "unbound external functions, static data and undeclared imports remain unresolved",
                            "this report does not publish targets into application acceptance"]}

"""Fresh native PE metadata qualification, not task scope or acceptance.

Projection excludes only build-identification fields from byte comparison. It
does not assert debugger identity, self-inspection equivalence, or runtime
acceptance. Import/bound-import timestamps are never build-ID exemptions.
Layout, pointers, sizes, and all other debug bytes remain literal. Optional
locator projection requires the exact declared sidecar and explicit encodings;
the caller must compare its logical output identity and projection field shape.
"""

from hashlib import sha256
from pathlib import Path
import struct

from .linked_context import LinkedBuild
from .model import Result, digest
from .pdb_fixups import bound_fixups
from .pe_debug import _bytes, _mapped, linked_coff_symbols
from .pe_output_residual import _geometry, _bodies


def _locator(native, spelling):
    """Match a full locator using declared codecs, never OS/Wine guessing."""
    from .build_graph_commands import _context
    from .command_paths import PathEncoding

    _, directory, _, link = _context(native.context)
    outputs = [row for row in native.context.report["outputs"] if row["format"] == "msvc-pdb"]
    if len(outputs) != 1:
        raise ValueError("NB10 locator needs exactly one declared sidecar output")
    output, = outputs
    path = directory / output["path"]
    if (path.resolve() != path or not path.is_relative_to(directory)
            or path.read_bytes() != native.inputs.artifacts["msvc-pdb"]):
        raise ValueError("NB10 locator sidecar differs from native artifact")
    operands = [item for item in link.operands if item.path == path]
    if len(operands) != 1 or operands[0].kind != "output_file":
        raise ValueError("NB10 locator lacks one actual declared output operand")
    # _context validated every declared codec, even codecs not used in an
    # argument slice (a relative command can produce an absolute locator).
    description = native.context.report["compile_link"]["command_bindings"]["link"]["description"]
    encodings = [PathEncoding(Path(item["anchor"]), item["prefix"], item["separator"])
                 for item in description["codecs"]]
    expected = {item.encode(path) for item in encodings if path.is_relative_to(item.anchor)}
    if spelling not in expected:
        raise ValueError("NB10 locator does not encode its actual declared sidecar")
    return {"format": "msvc-pdb", "path": path.relative_to(directory).as_posix()}


def qualify_metadata(native, bodies, *, project_locator=False, runtime_regions=()):
    """Derive exact protected/identification ranges from one live native PE.

    ``bodies`` are {entity_id,address,size}; no ownership is inferred here.
    ``runtime_regions`` are {address,size} protection-only extents, including
    associated data. They confer no omission permission. Every projected field
    also refuses executable backing independently of graph coverage.
    ``projected_bytes`` are disposable comparison bytes, never a replacement
    native image, artifact, PDB binding, or saved acceptance capability.
    """
    if type(native) is not LinkedBuild:
        raise ValueError("PE metadata requires a live native LinkedBuild")
    if type(project_locator) is not bool:
        raise ValueError("NB10 locator projection must be explicit")
    original_regions = None
    try:
        native.check_current()
        image = native.inputs.image
        _geometry(image)
        scopes = _bodies(image, bodies)
        raw = image.data
        pe, = struct.unpack_from("<I", raw, 0x3C)
        protected, fields = [], []
        runtime_protection = []
        if not isinstance(runtime_regions, (list, tuple)) or len(runtime_regions) > 100000:
            raise ValueError("runtime protection requires a bounded extent sequence")
        original_regions = digest(runtime_regions)
        for region in runtime_regions:
            if not isinstance(region, dict) or set(region) != {"address", "size"}:
                raise ValueError("runtime protection requires exact address and size fields")
            address, size = region["address"], region["size"]
            if (type(address) is not int or type(size) is not int or size <= 0
                    or address < image.base or address + size > 0x100000000):
                raise ValueError("runtime protection has an invalid extent")
            backing = [(span, offset) for span, offset, count in image.sections
                       if span.start <= address and address + size <= span.start + count]
            if len(backing) != 1:
                raise ValueError("runtime protection requires unique physical backing")
            span, offset = backing[0]
            runtime_protection.append({"offset": offset + address - span.start,
                                       "size": size, "kind": "runtime_region"})

        def physical(offset, size, kind):
            _bytes(raw, offset, size)
            if size <= 0:
                raise ValueError("metadata protection requires a positive range")
            value = {"offset": offset, "size": size, "kind": kind}
            protected.append(value)
            return value

        def mapped(rva, size, kind):
            contents, offset = _mapped(image, rva, size)
            return contents, physical(offset, size, kind)

        for index, (rva, size) in enumerate(image.directories):
            if (rva, size) == (0, 0):
                continue
            if index not in (1, 5, 6, 12):
                raise ValueError("unsupported nonzero PE directory: " + str(index))
            if not rva or not size:
                raise ValueError("incomplete PE metadata directory")
            mapped(rva, size, "directory:" + str(index))

        # Existing parser validates complete bounded descriptor/thunk/name
        # semantics. Rewalk only to preserve every indirect physical range,
        # including terminators/hints and tables beyond the directory span.
        imports = image.imports()
        import_rva, import_size = image.directories[1] if len(image.directories) > 1 else (0, 0)
        iat_rva, iat_size = image.directories[12] if len(image.directories) > 12 else (0, 0)
        if iat_size and not import_size:
            raise ValueError("IAT directory has no ordinary import domain")
        if import_size:
            table, _ = mapped(import_rva, import_size, "import_descriptors")
            entries = 0
            for at in range(0, min(len(table), 4096 * 20), 20):
                lookup, timestamp, chain, name, iat = struct.unpack_from("<IIIII", table, at)
                if not any((lookup, timestamp, chain, name, iat)):
                    break
                # Bound imports carry loader semantics, not compiler identity.
                # Their bytes are retained, but this first domain does not
                # independently verify a bound target DLL or forwarder chain.
                if timestamp or chain:
                    raise ValueError("bound or chained imports require separate loader evidence")
                module = image._import_string(name)
                mapped(name, len(module.encode("ascii")) + 1, "import_module_name")
                for index in range(65536):
                    data, _ = mapped((lookup or iat) + index * 4, 4, "import_lookup")
                    mapped(iat + index * 4, 4, "import_iat")
                    if iat_size and not iat_rva <= iat + index * 4 <= iat_rva + iat_size - 4:
                        raise ValueError("ordinary import slot is outside the declared IAT directory")
                    value, = struct.unpack("<I", data)
                    if not value:
                        break
                    entries += 1
                    if entries > 65536:
                        raise ValueError("PE metadata import range budget exceeded")
                    if not value & 0x80000000:
                        name = image._import_string(value + 2)
                        mapped(value, 2 + len(name.encode("ascii")) + 1, "import_hint_name")
                else:
                    raise ValueError("unterminated import metadata")
            else:
                raise ValueError("unterminated import descriptor metadata")
            if entries != len(imports):
                raise ValueError("import range inventory differs from independent parsing")

        linked_coff_symbols(image)  # Complete physical debug payload framing.
        debug_rva, debug_size = image.directories[6] if len(image.directories) > 6 else (0, 0)
        pdb = native.inputs.artifacts.get("msvc-pdb")
        if not debug_size or pdb is None:
            raise ValueError("native metadata projection requires its own NB10/PDB pair")
        binding = bound_fixups(image, pdb)
        if (native.inputs.fixups is None or binding.image_sha256 != native.inputs.fixups.image_sha256
                or binding.pdb_sha256 != native.inputs.fixups.pdb_sha256):
            raise ValueError("metadata PDB binding differs from native linked inputs")
        directory, location = mapped(debug_rva, debug_size, "debug_directory")
        # First supported domain deliberately has only one NB10 entry. No
        # blanket timestamp/payload exemption for unknown debug formats.
        if debug_size != 28:
            raise ValueError("unsupported multiple debug entry metadata domain")
        flags, _, major, minor, kind, length, address, offset = struct.unpack("<IIHHIIII", directory)
        if flags or major or minor or kind != 2:
            raise ValueError("unsupported debug identification entry")
        physical(offset, length, "nb10_payload")
        if _bytes(raw, offset, 4) != b"NB10":
            raise ValueError("debug entry is not NB10")
        fields.extend(({"offset": pe + 8, "size": 4, "kind": "coff_timestamp"},
                       {"offset": location["offset"] + 4, "size": 4, "kind": "debug_timestamp"},
                       {"offset": offset + 8, "size": 8, "kind": "nb10_signature_age"}))
        locator_identity = None
        if project_locator:
            # Own-PDB binding already validated the full terminated payload.
            spelling = _bytes(raw, offset + 16, length - 17).decode("ascii")
            locator_identity = _locator(native, spelling)
            fields.append({"offset": offset + 16, "size": length - 17, "kind": "nb10_locator",
                           "logical_output": locator_identity})

        def overlaps(a, b):
            return a["offset"] < b["offset"] + b["size"] and b["offset"] < a["offset"] + a["size"]

        for region in protected:
            if any(overlaps(region, {"offset": body["file_offset"], "size": body["size"]}) for body in scopes):
                raise ValueError("scoped body overlaps loader/debug metadata: " + region["kind"])
        allowed = {"coff_timestamp": set(), "debug_timestamp": {"directory:6", "debug_directory"},
                   "nb10_signature_age": {"nb10_payload"}, "nb10_locator": {"nb10_payload"}}
        for field in fields:
            if any(flags & 0x20000000 and size and overlaps(field, {"offset": start, "size": size})
                   for (_, start, size), flags in zip(image.sections, image.section_flags)):
                raise ValueError("projected identification field overlaps executable storage")
            if any(overlaps(field, region) for region in runtime_protection):
                raise ValueError("projected identification field overlaps an observed runtime region")
            if any(overlaps(field, region) and region["kind"] not in allowed[field["kind"]]
                   for region in protected):
                raise ValueError("projected identification field has another metadata use")
            # Metadata identification cannot contain a loader relocation,
            # including a write crossing its four/eight-byte boundary.
            header_size, = struct.unpack_from("<I", raw, pe + 24 + 60)
            if field["offset"] + field["size"] <= header_size:
                from .model import Span
                if image.absolute_sites(Span(image.base + field["offset"],
                                             image.base + field["offset"] + field["size"])):
                    raise ValueError("projected header identification field has a loader fixup")
            for region, start, size in image.sections:
                lo, hi = max(start, field["offset"]), min(start + size, field["offset"] + field["size"])
                if lo < hi:
                    from .model import Span
                    span = Span(region.start + lo - start, region.start + hi - start)
                    if image.absolute_sites(span):
                        raise ValueError("projected identification field has a loader fixup")
        projected = bytearray(raw)
        for field in fields:
            projected[field["offset"]:field["offset"] + field["size"]] = bytes(field["size"])
        return {"check": "native_pe_build_identification", "result": Result.UNKNOWN,
                "qualification": Result.PASS, "outside_scope_verification": Result.UNKNOWN,
                "projected_bytes": bytes(projected), "image_sha256": sha256(raw).hexdigest(),
                "projected_sha256": sha256(projected).hexdigest(), "pdb_sha256": binding.pdb_sha256,
                "locator_identity": locator_identity,
                "fields": fields, "protected_ranges": protected,
                "runtime_protection": runtime_protection,
                "ready_for_collection": False, "persisted": False,
                "limitation": "comparison modulo authenticated build-identification fields only; "
                              "not debugger identity, file self-inspection equivalence, body ownership or runtime acceptance"}
    finally:
        try:
            if original_regions is not None and digest(runtime_regions) != original_regions:
                raise ValueError("runtime protection changed during native qualification")
        finally:
            native.check_current()
